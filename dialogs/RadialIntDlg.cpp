/*
 * mieze-tool
 * @author tweber
 * @date 02-jul-2013
 */

#include "RadialIntDlg.h"
#include "../roi/roi.h"

RadialIntDlg::RadialIntDlg(QWidget* pParent)
			: QDialog(pParent), m_pPlot(new Plot(this))
{
	setupUi(this);

	QGridLayout *pGrid = new QGridLayout(frame);
	pGrid->addWidget(m_pPlot, 0, 0, 1, 1);


	std::vector<QDoubleSpinBox*> vecSpinBoxes = { spinX, spinY, spinRadius, spinInc };
	std::vector<QComboBox*> vecComboBoxes = { comboSrc };

	for(QDoubleSpinBox* pSpinBox : vecSpinBoxes)
		QObject::connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(AutoCalc()));
	for(QComboBox* pComboBox : vecComboBoxes)
		QObject::connect(pComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(AutoCalc()));

	QObject::connect(btnCalc, SIGNAL(clicked()), this, SLOT(Calc()));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
}

RadialIntDlg::~RadialIntDlg()
{
	if(m_pPlot)
	{
		delete m_pPlot;
		m_pPlot = 0;
	}
}

void RadialIntDlg::SubWindowRemoved(SubWindowBase *pSWB)
{
	if(!pSWB) return;
	SubWindowBase *pSW = pSWB->GetActualWidget();

	for(std::vector<Plot2d*>::iterator iter=m_vecPlots.begin(); iter!=m_vecPlots.end(); ++iter)
	{
		if((void*)(*iter) == ((void*)pSW))
		{
			comboSrc->removeItem(iter-m_vecPlots.begin());
			m_vecPlots.erase(iter--);
		}
	}

}

void RadialIntDlg::SubWindowAdded(SubWindowBase *pSWB)
{
	if(!pSWB) return;
	SubWindowBase *pSW = pSWB->GetActualWidget();

	if(pSW->GetType()==PLOT_2D || pSW->GetType()==PLOT_3D || pSW->GetType()==PLOT_4D)
	{
		m_vecPlots.push_back((Plot2d*)pSW);
		comboSrc->addItem(pSW->windowTitle());
	}
}

void RadialIntDlg::SetSubWindows(std::vector<SubWindowBase*> vecSWB)
{
	m_vecPlots.clear();

	for(SubWindowBase* pSWB : vecSWB)
	{
		if(!pSWB) continue;

		if(pSWB->GetType()==PLOT_2D || pSWB->GetType()==PLOT_3D || pSWB->GetType()==PLOT_4D)
		{
			Plot2d* pPlot = (Plot2d*)pSWB->GetActualWidget();
			m_vecPlots.push_back(pPlot);

			comboSrc->addItem(pPlot->windowTitle());
		}
	}
}


void RadialIntDlg::AutoCalc()
{

	int iSrcIdx = comboSrc->currentIndex();
	if(iSrcIdx<0)
	{
		m_pPlot->clear();
		return;
	}

	const SubWindowBase* pSWB = m_vecPlots[iSrcIdx];

	// no automatic calculation for large data sets
	if(pSWB->GetType()==PLOT_4D || pSWB->GetType()==PLOT_3D)
		return;

	Calc();
}

void RadialIntDlg::Calc()
{
	m_pPlot->clear();

	double dXStart = spinX->value();
	double dYStart = spinY->value();
	double dRadius = spinRadius->value();
	double dInc = spinInc->value();

	if(dInc<=0. || dRadius<=0.)
		return;

	const int iSrcIdx = comboSrc->currentIndex();
	if(iSrcIdx<0)
		return;

	const SubWindowBase* pSWB = m_vecPlots[iSrcIdx];
	SubWindowBase* pInterp = pSWB->clone();

	uint iOldW = 128;	// TODO
	uint iOldH = 128;
	double dResScale = 5.;
	pInterp->ChangeResolution(uint(iOldW*dResScale), uint(iOldH*dResScale), 1);

	dXStart *= dResScale;
	dYStart *= dResScale;
	dRadius *= dResScale;
	dInc *= dResScale;

	Data1 dat1d;

	ublas::vector<double> center(2);
	center[0] = dXStart;
	center[1] = dYStart;

	Roi roi;
	RoiCircleRing* circ = new RoiCircleRing;
	roi.add(circ);
	circ->GetCenter() = center;
	roi.SetRoiActive(1);

	// count data
	if(pSWB->GetType() == PLOT_2D)
	{
		Plot2d* pPlot = (Plot2d*)pInterp;
		uint iNumPts = uint(dRadius/dInc);
		dat1d.SetLength(iNumPts);

		for(uint iPt=0; iPt<iNumPts; ++iPt)
		{
			double dCircBegin = dInc*double(iPt);
			double dCircEnd = dInc*double(iPt+1);

			circ->GetInnerRadius() = dCircBegin;
			circ->GetOuterRadius() = dCircEnd;
			circ->CalculateBoundingRect();
			pPlot->SetROI(&roi);

			double dCnts = pPlot->GetData2().GetTotalInROI();
			dat1d.SetX(iPt, iPt);
			dat1d.SetY(iPt, dCnts);
		}
	}
	// mieze data
	else if(pSWB->GetType() == PLOT_3D)
	{
		std::cout << "TODO" << std::endl;
	}
	// mieze data
	else if(pSWB->GetType() == PLOT_4D)
	{
		std::cout << "TODO" << std::endl;
	}

	delete pInterp;

	m_pPlot->plot(dat1d);
	m_pPlot->RefreshPlot();
}

void RadialIntDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
		reject();
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		emit NewSubWindow(m_pPlot->clone());
		accept();
	}
}

#include "RadialIntDlg.moc"
