/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 30-sep-2013
 * @license GPLv3
 */

#ifndef __MIEZE_NORM_DLG__
#define __MIEZE_NORM_DLG__

#include <vector>
#include <QtGui/QDialog>
#include "ui/ui_normalize.h"
#include "main/subwnd.h"


class NormDlg : public QDialog, Ui::NormDlg
{Q_OBJECT
protected:
	std::vector<SubWindowBase*> m_allSubWnds;

	Plot* CreatePlot(const std::string& strTitle, QWidget* pPlotParent=0) const;

public:
	NormDlg(QWidget* pParent);
	virtual ~NormDlg();

public slots:
	void SubWindowRemoved(SubWindowBase*);
	void SubWindowAdded(SubWindowBase*);

	void ButtonBoxClicked(QAbstractButton* pBtn);

signals:
	void AddSubWindow(SubWindowBase*);
};


#endif
