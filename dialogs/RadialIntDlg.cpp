/*
 * mieze-tool
 * @author tweber
 * @date 02-jul-2013
 */

#include "RadialIntDlg.h"

RadialIntDlg::RadialIntDlg(QWidget* pParent)
			: QDialog(pParent), m_pPlot(new Plot(this))
{
	setupUi(this);

	QGridLayout *pGrid = new QGridLayout(frame);
	pGrid->addWidget(m_pPlot, 0, 0, 1, 1);


	std::vector<QDoubleSpinBox*> vecSpinBoxes = { spinX, spinY, spinXInc, spinYInc };
	std::vector<QComboBox*> vecComboBoxes = { comboSrc, comboType };

	for(QDoubleSpinBox* pSpinBox : vecSpinBoxes)
		QObject::connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(Calc()));
	for(QComboBox* pComboBox : vecComboBoxes)
		QObject::connect(pComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(Calc()));
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

	if(pSW->GetType() == PLOT_2D)
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

		if(pSWB->GetType() == PLOT_2D)
		{
			Plot2d* pPlot = (Plot2d*)pSWB->GetActualWidget();
			m_vecPlots.push_back(pPlot);

			comboSrc->addItem(pPlot->windowTitle());
		}
	}
}


#define TYPE_VALS_SPLIT 	0
#define TYPE_VALS_AGV 		1

void RadialIntDlg::Calc()
{
	const int iTypeIdx = comboType->currentIndex();

	int iSrcIdx = comboSrc->currentIndex();
	if(iSrcIdx<0)
	{
		m_pPlot->clear();
		return;
	}

	const Plot2d* pPlot2d = m_vecPlots[iSrcIdx];

	Plot2d* pPlotInterp = new Plot2d(*pPlot2d);
	pPlotInterp->ChangeResolution(512, 512, iTypeIdx==TYPE_VALS_SPLIT);
	// TODO
	delete pPlotInterp;

	std::cout << "TODO: Calc" << std::endl;
}

#include "RadialIntDlg.moc"
