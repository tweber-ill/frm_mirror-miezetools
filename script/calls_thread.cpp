/*
 * external thread functions
 * @author tweber
 * @date dec 2013
 */

#include "calls_thread.h"
#include "calls.h"
#include <thread>
#include <wait.h>


static Symbol* fkt_exec(const std::vector<Symbol*>& vecSyms,
						ParseInfo& info, SymbolTable* pSymTab)
{
	std::string strExec;

	for(Symbol *pSym : vecSyms)
		if(pSym)
		{
			strExec += pSym->print();
			strExec += " ";
		}

	bool bOk = 0;
	FILE *pPipe = ::popen(strExec.c_str(), "w");
	if(pPipe)
	{
		bOk = 1;
		int iRet = ::pclose(pPipe);
		if(iRet == -1)
		{
			bOk = 0;
		}
		else
		{
			//int bHasExited = WIFEXITED(iRet);
			int iExitCode = int(char(WEXITSTATUS(iRet)));
			//std::cout << "Exit code: " << iExitCode << std::endl;
			bOk = (iExitCode==0);
		}
	}

	return new SymbolInt(bOk);
}


// --------------------------------------------------------------------------------
// thread

std::vector<Symbol*>* clone_symbols(const std::vector<Symbol*>* pvecSyms,
								unsigned int iBegin=0)
{
	if(!pvecSyms)
		return 0;

	std::vector<Symbol*> *pvec = new std::vector<Symbol*>;
	pvec->reserve(pvecSyms->size());

	for(unsigned int i=iBegin; i<pvecSyms->size(); ++i)
	{
		Symbol *pSym = (*pvecSyms)[i];
		if(pSym) pSym = pSym->clone();
		pvec->push_back(pSym);
	}

	return pvec;
}

void delete_symbols(std::vector<Symbol*>* pvecSyms)
{
	for(Symbol *pSym : *pvecSyms)
		delete pSym;
	delete pvecSyms;
}

static void thread_proc(NodeFunction* pFunc, ParseInfo* pinfo, std::vector<Symbol*>* pvecSyms)
{
	if(!pFunc || !pinfo) return;

	pinfo->pmutexInterpreter->lock();
		NodeFunction* pThreadFunc = (NodeFunction*)pFunc->clone();
		pThreadFunc->SetArgSyms(pvecSyms);
		ParseInfo *pinfo2 = new ParseInfo(*pinfo);	// threads cannot share the same bWantReturn etc.
		pinfo2->bDestroyParseInfo = 0;
	pinfo->pmutexInterpreter->unlock();

	Symbol* pRet = pThreadFunc->eval(*pinfo2, 0);

	if(pRet) delete pRet;
	if(pvecSyms) delete_symbols(pvecSyms);
	if(pThreadFunc) delete pThreadFunc;
	if(pinfo2) delete pinfo2;
}

static Symbol* fkt_thread(const std::vector<Symbol*>& vecSyms,
						ParseInfo& info,
						SymbolTable* pSymTab)
{
	if(vecSyms.size()<1)
	{
		std::cerr << linenr("Error", info) << "Need thread proc identifier." << std::endl;
		return 0;
	}

	Symbol* _pSymIdent = vecSyms[0];
	if(_pSymIdent->GetType() != SYMBOL_STRING)
	{
		std::cerr << linenr("Error", info) << "Thread proc identifier needs to be a string." << std::endl;
		return 0;
	}

	SymbolString *pSymIdent = (SymbolString*)_pSymIdent;
	const std::string& strIdent = pSymIdent->m_strVal;


	NodeFunction* pFunc = info.GetFunction(strIdent);
	if(pFunc == 0)
	{
		std::cerr << linenr("Error", info) << "Thread proc \"" << strIdent << "\" not defined." << std::endl;
		return 0;
	}

	std::vector<Symbol*>* vecThreadSymsClone = clone_symbols(&vecSyms, 1);
	std::thread* pThread = new std::thread(::thread_proc, pFunc, &info, vecThreadSymsClone);
	unsigned int iHandle = info.phandles->AddHandle(new HandleThread(pThread));

	return new SymbolInt(iHandle);
}

static Symbol* fkt_thread_hwcount(const std::vector<Symbol*>& vecSyms,
						ParseInfo& info, SymbolTable* pSymTab)
{
	unsigned int iNumThreads = std::thread::hardware_concurrency();
	if(iNumThreads == 0)
		iNumThreads = 1;

	return new SymbolInt(iNumThreads);
}

static Symbol* fkt_begin_critical(const std::vector<Symbol*>& vecSyms,
								ParseInfo& info, SymbolTable* pSymTab)
{
	info.pmutexGlobal->lock();
	return 0;
}

static Symbol* fkt_end_critical(const std::vector<Symbol*>& vecSyms,
								ParseInfo& info, SymbolTable* pSymTab)
{
	info.pmutexGlobal->unlock();
	return 0;
}


static Symbol* fkt_thread_join(const std::vector<Symbol*>& vecSyms,
						ParseInfo& info, SymbolTable* pSymTab)
{
	if(vecSyms.size()<1)
	{
		std::cerr << linenr("Error", info) << "join needs at least one argument." << std::endl;
		return 0;
	}

	for(Symbol* pSym : vecSyms)
	{
		if(pSym == 0) continue;

		if(pSym->GetType() == SYMBOL_ARRAY)
		{
			return fkt_thread_join(((SymbolArray*)pSym)->m_arr, info, pSymTab);
		}

		if(pSym->GetType() != SYMBOL_INT)
		{
			std::cerr << linenr("Error", info) << "join needs thread handles." << std::endl;
			continue;
		}

		int iHandle = ((SymbolInt*)pSym)->m_iVal;
		Handle *pHandle = info.phandles->GetHandle(iHandle);

		if(pHandle==0 || pHandle->GetType()!=HANDLE_THREAD)
		{
			std::cerr << linenr("Error", info) << "Handle (" << iHandle << ") does not exist"
					 << " or is not a thread handle." << std::endl;
			continue;
		}

		HandleThread *pThreadHandle = (HandleThread*)pHandle;
		std::thread *pThread = pThreadHandle->GetInternalHandle();

		pThread->join();
	}
	return 0;
}


// --------------------------------------------------------------------------------
// nthread

// nthread(iNumThreads, strFunc, vecArgs, ...)
static Symbol* fkt_nthread(const std::vector<Symbol*>& vecSyms,
						ParseInfo& info, SymbolTable* pSymTab)
{
	if(vecSyms.size()<3)
	{
		std::cerr << linenr("Error", info) << "nthread needs at least 3 arguments: N, func, arg." << std::endl;
		return 0;
	}

	Symbol* _pSymN = vecSyms[0];
	if(_pSymN->GetType() != SYMBOL_INT)
	{
		std::cerr << linenr("Error", info) << "Number of threads has to be integer." << std::endl;
		return 0;
	}

	SymbolInt *pSymN = (SymbolInt*)_pSymN;
	int iNumThreads = pSymN->m_iVal;



	Symbol* _pSymIdent = vecSyms[1];
	if(_pSymIdent->GetType() != SYMBOL_STRING)
	{
		std::cerr << linenr("Error", info) << "Thread proc identifier needs to be a string." << std::endl;
		return 0;
	}

	SymbolString *pSymIdent = (SymbolString*)_pSymIdent;
	const std::string& strIdent = pSymIdent->m_strVal;



	Symbol* _pSymArr = vecSyms[2];
	if(_pSymArr->GetType() != SYMBOL_ARRAY)
	{
		std::cerr << linenr("Error", info) << "Thread arg has to be an array." << std::endl;
		return 0;
	}

	SymbolArray *pSymArr = (SymbolArray*)_pSymArr;
	const std::vector<Symbol*>& vecArr = pSymArr->m_arr;



	NodeFunction* pFunc = info.GetFunction(strIdent);
	if(pFunc == 0)
	{
		std::cerr << linenr("Error", info) << "Thread proc \"" << strIdent << "\" not defined." << std::endl;
		return 0;
	}





	if(iNumThreads > vecArr.size())
	{
		iNumThreads = vecArr.size();
		std::cerr << linenr("Warning", info) << "More threads requested in nthread than necessary, "
						  << "reducing to array size (" << iNumThreads << ")."
						  << std::endl;
	}


	std::vector<SymbolArray*> vecSymArrays;
	vecSymArrays.resize(iNumThreads);

	int iCurTh = 0;
	for(Symbol* pThisSym : vecArr)
	{
		if(!vecSymArrays[iCurTh])
			vecSymArrays[iCurTh] = new SymbolArray();

		vecSymArrays[iCurTh]->m_arr.push_back(pThisSym->clone());

		++iCurTh;
		if(iCurTh == iNumThreads)
			iCurTh = 0;
	}



	std::vector<std::thread*> vecThreads;
	vecThreads.reserve(iNumThreads);

	for(iCurTh=0; iCurTh<iNumThreads; ++iCurTh)
	{
		std::vector<Symbol*>* vecThreadSyms = new std::vector<Symbol*>;
		vecThreadSyms->reserve(vecSyms.size()-3+1);

		vecThreadSyms->push_back(vecSymArrays[iCurTh]);

		for(unsigned int iSym=3; iSym<vecSyms.size(); ++iSym)
			vecThreadSyms->push_back(vecSyms[iSym]->clone());

		std::thread *pth = new std::thread(::thread_proc, pFunc, &info, vecThreadSyms);
		vecThreads.push_back(pth);
	}

	/*
	// automatically join
	for(iCurTh=0; iCurTh<iNumThreads; ++iCurTh)
	{
		vecThreads[iCurTh]->join();
		delete vecThreads[iCurTh];
		vecThreads[iCurTh] = 0;
	}*/


	SymbolArray* pArrThreads = new SymbolArray();

	for(iCurTh=0; iCurTh<iNumThreads; ++iCurTh)
	{
		std::thread* pCurThread = vecThreads[iCurTh];
		unsigned int iHandle = info.phandles->AddHandle(new HandleThread(pCurThread));
		SymbolInt *pSymThreadHandle = new SymbolInt(iHandle);

		pArrThreads->m_arr.push_back(pSymThreadHandle);
	}

	return pArrThreads;
}

// --------------------------------------------------------------------------------


extern void init_ext_thread_calls()
{
	t_mapFkts mapFkts =
	{
		// threads
		t_mapFkts::value_type("thread", fkt_thread),
		t_mapFkts::value_type("nthread", fkt_nthread),
		t_mapFkts::value_type("thread_hwcount", fkt_thread_hwcount),
		t_mapFkts::value_type("join", fkt_thread_join),
		t_mapFkts::value_type("begin_critical", fkt_begin_critical),
		t_mapFkts::value_type("end_critical", fkt_end_critical),

		// processes
		t_mapFkts::value_type("exec", fkt_exec),
	};

	add_ext_calls(mapFkts);
}