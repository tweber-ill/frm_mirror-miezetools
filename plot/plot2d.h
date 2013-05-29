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
#include "../roi/roi.h"

class Plot2d : public SubWindowBase
{ Q_OBJECT
protected:
	virtual QSize	minimumSizeHint () const;
	virtual void paintEvent (QPaintEvent *pEvent);
	virtual void mouseMoveEvent(QMouseEvent* pEvent);
	virtual void RefreshStatusMsgs();

	Data2 m_dat;
	QImage *m_pImg;
	bool m_bLog;
	bool m_bCountData;
	bool m_bCyclicData;

	QRect m_rectImage, m_rectCB;
	QString m_strXAxis, m_strYAxis, m_strZAxis, m_strTitle;

	uint GetSpectroColor(double dVal) const;
	uint GetSpectroColor01(double dVal) const;

public:
	Plot2d(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1, bool bCyclicData=0);
	virtual ~Plot2d();

	const Data2& GetData2() const { return m_dat; }
	Data2& GetData2() { return m_dat; }

	void plot(unsigned int iW, unsigned int iH, const double *pdat, const double *perr=0);
	void plot(const Data2& dat);
	void clear();
	virtual void RefreshPlot();

	void SetLog(bool bLog);
	bool GetLog() const;

	void SetTitle(const char* pc) { m_strTitle = QString(pc); }
	virtual std::string GetTitle() const { return m_strTitle.toStdString(); }

	void SetLabels(const char* pcX, const char* pcY, const char* pcZ)
	{
		m_strXAxis = QString(pcX);
		m_strYAxis = QString(pcY);
		m_strZAxis = QString(pcZ);
	}

	const QString& GetZStr() const { return m_strZAxis; }

	virtual SubWindowType GetType() { return PLOT_2D; }
	virtual double GetTotalCounts() const { return m_dat.GetTotal(); }

protected:
	virtual DataInterface* GetInternalData() { return &m_dat; }

public:
	void SetGlobalROI(const Roi* pROI, const bool* pbROIActive);
};

#endif
