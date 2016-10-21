/*
 * mieze-tool
 * @author tweber
 * @date 02-apr-2013
 * @license GPLv3
 */

#ifndef __MIEZE_COMBINE_DLG__
#define __MIEZE_COMBINE_DLG__

#include <QtGui/QDialog>
#include <list>
#include "../ui/ui_combine_graphs.h"
#include "../main/subwnd.h"
#include "../plot/plot.h"

class CombineGraphsDlg : public QDialog, Ui::CombineDlg
{Q_OBJECT

protected:
	std::list<SubWindowBase*> m_allSubWnds;

protected:
	Plot* CreatePlot(const std::string& strTitle, QWidget* pPlotParent=0) const;
	void UpdateCommonParams();

protected slots:
	void AddItemSelected();
	void RemoveItemSelected();

	void TypeChanged();

	void ButtonBoxClicked(QAbstractButton*);

public:
	CombineGraphsDlg(QWidget* pParent);
	virtual ~CombineGraphsDlg();

public slots:
	void SubWindowRemoved(SubWindowBase *pSWB);
	void SubWindowAdded(SubWindowBase *pSWB);

signals:
	void AddSubWindow(SubWindowBase* pWnd);
};

#endif
