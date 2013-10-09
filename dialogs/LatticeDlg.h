/*
 * Lattice Dialog
 * @author tweber
 * @date 09-oct-2013
 */

#ifndef __MIEZE_LATTICE__
#define __MIEZE_LATTICE__

#include <QtGui/QDialog>
#include "../ui/ui_lattice.h"

#include <vector>


class LatticeDlg : public QDialog, Ui::LatticeDlg
{Q_OBJECT
protected:
	std::vector<QLineEdit*> m_vecEditReal;
	std::vector<QLineEdit*> m_vecEditRecip;

protected slots:
	void CalcRecip();
	void CalcReal();

public:
	LatticeDlg(QWidget* pParent);
	virtual ~LatticeDlg();
};


#endif
