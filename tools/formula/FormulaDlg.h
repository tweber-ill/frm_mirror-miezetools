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
#include "ui/ui_formulas.h"
#include "../../plot/plot.h"


class FormulaDlg : public QDialog, Ui::FormulaDlg
{ Q_OBJECT

protected:
	std::vector<QDoubleSpinBox*> m_vecSpins;
	std::vector<std::string> m_vecSpinNames;
	Plot *m_pPlanePlot, *m_pDebyePlot;

	void setupConstants();
	void setupPlanePlotter();
	void setupDebyePlotter();

protected slots:
	void CalcMIEZE();

	void CalcNeutronLam();
	void CalcNeutronk();
	void CalcNeutronv();
	void CalcNeutronE();
	void CalcNeutronT();

	void CalcBraggDirect();
	void CalcBraggReciprocal();

	void CalcPlane();
	void FixedKiKfToggled();
	void PyExport();
	void SetPlaneStatusMsg(const char* pcMsg, int iPos);

	void CalcDebye();
	void PyExportDebye();
	void SetDebyeStatusMsg(const char* pcMsg, int iPos);

	void CalcPowderLines();

public:
	FormulaDlg(QWidget* pParent);
	virtual ~FormulaDlg();

public slots:
	virtual void accept();
};


#endif
