/*
 * mieze-tool
 * @author tweber
 * @date 08-jul-2013
 */

#include "FormulaDlg.h"
#include <boost/units/io.hpp>
#include "../helper/mieze.hpp"
#include "../helper/string.h"
#include "../settings.h"

#include <sstream>
#include <iostream>

FormulaDlg::FormulaDlg(QWidget* pParent) : QDialog(pParent)
{
	setupUi(this);
	setupConstants();

	QObject::connect(editNeutronLam, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronLam()));
	QObject::connect(editNeutronE, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronE()));
	QObject::connect(editNeutronV, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronv()));
	QObject::connect(editNeutronK, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronk()));
	QObject::connect(editNeutronT, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronT()));


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
		QObject::connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(CalcMIEZE()));

	CalcNeutronLam();
	CalcMIEZE();
}

FormulaDlg::~FormulaDlg()
{}

void FormulaDlg::setupConstants()
{
	struct Constant
	{
		std::string strSymbol;
		std::string strName;

		std::string strVal;
	};

	std::vector<Constant> vecConsts;

	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << one_eV;

		Constant constant;
		constant.strSymbol = "eV";
		constant.strName = "1 electron volt";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::h;

		Constant constant;
		constant.strSymbol = "h";
		constant.strName = "Planck constant";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << (co::h / one_eV) << " eV";

		Constant constant;
		constant.strSymbol = "h";
		constant.strName = "Planck constant";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::hbar;

		Constant constant;
		constant.strSymbol = "hbar";
		constant.strName = "Planck constant";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << (co::hbar / one_eV) << " eV";

		Constant constant;
		constant.strSymbol = "hbar";
		constant.strName = "Planck constant";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::m_n;

		Constant constant;
		constant.strSymbol = "m_n";
		constant.strName = "Neutron mass";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::g_n;

		Constant constant;
		constant.strSymbol = "g_n";
		constant.strName = "Neutron g";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::gamma_n;

		Constant constant;
		constant.strSymbol = "gamma_n";
		constant.strName = "Neutron gyromagnetic ratio";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	/*{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::gamma_n/(2.*M_PI);

		Constant constant;
		constant.strSymbol = "gamma_n/(2pi)";
		constant.strName = "";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}*/
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::mu_n;

		Constant constant;
		constant.strSymbol = "mu_n";
		constant.strName = "Neutron magnetic moment";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::mu_N;

		Constant constant;
		constant.strSymbol = "mu_N";
		constant.strName = "Nuclear magneton";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::c;

		Constant constant;
		constant.strSymbol = "c";
		constant.strName = "Vacuum speed of light";
		constant.strVal = insert_before(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}


	tableConst->setColumnCount(2);
	tableConst->setRowCount(vecConsts.size());
	tableConst->setColumnWidth(1, 200);
	//tableConst->verticalHeader()->setDefaultSectionSize(tableConst->verticalHeader()->minimumSectionSize()+2);


	for(unsigned int iConst=0; iConst<vecConsts.size(); ++iConst)
	{
		const Constant& constant = vecConsts[iConst];

		QTableWidgetItem *pConstSym = new QTableWidgetItem();
		pConstSym->setText(constant.strSymbol.c_str());
		tableConst->setVerticalHeaderItem(iConst, pConstSym);

		QTableWidgetItem *pConstName = new QTableWidgetItem();
		pConstName->setText(constant.strName.c_str());
		tableConst->setItem(iConst,0,pConstName);

		QTableWidgetItem *pConstVal = new QTableWidgetItem();
		pConstVal->setText(constant.strVal.c_str());
		tableConst->setItem(iConst,1,pConstVal);
	}
}

static bool check_input(const std::string& strInput)
{
	double dVal = 0.;
	std::string strUnit;
	return get_val(strInput, dVal, strUnit);
}

void FormulaDlg::CalcNeutronLam()
{
	std::string strInput = editNeutronLam->text().toStdString();
	if(!check_input(strInput))
		return;

	units::quantity<units::si::length> lam_n = get_length(strInput);
	units::quantity<units::si::wavenumber> k_n = lam2k(lam_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);
	units::quantity<units::si::energy> E_n = p_n*p_n / (2.*co::m_n);

	std::ostringstream ostrk, ostrv, ostrE, ostrT;
	ostrk << k_n;
	ostrv << (p_n / co::m_n);
	ostrE << double(E_n / one_meV) << " meV";
	ostrT << (E_n / co::k_B);

	editNeutronE->setText(ostrE.str().c_str());
	editNeutronK->setText(ostrk.str().c_str());
	editNeutronV->setText(ostrv.str().c_str());
	editNeutronT->setText(ostrT.str().c_str());
}

void FormulaDlg::CalcNeutronk()
{
	std::string strInput = editNeutronK->text().toStdString();
	if(!check_input(strInput))
		return;

	units::quantity<units::si::wavenumber> k_n = get_wavenumber(strInput);
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);
	units::quantity<units::si::energy> E_n = p_n*p_n / (2.*co::m_n);

	std::ostringstream ostrlam, ostrv, ostrE, ostrT;
	ostrlam << lam_n;
	ostrv << (p_n / co::m_n);
	ostrE << double(E_n / one_meV) << " meV";
	ostrT << (E_n / co::k_B);

	editNeutronLam->setText(ostrlam.str().c_str());
	editNeutronE->setText(ostrE.str().c_str());
	editNeutronV->setText(ostrv.str().c_str());
	editNeutronT->setText(ostrT.str().c_str());
}

void FormulaDlg::CalcNeutronv()
{
	std::string strInput = editNeutronV->text().toStdString();
	if(!check_input(strInput))
		return;

	std::cerr << "Error: Velocity parsing not yet implemented." << std::endl;
}

void FormulaDlg::CalcNeutronE()
{
	std::string strInput = editNeutronE->text().toStdString();
	if(!check_input(strInput))
		return;

	bool bImag = 0;
	units::quantity<units::si::energy> E_n = get_energy(strInput);
	units::quantity<units::si::wavenumber> k_n = E2k(E_n, bImag);
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);

	std::ostringstream ostrlam, ostrv, ostrk, ostrT;
	ostrlam << lam_n;
	ostrk << k_n;
	ostrv << (p_n / co::m_n);
	ostrT << (E_n / co::k_B);

	editNeutronLam->setText(ostrlam.str().c_str());
	editNeutronK->setText(ostrk.str().c_str());
	editNeutronV->setText(ostrv.str().c_str());
	editNeutronT->setText(ostrT.str().c_str());
}

void FormulaDlg::CalcNeutronT()
{
	std::string strInput = editNeutronT->text().toStdString();
	if(!check_input(strInput))
		return;

	bool bImag;
	units::quantity<units::si::temperature> T_n = get_temperature(strInput);
	units::quantity<units::si::energy> E_n = T_n * co::k_B;
	units::quantity<units::si::wavenumber> k_n = E2k(E_n, bImag);
	units::quantity<units::si::length> lam_n = k2lam(k_n);
	units::quantity<units::si::momentum> p_n = lam2p(lam_n);

	std::ostringstream ostrlam, ostrv, ostrk, ostrE;
	ostrlam << lam_n;
	ostrk << k_n;
	ostrv << (p_n / co::m_n);
	ostrE << double(E_n / one_meV) << " meV";

	editNeutronLam->setText(ostrlam.str().c_str());
	editNeutronK->setText(ostrk.str().c_str());
	editNeutronV->setText(ostrv.str().c_str());
	editNeutronE->setText(ostrE.str().c_str());
}


void FormulaDlg::CalcMIEZE()
{
	static const units::quantity<units::si::length> cm = units::si::meter/100.;

	units::quantity<units::si::length> lam = spinLam->value() * angstrom;
	units::quantity<units::si::length> L1 = spinL1->value() * cm;
	units::quantity<units::si::length> Lb = spinLb->value() * cm;
	units::quantity<units::si::frequency> f1 = spinF1->value() * 1000. * units::si::hertz;
	units::quantity<units::si::frequency> f2 = spinF2->value() * 1000. * units::si::hertz;
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
