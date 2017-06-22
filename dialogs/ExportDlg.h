/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 25-jul-2013
 * @license GPLv3
 */

#ifndef __EXPORT_DLG_H__
#define __EXPORT_DLG_H__

#include <QtGui/QDialog>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>

#include "ui/ui_export.h"
#include "main/subwnd.h"


class ExportDlg : public QDialog, Ui::ExportDlg
{Q_OBJECT
protected:
	QMdiArea *m_pmdi;

public:
	ExportDlg(QWidget* pParent, QMdiArea* pMdi);
	virtual ~ExportDlg();

protected slots:
	void AddItemSelected();
	void AddActiveItemSelected();
	void RemoveItemSelected();

	void Export();

protected:
	void RemoveDuplicate();

public slots:
	void SubWindowRemoved(SubWindowBase *pSWB);
};

#endif
