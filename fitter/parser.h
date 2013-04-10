/*
 * Parser for the "free function" fitter model
 * Author: Tobias Weber
 * Date: April 2012
 */

#ifndef __FKT_PARSER__H__
#define __FKT_PARSER__H__

#include <string>
#include <vector>

//----------------------------------------------------------------------

struct Node
{
	int iType;

	double dVal;
	std::string strIdent;

	std::vector<Node> vecChildren;

	Node();
};

struct Symbol
{
	std::string strIdent;
	double dVal;
};


//----------------------------------------------------------------------

class Parser
{
	private:
		bool m_bVerbose;
	
	protected:
		Node m_node;
		std::vector<Symbol> m_syms;
		std::vector<Symbol> m_vecFreeParams;

		bool m_bOk;
	
	public:
		Parser(const std::vector<Symbol>* pvecFreeParams=0);
		Parser(const Parser& parser);
		
		Parser& operator=(const Parser& parser);

		virtual ~Parser();

		void SetFreeParams(const std::vector<Symbol>& vecFreeParams);
		
		void clear(bool bClearFreeParams=true);
		
		Node& GetRootNode();
		std::vector<Symbol>& GetSymbols();
		std::vector<Symbol>& GetFreeParams();

		const Node& GetRootNode() const;
		const std::vector<Symbol>& GetSymbols() const;
		const std::vector<Symbol>& GetFreeParams() const;
		
		bool IsSymbolInMap(const std::string& str, double* pdVal=0) const;

		// create a syntax tree out of a expression string
		bool ParseExpression(const std::string& str);
		
		// evaluate the syntax tree
		double EvalTree(const double *px=0);
		double EvalTree(double x);
		
		// get a string representation of the syntax tree's expression
		std::string GetExpression(bool bFillInSyms=false, bool bGnuPlotSyntax=true) const;
		
		void SetVerbose(bool bVerbose);
		bool IsOk() const;
		
		// print tree & symbol map
		void PrintTree() const;
		void PrintSymbolMap() const;

		static bool CheckValidLexemes(const std::string& str);
};

// global default
void parser_set_verbose(bool bVerbose);

//======================================================================



//----------------------------------------------------------------------
struct ParameterLimits
{
	std::string strSym;
	double dLower;
	double dUpper;
};

std::vector<ParameterLimits> parse_parameter_limits(const std::string& str);
//----------------------------------------------------------------------



//----------------------------------------------------------------------
struct ParameterHints
{
	std::string strSym;
	double dVal;
	double dErr;
};

std::vector<ParameterHints> parse_parameter_hints(const std::string& str);
//----------------------------------------------------------------------



#endif
