/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 02-jul-2013
 * @license GPLv3
 */

#ifndef __RADIAL_INT_DLG__
#define __RADIAL_INT_DLG__

#include <QtGui/QDialog>

#include "../ui/ui_radialint.h"
#include "../main/subwnd.h"
#include "../plot/plot2d.h"
#include "../plot/plot.h"

#include <vector>


class RadialIntDlg : public QDialog, Ui::RadialIntDlg
{ Q_OBJECT

protected:
	Plot* m_pPlot;
	std::vector<Plot2d*> m_vecPlots;

protected slots:
	void AutoCalc();
	void ButtonBoxClicked(QAbstractButton* pBtn);

public:
	RadialIntDlg(QWidget* pParent);
	virtual ~RadialIntDlg();

	void SetSubWindows(std::vector<SubWindowBase*> vecSWB);

public slots:
	void SubWindowRemoved(SubWindowBase *pSWB);
	void SubWindowAdded(SubWindowBase *pSWB);

	void Calc();

signals:
	void NewSubWindow(SubWindowBase* pWnd);
};


#endif
