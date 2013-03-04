/*
 * mieze-tool
 * @author tweber
 * @date 04-mar-2013
 */

#include <QtGui/QApplication>
#include <iostream>

#include "mainwnd.h"
#include "settings.h"

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
