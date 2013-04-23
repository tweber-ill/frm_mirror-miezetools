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
public:
	SubWindowBase(QWidget* pParent=0) : QWidget(pParent) {}
	virtual SubWindowType GetType() = 0;
	virtual SubWindowBase* GetActualWidget() { return this; }

	//virtual std::string GetTitle() const = 0;
	virtual double GetTotalCounts() const = 0;

	virtual Plot* ConvertTo1d(int iParam=-1) { return 0; }
	virtual Plot3d* ConvertTo3d(int iParam=-1) { return 0; }

signals:
		void SetStatusMsg(const char* pcMsg, int iPos);
		void DataLoaded();
};

#endif
