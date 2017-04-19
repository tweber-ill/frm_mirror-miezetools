/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 08-jul-2013
 * @license GPLv3
 */

#include "FormulaDlg.h"
#include "../../main/settings.h"
#include <QtGui/QApplication>

#ifdef Q_WS_X11
	extern "C" int XInitThreads();
#endif

int main(int argc, char **argv)
{
	try
	{
		QSettings *pGlobals = Settings::GetGlobals();
		QApplication a(argc, argv);

		::setlocale(LC_ALL, "C");
		QLocale::setDefault(QLocale::English);

		FormulaDlg wnd(0);
		wnd.exec();
	}
	catch(const std::exception& ex)
	{
		std::cerr << "\n********************************************************************************\n"
					 << "CRITICAL ERROR: " << ex.what()
					 << "\n********************************************************************************\n"
					 << std::endl;
	}

	Settings::free();
	return 0;
}

#include "../../main/subwnd.moc"
