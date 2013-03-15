/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 15-mar-2013
 */

#ifndef __MIEZE_PLOT3D__
#define __MIEZE_PLOT3D__

#include "plot2d.h"

class Plot3d : public Plot2d
{ Q_OBJECT
protected:
	Data3 m_dat3;
	uint m_iCurT;

public:
	Plot3d(QWidget* pParent=0, const char* pcTitle=0, bool bCountData=1);
	virtual ~Plot3d();

	void plot(uint iW, uint iH, uint iT, const double *pdat, const double *perr=0);
	void RefreshTSlice(uint iT);

	virtual SubWindowType GetType() { return PLOT_3D; }
};

#endif
