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
#include <QtGui/QPixmap>
#include <vector>

#ifdef USE_MGL
	#include <mgl2/base_cf.h>
	#include <mgl2/qt.h>
	#include <mgl2/qmathgl.h>
#endif

#include "../subwnd.h"
#include "../data/data.h"
#include "../fitter/fitter.h"
#include "../helper/string.h"

enum PlotType
{
	PLOT_DATA,
	PLOT_FKT,
};

struct PlotObj
{
	Data1 dat;
	//std::string strName;
	//std::string strFkt;
	PlotType plttype;

	bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const;
	bool LoadXML(Xml& xml, Blob& blob, const std::string& strBase);
};

class Plot : public SubWindowBase
					#ifdef USE_MGL
						, mglDraw
					#endif
{ Q_OBJECT
protected:
	virtual QSize minimumSizeHint() const;
	virtual void paintEvent(QPaintEvent *pEvent);
	virtual void resizeEvent(QResizeEvent *pEvent);
	virtual void RefreshStatusMsgs();

#ifdef USE_MGL
	QMathGL *m_pMGL;
#else
	void MapToCoordSys(double dPixelX, double dPixelY, double &dX, double &dY, bool *pbInside=0);
	virtual void mouseMoveEvent(QMouseEvent* pEvent);
#endif

	QPixmap *m_pPixmap;

	std::vector<PlotObj> m_vecObjs;

	double m_dxmin, m_dxmax, m_dymin, m_dymax;
	void estimate_minmax();

	bool m_bXIsLog, m_bYIsLog;
	QString m_strXAxis, m_strYAxis, m_strTitle;

	StringMap m_mapMerged;

	QColor GetColor(unsigned int iPlotObj) const;

public:
	Plot(QWidget* pParent=0, const char* pcTitle=0);
	virtual ~Plot();

	void plot(unsigned int iNum, const double *px, const double *py, const double *pyerr=0, const double *pdxerr=0, PlotType plttype=PLOT_DATA, const char* pcLegend=0);
	void plot(const Data1& dat, PlotType plttype=PLOT_DATA, const char* pcLegend=0);
	void plot_fkt(const FunctionModel& fkt, int iObj=-1, bool bKeepObj=false);
	void replot_fkts();
	void plot_param(const FunctionModel_param& fkt, int iObj=-1);

	void clear();
	void clearfkt();

#ifdef USE_MGL
	int Draw(mglGraph *pg);
#endif

	void paint();
	virtual void RefreshPlot();

	virtual SubWindowBase* clone() const;

	void SetTitle(const char* pc) { m_strTitle = QString(pc); }
	virtual std::string GetTitle() const { return m_strTitle.toStdString(); }

	unsigned int GetDataCount() const { return m_vecObjs.size(); }
	const PlotObj& GetData(unsigned int iIdx=0) const { return m_vecObjs[iIdx]; }
	PlotObj& GetData(unsigned int iIdx=0) { return m_vecObjs[iIdx]; }

	void MergeParamMaps();
	const StringMap& GetParamMapDynMerged() { return m_mapMerged; }

	virtual const DataInterface* GetDataInterface() const
	{
		if(GetDataCount()>0)
			return &GetData(0).dat;
		return 0;
	}

	void SetLabels(const char* pcX, const char* pcY)
	{
		m_strXAxis = QString(pcX);
		m_strYAxis = QString(pcY);
	}
	std::string GetXLabel() const { return m_strXAxis.toStdString(); }
	std::string GetYLabel() const { return m_strYAxis.toStdString(); }

	virtual std::string GetLabel(LabelType iWhich) const;
	virtual void SetLabel(LabelType iWhich, const char* pcLab);

	void SetXIsLog(bool bLogX) { m_bXIsLog = bLogX; }
	void SetYIsLog(bool bLogY) { m_bYIsLog = bLogY; }

	void SetXLimits(double dXMin, double dXMax) { m_dxmin=dXMin; m_dxmax=dXMax; }
	void SetYLimits(double dYMin, double dYMax) { m_dymin=dYMin; m_dymax=dYMax; }

	virtual SubWindowType GetType() const { return PLOT_1D; }
	virtual double GetTotalCounts() const { return 0.; }
	virtual Plot* ConvertTo1d(int iParam=0) { return (Plot*)this; }

	virtual void SetROI(const Roi* pROI, bool bAntiRoi=0);
	virtual Roi* GetROI(bool bAntiRoi=0);

	virtual bool LoadXML(Xml& xml, Blob& blob, const std::string& strBase);
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const;
};


#endif
