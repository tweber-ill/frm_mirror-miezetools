/*
 * mieze-tool
 * base class for mdi subwindows
 * @author tweber
 * @date 08-mar-2013
 */

#ifndef __MIEZE_SUBWNDBASE__
#define __MIEZE_SUBWNDBASE__

enum SubWindowType
{
	PLOT_1D,
	PLOT_2D
};

class SubWindowBase : public QWidget
{
public:
	SubWindowBase(QWidget* pParent=0) : QWidget(pParent) {}
	virtual SubWindowType GetType() = 0;
};

#endif
