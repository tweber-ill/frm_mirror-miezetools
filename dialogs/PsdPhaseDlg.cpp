/*
 * mieze-tool
 * @author tweber
 * @date 29-may-2013
 */

#include "PsdPhaseDlg.h"
#include "../helper/mieze.hpp"

PsdPhaseDlg::PsdPhaseDlg(QWidget* pParent)
					: QDialog(pParent), m_bAllowUpdate(0),
					  m_pPlot(new Plot2d(this, "PSD Phase", 0, 1))
{
	setupUi(this);
	m_pPlot->SetLabels("x Position (cm)", "y Position (cm)", "Phase (rad)");

	QGridLayout *pGrid = new QGridLayout(frame);
	pGrid->addWidget(m_pPlot, 0, 0, 1, 1);

	std::vector<QDoubleSpinBox*> vecDSpinBoxes = {spinlx, spinly, spinXCenter, spinYCenter, spinLam, spinTau, spinLs};
	std::vector<QSpinBox*> vecSpinBoxes = {spinXPix, spinYPix};
	for(QDoubleSpinBox* pSpin : vecDSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(Update()));
	for(QSpinBox* pSpin : vecSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(int)), this, SLOT(Update()));

	m_bAllowUpdate = 1;
	Update();
}

PsdPhaseDlg::~PsdPhaseDlg()
{
	delete m_pPlot;
}

void PsdPhaseDlg::Update()
{
	if(!m_bAllowUpdate)
		return;

	const units::quantity<units::si::length> cm = units::si::meter/100.;
	const units::quantity<units::si::length> angstrom = 1e-10 * units::si::meter;
	const units::quantity<units::si::time> ps = 1e-12 * units::si::second;

	units::quantity<units::si::length> lx = spinlx->value() *cm;
	units::quantity<units::si::length> ly = spinly->value()*cm;
	units::quantity<units::si::length> Ls = spinLs->value()*cm;
	units::quantity<units::si::time> tau = spinTau->value()*ps;
	units::quantity<units::si::length> lam = spinLam->value()*angstrom;
	unsigned int iXPixels = spinXPix->value();
	unsigned int iYPixels = spinYPix->value();

	ublas::matrix<double> matPhases;
	mieze_reduction_det(lx, ly, Ls, tau, lam, iXPixels, iYPixels, &matPhases);

	m_dat.FromMatrix(matPhases);
	m_dat.SetXRange(-lx/2./cm, lx/2./cm);
	m_dat.SetYRange(-ly/2./cm, ly/2./cm);
	m_dat.SetMinMax(0., 2.*M_PI);

	m_pPlot->plot(m_dat);
}


#include "PsdPhaseDlg.moc"
