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
#include <set>
#include <vector>
#include <iostream>
#include <sstream>

FitDlg::FitDlg(QWidget* pParent, QMdiArea *pmdi) : QDialog(pParent), m_pmdi(pmdi)
{
	this->setupUi(this);

	connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddItemSelected()));
	connect(btnAddActive, SIGNAL(clicked(bool)), this, SLOT(AddActiveItemSelected()));
	connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveItemSelected()));
	connect(editFkt, SIGNAL(textChanged(const QString&)), this, SLOT(FunctionChanged(const QString&)));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
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


        QTableWidgetItem *pItemHints = new QTableWidgetItem();
        pItemHints->setText(sym.strIdent.c_str());
        tableHints->setVerticalHeaderItem(iSym, pItemHints);

        QTableWidgetItem *pItemHint = new QTableWidgetItem();
        tableHints->setItem(iSym,0,pItemHint);

        QTableWidgetItem *pItemDelta = new QTableWidgetItem();
        tableHints->setItem(iSym,1,pItemDelta);
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

	for(int i=0; i<tableHints->rowCount(); ++i)
	{
		bool bOk = 0;
		QTableWidgetItem* pHint = tableHints->item(i,0);
		double dHint = pHint->text().toDouble(&bOk);
		if(!bOk)
		{
			dHint = 0.;
			pHint->setText("0");
		}

		QTableWidgetItem* pDelta = tableHints->item(i,1);
		double dDelta = pDelta->text().toDouble(&bOk);
		if(!bOk)
		{
			dDelta = 0.;
			pDelta->setText("0");
		}

		std::string strParam = tableHints->verticalHeaderItem(i)->text().toStdString();

		ostr << strParam << "=" << dHint << ":" << dDelta;
		if(i<tableHints->rowCount()-1) ostr << "; ";
	}

	std::cout << ostr.str() << std::endl;
	return ostr.str();
}

void FitDlg::DoFit()
{
	UpdateSourceList();

	const bool bLimits = checkLimits->isChecked();
	const bool bHints = checkHints->isChecked();

	std::string strLimits, strHints;
	if(bLimits) strLimits = GetTableString(tableLimits);
	if(bHints) strHints = GetTableString(tableHints);

	const std::string strFkt = editFkt->text().toStdString();

	if(comboFitType->currentIndex() == 0)		// 1d fit
	{
		for(int iWnd=0; iWnd<listGraphs->count(); ++iWnd)
		{
			SubWindowBase* pSWB = ((ListGraphsItem*)listGraphs->item(iWnd))->subWnd();
			Plot* pPlot = (Plot*)pSWB->ConvertTo1d();

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
