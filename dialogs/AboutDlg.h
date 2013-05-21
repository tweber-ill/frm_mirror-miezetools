/*
 * mieze-tool
 * @author tweber
 * @date 16-apr-2013
 */

#ifndef __MIEZE_ABOUT__
#define __MIEZE_ABOUT__

#include <QtGui/QDialog>
#include "../ui/ui_about.h"

#include <boost/version.hpp>

class AboutDlg : public QDialog, Ui::AboutDlg
{Q_OBJECT
public:
	AboutDlg(QWidget* pParent) : QDialog(pParent)
	{
		this->setupUi(this);
		QObject::connect(btnAboutQt, SIGNAL(clicked()), this, SLOT(ShowAboutQt()));

		QString strBuild;
		strBuild += "Built with CC version ";
		strBuild += QString(__VERSION__);

		QString strDate;
		strDate += "Build date: ";
		strDate += QString(__DATE__);
		strDate += ", ";
		strDate += QString(__TIME__);

		QString strLibs;
		strLibs += "Uses Qt version ";
		strLibs += QString(QT_VERSION_STR);
		strLibs += " (http://qt.digia.com).\n";
		strLibs += "Uses Boost version ";
		strLibs += QString(BOOST_LIB_VERSION);
		strLibs += " (http://www.boost.org).\n";
		strLibs += "Uses Minuit version 2 (http://root.cern.ch).";
		strLibs += "\n";
		strLibs += "Uses FFTW version 3 (http://www.fftw.org).";
		strLibs += "\n";
		strLibs += "Uses Lapack/e version 3 (http://www.netlib.org/lapack).";
		strLibs += "\n";

		this->labelTitle->setText("Cattus");
		this->labelVersion->setText("Version 0.1");
		this->labelAuthor->setText("Written by Tobias Weber, 2012-2013");
		this->labelBuild->setText(strBuild);
		this->labelDate->setText(strDate);

		this->labelLibs->setText(strLibs);
	}

	virtual ~AboutDlg()
	{}

public slots:
	void ShowAboutQt()
	{
		QMessageBox::aboutQt(this);
	}
};

//#include "AboutDlg.moc"

#endif
