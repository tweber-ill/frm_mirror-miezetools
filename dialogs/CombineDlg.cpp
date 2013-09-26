/*
 * mieze-tool
 * @author tweber
 * @date 02-apr-2013
 */

#include "CombineDlg.h"
#include "ListDlg.h"
#include "../data/fit_data.h"
#include "../fitter/models/msin.h"
#include "../fitter/parser.h"

#include <vector>
#include <algorithm>


#define COMBINE_TYPE_COUNTS 		0
#define COMBINE_TYPE_CONTRASTS 		1
#define COMBINE_TYPE_PHASES 		2
#define COMBINE_TYPE_PARAM	 		3
#define COMBINE_TYPE_EXPR	 		4


CombineGraphsDlg::CombineGraphsDlg(QWidget* pParent)
{
	this->setupUi(this);

	connect(comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(TypeChanged()));
	connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddItemSelected()));
	connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveItemSelected()));
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
}


CombineGraphsDlg::~CombineGraphsDlg()
{
}

void CombineGraphsDlg::TypeChanged()
{
	const int iCurIdx = comboType->currentIndex();

	bool bExpr = 0;
	bool bParam = 0;

	if(iCurIdx == COMBINE_TYPE_EXPR)
		bExpr = 1;
	else if(iCurIdx == COMBINE_TYPE_PARAM)
		bParam = 1;

	labelExpr->setEnabled(bExpr);
	editExpr->setEnabled(bExpr);
	labelParam->setEnabled(bParam);
	comboParam->setEnabled(bParam);
}

// create a set of all parameters common to all selected plots
void CombineGraphsDlg::UpdateCommonParams()
{
	std::vector<std::string> vecParams;
	bool bFirstParam = 1;

	for(int iCur=0; iCur<listGraphs->count(); ++iCur)
	{
		ListGraphsItem* pItem = (ListGraphsItem*)listGraphs->item(iCur);
		SubWindowBase *pSWB = pItem->subWnd();
		if(!pSWB || !pSWB->GetActualWidget())
			continue;
		pSWB = pSWB->GetActualWidget();

		const DataInterface *pIf = pSWB->GetDataInterface();
		if(!pIf)
		{
			vecParams.clear();
			break;
		}


		const StringMap* pParams = 0;
		if(pSWB->GetType() == PLOT_1D)
			pParams = &((Plot*)pSWB)->GetParamMapDynMerged();
		if(!pParams)
			pParams = &pIf->GetParamMapDyn();

		if(pParams)
		{
			std::vector<std::string> vecParams1 = vecParams;
			std::vector<std::string> vecParams2 = pParams->GetKeys();

			if(bFirstParam)
			{
				vecParams1 = vecParams2;
				bFirstParam = 0;
			}

			vecParams.resize(std::min(vecParams1.size(), vecParams2.size()));

			std::set_intersection(vecParams1.begin(), vecParams1.end(),
								vecParams2.begin(), vecParams2.end(),
								vecParams.begin());
		}
		else
		{
			vecParams.clear();
			break;
		}
	}


	QStringList lst;
	for(std::string& str : vecParams)
	{
		if(str.substr(0,6) == "param_")
			lst.append(str.c_str());
	}

	comboParam->clear();
	comboParam->addItems(lst);
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

		UpdateCommonParams();
	}
}

void CombineGraphsDlg::RemoveItemSelected()
{
	if(listGraphs->selectedItems().size() == 0)
		listGraphs->selectAll();

	for(auto pItem : listGraphs->selectedItems())
		delete pItem;

	UpdateCommonParams();
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
	std::string strLabY;

	for(int iCur=0; iCur<iCnt; ++iCur)
	{
		pdY[iCur] = 0.;
		pdYErr[iCur] = 0.;


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

			strLabY = "Counts";
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

			strLabY = "Contrast";

			delete pPlot1;
			delete pFkt;
		}



		else if(iComboIdx == COMBINE_TYPE_PARAM)
		{
			const DataInterface *pIf = pSWB->GetDataInterface();
			if(!pIf)
				continue;

			const StringMap* pParams = 0;
			if(pSWB->GetType() == PLOT_1D)
				pParams = &((Plot*)pSWB)->GetParamMapDynMerged();
			if(!pParams)
				pParams = &pIf->GetParamMapDyn();

			if(!pParams)
				continue;


			std::string strParam = comboParam->currentText().toStdString();

			std::string strVal = (*pParams)[strParam];
			get_val_and_err(strVal, pdY[iCur], pdYErr[iCur]);
			strLabY = strParam;
		}



		else if(iComboIdx == COMBINE_TYPE_EXPR)
		{
			// TODO

			pdY[iCur] = 0.;
			pdYErr[iCur] = 0.;
		}
	}

	Plot *pPlot = new Plot(pPlotParent, strTitle.c_str());
	pPlot->plot(iCnt, pdX, pdY, pdYErr);


	std::string strLabX, strPlotTitle;
	strLabX = editXLab->text().toStdString();
	strPlotTitle = editTitle->text().toStdString();


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
