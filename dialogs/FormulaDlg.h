/*
 * mieze-tool
 * @author tweber
 * @date 08-jul-2013
 */

#ifndef __MIEZE_FORMULA_DLG__
#define __MIEZE_FORMULA_DLG__

#include <QtGui/QDialog>
#include <vector>
#include <string>
#include "../ui/ui_formulas.h"


class FormulaDlg : public QDialog, Ui::FormulaDlg
{ Q_OBJECT

protected:
	std::vector<QDoubleSpinBox*> m_vecSpins;
	std::vector<std::string> m_vecSpinNames;

protected slots:
	void CalcMIEZE();
	void CalcNeutronLam();
	void CalcNeutronk();
	void CalcNeutronv();
	void CalcNeutronE();
	void CalcNeutronT();

public:
	FormulaDlg(QWidget* pParent);
	virtual ~FormulaDlg();

public slots:
	virtual void accept();
};


#endif
