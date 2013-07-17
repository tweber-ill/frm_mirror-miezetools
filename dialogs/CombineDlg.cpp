/*
 * mieze-tool
 * @author tweber
 * @date 02-apr-2013
 */

#include "CombineDlg.h"
#include "ListDlg.h"
#include "../data/fit_data.h"
#include "../fitter/models/msin.h"


CombineGraphsDlg::CombineGraphsDlg(QWidget* pParent)
{
	this->setupUi(this);

	connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddItemSelected()));
	connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveItemSelected()));
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
}


CombineGraphsDlg::~CombineGraphsDlg()
{
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

void CombineGraphsDlg::SubWindowRemoved(SubWindowBase *pSWB)
{
	if(!pSWB) return;

	m_allSubWnds.remove(pSWB);


	for(int i=0; i<listGraphs->count(); ++i)
	{
		SubWindowBase* pCurItem = ((ListGraphsItem*)listGraphs->item(i))->subWnd();

		if(pCurItem==pSWB)
		{
			delete listGraphs->item(i);
			--i;
		}
	}
}

void CombineGraphsDlg::SubWindowAdded(SubWindowBase *pSWB)
{
	m_allSubWnds.push_back(pSWB);
}



#define COMBINE_TYPE_COUNTS 		0
#define COMBINE_TYPE_CONTRASTS 		1
#define COMBINE_TYPE_PHASES 		2

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
		SubWindowBase *pSWB = pItem->subWnd();
		if(!pSWB || !pSWB->GetActualWidget())
			continue;
		pSWB = pSWB->GetActualWidget();

		pdX[iCur] = double(iCur)/double(iCnt-1) * (dXMax - dXMin) + dXMin;
		if(iComboIdx == COMBINE_TYPE_COUNTS)
		{
			pdY[iCur] = pSWB->GetTotalCounts();
			pdYErr[iCur] = sqrt(pdY[iCur]);
		}
		else if(iComboIdx == COMBINE_TYPE_CONTRASTS ||
				iComboIdx == COMBINE_TYPE_PHASES)
		{
			Plot* pPlot1 = pSWB->ConvertTo1d(-1);
			FunctionModel* pFkt = 0;
			FitDataParams fitparams;
			fitparams.iFkt = FIT_MIEZE_SINE;
			FitData::fit(pPlot1->GetData(0).dat, fitparams, &pFkt);

			MiezeSinModel* pM = (MiezeSinModel*)pFkt;

			if(iComboIdx == COMBINE_TYPE_CONTRASTS)
			{
				pdY[iCur] = pM ? pM->GetContrast() : 0.;
				pdYErr[iCur] = pM ? pM->GetContrastErr() : 0.;
			}
			else
			{
				pdY[iCur] = pM ? pM->GetPhase() : 0.;
				pdYErr[iCur] = pM ? pM->GetPhaseErr() : 0.;
			}

			delete pPlot1;
			delete pFkt;
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
	else if(iComboIdx == COMBINE_TYPE_CONTRASTS)
		strLabY = "Contrast";

	pPlot->SetLabels(strLabX.c_str(), strLabY.c_str());
	pPlot->SetTitle(strPlotTitle.c_str());

	delete[] pdX; delete[] pdY; delete[] pdYErr;
	return pPlot;
}


void CombineGraphsDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		Plot* pPlot = CreatePlot("combined graph", this);
		emit AddSubWindow(pPlot);
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		QDialog::accept();
	}
}

#include "CombineDlg.moc"
