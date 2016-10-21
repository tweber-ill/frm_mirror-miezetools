/*
 * Parser for the "free function" fitter model
 * Author: Tobias Weber
 * Date: April 2012
 * @license GPLv3
 */

#ifndef __FKT_PARSER__H__
#define __FKT_PARSER__H__

#include <string>
#include <vector>

#ifdef USE_JIT
	#define __STDC_CONSTANT_MACROS
	#define __STDC_FORMAT_MACROS
	#define __STDC_LIMIT_MACROS

	#include <llvm/IR/LLVMContext.h>
	#include <llvm/IR/Module.h>
	#include <llvm/IR/IRBuilder.h>
	#include <llvm/ExecutionEngine/JIT.h>
#endif



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
	protected:
		Node m_node;
		std::vector<Symbol> m_syms;
		std::vector<Symbol> m_vecFreeParams;

		bool m_bOk = false;

#ifdef USE_JIT
		static int s_iInstances;

		llvm::Module *m_pVMModule = nullptr;
		llvm::LLVMContext *m_pVMContext = nullptr;
		llvm::IRBuilder<> *m_pVMBuilder = nullptr;

		llvm::Function *m_pVMFunc = nullptr;
		llvm::BasicBlock *m_pVMBlock = nullptr;

		llvm::ExecutionEngine *m_pVMExec = nullptr;

		double (*m_pFunc)() = nullptr;

		void InitJIT();
		void DeinitJIT();
		llvm::Value* Compile(const Node& node);
#endif

		bool Compile();
		double Eval();

	public:
		Parser(const std::vector<Symbol>* pvecFreeParams=0);
		Parser(const Parser& parser);
		virtual ~Parser();

		Parser& operator=(const Parser& parser);

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

		bool IsOk() const;

		// print tree & symbol map
		void PrintTree() const;
		void PrintSymbolMap() const;

		static bool CheckValidLexemes(const std::string& str);
};


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
