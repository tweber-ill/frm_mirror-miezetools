/*
 * mieze-tool
 * @author tweber
 * @date 02-jul-2013
 */

#include "RadialIntDlg.h"

RadialIntDlg::RadialIntDlg(QWidget* pParent) : QDialog(pParent)
{
	setupUi(this);
}

RadialIntDlg::~RadialIntDlg()
{
}

void RadialIntDlg::SubWindowRemoved(SubWindowBase *pSWB)
{
	if(!pSWB) return;
	SubWindowBase *pSW = pSWB->GetActualWidget();

	for(std::vector<Plot2d*>::iterator iter=m_vecPlots.begin(); iter!=m_vecPlots.end(); ++iter)
	{
		if((void*)(*iter) == ((void*)pSW))
			m_vecPlots.erase(iter--);
	}

}

void RadialIntDlg::SubWindowAdded(SubWindowBase *pSWB)
{
	if(!pSWB) return;
	SubWindowBase *pSW = pSWB->GetActualWidget();

	if(pSW->GetType() == PLOT_2D)
		m_vecPlots.push_back((Plot2d*)pSW);
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
		}
	}
}

#include "RadialIntDlg.moc"
