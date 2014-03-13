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
#include "../helper/string_map.h"


class InfoWidget : public QWidget, public Ui::InfoWidget
{
Q_OBJECT
protected:

public:
	InfoWidget(QWidget* pParent);
	virtual ~InfoWidget();
};


class InfoDock : public QDockWidget
{
Q_OBJECT
protected:
	InfoWidget m_widget;

public:
	InfoDock(QWidget* pParent);
	virtual ~InfoDock();

	// static parameters (instrument specific)
	void SetParamsStat(const StringMap& mapStr);

public slots:
	// dynamic parameters
	void SetParamsDyn(const StringMap& mapStr);
};


#endif
