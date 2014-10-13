/*
 * Parse & Runtime Info
 * @author tweber
 * @date 12-oct-14
 */

#ifndef __SCRIPT_INFOS_H__
#define __SCRIPT_INFOS_H__

#include "node.h"
#include "handles.h"
#include "symbol.h"

// TODO
// stuff that can change during execution
struct RuntimeInfo
{
};

// stuff that is fixed after parsing
struct ParseInfo
{
	// external imported modules
	typedef std::unordered_map<t_string, Node*> t_mods;
	t_mods *pmapModules = nullptr;

	// function to execute, e.g. "main" (with external local symbol table)
	t_string strExecFkt;
	t_string strInitScrFile;
	SymbolTable *pLocalSymsOverride = nullptr;

	// implicitely return last symbol in function
	bool bImplicitRet = 0;

	// all functions from all modules
	typedef std::vector<NodeFunction*> t_funcs;
	t_funcs vecFuncs;

	// global symbol table
	SymbolTable *pGlobalSyms = nullptr;

	HandleManager *phandles = nullptr;
	// mutex for script if no explicit mutex given
	std::mutex *pmutexGlobal = nullptr;
	// mutex for interpreter
	std::mutex *pmutexInterpreter = nullptr;

	// currently active function
	const NodeFunction *pCurFunction = nullptr;
	const NodeCall *pCurCaller = nullptr;
	bool bWantReturn = 0;

	const Node* pCurLoop = nullptr;
	bool bWantBreak = 0;
	bool bWantContinue = 0;

	bool bDestroyParseInfo = true;



	bool bEnableDebug = 0;
	typedef std::deque<std::string> t_oneTraceback;
	typedef std::unordered_map<std::thread::id, t_oneTraceback> t_stckTraceback;
	t_stckTraceback stckTraceback;

	void PushTraceback(std::string&& strTrace);
	void PopTraceback();



	ParseInfo();
	~ParseInfo();

	NodeFunction* GetFunction(const t_string& strName);

	bool IsExecDisabled() const
	{
		return bWantReturn || bWantBreak || bWantContinue;
	}
	void EnableExec()
	{
		bWantReturn = bWantBreak = bWantContinue = 0;
	}
};

#endif
