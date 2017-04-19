/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 04-mar-2013
 * @license GPLv3
 */

#include <QtCore/QMetaType>
#include <QtGui/QApplication>
//#include <QtGui/QCleanlooksStyle>
#include <QtOpenGL/QGLWidget>
#include <iostream>
#include <fstream>
#include <string>

#include "../tlibs/string/string.h"
#include "../tlibs/string/spec_char.h"
#include "../tlibs/math/rand.h"
#include "../tlibs/log/log.h"

#ifdef Q_WS_X11
//#include <X11/Xlib.h>
extern "C" int XInitThreads();
#endif

#include "mainwnd.h"
#include "settings.h"

#include "../dialogs/SettingsDlg.h"

extern void init_formulas();

static inline void load_files(MiezeMainWnd& wnd, int iNum, char **pcFiles)
{
	for(int iFile=0; iFile<iNum; ++iFile)
	{
		std::string strFile = pcFiles[iFile];
		std::string strExt = tl::get_fileext(strFile);

		if(strExt == "cattus")
			wnd.LoadSession(strFile);
		else
			wnd.LoadFile(strFile);
	}
}


int main(int argc, char **argv)
{
	try
	{
		tl::log_info("Starting up Cattus.");

/*
#ifdef Q_WS_X11
		XInitThreads();
#endif
		QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);
*/

		int iRet = -1;
		tl::init_rand();
		tl::init_spec_chars();
		init_formulas();

		QSettings *pGlobals = Settings::GetGlobals();
		QApplication a(argc, argv);
		//std::cout << a.type() << std::endl;
		//a.setStyle(new QCleanlooksStyle);

		::setlocale(LC_ALL, "C");
		QLocale::setDefault(QLocale::English);

		MiezeMainWnd wnd;
		SettingsDlg::set_global_defaults();

		wnd.restoreGeometry(pGlobals->value("main/geo").toByteArray());
		wnd.show();

		if(argc>1)
			load_files(wnd, argc-1, argv+1);

		iRet = a.exec();
		pGlobals->setValue("main/geo", wnd.saveGeometry());

		Settings::free();

		tl::log_info("Shutting down Cattus.");
		return iRet;
	}
	catch(const std::exception& ex)
	{
		tl::log_crit(ex.what());
	}

	return -1;
}
