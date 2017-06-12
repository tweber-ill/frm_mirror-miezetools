/**
 * mieze-tool
 * plotter
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 15-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_PLOT3D__
#define __MIEZE_PLOT3D__

#include "plot2d.h"
#include "plot.h"

#include <QtGui/QSlider>
#include <QtGui/QLabel>

class Plot3d : public Plot2d
{ Q_OBJECT
protected:
	Data3 m_dat3;
	uint m_iCurT;

public:
	Plot3d(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	virtual ~Plot3d();

	void plot_manual();
	void plot(uint iW, uint iH, uint iT, const double *pdat, const double *perr=0);
	void plot(const Data3& dat3);
	void RefreshTSlice(uint iT);

	const Data3& GetData() const { return m_dat3; }
	Data3& GetData() { return m_dat3; }
	uint GetCurT() const { return m_iCurT; }

	virtual SubWindowType GetType() const override { return PLOT_3D; }
	virtual double GetTotalCounts() const override { return m_dat3.GetTotal(); }

	virtual Plot* ConvertTo1d(int iParam=0) override;
	virtual Plot2d* ConvertTo2d(int iFoil=-1) override ;
	virtual Plot3d* ConvertTo3d(int iParam=-1) override { return this; }

	virtual SubWindowBase* clone() const override;
	virtual void RefreshStatusMsgs() override;

	virtual void ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight,
		bool bKeepTotalCounts=false) override;

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase) override;
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const override;

	virtual const DataInterface* GetDataInterface() const override { return &m_dat3; }
	virtual DataInterface* GetDataInterface() override { return &m_dat3; }
};


class Plot3dWrapper : public SubWindowBase
{ Q_OBJECT;
protected:
	Plot3d *m_pPlot;
	QSlider *m_pSlider;
	QLabel *m_pLabel;

public:
	Plot3dWrapper(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	Plot3dWrapper(Plot3d* pPlot);
	virtual ~Plot3dWrapper();
	void Init();

	//operator Plot3d*() { return (Plot3d*)GetActualWidget(); }

	virtual SubWindowType GetType() const override { return PLOT_3D; }
	virtual SubWindowBase* GetActualWidget() override { return m_pPlot; }

	virtual std::string GetTitle() const { return m_pPlot->GetTitle(); }
	virtual void SetTitle(const char* pcTitle) { m_pPlot->SetTitle(pcTitle); }
	virtual double GetTotalCounts() const override { return m_pPlot->GetTotalCounts(); }

	virtual std::string GetLabel(LabelType iWhich) const override ;
	virtual void SetLabel(LabelType iWhich, const char* pcLab) override ;

	virtual Plot* ConvertTo1d(int iFoil) override { return m_pPlot->ConvertTo1d(iFoil); }
	virtual Plot2d* ConvertTo2d(int iFoil=-1) override { return m_pPlot->ConvertTo2d(iFoil); }
	virtual Plot3d* ConvertTo3d(int iParam=-1) override { return m_pPlot->ConvertTo3d(iParam); }

	virtual const DataInterface* GetDataInterface() const override { return m_pPlot->GetDataInterface(); }
	virtual DataInterface* GetDataInterface() override { return m_pPlot->GetDataInterface(); }

	virtual void ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight,
		bool bKeepTotalCounts=false) override
	{ m_pPlot->ChangeResolution(iNewWidth, iNewHeight, bKeepTotalCounts); }
	virtual PlotInfo GetPlotInfo() const override { return m_pPlot->GetPlotInfo(); }

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase) override
	{ return m_pPlot->LoadXML(xml, blob, strBase); }
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const override
	{ return m_pPlot->SaveXML(ostr, ostrBlob); }

public slots:
	void DataLoaded();
	void SliderValueChanged();
};

#endif
