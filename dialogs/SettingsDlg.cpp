/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 03-apr-2013
 * @license GPLv3
 */

#include "SettingsDlg.h"
#include "main/settings.h"
#include "tlibs/string/string.h"
#include "tlibs/log/log.h"
#include "fitter/parser.h"

#include <iostream>
#include <sstream>

SettingsDlg::SettingsDlg(QWidget* pParent) : QDialog(pParent)
{
	this->setupUi(this);
	LoadSettings();
}

SettingsDlg::~SettingsDlg()
{}

void SettingsDlg::accept()
{
	SaveSettings();
	set_global_defaults();

	QDialog::accept();
}

void SettingsDlg::set_global_defaults()
{
	int iDebugLevel = Settings::Get<int>("misc/debug_level");

	tl::log_debug.SetEnabled(iDebugLevel>=4);
	tl::log_info.SetEnabled(iDebugLevel>=3);
	tl::log_warn.SetEnabled(iDebugLevel>=2);
	tl::log_err.SetEnabled(iDebugLevel>=1);
	tl::log_crit.SetEnabled(1);
}


void SettingsDlg::LoadSettings()
{
	// --------------------------------------------------------------------------------
	// General
	checkSortX->setChecked(Settings::Get<int>("misc/sort_x"));
	spinSpline->setValue(Settings::Get<int>("interpolation/spline_degree"));
	spinDebug->setValue(Settings::Get<int>("misc/debug_level"));
	spinMinCts->setValue(Settings::Get<int>("misc/min_counts"));
	// --------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------
	// MIEZE
	spinNumOsc->setValue(Settings::Get<double>("mieze/num_osc"));
	// --------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------
	// TOF/PAD
	spinResX->setValue(Settings::Get<unsigned int>("casc/x_res"));
	spinResY->setValue(Settings::Get<unsigned int>("casc/y_res"));
	spinResT->setValue(Settings::Get<unsigned int>("casc/tc_cnt"));
	spinResFoils->setValue(Settings::Get<unsigned int>("casc/foil_cnt"));

	QList<QVariant> lst = Settings::Get<QList<QVariant> >("casc/foil_idx");

	std::ostringstream ostr;
	for(int i=0; i<lst.size(); ++i)
	{
		ostr << lst[i].toUInt();
		if(i!=lst.size()-1)
			ostr << ", ";
	}
	editStartIndices->setText(ostr.str().c_str());
	// --------------------------------------------------------------------------------

	// --------------------------------------------------------------------------------
	// Nicos data
	editCounts->setText(Settings::Get<QString>("nicos/counter_name"));
	editMon->setText(Settings::Get<QString>("nicos/monitor_name"));
	// --------------------------------------------------------------------------------
}

void SettingsDlg::SaveSettings()
{
	// --------------------------------------------------------------------------------
	// General
	Settings::Set<int>("misc/sort_x", checkSortX->isChecked());
	Settings::Set<int>("interpolation/spline_degree", spinSpline->value());
	Settings::Set<int>("misc/debug_level", spinDebug->value());
	Settings::Set<int>("misc/min_counts", spinMinCts->value());
	// --------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------
	// MIEZE
	Settings::Set<double>("mieze/num_osc", spinNumOsc->value());
	// --------------------------------------------------------------------------------


	// --------------------------------------------------------------------------------
	// TOF/PAD data
	Settings::Set<unsigned int>("casc/x_res", spinResX->value());
	Settings::Set<unsigned int>("casc/y_res", spinResY->value());
	Settings::Set<unsigned int>("casc/tc_cnt", spinResT->value());
	Settings::Set<unsigned int>("casc/foil_cnt", spinResFoils->value());

	std::string strStartIdx = editStartIndices->text().toStdString();
	std::vector<unsigned int> vecIndices;
	tl::get_tokens<unsigned int>(strStartIdx, std::string(",; "), vecIndices);

	QList<QVariant> lst;
	for(unsigned int iIdx : vecIndices)
		lst.push_back(iIdx);
	Settings::Set<QList<QVariant> >("casc/foil_idx", lst);
	// --------------------------------------------------------------------------------

	// --------------------------------------------------------------------------------
	// Nicos data
	Settings::Set<QString>("nicos/counter_name", editCounts->text());
	Settings::Set<QString>("nicos/monitor_name", editMon->text());
	// --------------------------------------------------------------------------------
}


#include "SettingsDlg.moc"
