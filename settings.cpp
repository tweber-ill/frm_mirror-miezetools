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

	// --------------------------------------------------------------------------------
	// General
	if(!keys.contains("general/sort_x")) s_pGlobals->setValue("general/sort_x", 1);
	if(!keys.contains("interpolation/spline_degree")) s_pGlobals->setValue("interpolation/spline_degree", 3);
	// --------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------
	// MIEZE
	if(!keys.contains("mieze/num_osc")) s_pGlobals->setValue("mieze/num_osc", 2.);
	// --------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------
	// PAD/TOF data
	if(!keys.contains("casc/foil_cnt")) s_pGlobals->setValue("casc/foil_cnt", 6);
	if(!keys.contains("casc/tc_cnt")) s_pGlobals->setValue("casc/tc_cnt", 16);
	if(!keys.contains("casc/x_res")) s_pGlobals->setValue("casc/x_res", 128);
	if(!keys.contains("casc/y_res")) s_pGlobals->setValue("casc/y_res", 128);


	// --------------------------------------------------------------------------------
	// Nicos data
	if(!keys.contains("nicos/counter_name")) s_pGlobals->setValue("nicos/counter_name", "ctr1");
	if(!keys.contains("nicos/monitor_name")) s_pGlobals->setValue("nicos/monitor_name", "mon2");


	if(!keys.contains("casc/foil_idx"))
	{
		QList<QVariant> lst = {0, 16, 32, 64, 80, 96};
		s_pGlobals->setValue("casc/foil_idx", lst);
	}
	// --------------------------------------------------------------------------------
}
