/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 15-mar-2013
 */
#include "plot3d.h"


Plot3d::Plot3d(QWidget* pParent, const char* pcTitle,  bool bCountData)
		: Plot2d(pParent, pcTitle, bCountData), m_iCurT(0)
{
	this->m_bLog = false;
}

Plot3d::~Plot3d()
{}

void Plot3d::plot(uint iW, uint iH, uint iT, const double *pdat, const double *perr)
{
	m_dat3.SetSize(iW, iH, iT);
	m_dat3.SetVals(pdat, perr);
	RefreshTSlice(0);
}

void Plot3d::RefreshTSlice(uint iT)
{
	m_iCurT = iT;
	m_dat = m_dat3.GetVal(iT);
	RefreshPlot();
}

#include "plot3d.moc"
