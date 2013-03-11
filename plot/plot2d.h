/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 11-mar-2013
 */

#ifndef __MIEZE_PLOT2D__
#define __MIEZE_PLOT2D__

#include <QtGui/QWidget>
#include <QtGui/QColor>
#include <vector>

#include "../subwnd.h"
#include "../data/data.h"

class Plot2d : public SubWindowBase
{ Q_OBJECT
protected:
	virtual QSize	minimumSizeHint () const;
	virtual void paintEvent (QPaintEvent *pEvent);

	Data2 m_dat;
	QImage *m_pImg;

	uint GetSpectroColor(double dVal) const;

public:
	Plot2d(QWidget* pParent=0, const char* pcTitle=0);
	virtual ~Plot2d();

	void plot(unsigned int iW, unsigned int iH, const double *pdat);
	void clear();
	void RefreshPlot();

	virtual SubWindowType GetType() { return PLOT_2D; }
};

#endif
