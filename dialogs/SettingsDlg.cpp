/*
 * mieze-tool
 * @author tweber
 * @date 03-apr-2013
 */

#include "SettingsDlg.h"
#include "../settings.h"
#include "../helper/string.h"

#include <iostream>
#include <sstream>

SettingsDlg::SettingsDlg(QWidget* pParent) : QDialog(pParent)
{
	this->setupUi(this);
	LoadSettings();
}

SettingsDlg::~SettingsDlg()
{
}

void SettingsDlg::accept()
{
	SaveSettings();
	QDialog::accept();
}


void SettingsDlg::LoadSettings()
{
	// --------------------------------------------------------------------------------
	// General
	checkSortX->setChecked(Settings::Get<int>("general/sort_x"));
	spinSpline->setValue(Settings::Get<int>("interpolation/spline_degree"));
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
	Settings::Set<int>("general/sort_x", checkSortX->isChecked());
	Settings::Set<int>("interpolation/spline_degree", spinSpline->value());
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
	::get_tokens<unsigned int>(strStartIdx, ",; ", vecIndices);

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
