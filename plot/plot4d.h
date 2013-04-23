/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 15-mar-2013
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

	virtual void RefreshStatusMsgs();

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

	virtual SubWindowType GetType() { return PLOT_4D; }
	virtual double GetTotalCounts() const { return m_dat4.GetTotal(); }

	virtual Plot* ConvertTo1d(int iFoil);
	virtual Plot3d* ConvertTo3d(int iFoil=-1);
};


class Plot4dWrapper : public SubWindowBase
{ Q_OBJECT;
protected:
	Plot4d *m_pPlot;
	QSlider *m_pSliderF, *m_pSliderT;
	QLabel *m_pLabelF, *m_pLabelT;

public:
	Plot4dWrapper(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	virtual ~Plot4dWrapper();

	//operator Plot4d*() { return (Plot4d*)GetActualWidget(); }

	virtual SubWindowType GetType() { return PLOT_4D; }
	virtual SubWindowBase* GetActualWidget() { return m_pPlot; }
	virtual std::string GetTitle() const { return m_pPlot->GetTitle(); }
	virtual double GetTotalCounts() const { return m_pPlot->GetTotalCounts(); }

	virtual Plot* ConvertTo1d(int iFoil) { return m_pPlot->ConvertTo1d(iFoil); }
	virtual Plot3d* ConvertTo3d(int iFoil=-1) { return m_pPlot->ConvertTo3d(iFoil); }

public slots:
	void DataLoaded();
	void SliderValueChanged();
};

#endif
