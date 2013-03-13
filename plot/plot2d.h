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
#include <QtGui/QKeyEvent>
#include <vector>

#include "../subwnd.h"
#include "../data/data.h"

class Plot2d : public SubWindowBase
{ Q_OBJECT
protected:
	virtual QSize	minimumSizeHint () const;
	virtual void paintEvent (QPaintEvent *pEvent);
	virtual void mouseMoveEvent(QMouseEvent* pEvent);
	void RefreshStatusMsgs();

	Data2 m_dat;
	QImage *m_pImg;
	bool m_bLog;

	QString m_strXAxis, m_strYAxis, m_strZAxis, m_strTitle;

	uint GetSpectroColor(double dVal) const;

public:
	Plot2d(QWidget* pParent=0, const char* pcTitle=0);
	virtual ~Plot2d();

	void plot(unsigned int iW, unsigned int iH, const double *pdat);
	void clear();
	void RefreshPlot();

	void SetLog(bool bLog);
	bool GetLog() const;

	void SetTitle(const char* pc) { m_strTitle = QString(pc); }
	void SetLabels(const char* pcX, const char* pcY, const char* pcZ)
	{
		m_strXAxis = QString(pcX);
		m_strYAxis = QString(pcY);
		m_strZAxis = QString(pcZ);
	}

	virtual SubWindowType GetType() { return PLOT_2D; }
};

#endif
