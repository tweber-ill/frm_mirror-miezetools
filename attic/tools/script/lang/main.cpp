/*
 * Simple Script
 * @author tweber
 * @date 2013
 */

#include "types.h"

#include "../helper/flags.h"
#include "../helper/string.h"
#include "../helper/spec_char.h"
#include "../helper/log.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <algorithm>
#include <boost/version.hpp>

#include "script_helper.h"
#include "globals.h"
#include "calls.h"
#include "info.h"

extern int yyparse(void*);
static bool g_bShowTiming = 0;


static inline void usage(const char* pcProg)
{
	G_COUT << "This is the " << g_pcVersion << ".\n";
	G_COUT << "Written by Tobias Weber, 2013-2014.\n";
	G_COUT << "Built on " << __DATE__ << ", " << __TIME__;
	//G_COUT << " with CC version " << __VERSION__ << ".\n\n";
	G_COUT << " with " << BOOST_COMPILER << ".\n\n";
	G_COUT << "Usage: " << pcProg << " [arguments to hermelin]"
		<< " <script file> [arguments to script]"
		<< "\n";

	G_COUT << "\n" <<
	R"RAW(Arguments to hermelin:
	-h, --help            This message.
	-i, --interactive     Interactive mode.
	-t, --timing          Show timing information.
	-s, --symbols         Show symbol tables.
	-d[0-4]               Verbosity (0=none, 1=errors, 2=warnings, 3=infos, 4=debug).
	)RAW";

	G_COUT << std::endl;
}



// execute interactive commands
static inline int interactive(bool bShowSymbols=0, unsigned int uiDebugLevel=3)
{
	static const t_char* pcCmdFunc = T_STR"__cmd__";

	ParseInfo info;
	RuntimeInfo runinfo;

	std::unique_ptr<SymbolTable> ptrLocalSym(new SymbolTable);
	runinfo.pLocalSymsOverride = ptrLocalSym.get();
	runinfo.bImplicitRet = 1;
	runinfo.strExecFkt = pcCmdFunc;
	runinfo.strInitScrFile = "<interactive>";

	init_global_syms(info.pGlobalSyms);

	std::function<void()> remove_cmdfunc = [&info]()
	{
		ParseInfo::t_funcs::iterator iterNewEnd =
			std::remove_if(info.vecFuncs.begin(), info.vecFuncs.end(),
			[](const NodeFunction* pFunc)->bool
			{ return pFunc->GetName() == pcCmdFunc; } );
		info.vecFuncs.resize(iterNewEnd-info.vecFuncs.begin());
	};

	while(1)
	{
		try
		{
			std::cout << "> ";
			std::string strLine;
			if(!std::getline(std::cin, strLine))
				break;

			runinfo.EnableExec();

			t_string strInput = t_string(pcCmdFunc) + "() { " + strLine + " }";
			Lexer lex(strInput);

			if(!lex.IsOk())
			{
				log_err("Lexer returned with errors.");
				continue;
			}


			ParseObj par;
			info.bEnableDebug = (uiDebugLevel>=4);
			par.pLexer = &lex;

			int iParseRet = yyparse(&par);
			if(iParseRet != 0)
			{
				log_err("Parser returned with error code ", iParseRet, ".");
				remove_cmdfunc();
				continue;
			}

			par.pRoot = par.pRoot->optimize();
			Symbol *pSymRet = par.pRoot->eval(info, runinfo, 0);

			if(bShowSymbols)
				runinfo.pLocalSymsOverride->print();

			if(pSymRet)
			{
				std::cout << pSymRet->print() << std::endl;
				safe_delete(pSymRet, runinfo.pLocalSymsOverride, &info);
			}

			remove_cmdfunc();

			if(par.pRoot)
			{
				delete par.pRoot;
				par.pRoot = 0;
			}
		}
		catch(const std::exception& ex)
		{
			log_crit(ex.what());
			remove_cmdfunc();
		}
	}

	return 0;
}



// execute script
static inline int script_main(int argc, char** argv)
{
	if(argc<=1)
	{
		usage(argv[0]);
		return -1;
	}

	bool bShowSymbols = 0;
	bool bInteractive = 0;
	unsigned int uiDebugLevel = 3;
#ifndef NDEBUG
	uiDebugLevel = 4;
#endif
	unsigned int iStartArg = 1;
	for(iStartArg=1; iStartArg<unsigned(argc); ++iStartArg)
	{
		t_string strArg = STR_TO_WSTR(argv[iStartArg]);
		trim(strArg);

		// end of arguments to hermelin
		if(strArg[0] != T_STR'-')
			break;

		if(strArg=="-t" || strArg == "--timing")
			g_bShowTiming = 1;
		else if(strArg=="-s" || strArg == "--symbols")
			bShowSymbols = 1;
		else if(strArg=="-i" || strArg == "--interactive")
			bInteractive = 1;
		else if(strArg=="-h" || strArg == "--help")
			{ usage(argv[0]); return 0; }

		else if(strArg=="-d0") uiDebugLevel = 0;
		else if(strArg=="-d1") uiDebugLevel = 1;
		else if(strArg=="-d2") uiDebugLevel = 2;
		else if(strArg=="-d3") uiDebugLevel = 3;
		else if(strArg=="-d4") uiDebugLevel = 4;
	}

	const std::array<Log*, 5> arrLogs{&log_crit, &log_err, &log_warn, &log_info, &log_debug};
	for(unsigned int iLog=0; iLog<arrLogs.size(); ++iLog)
		arrLogs[iLog]->SetEnabled(uiDebugLevel>=iLog);

	// debug in script.yy needs to be set
	yydebug = (uiDebugLevel>=4);

	if(bInteractive)
		return interactive(bShowSymbols, uiDebugLevel);


	if(iStartArg >= unsigned(argc))
	{
		log_err("No input file given.");
		return -1;
	}



	// loading of input file
	const char* pcFile = argv[iStartArg];
	t_string strFile = STR_TO_WSTR(pcFile);

	t_char* pcInput = load_file(pcFile);
	if(!pcInput)
		return -2;


	ParseObj par;
	ParseInfo info;
	RuntimeInfo runinfo;

	info.bEnableDebug = (uiDebugLevel>=4);


	// lexing
	par.strCurFile = strFile;
	par.pLexer = new Lexer(pcInput, strFile.c_str());

	delete[] pcInput;
	pcInput = 0;

	if(!par.pLexer->IsOk())
	{
		log_err("Lexer returned with errors.");
		return -3;
	}

	init_global_syms(info.pGlobalSyms);


	// parsing
	int iParseRet = yyparse(&par);

	delete par.pLexer;
	par.pLexer = 0;

	if(iParseRet != 0)
	{
		log_err("Parser returned with error code ", iParseRet, ".");
		return -4;
	}


	// optimizing
	par.pRoot = par.pRoot->optimize();



	// executing
	SymbolArray *parrMainArgs = new SymbolArray();
	for(int iArg=iStartArg; iArg<argc; ++iArg)
	{
		SymbolString *pSymArg = new SymbolString();
		pSymArg->SetVal(STR_TO_WSTR(argv[iArg]));
		parrMainArgs->GetArr().push_back(pSymArg);
	}
	//std::vector<Symbol*> vecMainArgs = { &arrMainArgs };

	SymbolTable *pTableSup = new SymbolTable();

	info.pmapModules->insert(ParseInfo::t_mods::value_type(strFile, par.pRoot));
	runinfo.strExecFkt = T_STR"main";
	//info.pvecExecArg = &vecMainArgs;
	runinfo.strInitScrFile = strFile;

	SymbolArray arrMainArgs;
	arrMainArgs.GetArr().push_back(parrMainArgs);
	pTableSup->InsertSymbol(T_STR"<args>", &arrMainArgs);
	par.pRoot->eval(info, runinfo, pTableSup);
	pTableSup->RemoveSymbolNoDelete(T_STR"<args>");
	delete pTableSup;


	if(bShowSymbols)
	{
		log_info("================================================================================");
		log_info("Global symbols:");
		info.pGlobalSyms->print();

		std::ostringstream ostrFkts;
		for(const NodeFunction* pFunc : info.vecFuncs)
			ostrFkts << pFunc->GetName() << ", ";
		log_info("Script functions: ", ostrFkts.str());


		const t_mapFkts* pExtFkts = get_ext_calls();

		std::ostringstream ostrSysFkts;
		for(const auto& fktpair : *pExtFkts)
			ostrSysFkts << fktpair.first << ", ";
		log_info("System functions: ", ostrSysFkts.str());
		log_info("================================================================================");
	}

	return 0;
}



#include <chrono>
#include <ctime>
typedef std::chrono::system_clock::time_point t_tp_sys;
typedef std::chrono::steady_clock::time_point t_tp_st;
typedef std::chrono::duration<double> t_dur;
typedef std::chrono::system_clock::duration t_dur_sys;

int main(int argc, char** argv)
{
	const std::array<Log*, 5> arrLogs{&log_crit, &log_err, &log_warn, &log_info, &log_debug};
	for(Log* pLog : arrLogs)
	{
		pLog->SetShowDate(0);
		pLog->SetShowThread(0);
	}

	int iRet = -99;
	t_tp_sys timeStart = std::chrono::system_clock::now();
	t_tp_st timeStart_st = std::chrono::steady_clock::now();

	try
	{
		init_spec_chars();
		iRet = script_main(argc, argv);
	}
	catch(const std::exception& ex)
	{
		log_crit(ex.what());
	}

	if(g_bShowTiming && iRet==0)
	{
		t_tp_st timeStop_st = std::chrono::steady_clock::now();

		t_dur dur = std::chrono::duration_cast<t_dur>(timeStop_st-timeStart_st);
		t_dur_sys dur_sys = std::chrono::duration_cast<t_dur_sys>(dur);
		double dDur = double(t_dur::period::num)/double(t_dur::period::den) * double(dur.count());

		std::time_t tStart = std::chrono::system_clock::to_time_t(timeStart);
		std::time_t tStop = std::chrono::system_clock::to_time_t(timeStart + dur_sys);

		std::tm tmStart = *std::localtime(&tStart);
		std::tm tmStop = *std::localtime(&tStop);

		char cStart[128], cStop[128];
		std::strftime(cStart, sizeof cStart, "%a %Y-%b-%d %H:%M:%S %Z", &tmStart);
		std::strftime(cStop, sizeof cStop, "%a %Y-%b-%d %H:%M:%S %Z", &tmStop);

		log_info("================================================================================");
		log_info("Script start time:     ", cStart);
		log_info("Script stop time:      ", cStop);
		log_info("Script execution time: ", dDur, " s");
		log_info("================================================================================");
	}

	return iRet;
}