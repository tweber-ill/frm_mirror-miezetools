/*
 * mieze-tool
 * @author tweber
 * @date 02-apr-2013
 */

#ifndef __MIEZE_COMBINE_DLG__
#define __MIEZE_COMBINE_DLG__

#include <QtGui/QDialog>
#include <list>
#include "../ui/ui_combine_graphs.h"
#include "../subwnd.h"
#include "../plot/plot.h"

class CombineGraphsDlg : public QDialog, Ui::CombineDlg
{Q_OBJECT

protected:
	std::list<SubWindowBase*> m_allSubWnds;

protected slots:
	void AddItemSelected();
	void RemoveItemSelected();

public:
	CombineGraphsDlg(QWidget* pParent);
	virtual ~CombineGraphsDlg();

	void AddAvailSubWnd(SubWindowBase* pSubWnd);

	Plot* CreatePlot(const std::string& strTitle, QWidget* pPlotParent=0) const;
};

#endif
