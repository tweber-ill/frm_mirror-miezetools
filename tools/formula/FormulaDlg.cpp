/*
 * mieze-tool
 * @author tweber
 * @date 08-jul-2013
 */

#include "FormulaDlg.h"
#include <boost/units/io.hpp>
#include "../../tlibs/math/mieze.hpp"
#include "../../tlibs/string/string.h"
#include "../../tlibs/math/math.h"
#include "../../helper/formulas.h"
#include "../../tlibs/helper/misc.h"
#include "../../tlibs/helper/log.h"
#include "../../main/settings.h"
#include "../../data/export.h"

#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <QtGui/QFileDialog>

FormulaDlg::FormulaDlg(QWidget* pParent) : QDialog(pParent), m_pPlanePlot(0)
{
	setupUi(this);
	setupPlanePlotter();
	setupDebyePlotter();
	setupConstants();

	QObject::connect(editNeutronLam, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronLam()));
	QObject::connect(editNeutronE, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronE()));
	QObject::connect(editNeutronV, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronv()));
	QObject::connect(editNeutronK, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronk()));
	QObject::connect(editNeutronT, SIGNAL(textEdited(const QString&)), this, SLOT(CalcNeutronT()));



	std::vector<QLineEdit*> editsDir = {editBraggDirN, editBraggDirLam, editBraggDirD, editBraggDirTT};
	std::vector<QLineEdit*> editsReci = {editBraggReciN, editBraggReciLam, editBraggReciQ, editBraggReciTT};
	std::vector<QRadioButton*> radioDir = {/*radioBraggDirN,*/ radioBraggDirLam, radioBraggDirD, radioBraggDirTT};
	std::vector<QRadioButton*> radioReci = {/*radioBraggReciN,*/ radioBraggReciLam, radioBraggReciQ, radioBraggReciTT};

	for(QLineEdit* pEdit : editsDir)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcBraggDirect()));
	for(QLineEdit* pEdit : editsReci)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)), this, SLOT(CalcBraggReciprocal()));
	for(QRadioButton* pRadio : radioDir)
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(CalcBraggDirect()));
	for(QRadioButton* pRadio : radioReci)
		QObject::connect(pRadio, SIGNAL(toggled(bool)), this, SLOT(CalcBraggReciprocal()));


	//--------------------------------------------------------------------------------
	// el. MIEZE

	std::vector<QDoubleSpinBox*> vecMCondSpins = {spinMF1, spinMF2, spinML1, spinML2};
	for(QDoubleSpinBox* pSpin : vecMCondSpins)
	{
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcMiezeCond()));
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcMiezeTime()));
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcMiezeFields()));
	}

	std::vector<QDoubleSpinBox*> vecMTimeSpins = {spinMLam, spinMTau, spinMLs};
	for(QDoubleSpinBox* pSpin : vecMTimeSpins)
	{
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcMiezeTime()));
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcMiezeCond()));
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcMiezeFields()));
	}
	
	QObject::connect(spinMl1, SIGNAL(valueChanged(double)), this, SLOT(CalcMiezeFields()));
	QObject::connect(spinMl2, SIGNAL(valueChanged(double)), this, SLOT(CalcMiezeFields()));


	//--------------------------------------------------------------------------------
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
	//--------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------
	tablePowderLines->horizontalHeader()->setVisible(true);
	tablePowderLines->setColumnWidth(0, 64);
	tablePowderLines->setColumnWidth(1, 64);
	tablePowderLines->setColumnWidth(2, 250);

	QObject::connect(spinCellD, SIGNAL(valueChanged(double)), this, SLOT(CalcPowderLines()));
	QObject::connect(spinCellLam, SIGNAL(valueChanged(double)), this, SLOT(CalcPowderLines()));
	QObject::connect(spinMaxHKL, SIGNAL(valueChanged(int)), this, SLOT(CalcPowderLines()));
	QObject::connect(comboCell, SIGNAL(currentIndexChanged(int)), this, SLOT(CalcPowderLines()));
	//--------------------------------------------------------------------------------


	CalcNeutronLam();
	CalcMiezeCond();
	CalcMiezeTime();
	CalcMiezeFields();
	CalcBraggDirect();
	CalcBraggReciprocal();
	CalcMIEZE();
	CalcPlane();
	CalcDebye();
	CalcPowderLines();
}

FormulaDlg::~FormulaDlg()
{
	if(m_pPlanePlot) delete m_pPlanePlot;
}

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
		ostrVal << tl::one_eV;

		Constant constant;
		constant.strSymbol = "eV";
		constant.strName = "1 electron volt";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::h;

		Constant constant;
		constant.strSymbol = "h";
		constant.strName = "Planck constant";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << (co::h / tl::one_eV) << " eV";

		Constant constant;
		constant.strSymbol = "h";
		constant.strName = "Planck constant";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::hbar;

		Constant constant;
		constant.strSymbol = "hbar";
		constant.strName = "Planck constant";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << (co::hbar / tl::one_eV) << " eV";

		Constant constant;
		constant.strSymbol = "hbar";
		constant.strName = "Planck constant";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::m_n;

		Constant constant;
		constant.strSymbol = "m_n";
		constant.strName = "Neutron mass";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::g_n;

		Constant constant;
		constant.strSymbol = "g_n";
		constant.strName = "Neutron g";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::gamma_n;

		Constant constant;
		constant.strSymbol = "gamma_n";
		constant.strName = "Neutron gyromagnetic ratio";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

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
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::mu_N;

		Constant constant;
		constant.strSymbol = "mu_N";
		constant.strName = "Nuclear magneton";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

		vecConsts.push_back(constant);
	}
	{
		std::ostringstream ostrVal;
		ostrVal << std::scientific;
		ostrVal << co::c;

		Constant constant;
		constant.strSymbol = "c";
		constant.strName = "Vacuum speed of light";
		constant.strVal = tl::insert_before<std::string>(ostrVal.str(), "(", "\n");

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

		//pConstVal->setFlags(pConstVal->flags() & ~Qt::ItemIsEditable);
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
	units::quantity<units::si::wavenumber> k_n = tl::lam2k(lam_n);
	units::quantity<units::si::momentum> p_n = tl::lam2p(lam_n);
	units::quantity<units::si::energy> E_n = p_n*p_n / (2.*co::m_n);

	std::ostringstream ostrk, ostrv, ostrE, ostrT;
	ostrk << k_n;
	ostrv << (p_n / co::m_n);
	ostrE << double(E_n / tl::one_meV) << " meV";
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
	units::quantity<units::si::length> lam_n = tl::k2lam(k_n);
	units::quantity<units::si::momentum> p_n = tl::lam2p(lam_n);
	units::quantity<units::si::energy> E_n = p_n*p_n / (2.*co::m_n);

	std::ostringstream ostrlam, ostrv, ostrE, ostrT;
	ostrlam << lam_n;
	ostrv << (p_n / co::m_n);
	ostrE << double(E_n / tl::one_meV) << " meV";
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

	tl::log_err("Velocity parsing not yet implemented.");
}

void FormulaDlg::CalcNeutronE()
{
	std::string strInput = editNeutronE->text().toStdString();
	if(!check_input(strInput))
		return;

	bool bImag = 0;
	units::quantity<units::si::energy> E_n = get_energy(strInput);
	units::quantity<units::si::wavenumber> k_n = tl::E2k(E_n, bImag);
	units::quantity<units::si::length> lam_n = tl::k2lam(k_n);
	units::quantity<units::si::momentum> p_n = tl::lam2p(lam_n);

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
	units::quantity<units::si::wavenumber> k_n = tl::E2k(E_n, bImag);
	units::quantity<units::si::length> lam_n = tl::k2lam(k_n);
	units::quantity<units::si::momentum> p_n = tl::lam2p(lam_n);

	std::ostringstream ostrlam, ostrv, ostrk, ostrE;
	ostrlam << lam_n;
	ostrk << k_n;
	ostrv << (p_n / co::m_n);
	ostrE << double(E_n / tl::one_meV) << " meV";

	editNeutronLam->setText(ostrlam.str().c_str());
	editNeutronK->setText(ostrk.str().c_str());
	editNeutronV->setText(ostrv.str().c_str());
	editNeutronE->setText(ostrE.str().c_str());
}


void FormulaDlg::CalcBraggDirect()
{
	std::string strN = editBraggDirN->text().toStdString();
	std::string strLam = editBraggDirLam->text().toStdString();
	std::string strD = editBraggDirD->text().toStdString();
	std::string strTT = editBraggDirTT->text().toStdString();

	int iOrder = tl::str_to_var<int>(strN);
	units::quantity<units::si::length> lam = get_length(strLam);
	units::quantity<units::si::length> d = get_length(strD);
	units::quantity<units::si::plane_angle> tt = get_angle(strTT);

	std::ostringstream ostrOut;

	if(radioBraggDirLam->isChecked())
	{
		lam = tl::bragg_real_lam(d, tt, double(iOrder));
		ostrOut << lam;
		editBraggDirLam->setText(ostrOut.str().c_str());
	}
	else if(radioBraggDirD->isChecked())
	{
		d = tl::bragg_real_d(lam, tt, double(iOrder));
		ostrOut << d;
		editBraggDirD->setText(ostrOut.str().c_str());
	}
	else if(radioBraggDirTT->isChecked())
	{
		tt = tl::bragg_real_twotheta(d, lam, double(iOrder));
		ostrOut << double(tt/units::si::radian) / M_PI * 180. << " deg";
		editBraggDirTT->setText(ostrOut.str().c_str());
	}
}

void FormulaDlg::CalcBraggReciprocal()
{
	std::string strN = editBraggReciN->text().toStdString();
	std::string strLam = editBraggReciLam->text().toStdString();
	std::string strQ = editBraggReciQ->text().toStdString();
	std::string strTT = editBraggReciTT->text().toStdString();

	int iOrder = tl::str_to_var<int>(strN);
	units::quantity<units::si::length> lam = get_length(strLam);
	units::quantity<units::si::wavenumber> Q = get_wavenumber(strQ);
	units::quantity<units::si::plane_angle> tt = get_angle(strTT);

	std::ostringstream ostrOut;

	if(radioBraggReciLam->isChecked())
	{
		lam = tl::bragg_recip_lam(Q, tt, double(iOrder));
		ostrOut << lam;
		editBraggReciLam->setText(ostrOut.str().c_str());
	}
	else if(radioBraggReciQ->isChecked())
	{
		Q = tl::bragg_recip_Q(lam, tt, double(iOrder));
		ostrOut << Q;
		editBraggReciQ->setText(ostrOut.str().c_str());
	}
	else if(radioBraggReciTT->isChecked())
	{
		tt = tl::bragg_recip_twotheta(Q, lam, double(iOrder));
		ostrOut << double(tt/units::si::radian) / M_PI * 180. << " deg";
		editBraggReciTT->setText(ostrOut.str().c_str());
	}
}


// --------------------------------------------------------------------------------

void FormulaDlg::CalcMIEZE()
{
	static const units::quantity<units::si::length> cm = units::si::meter/100.;

	units::quantity<units::si::length> lam = spinLam->value() * tl::angstrom;
	units::quantity<units::si::length> L1 = spinL1->value() * cm;
	units::quantity<units::si::length> Lb = spinLb->value() * cm;
	units::quantity<units::si::frequency> f1 = spinF1->value() * 1000. * units::si::hertz;
	units::quantity<units::si::frequency> f2 = spinF2->value() * 1000. * units::si::hertz;
	units::quantity<units::si::energy> E = spinE->value() * tl::one_meV;

	units::quantity<units::si::length> L2 = tl::mieze_condition_L2(f1, f2, L1);
	units::quantity<units::si::length> Ls = L2-Lb;
	units::quantity<units::si::time> tau = tl::mieze_tau(2.*(f2-f1), Ls, lam);

	units::quantity<units::si::length> Ls_inel = tl::mieze_condition_inel_Ls(Ls, E, lam);
	units::quantity<units::si::length> offs = Ls_inel-Ls;

	units::quantity<units::si::frequency> df1 = tl::mieze_det_misaligned_df1(L1, L2, -offs, f1, f2);
	units::quantity<units::si::frequency> df2 = tl::mieze_det_misaligned_df2(L1, L2, -offs, f1, f2);

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


// --------------------------------------------------------------------------------
// debye waller factor

static inline void export_python(const Plot* pPlot, QWidget* pWid=0)
{
	QString strFile = QFileDialog::getSaveFileName(pWid, "Save as Python file...", "",
					"Python files (*.py)"/*,0, QFileDialog::DontUseNativeDialog*/);
	if(strFile == "")
		return;

	std::string strFile1 = strFile.toStdString();
	std::string strExt = tl::get_fileext(strFile1);
	if(strExt != "py")
		strFile1 += ".py";

	::export_py(strFile1.c_str(), pPlot);
}

void FormulaDlg::PyExportDebye()
{
	export_python(m_pDebyePlot, this);
}

void FormulaDlg::SetDebyeStatusMsg(const char* pcMsg, int iPos)
{
	if(!pcMsg) return;
	if(iPos == 2)
		labelDebyeStatus->setText(pcMsg);
}

void FormulaDlg::setupDebyePlotter()
{
	m_pDebyePlot = new Plot(this);
	m_pDebyePlot->SetLabel(LABEL_X, "Q (1/A)");
	m_pDebyePlot->SetLabel(LABEL_Y, "Debye-Waller Factor");

	QGridLayout *pGrid = new QGridLayout(framePlanePlot_deb);
	pGrid->addWidget(m_pDebyePlot, 0, 0, 1, 1);

	std::vector<QDoubleSpinBox*> vecSpinBoxes = {spinAMU_deb, spinTD_deb, spinT_deb, spinMinQ_deb, spinMaxQ_deb};
	for(QDoubleSpinBox* pSpin : vecSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcDebye()));


	QObject::connect(btnPython_deb, SIGNAL(clicked(bool)), this, SLOT(PyExportDebye()));
	QObject::connect(m_pDebyePlot, SIGNAL(SetStatusMsg(const char*, int)), this, SLOT(SetDebyeStatusMsg(const char*, int)));
}

void FormulaDlg::CalcDebye()
{
	typedef units::quantity<units::si::length> length;
	typedef units::quantity<units::si::temperature> temp;
	typedef units::quantity<units::si::mass> mass;
	typedef units::quantity<units::si::wavenumber> wavenum;

    const temp kelvin = 1. * units::si::kelvin;
    const mass amu = co::m_u;

	const unsigned int NUM_POINTS = 512;

	double dMinQ = spinMinQ_deb->value();
	double dMaxQ = spinMaxQ_deb->value();

	temp T = spinT_deb->value() * kelvin;
	temp T_D = spinTD_deb->value() * kelvin;
	mass M = spinAMU_deb->value() * amu;


	m_pDebyePlot->clear();

	std::vector<double> vecQ, vecDeb;
	vecQ.reserve(NUM_POINTS);
	vecDeb.reserve(NUM_POINTS);

	bool bHasZetaSq = 0;

	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		units::quantity<units::si::wavenumber> Q = (dMinQ + (dMaxQ - dMinQ)/double(NUM_POINTS)*double(iPt)) / tl::angstrom;
		double dDWF = 0.;
		auto zetasq = 1.*tl::angstrom*tl::angstrom;

		if(T <= T_D)
			dDWF = tl::debye_waller_low_T(T_D, T, M, Q, &zetasq);
		else
			dDWF = tl::debye_waller_high_T(T_D, T, M, Q, &zetasq);

		vecQ.push_back(Q * tl::angstrom);

		if(!bHasZetaSq)
		{
			std::string strZetaSq = tl::var_to_str(units::sqrt(zetasq));
			editZetaSq->setText(strZetaSq.c_str());

			bHasZetaSq = 1;
		}

		vecDeb.push_back(dDWF);
	}

	m_pDebyePlot->plot(vecQ.size(), vecQ.data(), vecDeb.data());
	m_pDebyePlot->RefreshPlot();
}



// --------------------------------------------------------------------------------
// scattering plane

void FormulaDlg::setupPlanePlotter()
{
	m_pPlanePlot = new Plot(this);
	m_pPlanePlot->SetLabel(LABEL_X, "Q (1/A)");
	m_pPlanePlot->SetLabel(LABEL_Y, "E (meV)");

	QGridLayout *pGrid = new QGridLayout(framePlanePlot);
	pGrid->addWidget(m_pPlanePlot, 0, 0, 1, 1);


	QObject::connect(radioFixedKi, SIGNAL(toggled(bool)), this, SLOT(FixedKiKfToggled()));

	std::vector<QDoubleSpinBox*> vecSpinBoxes = {spinEiEf, spinMinQ, spinMaxQ, spinAngle};
	for(QDoubleSpinBox* pSpin : vecSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(CalcPlane()));


	QObject::connect(btnPython, SIGNAL(clicked(bool)), this, SLOT(PyExport()));
	QObject::connect(m_pPlanePlot, SIGNAL(SetStatusMsg(const char*, int)), this, SLOT(SetPlaneStatusMsg(const char*, int)));
}

void FormulaDlg::FixedKiKfToggled()
{
	if(radioFixedKi->isChecked())
		labelFixedKiKf->setText("E_i (meV):");
	else
		labelFixedKiKf->setText("E_f (meV):");

	CalcPlane();
}

void FormulaDlg::CalcPlane()
{
	const unsigned int NUM_POINTS = 512;

	double dMinQ = spinMinQ->value();
	double dMaxQ = spinMaxQ->value();
	double dAngle = spinAngle->value() / 180. * M_PI;

	units::quantity<units::si::energy> EiEf = spinEiEf->value() * tl::one_meV;


	m_pPlanePlot->clear();

	std::vector<double> vecQ[2], vecE[2];
	vecQ[0].reserve(NUM_POINTS); vecE[0].reserve(NUM_POINTS);
	vecQ[1].reserve(NUM_POINTS); vecE[1].reserve(NUM_POINTS);

	units::quantity<units::si::plane_angle> twotheta = dAngle * units::si::radians;

	for(unsigned int iPt=0; iPt<NUM_POINTS; ++iPt)
	{
		for(unsigned int iSign=0; iSign<=1; ++iSign)
		{
			units::quantity<units::si::wavenumber> Q = (dMinQ + (dMaxQ - dMinQ)/double(NUM_POINTS)*double(iPt)) / tl::angstrom;
			units::quantity<units::si::energy> dE = tl::kinematic_plane(radioFixedKi->isChecked(), iSign, EiEf, Q, twotheta);

			double _dQ = Q * tl::angstrom;
			double _dE = dE / tl::one_meV;

			if(!std::isnan(_dQ) && !std::isnan(_dE) && !std::isinf(_dQ) && !std::isinf(_dE))
			{
				vecQ[iSign].push_back(Q * tl::angstrom);
				vecE[iSign].push_back(dE / tl::one_meV);
			}
		}
	}

	std::vector<double> _vecQ;
	std::vector<double> _vecE;

	_vecQ.insert(_vecQ.end(), vecQ[0].rbegin(), vecQ[0].rend());
	_vecE.insert(_vecE.end(), vecE[0].rbegin(), vecE[0].rend());

	_vecQ.insert(_vecQ.end(), vecQ[1].begin(), vecQ[1].end());
	_vecE.insert(_vecE.end(), vecE[1].begin(), vecE[1].end());


	m_pPlanePlot->plot(_vecQ.size(), _vecQ.data(), _vecE.data());
	m_pPlanePlot->RefreshPlot();
}


void FormulaDlg::PyExport()
{
	export_python(m_pPlanePlot, this);
}

void FormulaDlg::SetPlaneStatusMsg(const char* pcMsg, int iPos)
{
	if(!pcMsg) return;
	if(iPos == 2)
		labelPlaneStatus->setText(pcMsg);
}

// --------------------------------------------------------------------------------


static inline bool is_peak_allowed_sc(int ih, int ik, int il)
{
	return true;
}

static inline bool is_peak_allowed_fcc(int ih, int ik, int il)
{
	return ((tl::is_even(ih) && tl::is_even(ik) && tl::is_even(il)) || 
			(tl::is_odd(ih) && tl::is_odd(ik) && tl::is_odd(il)));
}

static bool mixed_even_and_odd(int ih, int ik, int il)
{
	bool bHasEven = tl::is_even(ih) || tl::is_even(ik) || tl::is_even(il);
	bool bHasOdd = tl::is_odd(ih) || tl::is_odd(ik) || tl::is_odd(il);

	return bHasEven && bHasOdd;
}

static inline bool is_peak_allowed_diamond(int ih, int ik, int il)
{
	if(mixed_even_and_odd(ih,ik,il))
		return false;

	return !(tl::is_even(ih+ik+il) && ((ih+ik+il)%4 != 0));
}

static inline bool is_peak_allowed_bcc(int ih, int ik, int il)
{
	return tl::is_even(ih + ik + il);
}


static double calc_d_cubic(int ih, int ik, int il, double da)
{
        return da/std::sqrt(double(ih*ih) + double(ik*ik) + double(il*il));
}

// n lam = 2d sin th
// asin(n lam / (2d)) = th
static double get_bragg_angle(int ih, int ik, int il, double dLam, double da)
{
        double d = calc_d_cubic(ih, ik, il, da);
        double dTwotheta = std::asin(dLam / (2.*d)) * 2.;
        return dTwotheta;
}


struct PowderLine
{
	std::string strAngle;
	double dAngle;

	std::string strQ;
	std::string strPeaks;

	static bool comp(const PowderLine& line1, const PowderLine& line2)
	{
		return (line1.dAngle <= line2.dAngle);
	}
};

void FormulaDlg::CalcPowderLines()
{
	const double dLam = spinCellLam->value();
	const double da = spinCellD->value();
	const int iMaxHKL = spinMaxHKL->value();

	bool (*is_peak_allowed_all[])(int, int, int) = {is_peak_allowed_sc, is_peak_allowed_fcc, is_peak_allowed_bcc, is_peak_allowed_diamond};

	int iIdx = comboCell->currentIndex();
	bool (*is_peak_allowed)(int, int, int) = is_peak_allowed_all[iIdx];


	std::map<std::string, std::string> mapPeaks;

	for(int ih=0; ih<iMaxHKL; ++ih)
		for(int ik=0; ik<iMaxHKL; ++ik)
			for(int il=0; il<iMaxHKL; ++il)
			{
				if(ih==0 && ik==0 && il ==0) continue;
				if(!is_peak_allowed(ih, ik, il)) continue;

				double dAngle = get_bragg_angle(ih, ik, il, dLam, da) / M_PI * 180.;
				if(std::isnan(dAngle) || std::isinf(dAngle)) continue;

				std::ostringstream ostrAngle;
				ostrAngle.precision(6);
				ostrAngle << dAngle;

				std::ostringstream ostrPeak;
				ostrPeak << "(" << ih << " " << ik << " " << il << ") ";

				mapPeaks[ostrAngle.str()] += ostrPeak.str();
			}

	std::vector<PowderLine> vecPowderLines;

	for(const auto& pair : mapPeaks)
	{
		PowderLine powderline;
		powderline.strAngle = pair.first;
		powderline.strPeaks = pair.second;

		std::istringstream istrAngle(pair.first);
		istrAngle >> powderline.dAngle;
		double dQ = tl::bragg_recip_Q(dLam*tl::angstrom, powderline.dAngle/180.*M_PI*units::si::radians, 1.)*tl::angstrom;
		powderline.strQ = tl::var_to_str(dQ);

		vecPowderLines.push_back(powderline);
	}
	std::sort(vecPowderLines.begin(), vecPowderLines.end(), PowderLine::comp);


	const int iNumRows = vecPowderLines.size();
	tablePowderLines->setRowCount(iNumRows);

	for(int iRow=0; iRow<iNumRows; ++iRow)
	{
		//std::cout << pair.first << ": " << pair.second << std::endl;
		for(int iCol=0; iCol<3; ++iCol)
		{
			if(!tablePowderLines->item(iRow, iCol))
				tablePowderLines->setItem(iRow, iCol, new QTableWidgetItem());
		}

		tablePowderLines->item(iRow, 0)->setText(vecPowderLines[iRow].strAngle.c_str());
		tablePowderLines->item(iRow, 1)->setText(vecPowderLines[iRow].strQ.c_str());
		tablePowderLines->item(iRow, 2)->setText(vecPowderLines[iRow].strPeaks.c_str());
	}
}


// --------------------------------------------------------------------------------
// el. MIEZE

void FormulaDlg::CalcMiezeCond()
{
	using tl::length;
	using tl::freq;

	const bool bF1 = radioMF1->isChecked();
	const bool bF2 = radioMF2->isChecked();
	const bool bL1 = radioML1->isChecked();
	const bool bL2 = radioML2->isChecked();

	freq f1 = spinMF1->value() * 1000./units::si::second;
	freq f2 = spinMF2->value() * 1000./units::si::second;
	length L1 = spinML1->value() * 0.01*units::si::meter;
	length L2 = spinML2->value() * 0.01*units::si::meter;

	if(bF1)
	{
		f1 = f2/(L1/L2 + 1.);
		spinMF1->setValue(double(f1 / 1000.*units::si::second));
	}
	else if(bF2)
	{
		f2 = (L1/L2 + 1.)*f1;
		spinMF2->setValue(double(f2 / 1000.*units::si::second));
	}
	else if(bL1)
	{
		L1 = (f2/f1 - 1.)*L2;
		spinML1->setValue(double(L1 / 0.01/units::si::meter));
	}
	else if(bL2)
	{
		L2 = L1/(f2/f1 - 1.);
		spinML2->setValue(double(L2 / 0.01/units::si::meter));
	}
}

void FormulaDlg::CalcMiezeTime()
{
	using tl::length;
	using tl::time;
	using tl::freq;

	const bool bF1 = radioMF1->isChecked();
	const bool bF2 = radioMF2->isChecked();
	const bool bLam = radioMLam->isChecked();
	const bool bTau = radioMTau->isChecked();
	const bool bLs = radioMLs->isChecked();

	freq f1 = spinMF1->value() * 1000./units::si::second;
	freq f2 = spinMF2->value() * 1000./units::si::second;
	length Lam = spinMLam->value() * tl::angstrom;
	time Tau = spinMTau->value() * 1e-12 * units::si::second;
	length Ls = spinMLs->value() * 0.01*units::si::meter;

	if(bLam)
	{
		Lam = tl::mieze_tau_lam(Tau, 2.*(f2-f1), Ls);
		spinMLam->setValue(double(Lam / tl::angstrom));
	}
	else if(bTau)
	{
		Tau = tl::mieze_tau(2.*(f2-f1), Ls, Lam);
		spinMTau->setValue(double(Tau / 1e-12 / units::si::second));
	}
	else if(bLs)
	{
		Ls = tl::mieze_tau_Ls(Tau, 2.*(f2-f1), Lam);
		spinMLs->setValue(double(Ls / 0.01/units::si::meter));
	}
}

void FormulaDlg::CalcMiezeFields()
{	
	static const tl::angle ang = -M_PI*tl::radians;
	
	tl::length l1 = spinMl1->value()*tl::cm;
	tl::length l2 = spinMl2->value()*tl::cm;
	tl::freq om1 = 2.*M_PI * spinMF1->value() * 1000./tl::seconds;
	tl::freq om2 = 2.*M_PI * spinMF2->value() * 1000./tl::seconds;
	tl::length lam = spinMLam->value() * tl::angstrom;

	tl::flux Brf1 = tl::larmor_field(lam, l1, ang);
	tl::flux Brf2 = tl::larmor_field(lam, l2, ang);
	
	tl::flux B01 = tl::larmor_B(om1);
	tl::flux B02 = tl::larmor_B(om2);
	
	std::string strB01 = tl::var_to_str(double(B01/tl::teslas * 1000.));
	std::string strB02 = tl::var_to_str(double(B02/tl::teslas * 1000.));
	std::string strBrf1 = tl::var_to_str(double(Brf1/tl::teslas * 1000.));
	std::string strBrf2 = tl::var_to_str(double(Brf2/tl::teslas * 1000.));

	editMB01->setText(strB01.c_str());
	editMB02->setText(strB02.c_str());
	editMBrf1->setText(strBrf1.c_str());
	editMBrf2->setText(strBrf2.c_str());
}

// -------------------------------------------------------------------------------


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
