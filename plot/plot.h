/**
 * mieze-tool
 * plotter
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 06-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_PLOTTER__
#define __MIEZE_PLOTTER__

#include <QtGui/QWidget>
#include <QtGui/QColor>
#include <QtGui/QKeyEvent>
#include <QtGui/QPixmap>
#include <vector>

#ifdef USE_GPL
	#include <QtGnuplotInstance.h>
	#include <QtGnuplotWidget.h>
#endif

#include "main/subwnd.h"
#include "data/data.h"
#include "tlibs/fit/minuit.h"
#include "tlibs/string/string.h"

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
	bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase);
};

class Plot : public SubWindowBase
{ Q_OBJECT
protected slots:
	void Coord_Output(const QString&);

protected:
	virtual QSize minimumSizeHint() const override;
	virtual void paintEvent(QPaintEvent *pEvent) override;
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void RefreshStatusMsgs();

#ifdef USE_GPL
	QtGnuplotInstance *m_pGPLInst;
	QtGnuplotWidget *m_pGPLWidget;

	void GPL_Init();
	void GPL_Deinit();
	void GPL_Draw();
#else
	void MapToCoordSys(double dPixelX, double dPixelY, double &dX, double &dY, bool *pbInside=0);
	virtual void mouseMoveEvent(QMouseEvent* pEvent) override;
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
	void plot_fkt(const tl::FitterFuncModel<double>& fkt, int iObj=-1, bool bKeepObj=false);
	void replot_fkts();
	void plot_param(const tl::FunctionModel_param<>& fkt, int iObj=-1);

	void clear();
	void clearfkt();

	void paint();
	virtual void RefreshPlot() override;

	virtual SubWindowBase* clone() const override;

	void SetTitle(const char* pc) { m_strTitle = QString(pc); }
	virtual std::string GetTitle() const { return m_strTitle.toStdString(); }

	unsigned int GetDataCount() const { return m_vecObjs.size(); }
	const PlotObj& GetData(unsigned int iIdx=0) const { return m_vecObjs[iIdx]; }
	PlotObj& GetData(unsigned int iIdx=0) { return m_vecObjs[iIdx]; }

	void MergeParamMaps();
	const StringMap& GetParamMapDynMerged() { return m_mapMerged; }

	virtual const DataInterface* GetDataInterface() const override
	{
		if(GetDataCount()>0)
			return &GetData(0).dat;
		return 0;
	}

	virtual DataInterface* GetDataInterface() override
	{
		return const_cast<DataInterface*>(const_cast<const Plot*>(this)->GetDataInterface());
	}

	void SetLabels(const char* pcX, const char* pcY)
	{
		m_strXAxis = QString(pcX);
		m_strYAxis = QString(pcY);
	}
	std::string GetXLabel() const { return m_strXAxis.toStdString(); }
	std::string GetYLabel() const { return m_strYAxis.toStdString(); }

	virtual std::string GetLabel(LabelType iWhich) const override;
	virtual void SetLabel(LabelType iWhich, const char* pcLab) override;

	void SetXIsLog(bool bLogX) { m_bXIsLog = bLogX; }
	void SetYIsLog(bool bLogY) { m_bYIsLog = bLogY; }

	void SetXLimits(double dXMin, double dXMax) { m_dxmin=dXMin; m_dxmax=dXMax; }
	void SetYLimits(double dYMin, double dYMax) { m_dymin=dYMin; m_dymax=dYMax; }

	virtual SubWindowType GetType() const override { return PLOT_1D; }
	virtual double GetTotalCounts() const override { return 0.; }
	virtual Plot* ConvertTo1d(int iParam=0) override { return (Plot*)this; }

	virtual void SetROI(const Roi* pROI, bool bAntiRoi=0) override;
	virtual Roi* GetROI(bool bAntiRoi=0) override;

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase) override;
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const override;

	virtual void SaveImageAs() const override;
};


#endif
