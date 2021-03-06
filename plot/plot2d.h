/**
 * mieze-tool
 * plotter
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 11-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_PLOT2D__
#define __MIEZE_PLOT2D__

#include <QtGui/QWidget>
#include <QtGui/QColor>
#include <QtGui/QKeyEvent>
#include <vector>

#include "main/subwnd.h"
#include "data/data.h"
#include "roi/roi.h"


class Plot2d : public SubWindowBase
{ Q_OBJECT
protected:
	virtual QSize minimumSizeHint () const override;
	virtual void paintEvent(QPaintEvent *pEvent) override;
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void RefreshStatusMsgs();

	virtual void mouseMoveEvent(QMouseEvent* pEvent) override;
	uint GetSpectroColor(double dVal) const;
	uint GetSpectroColor01(double dVal) const;

	Data2 m_dat;
	QImage *m_pImg;

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

	virtual SubWindowBase* clone() const override;

	void plot(unsigned int iW, unsigned int iH, const double *pdat, const double *perr=0);
	void plot(const Data2& dat);
	void clear();
	virtual void RefreshPlot() override;

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

	virtual std::string GetLabel(LabelType iWhich) const override;
	virtual void SetLabel(LabelType iWhich, const char* pcLab) override;

	virtual SubWindowType GetType() const override { return PLOT_2D; }
	virtual double GetTotalCounts() const override { return m_dat.GetTotal(); }

	bool IsCountData() const { return m_bCountData; }
	bool IsCyclicData() const { return m_bCyclicData; }
	bool IsPhaseData() const { return m_bPhaseData; }

	void CheckCyclicData();

	virtual void ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight,
		bool bKeepTotalCounts=false) override;
	virtual PlotInfo GetPlotInfo() const override;

	virtual const DataInterface* GetDataInterface() const override { return &m_dat; }
	virtual DataInterface* GetDataInterface() override { return &m_dat; }

public:
	virtual void SetROI(const Roi* pROI, bool bAntiRoi=0) override;
	virtual Roi* GetROI(bool bAntiRoi=0) override;

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase) override;
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const override;
};

#endif
