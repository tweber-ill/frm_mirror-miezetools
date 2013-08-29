/*
 * mieze-tool
 * @author tweber
 * @date 29-aug-2013
 */

#ifndef __MIEZE_INFODOCK__
#define __MIEZE_INFODOCK__

#include <QtGui/QWidget>
#include <QtGui/QDockWidget>

#include <map>
#include <string>

#include "../ui/ui_infos.h"
#include "../helper/string.h"


class InfoWidget : public QWidget, public Ui::InfoWidget
{
protected:

public:
	InfoWidget(QWidget* pParent);
	virtual ~InfoWidget();
};


class InfoDock : public QDockWidget
{
protected:
	InfoWidget m_widget;

public:
	InfoDock(QWidget* pParent);
	virtual ~InfoDock();

	void SetMiscParams(const StringMap& mapStr);
};


#endif
