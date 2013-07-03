/*
 * mieze-tool
 * @author tweber
 * @date 02-jul-2013
 */

#ifndef __RADIAL_INT_DLG__
#define __RADIAL_INT_DLG__

#include <QtGui/QDialog>

#include "../ui/ui_radialint.h"
#include "../subwnd.h"
#include "../plot/plot2d.h"

#include <vector>


class RadialIntDlg : public QDialog, Ui::RadialIntDlg
{ Q_OBJECT

protected:
	std::vector<Plot2d*> m_vecPlots;

public:
	RadialIntDlg(QWidget* pParent);
	virtual ~RadialIntDlg();

	void SetSubWindows(std::vector<SubWindowBase*> vecSWB);

public slots:
	void SubWindowRemoved(SubWindowBase *pSWB);
	void SubWindowAdded(SubWindowBase *pSWB);

	void Calc();
};


#endif
