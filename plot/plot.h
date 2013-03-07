/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 06-mar-2013
 */

#include <QtGui/QWidget>
#include <QtGui/QColor>
#include <vector>

#ifndef __MIEZE_PLOTTER__
#define __MIEZE_PLOTTER__


struct PlotObj
{
	std::vector<QPointF> coord;
	std::vector<QPointF> err;
};

class Plot : public QWidget
{ Q_OBJECT
protected:
	virtual QSize	minimumSizeHint () const;
	virtual void paintEvent (QPaintEvent *pEvent);

	std::vector<PlotObj> m_vecObjs;
	void estimate_minmax(double& dxmin, double& dxmax, double& dymin, double& dymax);
	QColor GetColor(unsigned int iPlotObj);

public:
	Plot(QWidget* pParent=0, const char* pcTitle=0);
	virtual ~Plot();

	void plot(unsigned int iNum, const double *px, const double *py, const double *pyerr=0, const double *pdxerr=0);
	void clear();
};


#endif
