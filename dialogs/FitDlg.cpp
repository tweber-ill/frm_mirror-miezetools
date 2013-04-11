/*
 * mieze-tool
 * @author tweber
 * @date 10-apr-2013
 */

#include "FitDlg.h"
#include "ListDlg.h"
#include "../fitter/models/freefit.h"
#include "../helper/string.h"

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

std::string FitDlg::GetHintsString() const
{
	std::ostringstream ostr;

	for(unsigned int i=0; i<tableHints->rowCount(); ++i)
	{

	}

	return ostr.str();
}

std::string FitDlg::GetLimitsString() const
{
	std::ostringstream ostr;


	return ostr.str();
}

void FitDlg::DoFit()
{
	const bool bLimits = checkLimits->isChecked();
	const bool bHints = checkHints->isChecked();

	std::string strLimits, strHints;

	if(bLimits) strLimits = GetLimitsString();
	if(bHints) strHints = GetHintsString();

	const std::string strFkt = editFkt->text().toStdString();

	if(comboFitType->currentIndex() == 0)		// 1d fit
	{
/*		bool get_freefit(unsigned int iLen,
							const double* px, const double* py, const double* pdy,
							const char* pcExp, const char* pcLimits, const char* pcHints,
							std::vector<std::string>& vecFittedNames,
							std::vector<double>& vecFittedParams,
							std::vector<double>& vecFittedErrs,
							FreeFktModel** pFinalModel)*/


	}
	else if(comboFitType->currentIndex() == 1)		// 1d fit (pixel-wise)
	{

	}
	else if(comboFitType->currentIndex() == 2)		// 2d fit
	{

	}
}

void FitDlg::accept()
{
	DoFit();
	QDialog::accept();
}

#include "FitDlg.moc"
