/*
 * mieze-tool
 * @author tweber
 * @date 08-jul-2013
 */

#include "FormulaDlg.h"
#include <boost/units/io.hpp>
#include "../helper/mieze.hpp"
#include "../settings.h"

#include <sstream>
#include <iostream>

FormulaDlg::FormulaDlg(QWidget* pParent) : QDialog(pParent)
{
	setupUi(this);
	m_vecSpins = {spinLam, spinF1, spinF2, spinL1, spinLb, spinE};
	m_vecSpinNames = {"formulas/mieze/lam", "formulas/mieze/f1", "formulas/mieze/f2", "formulas/mieze/L1", "formulas/mieze/Lb", "formulas/mieze/E"};


	for(unsigned int iItem=0; iItem<m_vecSpins.size(); ++iItem)
	{
		const std::string& strName = m_vecSpinNames[iItem];
		QDoubleSpinBox* pSpin = m_vecSpins[iItem];

		if(!Settings::HasKey(strName.c_str()))
			continue;
		pSpin->setValue(Settings::Get<double>(strName.c_str()));
	}


	for(QDoubleSpinBox* pSpinBox : m_vecSpins)
		QObject::connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(Calc()));

	Calc();
}

FormulaDlg::~FormulaDlg()
{
}


void FormulaDlg::Calc()
{
	#define one_meV (1e-3 * co::e * units::si::volts)
	static const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;
	static const units::quantity<units::si::length> cm = units::si::meter/100.;

	units::quantity<units::si::length> lam = spinLam->value() * angstrom;
	units::quantity<units::si::length> L1 = spinL1->value() * cm;
	units::quantity<units::si::length> Lb = spinLb->value() * cm;
	units::quantity<units::si::frequency> f1 = spinF1->value() * units::si::hertz;
	units::quantity<units::si::frequency> f2 = spinF2->value() * units::si::hertz;
	units::quantity<units::si::energy> E = spinE->value() * one_meV;

	units::quantity<units::si::length> L2 = mieze_condition_L2(f1, f2, L1);
	units::quantity<units::si::length> Ls = L2-Lb;
	units::quantity<units::si::time> tau = mieze_tau(2.*(f2-f1), Ls, lam);

	units::quantity<units::si::length> Ls_inel = mieze_condition_inel_Ls(Ls, E, lam);
	units::quantity<units::si::length> offs = Ls_inel-Ls;

	units::quantity<units::si::frequency> df1 = mieze_det_misaligned_df1(L1, L2, -offs, f1, f2);
	units::quantity<units::si::frequency> df2 = mieze_det_misaligned_df2(L1, L2, -offs, f1, f2);

	std::ostringstream ostrResult;
	ostrResult << "(quasi-)elastic MIEZE\n" << "----------------------------------------\n";
	ostrResult << "L2 = " << L2 << "\n";
	ostrResult << "Ls = " << Ls << "\n";
	ostrResult << "tau = " << tau << "\n";
	ostrResult << "\n";

	ostrResult << "inelastic MIEZE\n";
	ostrResult << "----------------------------------------\n";
	ostrResult << "L2_inel = " << Ls_inel + Lb << "\n";
	ostrResult << "Ls_inel = " << Ls_inel << " (offset = " << offs << ")" << "\n";
	ostrResult << "\n";

	ostrResult << "inelastic MIEZE (alternatively)\n";
	ostrResult << "----------------------------------------\n";
	ostrResult << "f1_inel = " << f1 + df1 << " (offset = " << df1 << ")" << " or\n";
	ostrResult << "f2_inel = " << f2 + df2 << " (offset = " << df2 << ")" << "\n";

	labelResult->setText(ostrResult.str().c_str());
}

void FormulaDlg::accept()
{
	for(unsigned int iItem=0; iItem<m_vecSpins.size(); ++iItem)
	{
		const std::string& strName = m_vecSpinNames[iItem];
		const QDoubleSpinBox* pSpin = m_vecSpins[iItem];

		Settings::Set<double>(strName.c_str(), pSpin->value());
	}

	QDialog::accept();
}

#include "FormulaDlg.moc"
