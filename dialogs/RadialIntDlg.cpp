/*
 * mieze-tool
 * @author tweber
 * @date 02-jul-2013
 */

#include "RadialIntDlg.h"
#include "../roi/roi.h"
#include "../plot/plot3d.h"
#include "../plot/plot4d.h"
#include "../data/fit_data.h"
#include "../fitter/models/msin.h"

RadialIntDlg::RadialIntDlg(QWidget* pParent)
			: QDialog(pParent), m_pPlot(new Plot(this))
{
	setupUi(this);

	QGridLayout *pGrid = new QGridLayout(frame);
	pGrid->addWidget(m_pPlot, 0, 0, 1, 1);


	std::vector<QDoubleSpinBox*> vecSpinBoxes = { spinX, spinY, spinRadius, spinInc, spinScale };
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

	PlotInfo info = pSWB->GetPlotInfo();
	uint iOldW = info.iWidth;
	uint iOldH = info.iHeight;
	const double dResScale = spinScale->value();

	if(dResScale != 1.)
	{
		pInterp->ChangeResolution(uint(iOldW*dResScale), uint(iOldH*dResScale), 1);
		dXStart *= dResScale;
		dYStart *= dResScale;
		dRadius *= dResScale;
		dInc *= dResScale;

		Roi *pAntiRoi = pInterp->GetROI(1);
		if(pAntiRoi)
			pInterp->GetROI(1)->Scale(dResScale);
	}

	Data1 dat1d;

	ublas::vector<double> center(2);
	center[0] = dXStart;
	center[1] = dYStart;

	Roi roi = *pInterp->GetROI();
	RoiCircleRing* circ = new RoiCircleRing;
	roi.add(circ);
	circ->GetCenter() = center;
	roi.SetRoiActive(1);

	uint iNumPts = uint(dRadius/dInc);
	dat1d.SetLength(iNumPts);

	for(uint iPt=0; iPt<iNumPts; ++iPt)
	{
		double dCircBegin = dInc*double(iPt);
		double dCircEnd = dInc*double(iPt+1);

		circ->GetInnerRadius() = dCircBegin;
		circ->GetOuterRadius() = dCircEnd;
		circ->CalculateBoundingRect();

		// count data
		if(pSWB->GetType() == PLOT_2D)
		{
			Plot2d* pPlot = (Plot2d*)pInterp;
			pPlot->SetROI(&roi);

			double dCnts = pPlot->GetData2().GetTotalInROI();
			dat1d.SetX(iPt, dCircBegin + 0.5*(dCircEnd-dCircBegin));
			dat1d.SetY(iPt, dCnts);
			dat1d.SetYErr(iPt, std::sqrt(dCnts));

			m_pPlot->SetLabels("Radius", "Counts");
		}
		// mieze data
		else if(pSWB->GetType()==PLOT_3D || pSWB->GetType()==PLOT_4D)
		{
			Data1 dat1;

			if(pSWB->GetType()==PLOT_3D)
			{
				Plot3d* pPlot = (Plot3d*)pInterp->GetActualWidget();
				const Data3& dat = pPlot->GetData();
				pPlot->SetROI(&roi);

				dat1 = dat.GetXYSum();
			}
			else if(pSWB->GetType()==PLOT_4D)
			{
				Plot4d* pPlot = (Plot4d*)pInterp->GetActualWidget();
				const Data4& dat = pPlot->GetData();
				pPlot->SetROI(&roi);

				std::vector<Data1> foils;
				for(unsigned int iFoil=0; iFoil<dat.GetDepth2(); ++iFoil)
				{
					Data1 dat1 = dat.GetXYSum(iFoil);
					foils.push_back(dat1);
				}

				const std::vector<double> *pvecPhases = 0;
				if(dat.HasPhases())
					pvecPhases = &dat.GetPhases();
				dat1 = FitData::mieze_sum_foils(foils, pvecPhases);
			}

			FitDataParams params;
			params.iFkt = FIT_MIEZE_SINE;
			FunctionModel *pFkt = 0;
			bool bOk = FitData::fit(dat1, params, &pFkt);
			MiezeSinModel *pSin = (MiezeSinModel*)pFkt;

			double dContrast = 0.;
			double dContrastErr = 0.;

			if(bOk && pSin)
			{
				dContrast = pSin->GetContrast();
				dContrastErr = pSin->GetContrastErr();
			}

			dat1d.SetX(iPt, dCircBegin + 0.5*(dCircEnd-dCircBegin));
			dat1d.SetY(iPt, dContrast);
			dat1d.SetYErr(iPt, dContrastErr);

			m_pPlot->SetLabels("Radius", "Contrast");

			if(pFkt)
				delete pFkt;
		}
	}

	delete pInterp;

	QString strTitle = pSWB->windowTitle() + " -> rad int";
	m_pPlot->setWindowTitle(strTitle);

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
