/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 16-apr-2013
 * @license GPLv3
 */

#ifndef __MIEZE_ABOUT__
#define __MIEZE_ABOUT__

#include <QtGui/QDialog>
#include "../ui/ui_about.h"

#include <boost/version.hpp>
#include "../tlibs/version.h"

class AboutDlg : public QDialog, Ui::AboutDlg
{Q_OBJECT
protected:
	QImage m_katze;

public:
	AboutDlg(QWidget* pParent) : QDialog(pParent)
	{
		this->setupUi(this);
		QObject::connect(btnAboutQt, SIGNAL(clicked()), this, SLOT(ShowAboutQt()));	
		QString strLic;
		strLic += "Cattus is free software: you can redistribute it and/or modify\n";
		strLic += "it under the terms of the GNU General Public License as published by\n";
		strLic += "the Free Software Foundation, version 3 of the License.\n\n";
		strLic += "Cattus is distributed in the hope that it will be useful,\n";
		strLic += "but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
		strLic += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
		strLic += "GNU General Public License for more details.\n\n";
		strLic += "You should have received a copy of the GNU General Public License\n";
		strLic += "along with Cattus.  If not, see <http://www.gnu.org/licenses/>.";

		QString strBuild = QString("Built with ");
		strBuild += QString(BOOST_COMPILER);
		strBuild += ".";

		QString strDate;
		strDate += "Build date: ";
		strDate += QString(__DATE__);
		strDate += ", ";
		strDate += QString(__TIME__);
		strDate += ".";

		QString strLibs;
		strLibs += "Uses Qt version ";
		strLibs += QString(QT_VERSION_STR);
		strLibs += " (http://qt.digia.com).\n";
		strLibs += "Uses Boost version ";
		strLibs += QString(BOOST_LIB_VERSION);
		strLibs += " (http://www.boost.org).\n";
		strLibs += "Uses Minuit version 2 (http://root.cern.ch).";
		strLibs += "\n";
#ifdef USE_FFTW
		strLibs += "Uses FFTW version 3 (http://www.fftw.org).";
		strLibs += "\n";
#endif
		strLibs += "Uses TLIBS version " + QString(TLIBS_VERSION) + ".";
		//strLibs += "Uses Lapack/e version 3 (http://www.netlib.org/lapack).";
		//strLibs += "\n";

		this->labelTitle->setText("Cattus");
		this->labelVersion->setText("Version 0.4.1");
		this->labelAuthor->setText("Written by\nTobias Weber <tobias.weber@tum.de>,\n2012 - 2014.");
		this->labelLic->setText(strLic);
		this->labelBuild->setText(strBuild);
		this->labelDate->setText(strDate);
		this->labelLibs->setText(strLibs);


		m_katze.load("res/katze.jpg");
		m_katze = m_katze.scaledToWidth(64, Qt::SmoothTransformation);
	}

	virtual ~AboutDlg()
	{}

	virtual void paintEvent(QPaintEvent *pEvent)
	{
		QPainter painter(this);

		painter.drawImage(QPoint(width()-64-4,4), m_katze);
	}

public slots:
	void ShowAboutQt()
	{
		QMessageBox::aboutQt(this);
	}
};

//#include "AboutDlg.moc"

#endif
