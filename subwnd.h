/*
 * mieze-tool
 * base class for mdi subwindows
 * @author tweber
 * @date 08-mar-2013
 */

#ifndef __MIEZE_SUBWNDBASE__
#define __MIEZE_SUBWNDBASE__

#include <QtGui/QWidget>

enum SubWindowType
{
	PLOT_1D,
	PLOT_2D,
	PLOT_3D
};

class SubWindowBase : public QWidget
{
Q_OBJECT
public:
	SubWindowBase(QWidget* pParent=0) : QWidget(pParent) {}
	virtual SubWindowType GetType() = 0;

signals:
		void SetStatusMsg(const char* pcMsg, int iPos);
};

#endif
