/*
 * mieze-tool
 * base class for mdi subwindows
 * @author tweber
 * @date 08-mar-2013
 */

#ifndef __MIEZE_SUBWNDBASE__
#define __MIEZE_SUBWNDBASE__

#include <QtGui/QWidget>
#include <string>
#include <iostream>
#include "roi/roi.h"
#include "helper/xml.h"
#include "helper/blob.h"

class Plot;
class Plot2d;
class Plot3d;

enum SubWindowType
{
	PLOT_1D,
	PLOT_2D,
	PLOT_3D,
	PLOT_4D
};

class SubWindowBase : public QWidget
{
Q_OBJECT

signals:
	void SetStatusMsg(const char* pcMsg, int iPos);
	void DataLoaded();

	void WndDestroyed(SubWindowBase *pThis);

public:
	SubWindowBase(QWidget* pParent=0) : QWidget(pParent) {}
	virtual ~SubWindowBase() { emit WndDestroyed(this); }

	virtual SubWindowType GetType() const = 0;
	virtual SubWindowBase* GetActualWidget() { return this; }

	virtual SubWindowBase* clone() const { return 0; }

	virtual double GetTotalCounts() const = 0;

	virtual Plot* ConvertTo1d(int iParam=-1) { return 0; }
	virtual Plot3d* ConvertTo3d(int iParam=-1) { return 0; }

	virtual void SetROI(const Roi* pROI) {}
	virtual Roi* GetROI() { return 0; }

	virtual bool LoadXML(Xml& xml, Blob& blob, const std::string& strBase) { return false; }
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const { return false; }

	virtual void RefreshPlot() {}
};

#endif
