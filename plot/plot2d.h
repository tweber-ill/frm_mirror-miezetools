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

#ifdef USE_MGL
	#include <mgl2/base_cf.h>
	#include <mgl2/qt.h>
	#include <mgl2/qmathgl.h>
#endif


#include "../subwnd.h"
#include "../data/data.h"
#include "../roi/roi.h"

class Plot2d : public SubWindowBase
					#ifdef USE_MGL
						, mglDraw
					#endif
{ Q_OBJECT
protected:
	virtual QSize	minimumSizeHint () const;
	virtual void paintEvent (QPaintEvent *pEvent);
	virtual void resizeEvent(QResizeEvent *pEvent);
	virtual void RefreshStatusMsgs();

#ifdef USE_MGL
	QMathGL *m_pMGL;
	void CreateMGL();
#else
	virtual void mouseMoveEvent(QMouseEvent* pEvent);
	uint GetSpectroColor(double dVal) const;
	uint GetSpectroColor01(double dVal) const;
#endif

	Data2 m_dat;
#ifdef USE_MGL
	mglData *m_pImg;
#else
	QImage *m_pImg;
#endif
	bool m_bLog;
	bool m_bCountData;
	bool m_bCyclicData;
	bool m_bPhaseData;

	QRect m_rectImage, m_rectCB;
	QString m_strXAxis, m_strYAxis, m_strZAxis, m_strTitle;

public:
	Plot2d(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1, bool bPhaseData=0);
	Plot2d(const Plot2d& plot);
	virtual ~Plot2d();

	const Data2& GetData2() const { return m_dat; }
	Data2& GetData2() { return m_dat; }

	virtual SubWindowBase* clone() const;

	void plot(unsigned int iW, unsigned int iH, const double *pdat, const double *perr=0);
	void plot(const Data2& dat);
	void clear();
	virtual void RefreshPlot();

#ifdef USE_MGL
	int Draw(mglGraph *pg);
#endif

	void SetLog(bool bLog);
	bool GetLog() const;

	void SetTitle(const char* pc) { m_strTitle = QString(pc); }
	virtual std::string GetTitle() const { return m_strTitle.toStdString(); }

	void SetLabels(const char* pcX, const char* pcY, const char* pcZ=0)
	{
		if(pcX) m_strXAxis = QString(pcX);
		if(pcY) m_strYAxis = QString(pcY);
		if(pcZ) m_strZAxis = QString(pcZ);
	}

	const QString& GetXStr() const { return m_strXAxis; }
	const QString& GetYStr() const { return m_strYAxis; }
	const QString& GetZStr() const { return m_strZAxis; }

	virtual std::string GetLabel(LabelType iWhich) const;
	virtual void SetLabel(LabelType iWhich, const char* pcLab);

	virtual SubWindowType GetType() const { return PLOT_2D; }
	virtual double GetTotalCounts() const { return m_dat.GetTotal(); }

	bool IsCountData() const { return m_bCountData; }
	bool IsCyclicData() const { return m_bCyclicData; }
	bool IsPhaseData() const { return m_bPhaseData; }

	void CheckCyclicData();

	virtual void ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts=false);
	virtual PlotInfo GetPlotInfo() const;

	virtual const DataInterface* GetDataInterface() const { return &m_dat; }
	virtual DataInterface* GetDataInterface() { return &m_dat; }

public:
	virtual void SetROI(const Roi* pROI, bool bAntiRoi=0);
	virtual Roi* GetROI(bool bAntiRoi=0);

	virtual bool LoadXML(Xml& xml, Blob& blob, const std::string& strBase);
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const;
};

#endif
