/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#ifndef __RESO_DLG_H__
#define __RESO_DLG_H__

#include <QtGui/QDialog>
#include "../ui/ui_reso.h"

class ResoDlg : public QDialog, Ui::ResoDlg
{Q_OBJECT
protected:

public:
	ResoDlg(QWidget* pParent);
	virtual ~ResoDlg();

protected slots:
	void UpdateUI();
	void Calc();
};

#endif
