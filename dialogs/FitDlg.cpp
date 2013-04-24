/*
 * mieze-tool
 * @author tweber
 * @date 10-apr-2013
 */

#include "FitDlg.h"
#include "ListDlg.h"

#include "../fitter/models/freefit.h"
#include "../fitter/models/msin.h"
#include "../fitter/models/gauss.h"

#include "../helper/string.h"
#include "../helper/misc.h"
#include "../helper/fourier.h"

#include "../plot/plot.h"
#include "../plot/plot2d.h"
#include "../plot/plot3d.h"
#include "../plot/plot4d.h"

#include "../settings.h"

#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>

#include <set>
#include <vector>
#include <iostream>
#include <sstream>
#include <limits>
#include <math.h>

FitDlg::FitDlg(QWidget* pParent, QMdiArea *pmdi) : QDialog(pParent), m_pmdi(pmdi)
{
	this->setupUi(this);

	connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddItemSelected()));
	connect(btnAddActive, SIGNAL(clicked(bool)), this, SLOT(AddActiveItemSelected()));
	connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveItemSelected()));

	connect(comboFitType, SIGNAL(currentIndexChanged(int)), this, SLOT(FunctionTypeChanged()));
	connect(editFkt, SIGNAL(textChanged(const QString&)), this, SLOT(FunctionChanged(const QString&)));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	tableLimits->setColumnWidth(2,50);
	tableHints->setColumnWidth(2,50);

	spinFoil->setMaximum(Settings::Get<unsigned int>("casc/foil_cnt")-1);
}

FitDlg::~FitDlg()
{}

void FitDlg::FunctionTypeChanged()
{
	FunctionChanged(editFkt->text());
}

void FitDlg::FunctionChanged(const QString& strFkt)
{
	if(tableLimits->rowCount())
		SaveLastParams();

	tableLimits->setRowCount(0);
	tableHints->setRowCount(0);

	Parser parser;
	parser.SetVerbose(false);

	std::vector<Symbol> vecFreeParams;
	Symbol symFree;
	symFree.strIdent = "x";
	vecFreeParams.push_back(symFree);

	if(comboFitType->currentIndex() == 1)		// 2d fit
	{
		Symbol symFreeY;
		symFreeY.strIdent = "y";
		vecFreeParams.push_back(symFreeY);
	}

	parser.SetFreeParams(vecFreeParams);


	std::string strTheFkt = strFkt.toStdString();
	::trim(strTheFkt);
	if(strTheFkt.length()==0)
	{
		labelStatus->setText("No function given.");
		return;
	}

	if(!Parser::CheckValidLexemes(strTheFkt))
	{
		labelStatus->setText("Invalid characters in function.");
		return;
	}

	try
	{
		if(!parser.ParseExpression(strTheFkt))
		{
			labelStatus->setText("Invalid function.");
			return;
		}
	}
	catch(const std::exception& ex)
	{
		labelStatus->setText("Invalid function (bad input).");
		return;
	}
	labelStatus->setText("Function ok.");

	const std::vector<Symbol>&syms = parser.GetSymbols();
	tableLimits->setRowCount(syms.size());
	tableHints->setRowCount(syms.size());

	for(unsigned int iSym=0; iSym<syms.size(); ++iSym)
	{
		const Symbol& sym = syms[iSym];

        QTableWidgetItem *pItemParam = new QTableWidgetItem();
        pItemParam->setText(sym.strIdent.c_str());
        tableLimits->setVerticalHeaderItem(iSym, pItemParam);

        QTableWidgetItem *pItemLower = new QTableWidgetItem();
        tableLimits->setItem(iSym,0,pItemLower);
        QTableWidgetItem *pItemUpper = new QTableWidgetItem();
        tableLimits->setItem(iSym,1,pItemUpper);
        QTableWidgetItem *pItemActive0 = new QTableWidgetItem();
        pItemActive0->setText("0");
        tableLimits->setItem(iSym,2,pItemActive0);


        QTableWidgetItem *pItemHints = new QTableWidgetItem();
        pItemHints->setText(sym.strIdent.c_str());
        tableHints->setVerticalHeaderItem(iSym, pItemHints);

        QTableWidgetItem *pItemHint = new QTableWidgetItem();
        tableHints->setItem(iSym,0,pItemHint);
        QTableWidgetItem *pItemDelta = new QTableWidgetItem();
        tableHints->setItem(iSym,1,pItemDelta);
        QTableWidgetItem *pItemActive1 = new QTableWidgetItem();
        pItemActive1->setText("0");
        tableHints->setItem(iSym,2,pItemActive1);
	}

	RestoreLastParams();
}

void FitDlg::SaveLastParams()
{
	//m_mapParams.clear();

	if(tableHints->rowCount() != tableLimits->rowCount())
		return;

	for(int iRow=0; iRow<tableHints->rowCount(); ++iRow)
	{
		QTableWidgetItem* pHeader = tableHints->verticalHeaderItem(iRow);
		if(!pHeader) continue;
		std::string strParamName = pHeader->text().toStdString();

		FitParams params;

		const QTableWidgetItem* pItemHint = tableHints->item(iRow,0);
		const QTableWidgetItem* pItemErr = tableHints->item(iRow,1);
		const QTableWidgetItem* pItemHintActive = tableHints->item(iRow,2);
		const QTableWidgetItem* pItemMin = tableLimits->item(iRow,0);
		const QTableWidgetItem* pItemMax = tableLimits->item(iRow,1);
		const QTableWidgetItem* pItemLimitActive = tableLimits->item(iRow,2);

		if(pItemHint) params.strHint = pItemHint->text().toStdString();
		if(pItemErr) params.strErr = pItemErr->text().toStdString();
		if(pItemMin) params.strMin = pItemMin->text().toStdString();
		if(pItemMax) params.strMax = pItemMax->text().toStdString();
		if(pItemHintActive) params.bHintActive = pItemHintActive->text().toInt();
		if(pItemLimitActive) params.bLimitActive = pItemLimitActive->text().toInt();

		m_mapParams[strParamName] = params;
	}
}

void FitDlg::RestoreLastParams()
{
	if(tableHints->rowCount() != tableLimits->rowCount())
		return;

	for(int iRow=0; iRow<tableHints->rowCount(); ++iRow)
	{
		QTableWidgetItem* pHeader = tableHints->verticalHeaderItem(iRow);
		if(!pHeader) continue;
		std::string strParamName = pHeader->text().toStdString();

		t_mapParams::const_iterator iter = m_mapParams.find(strParamName);
		if(iter == m_mapParams.end())
			continue;

		FitParams params = (*iter).second;

		QTableWidgetItem* pItemHint = tableHints->item(iRow,0);
		QTableWidgetItem* pItemErr = tableHints->item(iRow,1);
		QTableWidgetItem* pItemHintActive = tableHints->item(iRow,2);
		QTableWidgetItem* pItemMin = tableLimits->item(iRow,0);
		QTableWidgetItem* pItemMax = tableLimits->item(iRow,1);
		QTableWidgetItem* pItemLimitActive = tableLimits->item(iRow,2);

		if(pItemHint) pItemHint->setText(params.strHint.c_str());
		if(pItemErr) pItemErr->setText(params.strErr.c_str());
		if(pItemMin) pItemMin->setText(params.strMin.c_str());
		if(pItemMax) pItemMax->setText(params.strMax.c_str());
		if(pItemHintActive) pItemHintActive->setText(params.bHintActive ? "1" : "0");
		if(pItemLimitActive) pItemLimitActive->setText(params.bLimitActive ? "1" : "0");
	}
}

void FitDlg::AddActiveItemSelected()
{
	if(!m_pmdi->activeSubWindow()) return;

	SubWindowBase* pWnd = (SubWindowBase*)m_pmdi->activeSubWindow()->widget();
	if(!pWnd || !pWnd->GetActualWidget()) return;
	pWnd = pWnd->GetActualWidget();

	ListGraphsItem* pItem = new ListGraphsItem(listGraphs);
	pItem->setSubWnd(pWnd);
	pItem->setText(pWnd->windowTitle());

	listGraphs->addItem(pItem);
	RemoveDuplicate();
}

void FitDlg::AddItemSelected()
{
	ListGraphsDlg dlg(this);

	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow *pItem : lst)
	{
		SubWindowBase *pWnd = (SubWindowBase *) pItem->widget();
		if(pWnd)
			dlg.AddSubWnd(pWnd);
	}

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

	RemoveDuplicate();
}

void FitDlg::RemoveItemSelected()
{
	if(listGraphs->selectedItems().size() == 0)
		listGraphs->selectAll();

	for(auto pItem : listGraphs->selectedItems())
		delete pItem;
}

void FitDlg::RemoveDuplicate()
{
	std::set<SubWindowBase*> setItems;

	for(int i=0; i<listGraphs->count(); ++i)
	{
		SubWindowBase* pCurItem = ((ListGraphsItem*)listGraphs->item(i))->subWnd();
		if(!setItems.insert(pCurItem).second)	// already in set?
		{
			delete listGraphs->item(i);
			--i;
		}
	}
}

void FitDlg::UpdateSourceList()
{
	std::set<SubWindowBase*> setItems;

	// get all plots into a set
	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow* pWnd : lst)
	{
		SubWindowBase* pPlot = (SubWindowBase*)pWnd->widget();

		setItems.insert(pPlot);
		setItems.insert(pPlot->GetActualWidget());
	}

	// check if the plots from the list are in the set of all windows
	for(int i=0; i<listGraphs->count(); ++i)
	{
		SubWindowBase* pCurItem = ((ListGraphsItem*)listGraphs->item(i))->subWnd();
		//pCurItem = pCurItem->GetActualWidget();
		if(setItems.find(pCurItem) == setItems.end())
		{
			delete listGraphs->item(i);
			--i;
		}
	}
}

std::string FitDlg::GetTableString(QTableWidget* pTable) const
{
	std::ostringstream ostr;

	for(int i=0; i<pTable->rowCount(); ++i)
	{
		QTableWidgetItem* pItemActive = pTable->item(i,2);

		bool bOk = 0;
		bool bActive = pItemActive->text().toInt(&bOk);
		if(!bOk) bActive = 0;
		if(!bActive) continue;

		QTableWidgetItem* pItem1 = pTable->item(i,0);
		std::string strVal1 = pItem1->text().toStdString();
		::trim(strVal1);
		if(strVal1.length() == 0)
		{
			if(bActive)
			{
				pItemActive->setText("0");
				continue;
			}
		}

		QTableWidgetItem* pItem2 = pTable->item(i,1);
		std::string strVal2 = pItem2->text().toStdString();
		::trim(strVal2);
		if(strVal2.length() == 0)
		{
			if(bActive)
			{
				pItemActive->setText("0");
				continue;
			}
		}

		std::string strParam = pTable->verticalHeaderItem(i)->text().toStdString();
		ostr << strParam << "=" << strVal1 << ":" << strVal2 << "; ";
	}

	std::string str = ostr.str();
	str = str.substr(0, str.length()-2);		// remove last "; "
	::trim(str);
	//std::cout << str << std::endl;
	return str;
}

void FitDlg::UpdateHint(const std::string& str, double dVal, double dErr)
{
	for(int iRow=0; iRow<tableHints->rowCount(); ++iRow)
	{
		QTableWidgetItem* pHeader = tableHints->verticalHeaderItem(iRow);
		if(pHeader && pHeader->text().toStdString() == str)
		{
			QTableWidgetItem* pItemVal = tableHints->item(iRow,0);
			QTableWidgetItem* pItemErr = tableHints->item(iRow,1);

			if(pItemVal && pItemErr)
			{
				std::ostringstream ostr;
				ostr << dVal;
				pItemVal->setText(ostr.str().c_str());

				std::ostringstream ostrErr;
				ostrErr << dErr;
				pItemErr->setText(ostrErr.str().c_str());
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

SpecialFitResult FitDlg::DoSpecialFit(SubWindowBase* pSWB, int iFkt)
{
	bool bAssumeErrorIfZero = 1;
	SpecialFitResult res;

	res.bOk = 0;
	res.pPlot = (Plot*)pSWB->ConvertTo1d();
	res.bCreatedNewPlot = (res.pPlot != pSWB);
	if(!res.pPlot)
		return res;

	if(res.pPlot->GetDataCount() == 0)
	{
		res.strErr = "No data found.";
		return res;
	}

	Data1& dat = res.pPlot->GetData(0).dat;
	double *px = vec_to_array<double>(dat.GetX());
	double *py = vec_to_array<double>(dat.GetY());
	double *pyerr = vec_to_array<double>(dat.GetYErr());
	autodeleter<double> _a0(px, 1);
	autodeleter<double> _a1(py, 1);
	autodeleter<double> _a2(pyerr, 1);

	if(bAssumeErrorIfZero)
	{
		double dMaxY = *std::max_element(py, py+dat.GetLength());
		for(unsigned int iErr=0; iErr<dat.GetLength(); ++iErr)
		{
			if(pyerr[iErr] < std::numeric_limits<double>::min())
				pyerr[iErr] = dMaxY * 0.1;
		}
	}


	FunctionModel* pFkt = 0;
	bool bOk = 0;
	if(iFkt == FIT_MIEZE_SINE) 				// MIEZE sine
	{
		if(dat.GetLength() < 2)
		{
			res.strErr = "Too few data points for MIEZE sine fit.";
			return res;
		}

		double dNumOsc = Settings::Get<double>("mieze/num_osc");
		double dFreq = dNumOsc * 2.*M_PI/((px[1]-px[0]) * double(dat.GetLength()));;

		MiezeSinModel *pModel = 0;
		bOk = ::get_mieze_contrast(dFreq, dNumOsc, dat.GetLength(), px, py, pyerr, &pModel);

		std::cout << "C = " << pModel->GetContrast() << " +- " << pModel->GetContrastErr()
						<< ", phase = " << pModel->GetPhase()/M_PI*180. << " +- " << pModel->GetPhaseErr()/M_PI*180.
						<< std::endl;
		pFkt = pModel;
	}
	else if(iFkt == FIT_GAUSSIAN) 		// Gaussian
	{
		GaussModel *pModel = 0;
		bOk = ::get_gauss(dat.GetLength(), px, py, pyerr, &pModel);
		pFkt = pModel;
	}

	if(pFkt)
	{
		res.pPlot->plotfit(*pFkt);
		res.pPlot->repaint();
		res.bOk = 1;

		delete pFkt;
	}

	return res;
}

void FitDlg::DoSpecialFit()
{
	UpdateSourceList();
	const int iFkt = comboSpecialFkt->currentIndex();

	for(int iWnd=0; iWnd<listGraphs->count(); ++iWnd)
	{
		SubWindowBase* pSWB = ((ListGraphsItem*)listGraphs->item(iWnd))->subWnd();
		SpecialFitResult res = DoSpecialFit(pSWB, iFkt);

		if(!res.pPlot)
		{
			delete listGraphs->item(iWnd);
			--iWnd;
			continue;
		}

		if(res.bCreatedNewPlot)
			emit AddSubWindow(res.pPlot);
	}
}

//-----------------------------------------------------------------------------------------------------------------------------

SpecialFitPixelResult FitDlg::DoSpecialFitPixel(SubWindowBase* pSWB, int iFoil, int iFkt)
{
	SpecialFitPixelResult res;
	res.bOk = 0;
	res.pPlot[0] = res.pPlot[1] = 0;

	Plot3d* pPlot3d = pSWB->ConvertTo3d(iFoil);
	bool bCreatedNewPlot = !(pPlot3d==pSWB || pPlot3d==pSWB->GetActualWidget());

	const Data3& dat3 = pPlot3d->GetData();
	const unsigned int iW = dat3.GetWidth(),
			 	 	 	 	 	   iH = dat3.GetHeight();

	Data2 dat2_c(iW, iH),
			 dat2_ph(iW, iH);
	dat2_c.SetZero();
	dat2_ph.SetZero();

	const unsigned int iTCnt = dat3.GetDepth();
	double *px = new double[iTCnt];
	double *py = new double[iTCnt];
	double *pyerr = new double[iTCnt];

	const double dNumOsc = Settings::Get<double>("mieze/num_osc");
	const double dMinCts = 25.;

	Fourier *pFFT = 0;
	if(iFkt == FIT_MIEZE_SINE_PIXELWISE_FFT)
		pFFT = new Fourier(iTCnt);

	for(unsigned int iY=0; iY<iH; ++iY)
	{
		for(unsigned int iX=0; iX<iW; ++iX)
		{
			Data1 dat1 = dat3.GetXY(iX, iY);
			if(dat1.SumY() < dMinCts)
				continue;

			dat1.ToArray<double>(px, py, pyerr);

			double dC=0., dCErr=0., dPh=0., dPhErr=0.;

			if(iFkt == FIT_MIEZE_SINE_PIXELWISE)
			{
				double dThisNumOsc = dNumOsc;
				double dFreq = dThisNumOsc * 2.*M_PI/((px[1]-px[0]) * double(dat1.GetLength()));;

				MiezeSinModel *pModel = 0;
				res.bOk = ::get_mieze_contrast(dFreq, dThisNumOsc, dat1.GetLength(), px, py, pyerr, &pModel);

				dC = pModel->GetContrast();
				dCErr = pModel->GetContrastErr();
				dPh = pModel->GetPhase();
				dPhErr = pModel->GetPhaseErr();

				if(pModel) delete pModel;
			}
			else if(iFkt == FIT_MIEZE_SINE_PIXELWISE_FFT)
			{
				res.bOk = pFFT->get_contrast(dNumOsc, py, dC, dPh);
			}

			if(res.bOk)
			{
				if(isnan(dC) || isinf(dC)) dC = 0.;
				if(isnan(dPh) || isinf(dPh)) dPh = 0.;

				dat2_c.SetVal(iX, iY, dC);
				dat2_c.SetErr(iX, iY, dCErr);
				dat2_ph.SetVal(iX, iY, dPh);
				dat2_ph.SetErr(iX, iY, dPhErr);
			}
		}
	}

	delete[] px;
	delete[] py;
	delete[] pyerr;

	if(pFFT) delete pFFT;

	std::string strTitle = pPlot3d->windowTitle().toStdString();
	strTitle += " -> ";

	res.pPlot[0] = new Plot2d(0, (strTitle + std::string("contrast")).c_str(), 0);
	res.pPlot[1] = new Plot2d(0, (strTitle + std::string("phase")).c_str(), 0);

	res.pPlot[0]->plot(dat2_c);
	res.pPlot[1]->plot(dat2_ph);


	if(bCreatedNewPlot)
		delete pPlot3d;

	return res;
}

void FitDlg::DoSpecialFitPixelwise()
{
	const int iFkt = comboBoxSpecialFktPixel->currentIndex();
	const int iFoil = spinFoil->value();

	for(int iWnd=0; iWnd<listGraphs->count(); ++iWnd)
	{
		SubWindowBase* pSWB = ((ListGraphsItem*)listGraphs->item(iWnd))->subWnd();
		SpecialFitPixelResult res = DoSpecialFitPixel(pSWB, iFoil, iFkt);

		if(!res.pPlot[0])
		{
			delete listGraphs->item(iWnd);
			--iWnd;
			continue;
		}

		emit AddSubWindow(res.pPlot[0]);
		if(!checkOnlyContrast->isChecked())
			emit AddSubWindow(res.pPlot[1]);
		else
		{
			if(res.pPlot[1])
				delete res.pPlot[1];
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------


void FitDlg::DoFit()
{
	bool bAssumeErrorIfZero = 1;
	bool bUpdateHints = 1;

	UpdateSourceList();
	if(tableLimits->rowCount() == 0)
	{
		QMessageBox::critical(this, "Error", "No parameters for fitting given.");
		return;
	}

	std::string strLimits = GetTableString(tableLimits);
	std::string strHints = GetTableString(tableHints);
	const bool bLimits = (strLimits.length()!=0);
	const bool bHints = (strHints.length()!=0);

	const std::string strFkt = editFkt->text().toStdString();

	if(comboFitType->currentIndex() == 0)		// 1d fit
	{
		for(int iWnd=0; iWnd<listGraphs->count(); ++iWnd)
		{
			SubWindowBase* pSWB = ((ListGraphsItem*)listGraphs->item(iWnd))->subWnd();
			Plot* pPlot = (Plot*)pSWB->ConvertTo1d();
			const bool bCreatedNewPlot = (pPlot != pSWB);

			if(!pPlot)
			{
				delete listGraphs->item(iWnd);
				--iWnd;
				continue;
			}

			if(pPlot->GetDataCount() == 0)
			{
				//QMessageBox::critical(this, "Error", "No data found.");
				continue;
			}

			Data1& dat = pPlot->GetData(0).dat;

			double *px = vec_to_array<double>(dat.GetX());
			double *py = vec_to_array<double>(dat.GetY());
			double *pyerr = vec_to_array<double>(dat.GetYErr());
			autodeleter<double> _a0(px, 1);
			autodeleter<double> _a1(py, 1);
			autodeleter<double> _a2(pyerr, 1);

			if(bAssumeErrorIfZero)
			{
				double dMaxY = *std::max_element(py, py+dat.GetLength());
				for(unsigned int iErr=0; iErr<dat.GetLength(); ++iErr)
				{
					if(pyerr[iErr] < std::numeric_limits<double>::min())
						pyerr[iErr] = dMaxY * 0.1;
				}
			}

			std::vector<std::string> vecFittedNames;
			std::vector<double> vecFittedParams;
			std::vector<double> vecFittedErrs;

			FreeFktModel *pModel;
			bool bOk = ::get_freefit(dat.GetLength(), px, py, pyerr,
									strFkt.c_str(), bLimits?strLimits.c_str():0, bHints?strHints.c_str():0,
									vecFittedNames, vecFittedParams, vecFittedErrs,
									&pModel);

			 if(pModel)
			 {
				 if(bUpdateHints)
				 {
					 for(unsigned int iParam=0; iParam<vecFittedNames.size(); ++iParam)
						 UpdateHint(vecFittedNames[iParam], vecFittedParams[iParam], vecFittedErrs[iParam]);
				 }

				pPlot->plotfit(*pModel);
				pPlot->repaint();

				delete pModel;
			 }

			if(bCreatedNewPlot)
				emit AddSubWindow(pPlot);
		}
	}
	else if(comboFitType->currentIndex() == 1)		// 2d fit
	{

	}
}

void FitDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		if(tabs->currentWidget() == tabUser)
			DoFit();
		else if(tabs->currentWidget() == tabSpecial)
		{
			if(radio1D->isChecked())
				DoSpecialFit();
			else if(radio1DPixel->isChecked())
				DoSpecialFitPixelwise();
		}
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

#include "FitDlg.moc"