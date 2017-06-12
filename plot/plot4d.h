/**
 * mieze-tool
 * plotter
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 15-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_PLOT4D__
#define __MIEZE_PLOT4D__

#include "plot2d.h"
#include "plot3d.h"
#include "plot.h"

#include <QtGui/QSlider>
#include <QtGui/QLabel>

class Plot4d : public Plot2d
{ Q_OBJECT
protected:
	Data4 m_dat4;
	uint m_iCurT, m_iCurF;

public:
	Plot4d(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	virtual ~Plot4d();

	void plot_manual();
	void plot(uint iW, uint iH, uint iT, uint iF, const double *pdat, const double *perr=0);
	void RefreshTFSlice(uint iT, uint iF);

	const Data4& GetData() const { return m_dat4; }
	Data4& GetData() { return m_dat4; }
	uint GetCurT() const { return m_iCurT; }
	uint GetCurF() const { return m_iCurF; }
	uint GetNumT() const { return m_dat4.GetDepth(); }
	uint GetNumF() const { return m_dat4.GetDepth2(); }

	virtual SubWindowType GetType() const override { return PLOT_4D; }
	virtual double GetTotalCounts() const override { return m_dat4.GetTotal(); }

	virtual Plot* ConvertTo1d(int iFoil) override;
	virtual Plot2d* ConvertTo2d(int iFoil=-1) override;
	virtual Plot3d* ConvertTo3d(int iFoil=-1) override;

	virtual SubWindowBase* clone() const override;
	virtual void RefreshStatusMsgs() override;

	virtual void ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight,
		bool bKeepTotalCounts=false) override;

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase) override;
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const override;

	virtual const DataInterface* GetDataInterface() const override { return &m_dat4; }
	virtual DataInterface* GetDataInterface() override { return &m_dat4; }
};


class Plot4dWrapper : public SubWindowBase
{ Q_OBJECT;
protected:
	Plot4d *m_pPlot;
	QSlider *m_pSliderF, *m_pSliderT;
	QLabel *m_pLabelF, *m_pLabelT;

public:
	Plot4dWrapper(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	Plot4dWrapper(Plot4d* pPlot);
	virtual ~Plot4dWrapper();
	void Init();

	//operator Plot4d*() { return (Plot4d*)GetActualWidget(); }

	virtual SubWindowType GetType() const override { return PLOT_4D; }
	virtual SubWindowBase* GetActualWidget() override { return m_pPlot; }
	virtual std::string GetTitle() const { return m_pPlot->GetTitle(); }
	virtual void SetTitle(const char* pcTitle) { m_pPlot->SetTitle(pcTitle); }
	virtual double GetTotalCounts() const override { return m_pPlot->GetTotalCounts(); }

	virtual std::string GetLabel(LabelType iWhich) const override;
	virtual void SetLabel(LabelType iWhich, const char* pcLab) override;

	virtual Plot* ConvertTo1d(int iFoil=-1) override { return m_pPlot->ConvertTo1d(iFoil); }
	virtual Plot2d* ConvertTo2d(int iFoil=-1) override { return m_pPlot->ConvertTo2d(iFoil); }
	virtual Plot3d* ConvertTo3d(int iFoil=-1) override { return m_pPlot->ConvertTo3d(iFoil); }

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
