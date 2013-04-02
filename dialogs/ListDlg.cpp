/*
 * mieze-tool
 * @author tweber
 * @date 02-apr-2013
 */

#include "ListDlg.h"


ListGraphsDlg::ListGraphsDlg(QWidget* pParent) : QDialog(pParent)
{
	setupUi(this);
}


ListGraphsDlg::~ListGraphsDlg()
{
}

void ListGraphsDlg::AddSubWnd(SubWindowBase* pSubWnd)
{
	ListGraphsItem* pItem = new ListGraphsItem(listGraphs);

	SubWindowBase* pActualWnd = pSubWnd->GetActualWidget();
	QString strTitle = pActualWnd->windowTitle();
	if(pActualWnd->GetType() == PLOT_1D)
		strTitle += QString(" (1D)");
	else if(pActualWnd->GetType() == PLOT_2D)
		strTitle += QString(" (2D)");
	else if(pActualWnd->GetType() == PLOT_3D)
		strTitle += QString(" (3D)");
	else if(pActualWnd->GetType() == PLOT_4D)
		strTitle += QString(" (4D)");

	pItem->setText(strTitle);
	pItem->setSubWnd(pSubWnd);

	listGraphs->addItem(pItem);
}

std::list<SubWindowBase*> ListGraphsDlg::GetSelectedSubWnds() const
{
	QList<QListWidgetItem*> lstItems = listGraphs->selectedItems();

	std::list<SubWindowBase*> lst;
	for(QListWidgetItem* pItem : lstItems)
	{
		ListGraphsItem* pTheItem = (ListGraphsItem*)pItem;
		lst.push_back(pTheItem->subWnd());
	}

	return lst;
}

#include "ListDlg.moc"
