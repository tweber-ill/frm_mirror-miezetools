/*
 * mieze-tool
 * main mdi window
 * @author tweber
 * @date 04-mar-2013
 */

#include "mainwnd.h"

#include<QtGui/QLabel>
#include<QtGui/QMenu>
#include<QtGui/QMenuBar>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QFileDialog>
#include <iostream>

#include "plot/plot.h"
#include "settings.h"
#include "helper/string.h"

MiezeMainWnd::MiezeMainWnd()
{
	this->setWindowTitle("mieze tool");

	m_pmdi = new QMdiArea(this);
	this->setCentralWidget(m_pmdi);

	//--------------------------------------------------------------------------------
	// Menus

	// File
	QMenu* pMenuFile = new QMenu(this);
	pMenuFile->setTitle("File");

	QAction *pLoad = new QAction(this);
	pLoad->setText("Load...");
	pMenuFile->addAction(pLoad);

	QAction *pExit = new QAction(this);
	pExit->setText("Exit");
	pMenuFile->addSeparator();
	pMenuFile->addAction(pExit);


	// Windows
	QMenu* pMenuWindows = new QMenu(this);
	pMenuWindows->setTitle("Windows");

	QAction *pWndTile = new QAction(this);
	pWndTile->setText("Tile");
	pMenuWindows->addAction(pWndTile);

	QAction *pWndCsc = new QAction(this);
	pWndCsc->setText("Cascade");
	pMenuWindows->addAction(pWndCsc);


	QMenuBar *pMenuBar = new QMenuBar(this);
	pMenuBar->addMenu(pMenuFile);
	pMenuBar->addMenu(pMenuWindows);
	this->setMenuBar(pMenuBar);
	//--------------------------------------------------------------------------------


	/*
	// Test
	Plot *pPlot = new Plot(this);
	m_pmdi->addSubWindow(pPlot);
	double dx[] = {10., 20., 30., 40., 50., 60.,};
	double dy[] = {10., 70., 30., 40., 50., 60.,};
	double dyerr[] = {5., 5., 5., 5., 5., 5.};
	pPlot->plot(6, dx, dy, dyerr);
	*/

	//--------------------------------------------------------------------------------
	// Connections

	QObject::connect(pLoad, SIGNAL(triggered()), this, SLOT(FileLoadTriggered()));
	QObject::connect(pExit, SIGNAL(triggered()), this, SLOT(close()));
	QObject::connect(pWndTile, SIGNAL(triggered()), m_pmdi, SLOT(tileSubWindows()));
	QObject::connect(pWndCsc, SIGNAL(triggered()), m_pmdi, SLOT(cascadeSubWindows()));

	QObject::connect(m_pmdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(SubWindowChanged()));


	//--------------------------------------------------------------------------------
}

MiezeMainWnd::~MiezeMainWnd()
{}


void MiezeMainWnd::SubWindowChanged()
{
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	if(pWnd)
	{
		QWidget *pWdg = pWnd->widget();
	}

	// ...
}

void MiezeMainWnd::FileLoadTriggered()
{
	QSettings *pGlobals = Settings::GetGlobals();

	QString strLastDir = pGlobals->value("main/lastdir", ".").toString();

	QString strFile = QFileDialog::getOpenFileName(this, "Open data file...", strLastDir,
					"DAT files (*.dat *.DAT);;PAD files (*.pad *.PAD);;TOF files (*.tof *.TOF);;All files (*.*)");
	if(strFile.size() == 0)
		return;

	std::string strFile1 = strFile.toAscii().data();
	std::string strExt = get_fileext(strFile1);

	pGlobals->setValue("main/lastdir", QString(::get_dir(strFile1).c_str()));

	if(is_equal(strExt, "tof"))
	{
	}
	else 	if(is_equal(strExt, "pad"))
	{
	}
	else 	if(is_equal(strExt, "dat"))
	{
	}
}

#include "mainwnd.moc"

