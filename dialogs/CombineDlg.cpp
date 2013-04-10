/*
 * mieze-tool
 * @author tweber
 * @date 02-apr-2013
 */

#include "CombineDlg.h"
#include "ListDlg.h"


CombineGraphsDlg::CombineGraphsDlg(QWidget* pParent)
{
	this->setupUi(this);

	connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddItemSelected()));
	connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveItemSelected()));
}


CombineGraphsDlg::~CombineGraphsDlg()
{
}

void CombineGraphsDlg::AddAvailSubWnd(SubWindowBase* pSubWnd)
{
	m_allSubWnds.push_back(pSubWnd);
}

void CombineGraphsDlg::AddItemSelected()
{
	ListGraphsDlg dlg(this);

	for(SubWindowBase *pItem : m_allSubWnds)
		dlg.AddSubWnd(pItem);

	if(dlg.exec() == QDialog::Accepted)
	{
		std::list<SubWindowBase*> lstWnds = dlg.GetSelectedSubWnds();

		for(auto wnd : lstWnds)
		{
			ListGraphsItem* pItem = new ListGraphsItem(listGraphs);
			pItem->setSubWnd(wnd);
			pItem->setText(wnd->windowTitle());

			listGraphs->addItem(pItem);
		}
	}
}

void CombineGraphsDlg::RemoveItemSelected()
{
	if(listGraphs->selectedItems().size() == 0)
		listGraphs->selectAll();

	for(auto pItem : listGraphs->selectedItems())
		delete pItem;
}


#define COMBINE_TYPE_COUNTS 		0
#define COMBINE_TYPE_CONTRASTS 	1

Plot* CombineGraphsDlg::CreatePlot(const std::string& strTitle, QWidget* pPlotParent) const
{
	bool bXMinOk=0, bXMaxOk=0;
	double dXMin = editXMin->text().toDouble(&bXMinOk);
	double dXMax = editXMax->text().toDouble(&bXMaxOk);
	if(!bXMinOk || !bXMaxOk)
	{
		dXMin = 0.;
		dXMax = 1.;
	}

	const int iCnt = listGraphs->count();
	double *pdX = new double[iCnt];
	double *pdY = new double[iCnt];
	double *pdYErr = new double[iCnt];

	const int iComboIdx = comboType->currentIndex();

	for(int iCur=0; iCur<iCnt; ++iCur)
	{
		ListGraphsItem* pItem = (ListGraphsItem*)listGraphs->item(iCur);

		pdX[iCur] = double(iCur)/double(iCnt-1) * (dXMax - dXMin) + dXMin;
		if(iComboIdx == COMBINE_TYPE_COUNTS)
		{
			pdY[iCur] = pItem->subWnd()->GetTotalCounts();
			pdYErr[iCur] = sqrt(pdY[iCur]);
		}
		else
		{
			pdY[iCur] = 0.;
			pdYErr[iCur] = 0.;
		}
	}

	Plot *pPlot = new Plot(pPlotParent, strTitle.c_str());
	pPlot->plot(iCnt, pdX, pdY, pdYErr);


	std::string strLabX, strLabY, strPlotTitle;
	strLabX = editXLab->text().toStdString();
	strPlotTitle = editTitle->text().toStdString();

	if(iComboIdx == COMBINE_TYPE_COUNTS)
		strLabY = "Counts";

	pPlot->SetLabels(strLabX.c_str(), strLabY.c_str());
	pPlot->SetTitle(strPlotTitle.c_str());

	delete[] pdX; delete[] pdY; delete[] pdYErr;
	return pPlot;
}


#include "CombineDlg.moc"
