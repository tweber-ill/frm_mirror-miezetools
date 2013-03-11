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
#include <fstream>
#include <math.h>

#include "plot/plot.h"
#include "plot/plot2d.h"
#include "settings.h"
#include "helper/string.h"
#include "helper/file.h"
#include "helper/misc.h"
#include "loader/loadtxt.h"

MiezeMainWnd::MiezeMainWnd() : m_iPlotCnt(1)
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
	pLoad->setShortcut(Qt::CTRL + Qt::Key_L);
	pMenuFile->addAction(pLoad);

	QAction *pExit = new QAction(this);
	pExit->setText("Exit");
	pMenuFile->addSeparator();
	pMenuFile->addAction(pExit);


	// Windows
	/*QMenu**/ pMenuWindows = new QMenu(this);
	pMenuWindows->setTitle("Windows");

	QAction *pWndTile = new QAction(this);
	pWndTile->setText("Tile");
	pMenuWindows->addAction(pWndTile);

	QAction *pWndCsc = new QAction(this);
	pWndCsc->setText("Cascade");
	pMenuWindows->addAction(pWndCsc);

	pMenuWindows->addSeparator();



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

	QObject::connect(pMenuWindows, SIGNAL(aboutToShow()), this, SLOT(UpdateSubWndList()));
	QObject::connect(m_pmdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(SubWindowChanged()));


	//--------------------------------------------------------------------------------
}

MiezeMainWnd::~MiezeMainWnd()
{}

void MiezeMainWnd::UpdateSubWndList()
{
	// remove previous list
	for(QAction* pAction : m_vecSubWndActions)
	{
		pMenuWindows->removeAction(pAction);
		pAction->disconnect();
		delete pAction;
	}
	m_vecSubWndActions.clear();

	// add new list
	QList<QMdiSubWindow*> list = m_pmdi->subWindowList();
	for(auto item : list)
	{
		const QWidget* pWidget = item->widget();
		if(pWidget)
		{
			QString strTitle = pWidget->windowTitle();

			QAction *pAction = new QAction(pMenuWindows);
			pAction->setText(strTitle);
			m_vecSubWndActions.push_back(pAction);

			pMenuWindows->addAction(pAction);

			QObject::connect(pAction, SIGNAL(triggered()), pWidget, SLOT(showNormal()));
			QObject::connect(pAction, SIGNAL(triggered()), pWidget, SLOT(setFocus()));
		}
	}
}

void MiezeMainWnd::SubWindowChanged()
{
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	if(pWnd)
	{
		QWidget *pWdg = pWnd->widget();

		SubWindowType type = ((SubWindowBase*)pWdg)->GetType();
		if(type == PLOT_1D)
		{

		}
	}

	// ...
}

void MiezeMainWnd::LoadFile(const std::string& strFile)
{
	std::string strExt = get_fileext(strFile);

	if(is_equal(strExt, "tof"))
	{
	}
	else 	if(is_equal(strExt, "pad"))
	{
		std::ifstream ifstr(strFile.c_str(), std::ios_base::binary);
		if(!ifstr.is_open())
			return;

		uint iW, iH;
		uint uiSize = ::get_file_size(ifstr);
		iW = iH = sqrt(uiSize/4);

		uint *pDat = new uint[iW*iH];

		ifstr.read((char*)pDat, iW*iH*4);
		double *pdDat = new double[iW*iH];
		convert(pdDat, pDat, iW*iH);
		delete[] pDat;

		std::string strTitle = GetPlotTitle(get_file(strFile));
		Plot2d *pPlot = new Plot2d(m_pmdi, strTitle.c_str());
		pPlot->plot(iW, iH, pdDat);
		AddSubWindow(pPlot);

		delete[] pdDat;
	}
	else 	if(is_equal(strExt, "dat"))
	{
		LoadTxt * pdat = new LoadTxt();
		if(!pdat->Load(strFile.c_str()))
		{
			QString strErr = QString("Could not load \"") + QString(strFile.c_str()) +QString("\".");
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

			std::string strTitle = GetPlotTitle(get_file(strFile));

			Plot *pPlot = new Plot(m_pmdi, strTitle.c_str());
			pPlot->plot(pdat1d->GetDim(), pdx, pdy, pdyerr);
			AddSubWindow(pPlot);

			delete pdat1d;
		}
		else if(iArrayDim == 2)
		{
			Data2D * pdat2d = new Data2D(*pdat);

			uint iW = pdat2d->GetXDim();
			uint iH = pdat2d->GetYDim();

			double *pDat = new double[iW*iH];

			for(uint iY=0; iY<iH; ++iY)
				for(uint iX=0; iX<iW; ++iX)
					pDat[iY*iW + iX] = pdat2d->GetVal(iX, iY);

			std::string strTitle = GetPlotTitle(get_file(strFile));
			Plot2d *pPlot = new Plot2d(m_pmdi, strTitle.c_str());
			pPlot->plot(iW, iH, pDat);
			AddSubWindow(pPlot);

			delete[] pDat;
			delete pdat2d;
		}

		delete pdat;
	}
}

std::string MiezeMainWnd::GetPlotTitle(const std::string& strFile)
{
	std::ostringstream ostrTitle;
	ostrTitle << "Plot #" << m_iPlotCnt++ << " - " << strFile;
	return ostrTitle.str();
}

void MiezeMainWnd::FileLoadTriggered()
{
	QSettings *pGlobals = Settings::GetGlobals();

	QString strLastDir = pGlobals->value("main/lastdir", ".").toString();

	QStringList strFiles = QFileDialog::getOpenFileNames(this, "Open data file...", strLastDir,
					"DAT files (*.dat *.DAT);;PAD files (*.pad *.PAD);;TOF files (*.tof *.TOF);;All files (*.*)");
	if(strFiles.size() == 0)
		return;

	bool bDirSet=false;
	for(const QString& strFile : strFiles)
	{
		if(strFile == "")
			continue;

		std::string strFile1 = strFile.toAscii().data();
		std::string strExt = get_fileext(strFile1);

		if(!bDirSet)
		{
			pGlobals->setValue("main/lastdir", QString(::get_dir(strFile1).c_str()));
			bDirSet = true;
		}

		LoadFile(strFile1);
	}
}

void MiezeMainWnd::AddSubWindow(QWidget* pWnd)
{
	m_pmdi->addSubWindow(pWnd);
	pWnd->show();
}



#include "mainwnd.moc"

