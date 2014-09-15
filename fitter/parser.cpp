/*
 * Parser for the "free function" fitter model
 * Author: Tobias Weber
 * Date: April 2012
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

#include <ctype.h>
#include <iostream>
#include <sstream>
#include <map>
#include "parser.h"
#include "functions.h"

#include "../helper/math.h"
#include "../helper/log.h"

//----------------------------------------------------------------------
// old procedural interface

// there are two versions of these functions:
// the first includes a vector of free parameters, e.g. x,y,z
// the second assumes one default free parameter, namely x

static bool is_symbol_in_map(const std::vector<Symbol>& syms, const std::string& str, const std::vector<Symbol>& vecFreeParams, double* pdVal=0);
static bool is_symbol_in_map(const std::vector<Symbol>& syms, const std::string& str, double* pdVal=0);

// create a syntax tree out of a expression string
static bool parse_expression(Node& node, std::vector<Symbol>& syms, const std::string& str, const std::vector<Symbol>& vecFreeParams);
static bool parse_expression(Node& node, std::vector<Symbol>& syms, const std::string& str);

// evaluate the syntax tree
static double eval_tree(const Node& node, const std::vector<Symbol>& syms, const std::vector<Symbol>& vecFreeParams);
static double eval_tree(const Node& node, const std::vector<Symbol>& syms, double x);

// get a string representation of the syntax tree's expression
static std::string get_expression(const Node& node, const std::vector<Symbol>& syms, const std::vector<Symbol>& vecFreeParams, bool bFillInSyms=false, bool bGnuPlotSyntax=true);
static std::string get_expression(const Node& node, const std::vector<Symbol>& syms, bool bFillInSyms=false, bool bGnuPlotSyntax=true);

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
static std::map<std::string, double> g_syms;

// functions with zero arguments
static std::map<std::string, double (*)(void)> g_map_fkt0;
// functions with one argument
static std::map<std::string, double (*)(double)> g_map_fkt1;
// functions with two arguments
static std::map<std::string, double (*)(double, double)> g_map_fkt2;
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
			log_err("operation 'pow' needs exactly two operands.");
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
			log_err("Symbol \"", node.strIdent, "\" is not in map!");
		}
		return dVal;
	}
	else if(node.iType == NODE_CALL)
	{
		std::string strFkt = node.strIdent;
		int iNumArgs = node.vecChildren.size();
		if(iNumArgs==0)
		{
			std::map<std::string, double (*)(void)>::iterator iter0
					= g_map_fkt0.find(strFkt);

			if(iter0 != g_map_fkt0.end())
			{
				double (*pFkt)(void) = (*iter0).second;
				return pFkt();
			}
		}
		else if(iNumArgs==1)
		{
			std::map<std::string, double (*)(double)>::iterator iter1
					= g_map_fkt1.find(strFkt);

			if(iter1 != g_map_fkt1.end())
			{
				double (*pFkt)(double) = (*iter1).second;
				return pFkt(eval_tree(node.vecChildren[0], syms, vecFreeParams));
			}
		}
		else if(iNumArgs==2)
		{
			std::map<std::string, double (*)(double, double)>::iterator iter2
					= g_map_fkt2.find(strFkt);

			if(iter2 != g_map_fkt2.end())
			{
				double (*pFkt)(double, double) = (*iter2).second;
				return pFkt(eval_tree(node.vecChildren[0], syms, vecFreeParams),
					    eval_tree(node.vecChildren[1], syms, vecFreeParams));
			}
		}

		log_err("No function \"", strFkt, "\" taking ", iNumArgs, " arguments known.");
		return 0.;
	}
	else if(node.iType == NODE_NOP)
	{
		return 0.;
	}
	else if(node.iType == NODE_INVALID)
	{
		log_err("Syntax tree is invalid.");
		return 0.;
	}

	log_err("Unknown syntax node type ", node.iType);
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

	log_err("Unknown syntax node type ", node.iType);
	return "";
}

static std::string get_expression(const Node& node, const std::vector<Symbol>& syms, bool bFillInSyms, bool bGnuPlotSyntax)
{
	std::vector<Symbol> vecFreeParams;
	get_default_free_params(vecFreeParams);

	return get_expression(node, syms, vecFreeParams, bFillInSyms, bGnuPlotSyntax);
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
	std::map<std::string,double>::const_iterator iter_c = g_syms.find(str);
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

static bool is_symbol_in_map(const std::vector<Symbol>& syms, const std::string& str, double* pdVal)
{
	std::vector<Symbol> vecFreeParams;
	get_default_free_params(vecFreeParams);

	return is_symbol_in_map(syms, str, vecFreeParams, pdVal);
}

static void print_symbol_map(const std::vector<Symbol>& syms)
{
	log_info("Symbols map: ");
	for(const Symbol& sym : syms)
		log_info("\t", sym.strIdent, " = ", sym.dVal);

	log_info("Constants map: ");
	for(const auto& sym : g_syms)
		log_info("\t", sym.first, " = ", sym.second);

	std::ostringstream ostrFkts;
	log_info("Functions map: ");
	for(const auto& pairFkt : g_map_fkt0)
		ostrFkts << pairFkt.first << "(), ";
	for(const auto& pairFkt : g_map_fkt1)
		ostrFkts << pairFkt.first << "(a), ";
	for(const auto& pairFkt : g_map_fkt2)
		ostrFkts << pairFkt.first << "(a,b), ";
	log_info(ostrFkts.str());
}

static void add_constants()
{
	// already inited?
	if(g_syms.size()!=0)
		return;

	g_syms["pi"] = M_PI;
}

static void add_functions()
{
	// already inited?
	if(g_map_fkt0.size()!=0 || g_map_fkt1.size()!=0 || g_map_fkt2.size()!=0)
		return;

	init_special_functions();
	g_map_fkt0["rand01"] = ::my_rand01;


	g_map_fkt1["abs"] = ::fabs;
	g_map_fkt1["sin"] = ::sin;
	g_map_fkt1["cos"] = ::cos;
	g_map_fkt1["tan"] = ::tan;
	g_map_fkt1["asin"] = ::asin;
	g_map_fkt1["acos"] = ::acos;
	g_map_fkt1["atan"] = ::atan;

	g_map_fkt1["sinh"] = ::sinh;
	g_map_fkt1["cosh"] = ::cosh;
	g_map_fkt1["tanh"] = ::tanh;
	g_map_fkt1["asinh"] = ::my_asinh;
	g_map_fkt1["acosh"] = ::my_acosh;
	g_map_fkt1["atanh"] = ::my_atanh;

	g_map_fkt1["exp"] = ::exp;
	g_map_fkt1["log"] = ::log;
	g_map_fkt1["log10"] = ::log10;

	g_map_fkt1["sqrt"] = ::sqrt;

	g_map_fkt1["ceil"] = ::ceil;
	g_map_fkt1["floor"] = ::floor;
	g_map_fkt1["fabs"] = ::fabs;
	g_map_fkt1["round"] = ::my_round;
	g_map_fkt1["sign"] = ::my_sign;

	g_map_fkt1["erf"] = ::my_erf;
	g_map_fkt1["erf_inv"] = ::my_erf_inv;

	g_map_fkt2["atan2"] = ::atan2;
	g_map_fkt2["pow"] = ::pow;
	g_map_fkt2["fmod"] = ::fmod;
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
	add_constants();
	add_functions();

	expression_parser<std::string::const_iterator> pars;

	bool bOk = qi::phrase_parse(str.begin(), str.end(), pars, ascii::space, node);
	if(!bOk)
	{
		log_err("Error parsing expression \"", str, "\": ", pars.ostrErr.str());
		return false;
	}

	if(!check_tree_sanity(node))
	{
		log_err("Syntax tree is not sane.");
		return false;
	}

	optimize_tree(node);

	if(!check_tree_sanity(node))
	{
		log_err("Optimized syntax tree is not sane.");
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
	// set "x" as default free param if no others given
	if(!pvecFreeParams)
	{
		Symbol sym;
		sym.strIdent = "x";

		m_vecFreeParams.push_back(sym);
	}
}

Parser::Parser(const Parser& parser) { this->operator=(parser); }

Parser& Parser::operator=(const Parser& parser)
{
	this->m_node = parser.m_node;
	this->m_syms = parser.m_syms;
	this->m_vecFreeParams = parser.m_vecFreeParams;
	this->m_bOk = parser.m_bOk;

	return *this;
}
	
Parser::~Parser() {}
	
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

	//log_info("--------------------------------------------------------------------------------");
	log_info("Parsing ", (m_bOk ? "successful" : "failed"));
	log_info("Parsed expression: ", GetExpression(false, false));
	PrintSymbolMap();
	//log_info("--------------------------------------------------------------------------------");

	return m_bOk;
}

// evaluate the syntax tree
double Parser::EvalTree(const double *px)
{
	if(px)
	{
		for(unsigned int i=0; i<m_vecFreeParams.size(); ++i)
			m_vecFreeParams[i].dVal = px[i];
	}
	return ::eval_tree(m_node, m_syms, m_vecFreeParams);
}

double Parser::EvalTree(double x)
{
	if(m_vecFreeParams.size() != 1)
	{
		log_err("Symbol table has more than one free parameter, but only one given!");
		return 0.;
	}

	m_vecFreeParams[0].dVal = x;
	return ::eval_tree(m_node, m_syms, m_vecFreeParams);
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
			log_warn("Free parameters specified in ", strWhich, ", assuming value 0.");
		}

		if(!bOk0 || !bOk1)
		{
			log_err("Could not parse ", strWhich, " expression.");
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
		log_err("Error parsing ", strWhich, ": ", lp.ostrErr.str());
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
