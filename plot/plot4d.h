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

class Plot4d : public Plot2d
{ Q_OBJECT
protected:
	Data4 m_dat4;
	uint m_iCurT, m_iCurF;

public:
	Plot4d(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	virtual ~Plot4d();

	void plot(uint iW, uint iH, uint iT, uint iF, const double *pdat, const double *perr=0);
	void RefreshTFSlice(uint iT, uint iF);

	const Data4& GetData() const { return m_dat4; }
	uint GetCurT() const { return m_iCurT; }
	uint GetCurF() const { return m_iCurF; }

	virtual SubWindowType GetType() { return PLOT_4D; }
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

public slots:
	void DataLoaded();
	void SliderValueChanged();
};

#endif
