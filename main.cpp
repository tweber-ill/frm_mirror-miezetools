/*
 * mieze-tool
 * @author tweber
 * @date 04-mar-2013
 */

#include <QtGui/QApplication>
#include <iostream>
#include <string>

#include "mainwnd.h"
#include "settings.h"

static inline void load_files(MiezeMainWnd& wnd, int iNum, char **pcFiles)
{
	for(int iFile=0; iFile<iNum; ++iFile)
		wnd.LoadFile(pcFiles[iFile]);
}

int main(int argc, char **argv)
{
	int iRet = -1;

	try
	{
		QSettings *pGlobals = Settings::GetGlobals();
		QApplication a(argc, argv);

		MiezeMainWnd wnd;
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
