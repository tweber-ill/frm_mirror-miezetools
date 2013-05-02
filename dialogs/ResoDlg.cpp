/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#include "ResoDlg.h"
#include <iostream>
#include "../settings.h"

#include "../tools/res/cn.h"

ResoDlg::ResoDlg(QWidget *pParent) : QDialog(pParent)
{
	setupUi(this);
	m_vecSpinBoxes = {spinMonod, spinMonoMosaic, spinAnad,
															spinAnaMosaic, spinSampleMosaic, spinkfix,
															spinE, spinQ, spinHCollMono, spinHCollBSample,
															spinHCollASample, spinHCollAna, spinVCollMono,
															spinVCollBSample, spinVCollASample, spinVCollAna,
															spinMonoRefl, spinAnaEffic};
	m_vecSpinNames = {"reso/mono_d", "reso/mono_mosaic", "reso/ana_d",
														"reso/ana_mosaic", "reso/sample_mosaic", "reso/k_fix",
														"reso/E", "reso/Q", "reso/h_coll_mono", "reso/h_coll_before_sample",
														"reso/h_coll_after_sample", "reso/h_coll_ana",
														"reso/v_coll_mono", "reso/v_coll_before_sample",
														"reso/v_coll_after_sample", "reso/v_coll_ana",
														"reso/mono_refl", "reso/ana_effic"};

	m_vecRadioPlus = {radioFixedki, radioMonoScatterPlus, radioAnaScatterPlus,
								radioSampleScatterPlus, radioConstMon, radioCN};
	m_vecRadioMinus = {radioFixedkf, radioMonoScatterMinus, radioAnaScatterMinus,
								radioSampleScatterMinus, radioConstTime, radioPop};
	m_vecRadioNames = {"reso/check_fixed_ki", "reso/mono_scatter_sense", "reso/ana_scatter_sense",
									"reso/sample_scatter_sense", "reso/meas_const_mon",
									"reso/algo"};


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

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	ReadLastConfig();
	Calc();
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

	cn.dmono_refl = spinMonoRefl->value();
	cn.dana_effic = spinAnaEffic->value();
	cn.bConstMon = radioConstMon->isChecked();

	bool bUseCN = radioCN->isChecked();
	// TODO

	CNResults res = calc_cn(cn);

	if(res.bOk)
	{
		std::ostringstream ostrRes;

		//ostrRes << std::scientific;
		ostrRes.precision(8);
		ostrRes << "Resolution Volume: " << res.dR0 << " meV Å^(-3)";
		ostrRes << "\n\n\n";
		ostrRes << "Resolution Matrix: \n\n";

		for(unsigned int i=0; i<res.reso.size1(); ++i)
		{
			for(unsigned int j=0; j<res.reso.size2(); ++j)
				ostrRes << std::setw(20) << res.reso(i,j);

			if(i!=res.reso.size1()-1)
				ostrRes << "\n";
		}

		labelStatus->setText("Calculation successful.");
		this->labelResult->setText(QString::fromUtf8(ostrRes.str().c_str()));
		//std::cout << "res = " << res.reso << std::endl;
		//std::cout << "vol = " << res.dR0 << std::endl;
	}
	else
	{
		QString strErr = "Error: ";
		strErr += res.strErr.c_str();
		labelStatus->setText(strErr);
	}
}

void ResoDlg::WriteLastConfig()
{
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
		Settings::Set<double>(m_vecSpinNames[iSpinBox].c_str(), m_vecSpinBoxes[iSpinBox]->value());

	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
		Settings::Set<bool>(m_vecRadioNames[iRadio].c_str(), m_vecRadioPlus[iRadio]->isChecked());
}

void ResoDlg::ReadLastConfig()
{
	for(unsigned int iSpinBox=0; iSpinBox<m_vecSpinBoxes.size(); ++iSpinBox)
	{
		if(!Settings::HasKey(m_vecSpinNames[iSpinBox].c_str()))
			continue;
		m_vecSpinBoxes[iSpinBox]->setValue(Settings::Get<double>(m_vecSpinNames[iSpinBox].c_str()));
	}

	for(unsigned int iRadio=0; iRadio<m_vecRadioPlus.size(); ++iRadio)
	{
		if(!Settings::HasKey(m_vecRadioNames[iRadio].c_str()))
			continue;

		bool bChecked = Settings::Get<bool>(m_vecRadioNames[iRadio].c_str());
		if(bChecked)
		{
			m_vecRadioPlus[iRadio]->setChecked(1);
			//m_vecRadioMinus[iRadio]->setChecked(0);;
		}
		else
		{
			//m_vecRadioPlus[iRadio]->setChecked(0);
			m_vecRadioMinus[iRadio]->setChecked(1);;
		}
	}
}

void ResoDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		WriteLastConfig();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		QDialog::accept();
	}
}

#include "ResoDlg.moc"
