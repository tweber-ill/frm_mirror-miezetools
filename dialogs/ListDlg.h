/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 02-apr-2013
 * @license GPLv3
 */

#ifndef __MIEZE_LIST_DLG__
#define __MIEZE_LIST_DLG__

#include <QtGui/QDialog>
#include <list>
#include "../ui/ui_list_graphs.h"
#include "../main/subwnd.h"


class ListGraphsItem : public QListWidgetItem
{
	protected:
		SubWindowBase *m_pwnd;

	public:
		ListGraphsItem(QListWidget *pParent) : QListWidgetItem(pParent), m_pwnd(0)
		{}

		void setSubWnd(SubWindowBase *pWnd) { m_pwnd = pWnd; }
		SubWindowBase* subWnd() { return m_pwnd; }
};


class ListGraphsDlg : public QDialog, Ui::SelectGraphDlg
{
	Q_OBJECT

	protected:

	public:
		ListGraphsDlg(QWidget* pParent);
		virtual ~ListGraphsDlg();

		void AddSubWnd(SubWindowBase* pSubWnd);
		std::list<SubWindowBase*> GetSelectedSubWnds() const;
};

#endif
