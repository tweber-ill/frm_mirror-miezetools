/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 22-jul-2013
 * @license GPLv3
 */

#ifndef __PLOT_PROP_DLG_H__
#define  __PLOT_PROP_DLG_H__

#include <QtGui/QDialog>
#include <QtGui/QWidget>
#include "ui/ui_plot_properties.h"

#include "main/subwnd.h"


class PlotPropDlg : public QDialog, Ui::PlotPropertiesDlg
{ Q_OBJECT
protected:
	SubWindowBase *m_pCurPlot;

public:
	PlotPropDlg(QWidget* pParent);
	virtual ~PlotPropDlg();

protected:
	void SaveSettings();

public slots:
	void SubWindowActivated(SubWindowBase* pSWB);
	void SubWindowRemoved(SubWindowBase* pSWB);

protected slots:
	void ButtonBoxClicked(QAbstractButton* pBtn);
};

#endif
