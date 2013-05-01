/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#include "ResoDlg.h"
#include <iostream>

#include "../tools/res/cn.h"

ResoDlg::ResoDlg(QWidget *pParent) : QDialog(pParent)
{
	setupUi(this);
	UpdateUI();

	QObject::connect(radioFixedki, SIGNAL(toggled(bool)), this, SLOT(UpdateUI()));

	QObject::connect(spinMonod, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinMonoMosaic, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinAnad, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinAnaMosaic, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinSampleMosaic, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinkfix, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinQ, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinE, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinHCollMono, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinHCollBSample, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinHCollASample, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinHCollAna, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinVCollMono, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinVCollBSample, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinVCollASample, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	QObject::connect(spinVCollAna, SIGNAL(valueChanged(double)), this, SLOT(Calc()));

	QObject::connect(radioMonoScatterPlus, SIGNAL(toggled(bool)), this, SLOT(Calc()));
	QObject::connect(radioAnaScatterPlus, SIGNAL(toggled(bool)), this, SLOT(Calc()));
	QObject::connect(radioSampleScatterPlus, SIGNAL(toggled(bool)), this, SLOT(Calc()));
	QObject::connect(radioFixedki, SIGNAL(toggled(bool)), this, SLOT(Calc()));
}

ResoDlg::~ResoDlg()
{}


void ResoDlg::UpdateUI()
{
	if(radioFixedki->isChecked())
		labelkfix->setText(QString::fromUtf8("k_i (1/Å):"));
	else
		labelkfix->setText(QString::fromUtf8("k_f (1/Å):"));
}

void ResoDlg::Calc()
{
	const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;

	CNParams cn;
	cn.mono_d = spinMonod->value() * angstrom;
	cn.mono_mosaic = spinMonoMosaic->value() / (180.*60.) * M_PI * units::si::radians;
	cn.ana_d = spinAnad->value() * angstrom;
	cn.ana_mosaic = spinAnaMosaic->value() / (180.*60.) * M_PI * units::si::radians;
	cn.sample_mosaic = spinSampleMosaic->value() / (180.*60.) * M_PI * units::si::radians;

	cn.bki_fix = radioFixedki->isChecked();
	cn.ki = cn.kf = spinkfix->value() / angstrom;
	cn.E = spinE->value() * (1e-3 * codata::e * units::si::volts);
	cn.Q = spinQ->value() / angstrom;

	cn.dmono_sense = (radioMonoScatterPlus->isChecked() ? +1. : -1.);
	cn.dana_sense = (radioAnaScatterPlus->isChecked() ? +1. : -1.);
	cn.dsample_sense = (radioSampleScatterPlus->isChecked() ? +1. : -1.);

	cn.coll_h_pre_mono = spinHCollMono->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_pre_sample = spinHCollBSample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_post_sample = spinHCollASample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_h_post_ana = spinHCollAna->value() / (180.*60.) * M_PI * units::si::radians;

	cn.coll_v_pre_mono = spinVCollMono->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_pre_sample = spinVCollBSample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_post_sample = spinVCollASample->value() / (180.*60.) * M_PI * units::si::radians;
	cn.coll_v_post_ana = spinVCollAna->value() / (180.*60.) * M_PI * units::si::radians;

	CNResults res = calc_cn(cn);

	if(res.bOk)
	{
		std::cout << res.reso << std::endl;
	}
	else
	{
		std::cerr << "Error: " << res.strErr << std::endl;
	}
}


#include "ResoDlg.moc"
