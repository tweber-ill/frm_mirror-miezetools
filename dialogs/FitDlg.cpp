/*
 * mieze-tool
 * @author tweber
 * @date 10-apr-2013
 */

#include "FitDlg.h"
#include "ListDlg.h"

#include "../fitter/models/freefit.h"

#include "../helper/string.h"
#include "../helper/misc.h"

#include "../plot/plot.h"
#include "../plot/plot2d.h"
#include "../plot/plot3d.h"
#include "../plot/plot4d.h"

#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>

#include <set>
#include <vector>
#include <iostream>
#include <sstream>
#include <limits>

FitDlg::FitDlg(QWidget* pParent, QMdiArea *pmdi) : QDialog(pParent), m_pmdi(pmdi)
{
	this->setupUi(this);

	connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddItemSelected()));
	connect(btnAddActive, SIGNAL(clicked(bool)), this, SLOT(AddActiveItemSelected()));
	connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveItemSelected()));
	connect(editFkt, SIGNAL(textChanged(const QString&)), this, SLOT(FunctionChanged(const QString&)));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	tableLimits->setColumnWidth(2,50);
	tableHints->setColumnWidth(2,50);
}

FitDlg::~FitDlg()
{}

void FitDlg::FunctionChanged(const QString& strFkt)
{
	tableLimits->setRowCount(0);
	tableHints->setRowCount(0);

	Parser parser;
	parser.SetVerbose(false);

	std::vector<Symbol> vecFreeParams;
	Symbol symFree;
	symFree.strIdent = "x";
	vecFreeParams.push_back(symFree);
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
		//pPlot = pPlot->GetActualWidget();

		setItems.insert(pPlot);
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

void FitDlg::DoFit()
{
	bool bAssumeErrorIfZero = 1;

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

			if(bOk && pModel)
			{
				pPlot->plotfit(*pModel);
				pPlot->repaint();
			}
		}
	}
	else if(comboFitType->currentIndex() == 1)		// 1d fit (pixel-wise)
	{

	}
	else if(comboFitType->currentIndex() == 2)		// 2d fit
	{

	}
}

void FitDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole)
	{
			DoFit();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		DoFit();
		QDialog::accept();
	}
}

#include "FitDlg.moc"
