/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 15-mar-2013
 */

#ifndef __MIEZE_PLOT3D__
#define __MIEZE_PLOT3D__

#include "plot2d.h"
#include <QtGui/QSlider>
#include <QtGui/QLabel>

class Plot3d : public Plot2d
{ Q_OBJECT
protected:
	Data3 m_dat3;
	uint m_iCurT;

	virtual void RefreshStatusMsgs();

public:
	Plot3d(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	virtual ~Plot3d();

	void plot_manual();
	void plot(uint iW, uint iH, uint iT, const double *pdat, const double *perr=0);
	void RefreshTSlice(uint iT);

	const Data3& GetData() const { return m_dat3; }
	Data3& GetData() { return m_dat3; }
	uint GetCurT() const { return m_iCurT; }

	virtual SubWindowType GetType() { return PLOT_3D; }
	virtual double GetTotalCounts() const { return m_dat3.GetTotal(); }
};


class Plot3dWrapper : public SubWindowBase
{ Q_OBJECT;
protected:
	Plot3d *m_pPlot;
	QSlider *m_pSlider;
	QLabel *m_pLabel;

public:
	Plot3dWrapper(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	virtual ~Plot3dWrapper();

	//operator Plot3d*() { return (Plot3d*)GetActualWidget(); }

	virtual SubWindowType GetType() { return PLOT_3D; }
	virtual SubWindowBase* GetActualWidget() { return m_pPlot; }

	virtual std::string GetTitle() const { return m_pPlot->GetTitle(); }
	virtual double GetTotalCounts() const { return m_pPlot->GetTotalCounts(); }

public slots:
	void DataLoaded();
	void SliderValueChanged();
};

#endif
