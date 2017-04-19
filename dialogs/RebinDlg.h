/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 31-oct-2013
 * @license GPLv3
 */

#ifndef __REBIN_DLG_H__
#define __REBIN_DLG_H__

#include <QtGui/QDialog>
#include "../main/subwnd.h"
#include "../ui/ui_rebin.h"


class RebinDlg : public QDialog, Ui::RebinDlg
{Q_OBJECT
protected:
	SubWindowBase *m_pCurPlot;

	void ApplyChanges();

public:
	RebinDlg(QWidget* pParent);
	virtual ~RebinDlg();

public slots:
	void SubWindowActivated(SubWindowBase* pSWB);
	void SubWindowRemoved(SubWindowBase* pSWB);

protected slots:
	void ButtonBoxClicked(QAbstractButton* pBtn);
};


#endif
