/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 06-mar-2013
 */

#ifndef __MIEZE_PLOTTER__
#define __MIEZE_PLOTTER__

#include <QtGui/QWidget>
#include <QtGui/QColor>
#include <vector>

#include "../subwnd.h"
#include "../data/data.h"


class Plot : public SubWindowBase
{ Q_OBJECT
protected:
	virtual QSize	minimumSizeHint () const;
	virtual void paintEvent (QPaintEvent *pEvent);

	std::vector<Data1> m_vecObjs;
	void estimate_minmax(double& dxmin, double& dxmax, double& dymin, double& dymax);
	QColor GetColor(unsigned int iPlotObj);

public:
	Plot(QWidget* pParent=0, const char* pcTitle=0);
	virtual ~Plot();

	void plot(unsigned int iNum, const double *px, const double *py, const double *pyerr=0, const double *pdxerr=0);
	void clear();

	SubWindowType GetType() { return PLOT_1D; }
};


#endif
