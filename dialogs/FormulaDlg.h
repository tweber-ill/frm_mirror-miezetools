/*
 * mieze-tool
 * @author tweber
 * @date 08-jul-2013
 */

#ifndef __MIEZE_FORMULA_DLG__
#define __MIEZE_FORMULA_DLG__

#include <QtGui/QDialog>

#include "../ui/ui_formulas.h"


class FormulaDlg : public QDialog, Ui::FormulaDlg
{ Q_OBJECT

protected:

protected slots:
	void Calc();

public:
	FormulaDlg(QWidget* pParent);
	virtual ~FormulaDlg();

};


#endif
