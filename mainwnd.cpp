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
#include <QtGui/QMessageBox>
#include <iostream>

#include "plot/plot.h"
#include "settings.h"
#include "helper/string.h"
#include "loader/loadtxt.h"

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
		LoadTxt * pdat = new LoadTxt();
		if(!pdat->Load(strFile1.c_str()))
		{
			QString strErr = QString("Could not load \"") + strFile +QString("\".");
			QMessageBox::critical(this, "Error", strErr);
			return;
		}

		//dat.SetMapVal<int>("which-col-is-x", 0);
        //dat.SetMapVal<int>("which-col-is-y", 1);
        //dat.SetMapVal<int>("which-col-is-yerr", 2);

        int iArrayDim = 1;
        const LoadTxt::t_mapComm& mapComm = pdat->GetCommMap();
		LoadTxt::t_mapComm::const_iterator iter = mapComm.find("type");
		if(iter != mapComm.end() && (*iter).second.size()>=1)
		{
				const std::string& strType = (*iter).second[0];

				if(strType.compare(0,8, "array_1d") == 0)
						iArrayDim = 1;
				else if(strType.compare(0,8, "array_2d") == 0)
						iArrayDim = 2;
				else if(strType.compare(0,8, "array_3d") == 0)
						iArrayDim = 3;
		}

		if(iArrayDim == 1)
		{
			Data1D * pdat1d = new Data1D(*pdat);

			int iX=0, iY=1, iYErr=2;
			pdat1d->GetColumnIndices(iX, iY, iYErr);
			const double *pdx = pdat1d->GetColumn(iX);
			const double *pdy = pdat1d->GetColumn(iY);
			const double *pdyerr = pdat1d->GetColumn(iYErr);

			Plot *pPlot = new Plot(m_pmdi);
			pPlot->plot(pdat1d->GetDim(), pdx, pdy, pdyerr);
			m_pmdi->addSubWindow(pPlot);
			pPlot->show();

			delete pdat1d;
		}

		delete pdat;
	}
}

#include "mainwnd.moc"

