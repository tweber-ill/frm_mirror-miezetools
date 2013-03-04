/*
 * mieze-tool
 * global settings
 * @author tweber
 * @date 04-mar-2013
 */

#include "settings.h"

QSettings * Settings::s_pGlobals = 0;

QSettings *Settings::GetGlobals()
{
	if(!s_pGlobals)
	{
		s_pGlobals = new QSettings("tobis_stuff", "mieze");
	}

	return s_pGlobals;
}


void Settings::free()
{
	if(s_pGlobals)
	{
		delete s_pGlobals;
		s_pGlobals = 0;
	}
}
