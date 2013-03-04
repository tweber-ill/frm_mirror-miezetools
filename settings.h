/*
 * mieze-tool
 * global settings
 * @author tweber
 * @date 04-mar-2013
 */

#ifndef __MIEZE_SETTINGS__
#define __MIEZE_SETTINGS__

#include <QtCore/QSettings>

class Settings
{
	protected:
		static QSettings *s_pGlobals;

	public:
		static QSettings *GetGlobals();
		static void free();
};

#endif
