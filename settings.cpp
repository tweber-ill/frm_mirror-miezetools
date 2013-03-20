/*
 * mieze-tool
 * global settings
 * @author tweber
 * @date 04-mar-2013
 */

#include "settings.h"
#include <QtCore/QList>
#include <QtCore/QStringList>

QSettings * Settings::s_pGlobals = 0;

QSettings *Settings::GetGlobals()
{
	if(!s_pGlobals)
	{
		s_pGlobals = new QSettings("tobis_stuff", "mieze");
		SetDefaults();
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

void Settings::SetDefaults()
{
	if(!s_pGlobals) return;

	QStringList keys = s_pGlobals->allKeys();
	if(!keys.contains("casc/foil_cnt")) s_pGlobals->setValue("casc/foil_cnt", 6);
	if(!keys.contains("casc/tc_cnt")) s_pGlobals->setValue("casc/tc_cnt", 16);
	if(!keys.contains("casc/x_res")) s_pGlobals->setValue("casc/x_res", 128);
	if(!keys.contains("casc/y_res")) s_pGlobals->setValue("casc/y_res", 128);

	if(!keys.contains("casc/foil_idx"))
	{
		QList<QVariant> lst = {0, 16, 32, 64, 80, 96};
		s_pGlobals->setValue("casc/foil_idx", lst);
	}
}
