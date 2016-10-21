/*
 * Parser for the "free function" fitter model
 * @author: Tobias Weber
 * @date: April 2012
 * @license GPLv3
 */

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace ph = boost::phoenix;
namespace fus = boost::fusion;

#include <boost/math/special_functions/erf.hpp>
#include <boost/math/special_functions/sign.hpp>

#include <ctype.h>
#include <iostream>
#include <sstream>
#include <map>
#include <unordered_map>

#include "parser.h"
#include "tlibs/math/math.h"
#include "tlibs/math/rand.h"
#include "tlibs/log/log.h"

#ifdef USE_JIT
	#include <llvm/ExecutionEngine/GenericValue.h>
	#include <llvm/Support/ManagedStatic.h>
	#include <llvm/Support/TargetSelect.h>
	#include <llvm/IR/LegacyPassManager.h>
	#include <llvm/LinkAllPasses.h>
#endif


//----------------------------------------------------------------------
// old procedural interface

// there are two versions of these functions:
// the first includes a vector of free parameters, e.g. x,y,z
// the second assumes one default free parameter, namely x

static bool is_symbol_in_map(const std::vector<Symbol>& syms, const std::string& str, const std::vector<Symbol>& vecFreeParams, double* pdVal=0);
static const double *get_symbol_ptr(const std::vector<Symbol>& syms, const std::string& str, const std::vector<Symbol>& vecFreeParams);

// create a syntax tree out of a expression string
static bool parse_expression(Node& node, std::vector<Symbol>& syms, const std::string& str, const std::vector<Symbol>& vecFreeParams);
static bool parse_expression(Node& node, std::vector<Symbol>& syms, const std::string& str);

// evaluate the syntax tree
static double eval_tree(const Node& node, const std::vector<Symbol>& syms, const std::vector<Symbol>& vecFreeParams);
static double eval_tree(const Node& node, const std::vector<Symbol>& syms, double x);

// get a string representation of the syntax tree's expression
static std::string get_expression(const Node& node, const std::vector<Symbol>& syms, const std::vector<Symbol>& vecFreeParams, bool bFillInSyms=false, bool bGnuPlotSyntax=true);

// print tree & symbol map
static void print_node(const Node& n, int iIndent=0);
static void print_symbol_map(const std::vector<Symbol>& syms);
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// expressions
//----------------------------------------------------------------------


enum
{
	NODE_PLUS=1,
	NODE_MINUS,
	NODE_MULT,
	NODE_DIV,
	NODE_POW,

	NODE_DOUBLE,
	NODE_IDENT,

	NODE_CALL,
	NODE_NOP,

	// internal nodes
	NODE_INVALID=1000,
	NODE_MINUS_INV,
	NODE_DIV_INV
};


BOOST_FUSION_ADAPT_STRUCT
(
	Node,
	(int, iType)

	(double, dVal)
	(std::string, strIdent)

	(std::vector<Node>, vecChildren)
)


//----------------------------------------
// globals
// symbol tables with constants
typedef std::unordered_map<std::string, double> t_syms;
static t_syms g_syms =
{
	t_syms::value_type("pi", M_PI)
};

// functions with zero arguments
typedef std::unordered_map<std::string, double (*)(void)> t_map_fkt0;
static t_map_fkt0 g_map_fkt0 =
{
	t_map_fkt0::value_type("rand01", tl::rand01<double>)
};

// functions with one argument
typedef std::unordered_map<std::string, double (*)(double)> t_map_fkt1;
static t_map_fkt1 g_map_fkt1 =
{
	t_map_fkt1::value_type("abs", std::fabs),
	t_map_fkt1::value_type("sin", std::sin),
	t_map_fkt1::value_type("cos", std::cos),
	t_map_fkt1::value_type("tan", std::tan),
	t_map_fkt1::value_type("asin", std::asin),
	t_map_fkt1::value_type("acos", std::acos),
	t_map_fkt1::value_type("atan", std::atan),
	t_map_fkt1::value_type("sinh", std::sinh),
	t_map_fkt1::value_type("cosh", std::cosh),
	t_map_fkt1::value_type("tanh", std::tanh),
	t_map_fkt1::value_type("asinh", std::asinh),
	t_map_fkt1::value_type("acosh", std::acosh),
	t_map_fkt1::value_type("atanh", std::atanh),
	t_map_fkt1::value_type("exp", std::exp),
	t_map_fkt1::value_type("log", std::log),
	t_map_fkt1::value_type("log10", std::log10),
	t_map_fkt1::value_type("sqrt", std::sqrt),
	t_map_fkt1::value_type("ceil", std::ceil),
	t_map_fkt1::value_type("floor", std::floor),
	t_map_fkt1::value_type("fabs", std::fabs),
	t_map_fkt1::value_type("round", std::round),
	t_map_fkt1::value_type("sign", [](double d)->double { return double(boost::math::sign(d)); }),
	t_map_fkt1::value_type("erf", std::erf),
	t_map_fkt1::value_type("erf_inv", [](double d)->double { return boost::math::erf_inv(d); }),
	t_map_fkt1::value_type("rand_poisson", [](double d)->double { return double(tl::rand_poisson<int, double>(d)); })
};

// functions with two arguments
typedef std::unordered_map<std::string, double (*)(double, double)> t_map_fkt2;
static t_map_fkt2 g_map_fkt2 =
{
	t_map_fkt2::value_type("atan2", std::atan2),
	t_map_fkt2::value_type("pow", std::pow),
	t_map_fkt2::value_type("fmod", std::fmod),
	t_map_fkt2::value_type("rand_norm", tl::rand_norm<double>),
	t_map_fkt2::value_type("rand_real", tl::rand_real<double>)
};
//----------------------------------------

static std::string get_op_name(int iOp)
{
	switch(iOp)
	{
		case NODE_PLUS: return "plus";
		case NODE_MINUS: return "minus";
		case NODE_MULT: return "mult";
		case NODE_DIV: return "div";
		case NODE_POW: return "pow";
		case NODE_DOUBLE: return "double";
		case NODE_IDENT: return "ident";
		case NODE_CALL: return "call";
		case NODE_NOP: return "nop";

		case NODE_INVALID: return "invalid";
		case NODE_MINUS_INV: return "minus_inv";
		case NODE_DIV_INV: return "div_inv";
	}

	return "<unknown>";
}

static void get_default_free_params(std::vector<Symbol>& vecFreeParams)
{
	Symbol sym;
	sym.strIdent = "x";

	vecFreeParams.push_back(sym);
}

static double eval_tree(const Node& node, const std::vector<Symbol>& syms, const std::vector<Symbol>& vecFreeParams)
{
	if(node.iType == NODE_PLUS)
	{
		double dRes = 0.;
		for(const Node& child : node.vecChildren)
			dRes += eval_tree(child, syms, vecFreeParams);
		return dRes;
	}
	else if(node.iType == NODE_MINUS)
	{
		// unary minus
		if(node.vecChildren.size()==1)
		{
			return -eval_tree(node.vecChildren[0], syms, vecFreeParams);
		}
		// more than one operand, subtract them all from the first
		else
		{
			double dRes = eval_tree(node.vecChildren[0], syms, vecFreeParams);

			for(unsigned int i=1; i<node.vecChildren.size(); ++i)
				dRes -= eval_tree(node.vecChildren[i], syms, vecFreeParams);

			return dRes;
		}
	}
	else if(node.iType == NODE_MULT)
	{
		double dRes = 1.;
		for(const Node& child : node.vecChildren)
			dRes *= eval_tree(child, syms, vecFreeParams);
		return dRes;
	}
	else if(node.iType == NODE_DIV)
	{
		double dRes = eval_tree(node.vecChildren[0], syms, vecFreeParams);
		for(unsigned int i=1; i<node.vecChildren.size(); ++i)
			dRes /= eval_tree(node.vecChildren[i], syms, vecFreeParams);
		return dRes;
	}
	else if(node.iType == NODE_POW)
	{
		if(node.vecChildren.size()!=2)
		{
			tl::log_err("operation 'pow' needs exactly two operands.");
			return 0.;
		}

		double d1 = eval_tree(node.vecChildren[0], syms, vecFreeParams);
		double d2 = eval_tree(node.vecChildren[1], syms, vecFreeParams);

		return pow(d1,d2);
	}
	else if(node.iType == NODE_DOUBLE)
	{
		return node.dVal;
	}
	else if(node.iType == NODE_IDENT)
	{
		// try to find ident in free parameter "map"
		for(const Symbol& symfree : vecFreeParams)
			if(symfree.strIdent == node.strIdent)
				return symfree.dVal;

		// else try to find ident in symbol "map"
		double dVal = 0.;
		bool bSymInMap = is_symbol_in_map(syms, node.strIdent, vecFreeParams, &dVal);
		if(!bSymInMap)
		{
			tl::log_err("Symbol \"", node.strIdent, "\" is not in map!");
		}
		return dVal;
	}
	else if(node.iType == NODE_CALL)
	{
		std::string strFkt = node.strIdent;
		int iNumArgs = node.vecChildren.size();
		if(iNumArgs==0)
		{
			t_map_fkt0::iterator iter0 = g_map_fkt0.find(strFkt);

			if(iter0 != g_map_fkt0.end())
			{
				double (*pFkt)(void) = (*iter0).second;
				return pFkt();
			}
		}
		else if(iNumArgs==1)
		{
			t_map_fkt1::iterator iter1 = g_map_fkt1.find(strFkt);

			if(iter1 != g_map_fkt1.end())
			{
				double (*pFkt)(double) = (*iter1).second;
				return pFkt(eval_tree(node.vecChildren[0], syms, vecFreeParams));
			}
		}
		else if(iNumArgs==2)
		{
			t_map_fkt2::iterator iter2 = g_map_fkt2.find(strFkt);

			if(iter2 != g_map_fkt2.end())
			{
				double (*pFkt)(double, double) = (*iter2).second;
				return pFkt(eval_tree(node.vecChildren[0], syms, vecFreeParams),
					    eval_tree(node.vecChildren[1], syms, vecFreeParams));
			}
		}

		tl::log_err("No function \"", strFkt, "\" taking ", iNumArgs, " arguments known.");
		return 0.;
	}
	else if(node.iType == NODE_NOP)
	{
		return 0.;
	}
	else if(node.iType == NODE_INVALID)
	{
		tl::log_err("Syntax tree is invalid.");
		return 0.;
	}

	tl::log_err("Unknown syntax node type ", node.iType);
	return 0.;
}

static double eval_tree(const Node& node, const std::vector<Symbol>& syms, double x)
{
	Symbol sym;
	sym.strIdent = "x";
	sym.dVal = x;

	std::vector<Symbol> vecFreeParams;
	vecFreeParams.push_back(sym);

	return eval_tree(node,syms, vecFreeParams);
}

static std::string get_expression(const Node& node, const std::vector<Symbol>& syms, const std::vector<Symbol>& vecFreeParams, bool bFillInSyms, bool bGnuPlotSyntax)
{
	std::ostringstream ostr;

	if(node.iType == NODE_PLUS)
	{
		bool bNeedBrackets=true;
		if(node.vecChildren.size()==1)
			bNeedBrackets=false;

		if(bNeedBrackets)
			ostr << "(";

		for(unsigned int i=0; i<node.vecChildren.size(); ++i)
		{
			ostr << get_expression(node.vecChildren[i], syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
			if(i!=node.vecChildren.size()-1)
				ostr << "+";
		}

		if(bNeedBrackets)
			ostr << ")";
		return ostr.str();
	}
	else if(node.iType == NODE_MINUS)
	{
		ostr << "(";

		// unary minus
		if(node.vecChildren.size()==1)
		{
			ostr << "-"
			<< get_expression(node.vecChildren[0], syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
		}
		// non-unary minus
		else
		{
			for(unsigned int i=0; i<node.vecChildren.size(); ++i)
			{
				ostr << get_expression(node.vecChildren[i], syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
				if(i!=node.vecChildren.size()-1)
					ostr << "-";
			}
		}

		ostr << ")";
		return ostr.str();
	}
	else if(node.iType == NODE_MULT)
	{
		ostr << "(";

		for(unsigned int i=0; i<node.vecChildren.size(); ++i)
		{
			ostr << get_expression(node.vecChildren[i], syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
			if(i!=node.vecChildren.size()-1)
				ostr << "*";
		}

		ostr << ")";
		return ostr.str();
	}
	else if(node.iType == NODE_DIV)
	{
		ostr << "(";

		for(unsigned int i=0; i<node.vecChildren.size(); ++i)
		{
			ostr << get_expression(node.vecChildren[i], syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
			if(i!=node.vecChildren.size()-1)
				ostr << "/";
		}

		ostr << ")";
		return ostr.str();
	}
	else if(node.iType == NODE_POW)
	{
		if(node.vecChildren.size()==2)
		{
			bool bNeedBrackets = true;
			if(node.vecChildren[0].vecChildren.size()==0)
				bNeedBrackets = false;

			// '+' and '-' already print brackets
				if(node.vecChildren[0].iType==NODE_PLUS || node.vecChildren[0].iType==NODE_MINUS)
					bNeedBrackets = false;

				if(bNeedBrackets) ostr << "(";
				ostr << get_expression(node.vecChildren[0], syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
				if(bNeedBrackets) ostr << ")";

				ostr << (bGnuPlotSyntax ? "**" : "^");

				bNeedBrackets = true;
				if(node.vecChildren[1].vecChildren.size()==0)
					bNeedBrackets = false;

				// '+' and '-' already print brackets
					if(node.vecChildren[1].iType==NODE_PLUS || node.vecChildren[1].iType==NODE_MINUS)
						bNeedBrackets = false;

					if(bNeedBrackets) ostr << "(";
					ostr << get_expression(node.vecChildren[1], syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
					if(bNeedBrackets) ostr << ")";
		}
		return ostr.str();
	}
	else if(node.iType == NODE_DOUBLE)
	{
		if(node.dVal < 0.) ostr << "(";
		ostr << node.dVal;
		if(node.dVal < 0.) ostr << ")";

		return ostr.str();
	}
	else if(node.iType == NODE_IDENT)
	{
		// free parameter?
		for(const Symbol &symfree : vecFreeParams)
			if(symfree.strIdent == node.strIdent)
				return symfree.strIdent;

		double dVal = 0.;

		// if the symbol is not in the map, also treat it as free parameter
		bool bSymInMap = is_symbol_in_map(syms, node.strIdent, vecFreeParams, &dVal);
		if(!bSymInMap)
			return node.strIdent;

		if(bFillInSyms)
		{
			// negative values need a bracket
			if(dVal < 0.) ostr << "(";
			ostr << dVal;
			if(dVal < 0.) ostr << ")";
		}
		else
		{
			ostr << node.strIdent;
		}

		return ostr.str();
	}
	else if(node.iType == NODE_CALL)
	{
		std::string strFkt = node.strIdent;

		bool bNeedsBrackets=true;
		// binary plus&minus & unary minus already print brackets
		if(node.vecChildren.size()==1)
		{
			if(node.vecChildren[0].iType==NODE_MINUS)
				bNeedsBrackets=false;
			else if(node.vecChildren[0].iType==NODE_PLUS &&
				node.vecChildren[0].vecChildren.size()==2)
				bNeedsBrackets=false;
		}

		ostr << strFkt;
		if(bNeedsBrackets)
			ostr << "(";
		for(unsigned int i=0; i<node.vecChildren.size(); ++i)
		{
			ostr << get_expression(node.vecChildren[i], syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
			if(i!=node.vecChildren.size()-1)
				ostr << ", ";
		}
		if(bNeedsBrackets)
			ostr << ")";

		return ostr.str();
	}
	else if(node.iType == NODE_NOP)
	{
		return "0";
	}
	else if(node.iType == NODE_INVALID)
	{
		return "0";
	}

	tl::log_err("Unknown syntax node type ", node.iType);
	return "";
}

static bool check_tree_sanity(const Node& n)
{
	for(const  Node& child : n.vecChildren)
		if(!check_tree_sanity(child))
			return false;

	if((n.iType==NODE_MINUS_INV || n.iType==NODE_DIV_INV) && n.vecChildren.size() < 2)
		return false;

	if((n.iType==NODE_PLUS || n.iType==NODE_MINUS) && n.vecChildren.size() < 1)
		return false;
	if((n.iType==NODE_MULT || n.iType==NODE_DIV) && n.vecChildren.size() < 2)
		return false;

	if((n.iType==NODE_POW) && n.vecChildren.size() < 2)
		return false;


	return true;
}

static void optimize_tree(Node& n)
{
	for(Node& child : n.vecChildren)
		optimize_tree(child);


	if(n.iType==NODE_MINUS_INV)
	{
		n.iType = NODE_MINUS;
		Node tmp = n.vecChildren[1];
		n.vecChildren[1] = n.vecChildren[0];
		n.vecChildren[0] = tmp;
	}
	if(n.iType==NODE_DIV_INV)
	{
		n.iType = NODE_DIV;
		Node tmp = n.vecChildren[1];
		n.vecChildren[1] = n.vecChildren[0];
		n.vecChildren[0] = tmp;
	}


	// (x+0) => (x)
	// (x-0) => (x)
	if((n.iType==NODE_PLUS || n.iType==NODE_MINUS) && n.vecChildren.size()==2 && n.vecChildren[1].iType==NODE_DOUBLE && n.vecChildren[1].dVal==0.)
	{
		Node tmp = n.vecChildren[0];
		n = tmp;
	}
	// (x*1) => (x)
	// (x/1) => (x)
	if((n.iType==NODE_MULT || n.iType==NODE_DIV) && n.vecChildren[1].iType==NODE_DOUBLE && n.vecChildren[1].dVal==1.)
	{
		Node tmp = n.vecChildren[0];
		n = tmp;
	}
	// (0+x) => (x)
	if((n.iType==NODE_PLUS) && n.vecChildren.size()==2 && n.vecChildren[0].iType==NODE_DOUBLE && n.vecChildren[0].dVal==0.)
	{
		Node tmp = n.vecChildren[1];
		n = tmp;
	}
	// (1*x) => (x)
	if((n.iType==NODE_MULT) && n.vecChildren[0].iType==NODE_DOUBLE && n.vecChildren[0].dVal==1.)
	{
		Node tmp = n.vecChildren[1];
		n = tmp;
	}
	// (x^1) => (x)
	if(n.iType==NODE_POW && n.vecChildren[1].iType==NODE_DOUBLE && n.vecChildren[1].dVal==1.)
	{
		Node tmp = n.vecChildren[0];
		n = tmp;
	}

	// (0-x) => (-x)
	// (0+x) => (+x)
	if((n.iType==NODE_PLUS || n.iType==NODE_MINUS) && n.vecChildren.size()==2 && n.vecChildren[0].iType==NODE_DOUBLE && n.vecChildren[0].dVal==0.)
	{
		Node tmp = n.vecChildren[1];
		n.vecChildren[0] = tmp;
		n.vecChildren.pop_back();
	}


	// -(-x) => +x
	if(n.iType==NODE_MINUS && n.vecChildren.size()==1
		&& n.vecChildren[0].iType==NODE_MINUS && n.vecChildren[0].vecChildren.size()==1)
	{
		Node tmp = n.vecChildren[0];
		n = tmp;
		n.iType = NODE_PLUS;
	}
	// -(+x) => -x
	if(n.iType==NODE_MINUS && n.vecChildren.size()==1
		&& n.vecChildren[0].iType==NODE_PLUS && n.vecChildren[0].vecChildren.size()==1)
	{
		Node tmp = n.vecChildren[0];
		n = tmp;
		n.iType = NODE_MINUS;
	}

	// +(...) => (...)
	if(n.iType==NODE_PLUS && n.vecChildren.size()==1)
	{
		Node tmp = n.vecChildren[0];
		n = tmp;
	}

	// a*(1/x) => a/x
	if(n.iType==NODE_MULT && n.vecChildren.size()==2 &&
	   n.vecChildren[1].iType==NODE_DIV && n.vecChildren[1].vecChildren.size()==2 &&
	   n.vecChildren[1].vecChildren[0].iType==NODE_DOUBLE &&
	   n.vecChildren[1].vecChildren[0].dVal==1.)
	{
		n.iType = NODE_DIV;
		Node tmp = n.vecChildren[1].vecChildren[1];
		n.vecChildren[1] = tmp;
	}

	// a+(-x) => a-x
	if(n.iType==NODE_PLUS && n.vecChildren.size()==2 &&
	   n.vecChildren[1].iType==NODE_MINUS && n.vecChildren[1].vecChildren.size()==1)
	{
		n.iType = NODE_MINUS;
		Node tmp = n.vecChildren[1].vecChildren[0];
		n.vecChildren[1] = tmp;
	}

	/*if(n.iType==NODE_NOP && n.vecChildren.size()==1)
	{
		Node tmp =n.vecChildren[0];
		n = tmp;
	}*/
}

static void print_node(const Node& n, int iIndent)
{
	for(int k=0; k<iIndent; ++k)
		std::cout << "\t";

	std::cout << "* level " << iIndent << ", type: " << get_op_name(n.iType);
	std::cout << ", val: " << n.dVal;
	std::cout << ", ident: " << n.strIdent;
	std::cout << ", children: " << n.vecChildren.size() << "\n";

	unsigned int iChild=0;
	for(const Node& child : n.vecChildren)
	{
		for(int j=0; j<iIndent; ++j)
			std::cout << "\t";

		std::cout << "child " << (iChild++) << ":\n";
		print_node(child, iIndent+1);
	}

	std::cout << std::endl;
}

/*
 * expression parser class
 * TODO: better integration of the semantic rules into Spirit
 */
template<typename Iter>
struct expression_parser
	: qi::grammar<Iter, Node(), ascii::space_type>
{
	qi::rule<Iter, Node(), ascii::space_type> start;
	qi::rule<Iter, Node(), ascii::space_type> expr;
	qi::rule<Iter, Node(), ascii::space_type> factor;
	qi::rule<Iter, Node(), ascii::space_type> factor_tail;
	qi::rule<Iter, Node(), ascii::space_type> term;
	qi::rule<Iter, Node(), ascii::space_type> term_tail;
	qi::rule<Iter, Node(), ascii::space_type> power;
	qi::rule<Iter, Node(), ascii::space_type> power_tail;

	qi::rule<Iter, std::string(), ascii::space_type> ident;

	std::ostringstream ostrErr;


	// grammar originally based on the LL(1) expression example in this document:
	// http://cs.nyu.edu/courses/summer11/G22.2110-002/Parsing.pdf (page 13)
	expression_parser()
		: expression_parser::base_type(start, "start")
	{
		start = (expr 					[ qi::_val = qi::_1 ]
				)
			  | qi::eps					[ ph::at_c<0>(qi::_val) = NODE_NOP ]
			  ;

		expr = ( -term					[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
				> term_tail				[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
				)						[ ph::at_c<0>(qi::_val) = NODE_PLUS ]
			 ;

		term = (factor 					[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
				> factor_tail			[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
				)						[ ph::at_c<0>(qi::_val) = NODE_MULT ]
			;

		term_tail = ("+"
					> term				[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					> term_tail			[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					)					[ ph::at_c<0>(qi::_val) = NODE_PLUS ]
				  | ("-"
					> term				[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					> term_tail			[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					)					[ ph::at_c<0>(qi::_val) = NODE_MINUS_INV ]
				  | (qi::eps			[ ph::at_c<1>(qi::_val) = 0. ]
					)					[ ph::at_c<0>(qi::_val) = NODE_DOUBLE ]
				  ;

		factor = (power					[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					> power_tail		[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					)					[ ph::at_c<0>(qi::_val) = NODE_POW ]
				;

		factor_tail = ("*" > factor 	[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
						> factor_tail	[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					  )					[ ph::at_c<0>(qi::_val) = NODE_MULT ]
				| ("/" > factor			[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					> factor_tail		[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					)					[ ph::at_c<0>(qi::_val) = NODE_DIV_INV ]
				| (qi::eps				[ ph::at_c<1>(qi::_val) = 1. ]
					)					[ ph::at_c<0>(qi::_val) = NODE_DOUBLE ]
				;

		power = ( "(" >
					expr 				[ qi::_val = qi::_1 ]
					> ")" )
			   | (qi::double_			[ ph::at_c<1>(qi::_val) = qi::_1 ]
					)					[ ph::at_c<0>(qi::_val) = NODE_DOUBLE ]
			   | (ident					[ ph::at_c<2>(qi::_val) = qi::_1 ]
					>> "(">>")")		[ ph::at_c<0>(qi::_val) = NODE_CALL ]
			   | (ident 				[ ph::at_c<2>(qi::_val) = qi::_1 ]
					>> "(" > expr		[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
					>> *("," > expr		[ ph::push_back(ph::at_c<3>(qi::_val), qi::_1) ]
						)
					 > ")"
				 )						[ ph::at_c<0>(qi::_val) = NODE_CALL ]
			   | (ident					[ ph::at_c<2>(qi::_val) = qi::_1 ]
					)					[ ph::at_c<0>(qi::_val) = NODE_IDENT ]
			   ;

		power_tail = ("^" > power		[ qi::_val = qi::_1 ]
						)
					| (qi::eps			[ ph::at_c<1>(qi::_val) = 1. ]
						)				[ ph::at_c<0>(qi::_val) = NODE_DOUBLE ]
					;


		ident %= qi::lexeme[qi::char_("A-Za-z_") >> *(qi::char_("A-Za-z0-9_"))];


		start.name("start");
		expr.name("expr");
		factor.name("factor");
		factor_tail.name("factor_tail");
		term.name("term");
		term_tail.name("term_tail");
		power.name("power");
		power_tail.name("power_tail");
		ident.name("ident");

		qi::on_error<qi::fail>
		(
			start,
			(std::ostream&) ostrErr
				<< ph::val("Expected ") << qi::labels::_4
				<< ph::val(" at token \"")
				<< ph::construct<std::string>(qi::labels::_3, qi::labels::_2)
				<< ph::val("\".")
		);
	}
};

static void create_symbol_map(const Node& n, std::vector<Symbol>& syms, const std::vector<Symbol>& vecFreeParams)
{
	for(unsigned int i=0; i<n.vecChildren.size(); ++i)
		create_symbol_map(n.vecChildren[i], syms, vecFreeParams);

	if(n.iType==NODE_IDENT)
	{
		// ignore free parameters
		for(const Symbol& symfree : vecFreeParams)
		{
			if(symfree.strIdent == n.strIdent)
			{
				//std::cout << "ignoring " << n.strIdent << std::endl;
				return;
			}
		}

		// include it in the symbol map if it is not already there
		if(!is_symbol_in_map(syms, n.strIdent, vecFreeParams))
		{
			Symbol symbol;
			symbol.strIdent = n.strIdent;
			symbol.dVal = 0.;
			syms.push_back(symbol);
		}
	}
}

static bool is_symbol_in_map(const std::vector<Symbol>& syms, const std::string& str, const std::vector<Symbol>& vecFreeParams, double* pdVal)
{
	// look in free parameters
	for(const Symbol& symfree : vecFreeParams)
		if(symfree.strIdent == str)
			return true;

	// look in constants map
	t_syms::const_iterator iter_c = g_syms.find(str);
	if(iter_c != g_syms.end())
	{
		if(pdVal)
			*pdVal = (*iter_c).second;
		return true;
	}

	// look in symbol map
	for(const Symbol& sym : syms)
		if(sym.strIdent == str)
		{
			if(pdVal) *pdVal = sym.dVal;
			return true;
		}
		return false;
}

static const double *get_symbol_ptr(const std::vector<Symbol>& syms, const std::string& str, const std::vector<Symbol>& vecFreeParams)
{
	// look in free parameters
	for(const Symbol& symfree : vecFreeParams)
		if(symfree.strIdent == str)
			return &symfree.dVal;

	// look in constants map
	t_syms::const_iterator iter_c = g_syms.find(str);
	if(iter_c != g_syms.end())
		return &(*iter_c).second;

	// look in symbol map
	for(const Symbol& sym : syms)
		if(sym.strIdent == str)
			return &sym.dVal;

	return nullptr;
}

static void print_symbol_map(const std::vector<Symbol>& syms)
{
	tl::log_info("Symbols map: ");
	for(const Symbol& sym : syms)
		tl::log_info("\t", sym.strIdent, " = ", sym.dVal);

	tl::log_info("Constants map: ");
	for(const auto& sym : g_syms)
		tl::log_info("\t", sym.first, " = ", sym.second);

	std::ostringstream ostrFkts;
	tl::log_info("Functions map: ");
	for(const auto& pairFkt : g_map_fkt0)
		ostrFkts << pairFkt.first << "(), ";
	for(const auto& pairFkt : g_map_fkt1)
		ostrFkts << pairFkt.first << "(a), ";
	for(const auto& pairFkt : g_map_fkt2)
		ostrFkts << pairFkt.first << "(a,b), ";
	tl::log_info(ostrFkts.str());
}

static void init_globals()
{
	static bool bInited = 0;
	if(bInited) return;
	bInited = 1;

	tl::init_rand();
}

static void clear_node(Node& node)
{
	node.iType = NODE_NOP;
	node.dVal = 0.;
	node.vecChildren.clear();
}

Node::Node() : iType(NODE_INVALID), dVal(0)
{}

static bool parse_expression(Node& node, std::vector<Symbol>& syms, const std::string& str, const std::vector<Symbol>& vecFreeParams)
{
	clear_node(node);

	// fill in globals
	init_globals();

	expression_parser<std::string::const_iterator> pars;

	bool bOk = qi::phrase_parse(str.begin(), str.end(), pars, ascii::space, node);
	if(!bOk)
	{
		tl::log_err("Error parsing expression \"", str, "\": ", pars.ostrErr.str());
		return false;
	}

	if(!check_tree_sanity(node))
	{
		tl::log_err("Syntax tree is not sane.");
		return false;
	}

	optimize_tree(node);

	if(!check_tree_sanity(node))
	{
		tl::log_err("Optimized syntax tree is not sane.");
		return false;
	}

	create_symbol_map(node, syms, vecFreeParams);
	//print_node(node,0);

	return bOk;
}

static bool parse_expression(Node& node, std::vector<Symbol>& syms, const std::string& str)
{
	std::vector<Symbol> vecFreeParams;
	get_default_free_params(vecFreeParams);

	return parse_expression(node, syms, str, vecFreeParams);
}


//======================================================================
// class interface

Parser::Parser(const std::vector<Symbol>* pvecFreeParams)
	: m_bOk(false)
{
#ifdef USE_JIT
	if(s_iInstances++ == 0)
		llvm::InitializeNativeTarget();
#endif

	// set "x" as default free param if no others given
	if(!pvecFreeParams)
	{
		Symbol sym;
		sym.strIdent = "x";

		m_vecFreeParams.push_back(sym);
	}
}

Parser::Parser(const Parser& parser)
{
#ifdef USE_JIT
	if(s_iInstances++ == 0)
		llvm::InitializeNativeTarget();
#endif

	this->operator=(parser);
}

Parser& Parser::operator=(const Parser& parser)
{
	this->m_node = parser.m_node;
	this->m_syms = parser.m_syms;
	this->m_vecFreeParams = parser.m_vecFreeParams;
	this->m_bOk = parser.m_bOk;

	// TODO: Copy already compiled bitcode
	this->Compile();

	return *this;
}

Parser::~Parser()
{
#ifdef USE_JIT
	DeinitJIT();

	if(--s_iInstances == 0)
		llvm::llvm_shutdown();
#endif
}

void Parser::SetFreeParams(const std::vector<Symbol>& vecFreeParams)
{
	m_vecFreeParams = vecFreeParams;
}

void Parser::clear(bool bClearFreeParams)
{
	m_node.vecChildren.clear();
	m_node.iType = NODE_INVALID;

	m_syms.clear();

	if(bClearFreeParams)
		m_vecFreeParams.clear();
}

Node& Parser::GetRootNode() { return m_node; }
std::vector<Symbol>& Parser::GetSymbols() { return m_syms; }
std::vector<Symbol>& Parser::GetFreeParams() { return m_vecFreeParams; }

const Node& Parser::GetRootNode() const { return m_node; }
const std::vector<Symbol>& Parser::GetSymbols() const { return m_syms; }
const std::vector<Symbol>& Parser::GetFreeParams() const { return m_vecFreeParams; }


bool Parser::IsSymbolInMap(const std::string& str, double* pdVal) const
{
	return ::is_symbol_in_map(m_syms, str, m_vecFreeParams, pdVal);
}

// create a syntax tree out of a expression string
bool Parser::ParseExpression(const std::string& str)
{
	clear(false);
	m_bOk = ::parse_expression(m_node, m_syms, str, m_vecFreeParams);
	Compile();

	//log_info("--------------------------------------------------------------------------------");
	tl::log_info("Parsing ", (m_bOk ? "successful" : "failed"));
	tl::log_info("Parsed expression: ", GetExpression(false, false));
	PrintSymbolMap();
	//log_info("--------------------------------------------------------------------------------");

	return m_bOk;
}



// evaluate the syntax tree
#ifndef USE_JIT
double Parser::Eval()
{
	return ::eval_tree(m_node, m_syms, m_vecFreeParams);
}
#endif

double Parser::EvalTree(const double *px)
{
	if(px)
	{
		for(unsigned int i=0; i<m_vecFreeParams.size(); ++i)
			m_vecFreeParams[i].dVal = px[i];
	}
	return Eval();
}

double Parser::EvalTree(double x)
{
	if(m_vecFreeParams.size() != 1)
	{
		tl::log_err("Symbol table has more than one free parameter, but only one given!");
		return 0.;
	}

	m_vecFreeParams[0].dVal = x;
	return Eval();
}

// get a string representation of the syntax tree's expression
std::string Parser::GetExpression(bool bFillInSyms, bool bGnuPlotSyntax) const
{
	return ::get_expression(m_node, m_syms, m_vecFreeParams, bFillInSyms, bGnuPlotSyntax);
}

bool Parser::IsOk() const { return m_bOk; }

// print tree & symbol map
void Parser::PrintTree() const { ::print_node(m_node); }
void Parser::PrintSymbolMap() const { ::print_symbol_map(m_syms); }

bool Parser::CheckValidLexemes(const std::string& str)
{
	for(char c : str)
	{
		if(isalnum(c) || isblank(c) ||
				c=='+' || c=='-' || c=='*' || c=='/' || c=='^' ||
				c=='(' || c==')' || c=='_' || c=='.' || c==',')
			continue;
		else
			return false;
	}
	return true;
}



//======================================================================
// using LLVM JIT compiler

// llvm-codegen: clang -c -emit-llvm -o 0.ll 0.cpp && llc -march=cpp -o 0.cxx 0.ll
#ifdef USE_JIT

int Parser::s_iInstances = 0;

void Parser::InitJIT()
{
	DeinitJIT();

	m_pVMContext = new llvm::LLVMContext();
	m_pVMModule = new llvm::Module("parser", *m_pVMContext);

	llvm::FunctionType* pFuncTy = llvm::FunctionType::get(
			llvm::Type::getDoubleTy(*m_pVMContext),
			std::vector<llvm::Type*>(), false);
	m_pVMFunc = llvm::Function::Create(pFuncTy,
			llvm::GlobalValue::LinkageTypes::InternalLinkage, "func", m_pVMModule);
	m_pVMBlock = llvm::BasicBlock::Create(*m_pVMContext, "block", m_pVMFunc);

	m_pVMBuilder = new llvm::IRBuilder<true>(m_pVMBlock);
}

void Parser::DeinitJIT()
{
	if(m_pVMBuilder) { delete m_pVMBuilder; m_pVMBuilder = nullptr; }
	if(m_pVMBlock) { /*delete m_pVMBlock;*/ m_pVMBlock = nullptr; }
	if(m_pVMFunc) { /*delete m_pVMFunc;*/ m_pVMFunc = nullptr; }
	if(m_pVMExec) { /*delete m_pVMExec;*/ m_pVMExec = nullptr; }
	if(m_pVMModule) { delete m_pVMModule; m_pVMModule = nullptr; }
	if(m_pVMContext) { delete m_pVMContext; m_pVMContext = nullptr; }
}

static llvm::LoadInst* get_fkt_ptr(llvm::Module* pMod, llvm::BasicBlock *pBlock,
		const std::string& strName, unsigned int iNumArgs)
{
	std::vector<llvm::Type*> vecArgs;
	vecArgs.reserve(iNumArgs);
	for(unsigned int iArg=0; iArg<iNumArgs; ++iArg)
		vecArgs.push_back(llvm::Type::getDoubleTy(pMod->getContext()));

	llvm::FunctionType* pFuncTy = llvm::FunctionType::get(
			llvm::Type::getDoubleTy(pMod->getContext()), vecArgs, false);
	const void *pvFunc = nullptr;

	if(iNumArgs == 0)
	{
		typename decltype(g_map_fkt0)::const_iterator iter = g_map_fkt0.find(strName);
		if(iter != g_map_fkt0.end())
			pvFunc = (const void*)iter->second;
	}
	else if(iNumArgs == 1)
	{
		typename decltype(g_map_fkt1)::const_iterator iter = g_map_fkt1.find(strName);
		if(iter != g_map_fkt1.end())
			pvFunc = (const void*)iter->second;
	}
	else if(iNumArgs == 2)
	{
		typename decltype(g_map_fkt2)::const_iterator iter = g_map_fkt2.find(strName);
		if(iter != g_map_fkt2.end())
			pvFunc = (const void*)iter->second;
	}

	if(pvFunc)
	{
		llvm::PointerType* ptrtyFkt = llvm::PointerType::get(pFuncTy, 0);
		llvm::AllocaInst* ptrFkt = new llvm::AllocaInst(ptrtyFkt, "pFkt", pBlock);
		ptrFkt->setAlignment(8);

		llvm::Value *pValFkt = llvm::ConstantInt::get(pMod->getContext(),
				llvm::APInt(sizeof(void*)*8, long(pvFunc)));
		llvm::StoreInst *pStore = new llvm::StoreInst(pValFkt, ptrFkt, false, pBlock);
		pStore->setAlignment(8);

		llvm::LoadInst *pLoadInst = new llvm::LoadInst(ptrFkt, "fkt", false, pBlock);
		return pLoadInst;
	}

	tl::log_err("No such function: \"", strName, "\"");
	return nullptr;

/*
	// try externally linked functions
	llvm::Function* pFunc = llvm::Function::Create(pFuncTy, llvm::GlobalValue::ExternalLinkage, strName, pMod);
	return pFunc;
*/
}

static llvm::LoadInst* deref_dbl_ptr(llvm::Module* pMod, llvm::BasicBlock *pBlock,
		const double *pd)
{
	llvm::PointerType* ptrtyDbl = llvm::PointerType::get(
			llvm::Type::getDoubleTy(pMod->getContext()), 0);

	llvm::AllocaInst* ptrDbl = new llvm::AllocaInst(ptrtyDbl, "pSym", pBlock);
	ptrDbl->setAlignment(8);

	llvm::Value *pVal = llvm::ConstantInt::get(pMod->getContext(),
			llvm::APInt(sizeof(double*)*8, long(pd)));
	llvm::StoreInst* pStore = new llvm::StoreInst(pVal, ptrDbl, false, pBlock);
	pStore->setAlignment(8);

	return new llvm::LoadInst(new llvm::LoadInst(ptrDbl, "", false, pBlock),
			"", false, pBlock);
}

llvm::Value* Parser::Compile(const Node& node)
{
	const unsigned int iNumSub = node.vecChildren.size();

	std::vector<llvm::Value*> vecSub;
	vecSub.reserve(iNumSub);

	for(const Node& child : node.vecChildren)
		vecSub.push_back(Compile(child));

	if(node.iType == NODE_CALL)
	{
		llvm::LoadInst *pFkt = get_fkt_ptr(m_pVMModule, m_pVMBlock, node.strIdent, vecSub.size());
		return m_pVMBuilder->CreateCall(pFkt, vecSub, node.strIdent);
		//return llvm::CallInst::Create(pFkt, vecSub, node.strIdent, m_pVMBlock);
	}

	if(iNumSub==0)
	{
		if(node.iType == NODE_DOUBLE)
			return llvm::ConstantFP::get(*m_pVMContext, llvm::APFloat(node.dVal));
		else if(node.iType == NODE_IDENT)
		{
			const double *pSym = get_symbol_ptr(m_syms, node.strIdent, m_vecFreeParams);
			if(!pSym)
			{
				tl::log_err("Invalid symbol: \"", node.strIdent, "\"");
				return nullptr;
			}

			return deref_dbl_ptr(m_pVMModule, m_pVMBlock, pSym);
		}
		else if(node.iType == NODE_NOP)
		{
			// nop: 0+0
			llvm::Value* pZero = llvm::ConstantFP::get(*m_pVMContext, llvm::APFloat(0.));
			return m_pVMBuilder->CreateFAdd(pZero, pZero, get_op_name(node.iType));
		}
	}
	else if(iNumSub==1)		// unary operators
	{
		//llvm::Value *pValZero = llvm::ConstantFP::get(*m_pVMContext, llvm::APFloat(0.));
		if(node.iType == NODE_PLUS)
			return vecSub[0];
			//return m_pVMBuilder->CreateFAdd(vecSub[0], pValZero, get_op_name(node.iType));
		else if(node.iType == NODE_MINUS)
			return m_pVMBuilder->CreateFNeg(vecSub[0], get_op_name(node.iType));
			//return m_pVMBuilder->CreateFSub(pValZero, vecSub[0], get_op_name(node.iType));
	}
	else if(iNumSub==2)		// binary operators
	{
		if(node.iType == NODE_PLUS)
			return m_pVMBuilder->CreateFAdd(vecSub[0], vecSub[1], get_op_name(node.iType));
		else if(node.iType == NODE_MINUS)
			return m_pVMBuilder->CreateFSub(vecSub[0], vecSub[1], get_op_name(node.iType));
		else if(node.iType == NODE_MULT)
			return m_pVMBuilder->CreateFMul(vecSub[0], vecSub[1], get_op_name(node.iType));
		else if(node.iType == NODE_DIV)
			return m_pVMBuilder->CreateFDiv(vecSub[0], vecSub[1], get_op_name(node.iType));
		else if(node.iType == NODE_POW)
		{
			llvm::LoadInst *pPow = get_fkt_ptr(m_pVMModule, m_pVMBlock, "pow", 2);
			return m_pVMBuilder->CreateCall(pPow, vecSub, "pow");
		}
		else if(node.iType == NODE_MINUS_INV)
			return m_pVMBuilder->CreateFSub(vecSub[1], vecSub[0], get_op_name(node.iType));
		else if(node.iType == NODE_DIV_INV)
			return m_pVMBuilder->CreateFDiv(vecSub[1], vecSub[0], get_op_name(node.iType));
	}

	tl::log_err("Cannot compile node \"", get_op_name(node.iType),
			"\" having ", iNumSub, " children.");
	return nullptr;
}

bool Parser::Compile()
{
	tl::log_info("Using JIT compiler");
	InitJIT();

	m_pVMBuilder->CreateRet(Compile(m_node));

	llvm::EngineBuilder builder(m_pVMModule);
	builder.setEngineKind(llvm::EngineKind::JIT);
	builder.setOptLevel(llvm::CodeGenOpt::Default);
	m_pVMExec = builder.create();

	/*// optimisation
	m_pVMModule->setDataLayout(m_pVMExec->getDataLayout());
	llvm::legacy::FunctionPassManager fpm(m_pVMModule);
	//fpm.add(llvm::createInstructionCombiningPass());
	fpm.add(llvm::createGVNPass());
	fpm.add(llvm::createSCCPPass());
	fpm.add(llvm::createCFGSimplificationPass());
	fpm.run(*m_pVMFunc);*/

	m_pFunc = (double(*)())m_pVMExec->getPointerToFunction(m_pVMFunc);
	return 1;
}

double Parser::Eval()
{
	if(m_pFunc)
		return m_pFunc();
	else
	{
		llvm::GenericValue gv = m_pVMExec->runFunction(m_pVMFunc,
				std::vector<llvm::GenericValue>());
		return double(gv.DoubleVal);
	}
}

#else

bool Parser::Compile() { return 0; }

#endif


//======================================================================




//----------------------------------------------------------------------
// symbol with 2 values (for parameter hints & limits)
//----------------------------------------------------------------------

struct SymbolWith2Values
{
	std::string strSym;
	double dVal0, dVal1;

	std::string strExp0, strExp1;
};

BOOST_FUSION_ADAPT_STRUCT
(
	SymbolWith2Values,
	(std::string, strSym)
	(double, dVal0)
	(double, dVal1)
	(std::string, strExp0)
	(std::string, strExp1)
)

template<typename Iter>
struct symbol_with_2values_parser
	: qi::grammar<Iter, std::vector<SymbolWith2Values>(), ascii::space_type>
{
	qi::rule<Iter, std::string(), ascii::space_type> ident;
	qi::rule<Iter, std::string(), ascii::space_type> expression;
	qi::rule<Iter, SymbolWith2Values(), ascii::space_type> onelimit;
	qi::rule<Iter, std::vector<SymbolWith2Values>(), ascii::space_type> limits;

	std::ostringstream ostrErr;

	symbol_with_2values_parser() :
		symbol_with_2values_parser::base_type(limits, "limits")
	{
		ident %= qi::lexeme[qi::char_("A-Za-z_") >> *(qi::char_("A-Za-z0-9_"))];
		expression %= qi::lexeme[+(qi::char_ - qi::char_(':') - qi::char_(';'))];


		onelimit =
			ident							[ph::at_c<0>(qi::_val) = qi::_1]
			> '='
			> expression					[ph::at_c<4>(qi::_val) = ph::at_c<3>(qi::_val) = qi::_1]
			>> -(
					qi::lit(":")
					> expression			[ph::at_c<4>(qi::_val) = qi::_1]
				)
			;

		limits = qi::eps || (onelimit % ';');


		ident.name("ident");
		onelimit.name("onelimit");
		limits.name("limits");
		expression.name("expression");

		qi::on_error<qi::fail>
		(
			limits,
			(std::ostream&) ostrErr
				<< ph::val("Expected ") << qi::labels::_4
				<< ph::val(" at token \"")
				<< ph::construct<std::string>(qi::labels::_3, qi::labels::_2)
				<< ph::val("\".")
		);
	}
};

// evaluate the expressions stored in the strings
static void symbol_with_2values_fill_in(std::vector<SymbolWith2Values>& vec,
										const std::string& strWhich)
{
	for(SymbolWith2Values& sym2 : vec)
	{
		Node node0, node1;
		std::vector<Symbol> syms0, syms1;

		bool bOk0 = parse_expression(node0, syms0, sym2.strExp0);
		bool bOk1 = parse_expression(node1, syms1, sym2.strExp1);

		if(syms0.size()!=0 || syms1.size()!=0)
		{
			tl::log_warn("Free parameters specified in ", strWhich, ", assuming value 0.");
		}

		if(!bOk0 || !bOk1)
		{
			tl::log_err("Could not parse ", strWhich, " expression.");
		}

		sym2.dVal0 = eval_tree(node0, syms0, 0);
		sym2.dVal1 = eval_tree(node1, syms1, 0);
	}
}

static std::vector<SymbolWith2Values> parse_symbol_with_2values(const std::string& str,
																const std::string& strWhich)
{
	std::vector<SymbolWith2Values> params;
	symbol_with_2values_parser<std::string::const_iterator> lp;

	bool bOk = qi::phrase_parse(str.begin(), str.end(), lp, ascii::space, params);
	if(!bOk)
	{
		tl::log_err("Error parsing ", strWhich, ": ", lp.ostrErr.str());
	}

	symbol_with_2values_fill_in(params, strWhich);

	/*
	if(g_bVerbose && params.size()!=0)
	{
		std::cout << "\nParsed " << strWhich << ":\n";
		for(unsigned int i=0; i<params.size(); ++i)
			std::cout << fus::as_vector(params[i]) << "\n";
		std::cout << "\n";
	}*/
	return params;
}

// parameter limits
std::vector<ParameterLimits> parse_parameter_limits(const std::string& str)
{
	std::vector<SymbolWith2Values> vec = parse_symbol_with_2values(str, "parameter limits");
	std::vector<ParameterLimits> params;

	for(SymbolWith2Values& sym2 : vec)
	{
		ParameterLimits param;

		param.strSym = sym2.strSym;
		param.dLower = sym2.dVal0;
		param.dUpper = sym2.dVal1;

		if(param.dLower > param.dUpper)
			std::swap(param.dLower, param.dUpper);

		params.push_back(param);
	}
	return params;
}

// parameter hints
std::vector<ParameterHints> parse_parameter_hints(const std::string& str)
{
	std::vector<SymbolWith2Values> vec = parse_symbol_with_2values(str, "parameter hints");
	std::vector<ParameterHints> params;

	for(SymbolWith2Values& sym2 : vec)
	{
		ParameterHints param;

		param.strSym = sym2.strSym;
		param.dVal = sym2.dVal0;
		param.dErr = sym2.dVal1;

		params.push_back(param);
	}
	return params;
}



/*
// test
// clang -I.. -o parsertst -DUSE_JIT parser.cpp ../tlibs/helper/log.cpp ../tlibs/math/rand.cpp -std=c++11 -lstdc++ -lm -L/usr/lib64/llvm -lLLVM-3.5
#include <iostream>
#include <limits>

int main()
{
	tl::log_debug.SetEnabled(0);
	tl::log_info.SetEnabled(0);

	Parser pars;

	while(1)
	{
		std::cout << "> ";
		std::string strLine;
		std::getline(std::cin, strLine);

		pars.ParseExpression(strLine);
		//pars.PrintTree();
		std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
				<< pars.EvalTree() << std::endl;
	}

	return 0;
}
*/
