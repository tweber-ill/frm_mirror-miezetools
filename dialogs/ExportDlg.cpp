/*
 * mieze-tool
 * @author tweber
 * @date 25-jul-2013
 */

#include "ExportDlg.h"
#include "ListDlg.h"
#include "../helper/string.h"
#include "../settings.h"
#include "../data/export.h"

#include <set>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

ExportDlg::ExportDlg(QWidget* pParent, QMdiArea* pMdi)
			: QDialog(pParent), m_pmdi(pMdi)
{
	setupUi(this);

	connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddItemSelected()));
	connect(btnAddActive, SIGNAL(clicked(bool)), this, SLOT(AddActiveItemSelected()));
	connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveItemSelected()));

	connect(btnExport, SIGNAL(clicked(bool)), this, SLOT(Export()));
}

ExportDlg::~ExportDlg()
{
}


void ExportDlg::AddActiveItemSelected()
{
	if(!m_pmdi->activeSubWindow()) return;

	SubWindowBase* pWnd = (SubWindowBase*)m_pmdi->activeSubWindow()->widget();
	if(!pWnd || !pWnd->GetActualWidget()) return;

	pWnd = pWnd->GetActualWidget();
	if(pWnd->GetType()!=PLOT_1D && pWnd->GetType()!=PLOT_2D)
	{
		QMessageBox::critical(this, "Error", "Wrong data type, need 1D or 2D.");
		return;
	}

	ListGraphsItem* pItem = new ListGraphsItem(listGraphs);
	pItem->setSubWnd(pWnd);
	pItem->setText(pWnd->windowTitle());

	listGraphs->addItem(pItem);
	RemoveDuplicate();
}

void ExportDlg::AddItemSelected()
{
	ListGraphsDlg dlg(this);

	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow *pItem : lst)
	{
		SubWindowBase *pWnd = (SubWindowBase *) pItem->widget();
		if(pWnd && (pWnd->GetType()==PLOT_1D || pWnd->GetType()==PLOT_2D))
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

void ExportDlg::SubWindowRemoved(SubWindowBase *pSWB)
{
	if(!pSWB) return;

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

void ExportDlg::RemoveItemSelected()
{
	if(listGraphs->selectedItems().size() == 0)
		listGraphs->selectAll();

	for(auto pItem : listGraphs->selectedItems())
		delete pItem;
}

void ExportDlg::RemoveDuplicate()
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


void ExportDlg::Export()
{
	if(listGraphs->count()==0)
	{
		QMessageBox::critical(this, "Error", "No plots selected.");
		return;
	}

	if(listGraphs->count() > spinSubplotH->value()*spinSubplotV->value())
	{
		QMessageBox::critical(this, "Error", "Number of subplots too low.");
		return;
	}


	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir_py", ".").toString();

	QString strFile = QFileDialog::getSaveFileName(this,
					"Save as Python file...", strLastDir,
					"Python files (*.py)", 0,
					QFileDialog::DontUseNativeDialog);
	if(strFile == "")
		return;

	std::string strFile1 = strFile.toStdString();
	std::string strExt = get_fileext(strFile1);
	if(strExt != "py")
		strFile1 += ".py";


	std::vector<SubWindowBase*> vecSWB;
	for(int i=0; i<listGraphs->count(); ++i)
	{
		SubWindowBase* pCurItem = ((ListGraphsItem*)listGraphs->item(i))->subWnd();
		vecSWB.push_back(pCurItem);
	}


	if(export_subplots_py(strFile1.c_str(), vecSWB, spinSubplotH->value(), spinSubplotV->value()))
		pGlobals->setValue("main/lastdir_py", QString(::get_dir(strFile1).c_str()));
	else
		QMessageBox::critical(this, "Error", "Export to Python failed.");

}

#include "ExportDlg.moc"
