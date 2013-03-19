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
#include <QtGui/QKeyEvent>
#include <vector>

#include "../subwnd.h"
#include "../data/data.h"


class Plot : public SubWindowBase
{ Q_OBJECT
protected:
	virtual QSize	minimumSizeHint () const;
	virtual void paintEvent (QPaintEvent *pEvent);
	virtual void mouseMoveEvent(QMouseEvent* pEvent);
	virtual void RefreshStatusMsgs();

	std::vector<Data1> m_vecObjs;

	double m_dxmin, m_dxmax, m_dymin, m_dymax;
	void estimate_minmax();

	QString m_strXAxis, m_strYAxis, m_strTitle;

	QColor GetColor(unsigned int iPlotObj);

public:
	Plot(QWidget* pParent=0, const char* pcTitle=0);
	virtual ~Plot();

	void plot(unsigned int iNum, const double *px, const double *py, const double *pyerr=0, const double *pdxerr=0);
	void clear();

	void SetTitle(const char* pc) { m_strTitle = QString(pc); }
	void SetLabels(const char* pcX, const char* pcY)
	{
		m_strXAxis = QString(pcX);
		m_strYAxis = QString(pcY);
	}

	virtual SubWindowType GetType() { return PLOT_1D; }
};


#endif
