/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#include "ResoDlg.h"
#include <iostream>
#include "../settings.h"

#include "../tools/res/cn.h"
#include "../tools/res/pop.h"

ResoDlg::ResoDlg(QWidget *pParent) : QDialog(pParent)
{
	setupUi(this);
	m_vecSpinBoxes = {spinMonod, spinMonoMosaic, spinAnad,
															spinAnaMosaic, spinSampleMosaic, spinkfix,
															spinE, spinQ, spinHCollMono, spinHCollBSample,
															spinHCollASample, spinHCollAna, spinVCollMono,
															spinVCollBSample, spinVCollASample, spinVCollAna,
															spinMonoRefl, spinAnaEffic,

															spinMonoW, spinMonoH, spinMonoThick, spinMonoCurvH, spinMonoCurvV,
															spinSampleW_Q, spinSampleW_perpQ, spinSampleH,
															spinAnaW, spinAnaH, spinAnaThick, spinAnaCurvH, spinAnaCurvV,
															spinSrcW, spinSrcH,
															spinGuideDivH, spinGuideDivV,
															spinDetW, spinDetH,
															spinDistMonoSample, spinDistSampleAna, spinDistAnaDet, spinDistSrcMono};

	m_vecSpinNames = {"reso/mono_d", "reso/mono_mosaic", "reso/ana_d",
										"reso/ana_mosaic", "reso/sample_mosaic", "reso/k_fix",
										"reso/E", "reso/Q", "reso/h_coll_mono", "reso/h_coll_before_sample",
										"reso/h_coll_after_sample", "reso/h_coll_ana",
										"reso/v_coll_mono", "reso/v_coll_before_sample",
										"reso/v_coll_after_sample", "reso/v_coll_ana",
										"reso/mono_refl", "reso/ana_effic",

										"reso/pop_mono_w", "reso/pop_mono_h", "reso/pop_mono_thick", "reso/pop_mono_curvh", "reso/pop_mono_curvv",
										"reso/pop_sample_wq", "reso/pop_sampe_wperpq", "reso/pop_sample_h",
										"reso/pop_ana_w", "reso/pop_ana_h", "reso/pop_ana_thick", "reso/pop_ana_curvh", "reso/pop_ana_curvv",
										"reso/pop_src_w", "reso/pop_src_h",
										"reso/pop_guide_divh", "reso/pop_guide_divv",
										"reso/pop_det_w", "reso/pop_det_h",
										"reso/pop_dist_mono_sample", "reso/pop_dist_sample_ana", "reso/pop_dist_ana_det", "reso/pop_dist_src_mono"};

	m_vecRadioPlus = {radioFixedki, radioMonoScatterPlus, radioAnaScatterPlus,
								radioSampleScatterPlus, radioConstMon, radioCN,
								radioSampleCub, radioSrcRect, radioDetRect};
	m_vecRadioMinus = {radioFixedkf, radioMonoScatterMinus, radioAnaScatterMinus,
								radioSampleScatterMinus, radioConstTime, radioPop,
								radioSampleCyl, radioSrcCirc, radioDetCirc};
	m_vecRadioNames = {"reso/check_fixed_ki", "reso/mono_scatter_sense", "reso/ana_scatter_sense",
									"reso/sample_scatter_sense", "reso/meas_const_mon",
									"reso/algo",
									"reso/pop_sample_cuboid", "reso/pop_source_rect", "reso/pop_det_rect"};


	UpdateUI();
	QObject::connect(radioFixedki, SIGNAL(toggled(bool)), this, SLOT(UpdateUI()));

	for(QDoubleSpinBox* pSpinBox : m_vecSpinBoxes)
		QObject::connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	for(QRadioButton* pRadio : m_vecRadioPlus)
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(Calc()));

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

	PopParams cn;

	// CN
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

	// Pop
	cn.mono_w = spinMonoW->value()*0.01*units::si::meter;
	cn.mono_h = spinMonoH->value()*0.01*units::si::meter;
	cn.mono_thick = spinMonoThick->value()*0.01*units::si::meter;
	cn.mono_curvh = spinMonoCurvH->value()*0.01*units::si::meter;
	cn.mono_curvv = spinMonoCurvV->value()*0.01*units::si::meter;
	cn.bMonoIsCurvedH = checkMonoCurvH->isChecked();
	cn.bMonoIsCurvedV = checkMonoCurvV->isChecked();

	cn.ana_w = spinAnaW->value()*0.01*units::si::meter;
	cn.ana_h = spinAnaH->value()*0.01*units::si::meter;
	cn.ana_thick = spinAnaThick->value()*0.01*units::si::meter;
	cn.ana_curvh = spinAnaCurvH->value()*0.01*units::si::meter;
	cn.ana_curvv = spinAnaCurvV->value()*0.01*units::si::meter;
	cn.bAnaIsCurvedH = checkAnaCurvH->isChecked();
	cn.bAnaIsCurvedV = checkAnaCurvV->isChecked();

	cn.bSampleCub = radioSampleCub->isChecked();
	cn.sample_w_q = spinSampleW_Q->value()*0.01*units::si::meter;
	cn.sample_w_perpq = spinSampleW_perpQ->value()*0.01*units::si::meter;
	cn.sample_h = spinSampleH->value()*0.01*units::si::meter;

	cn.bSrcRect = radioSrcRect->isChecked();
	cn.src_w = spinSrcW->value()*0.01*units::si::meter;
	cn.src_h = spinSrcH->value()*0.01*units::si::meter;

	cn.bDetRect = radioDetRect->isChecked();
	cn.det_w = spinDetW->value()*0.01*units::si::meter;
	cn.det_h = spinDetH->value()*0.01*units::si::meter;

	cn.bGuide = groupGuide->isChecked();
	cn.guide_div_h = spinGuideDivH->value() / (180.*60.) * M_PI * units::si::radians;
	cn.guide_div_v = spinGuideDivV->value() / (180.*60.) * M_PI * units::si::radians;

	cn.dist_mono_sample = spinDistMonoSample->value()*0.01*units::si::meter;
	cn.dist_sample_ana = spinDistSampleAna->value()*0.01*units::si::meter;
	cn.dist_ana_det = spinDistAnaDet->value()*0.01*units::si::meter;
	cn.dist_src_mono = spinDistSrcMono->value()*0.01*units::si::meter;

	const bool bUseCN = radioCN->isChecked();
	CNResults res = (bUseCN ? calc_cn(cn) : calc_pop(cn));

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
