/*
 * mieze-tool
 * @author tweber
 * @date 04-mar-2013
 */

#include <QtCore/QMetaType>
#include <QtGui/QApplication>
#include <iostream>
#include <string>

#ifdef Q_WS_X11
//#include <X11/Xlib.h>
extern "C" int XInitThreads();
#endif

#include "mainwnd.h"
#include "settings.h"

#include "dialogs/SettingsDlg.h"

extern void init_formulas();

static inline void load_files(MiezeMainWnd& wnd, int iNum, char **pcFiles)
{
	for(int iFile=0; iFile<iNum; ++iFile)
		wnd.LoadFile(std::string(pcFiles[iFile]));
}

int main(int argc, char **argv)
{
#ifdef Q_WS_X11
	XInitThreads();
#endif
	QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

	init_formulas();

	int iRet = -1;
	try
	{
		QSettings *pGlobals = Settings::GetGlobals();
		QApplication a(argc, argv);

		MiezeMainWnd wnd;
		SettingsDlg::set_global_defaults();

		wnd.restoreGeometry(pGlobals->value("main/geo").toByteArray());
		wnd.show();

		if(argc>1)
			load_files(wnd, argc-1, argv+1);

		iRet = a.exec();
		pGlobals->setValue("main/geo", wnd.saveGeometry());
	}
	catch(const std::exception& ex)
	{
		std::cerr << "\n********************************************************************************\n"
					 << "CRITICAL ERROR: " << ex.what()
					 << "\n********************************************************************************\n"
					 << std::endl;
	}

	Settings::free();
	return iRet;
}
