/*
 * Simple Script
 * @author tweber
 */

#ifndef __MIEZE_SCRIPT_GLOBAL__
#define __MIEZE_SCRIPT_GLOBAL__

#include "lexer.h"
#include "node.h"
#include "symbol.h"
#include <string>

struct ParseObj
{
	Lexer* pLexer;
	Node* pRoot;

	// only used during parsing/lexing for yyerror(), NOT during exec
	unsigned int iCurLine;
	std::string strCurFile;
};

#endif
