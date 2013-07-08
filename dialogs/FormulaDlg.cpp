/*
 * mieze-tool
 * @author tweber
 * @date 08-jul-2013
 */

#include "FormulaDlg.h"
#include <boost/units/io.hpp>
#include "../helper/mieze.hpp"

#include <vector>
#include <sstream>
#include <iostream>

FormulaDlg::FormulaDlg(QWidget* pParent) : QDialog(pParent)
{
	setupUi(this);
	std::vector<QDoubleSpinBox*> vecSpins = {spinLam, spinF1, spinF2, spinL1, spinLb, spinE};

	for(QDoubleSpinBox* pSpinBox : vecSpins)
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


	std::ostringstream ostrResult;
	ostrResult << "L2 = " << L2 << "\n";
	ostrResult << "Ls = " << Ls << "\n";
	ostrResult << "tau = " << tau << "\n";
	ostrResult << "\n";
	ostrResult << "Ls_inel = " << Ls_inel << " (offset = " << Ls_inel-Ls <<")" << "\n";

	labelResult->setText(ostrResult.str().c_str());
}


#include "FormulaDlg.moc"
