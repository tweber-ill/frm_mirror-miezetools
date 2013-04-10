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
		labelStatus->setText("Invalid function.");
		return;
	}
	labelStatus->setText("Function ok.");

	const std::vector<Symbol>&syms = parser.GetSymbols();
	for(const Symbol& sym : syms)
	{

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

void FitDlg::DoFit()
{

}

void FitDlg::accept()
{
	DoFit();
	QDialog::accept();
}

#include "FitDlg.moc"
