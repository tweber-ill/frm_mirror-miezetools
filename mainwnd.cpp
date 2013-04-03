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
#include <QtGui/QStatusBar>

#include <iostream>
#include <fstream>
#include <math.h>

#include "plot/plot.h"
#include "plot/plot2d.h"
#include "plot/plot3d.h"
#include "plot/plot4d.h"
#include "settings.h"
#include "helper/string.h"
#include "helper/file.h"
#include "helper/misc.h"
#include "loader/loadtxt.h"
#include "loader/loadcasc.h"
#include "dialogs/ListDlg.h"
#include "dialogs/CombineDlg.h"
#include "dialogs/SettingsDlg.h"


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
	pLoad->setText("Open...");
	pLoad->setShortcut(Qt::CTRL + Qt::Key_L);
	pMenuFile->addAction(pLoad);

	QAction *pCloseAll = new QAction(this);
	pCloseAll->setText("Close All");
	pMenuFile->addAction(pCloseAll);

	pMenuFile->addSeparator();

	QAction *pSettings = new QAction(this);
	pSettings->setText("Settings...");
	pMenuFile->addAction(pSettings);

	pMenuFile->addSeparator();

	QAction *pExit = new QAction(this);
	pExit->setText("Exit");
	pMenuFile->addAction(pExit);


	// Tools
	QMenu *pMenuTools = new QMenu(this);
	pMenuTools->setTitle("Tools");

	QAction *pCombineGraphs = new QAction(this);
	pCombineGraphs->setText("Combine Graphs...");
	pMenuTools->addAction(pCombineGraphs);



	// Windows
	/*QMenu**/ pMenuWindows = new QMenu(this);
	pMenuWindows->setTitle("Windows");

	QAction *pWndList = new QAction(this);
	pWndList->setText("List...");
	pMenuWindows->addAction(pWndList);

	pMenuWindows->addSeparator();

	QAction *pWndTile = new QAction(this);
	pWndTile->setText("Tile");
	pMenuWindows->addAction(pWndTile);

	QAction *pWndCsc = new QAction(this);
	pWndCsc->setText("Cascade");
	pMenuWindows->addAction(pWndCsc);

	pMenuWindows->addSeparator();



	QMenuBar *pMenuBar = new QMenuBar(this);
	pMenuBar->addMenu(pMenuFile);
	pMenuBar->addMenu(pMenuTools);
	pMenuBar->addMenu(pMenuWindows);
	this->setMenuBar(pMenuBar);
	//--------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------
	// Status Bar
	QStatusBar* pStatusBar = new QStatusBar(this);
	this->setStatusBar(pStatusBar);
	m_pStatusLabelLeft = new QLabel(this);
	m_pStatusLabelMiddle = new QLabel(this);
	m_pStatusLabelRight = new QLabel(this);
	m_pStatusLabelLeft->setAlignment(Qt::AlignLeft);
	m_pStatusLabelMiddle->setAlignment(Qt::AlignHCenter);
	m_pStatusLabelRight->setAlignment(Qt::AlignRight);
	pStatusBar->addWidget(m_pStatusLabelLeft, 1);
	pStatusBar->addWidget(m_pStatusLabelMiddle, 1);
	pStatusBar->addWidget(m_pStatusLabelRight, 1);
	//--------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------
	// Connections
	QObject::connect(pLoad, SIGNAL(triggered()), this, SLOT(FileLoadTriggered()));
	QObject::connect(pSettings, SIGNAL(triggered()), this, SLOT(SettingsTriggered()));
	QObject::connect(pExit, SIGNAL(triggered()), this, SLOT(close()));

	QObject::connect(pCombineGraphs, SIGNAL(triggered()), this, SLOT(ShowCombineGraphsDlg()));

	QObject::connect(pWndList, SIGNAL(triggered()), this, SLOT(ShowListWindowsDlg()));
	QObject::connect(pWndTile, SIGNAL(triggered()), m_pmdi, SLOT(tileSubWindows()));
	QObject::connect(pWndCsc, SIGNAL(triggered()), m_pmdi, SLOT(cascadeSubWindows()));
	QObject::connect(pCloseAll, SIGNAL(triggered()), m_pmdi, SLOT(closeAllSubWindows()));

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
	std::string strFileNoDir = ::get_file(strFile);
	std::string strExt = get_fileext(strFile);

	std::ostringstream ostrMsg;
	ostrMsg << "Loading " << strFileNoDir << "...";
	this->SetStatusMsg(ostrMsg.str().c_str(),0);

	if(is_equal(strExt, "tof"))
	{
		TofFile tof(strFile.c_str());
		if(!tof.IsOpen())
			return;

		const uint iW = tof.GetWidth();
		const uint iH = tof.GetHeight();
		const uint iTcCnt = tof.GetTcCnt();
		const uint iFoilCnt = tof.GetFoilCnt();

		std::string strTitle = GetPlotTitle(strFileNoDir);
		Plot4dWrapper *pPlotWrapper = new Plot4dWrapper(m_pmdi, strTitle.c_str(), true);
		Plot4d *pPlot = (Plot4d*)pPlotWrapper->GetActualWidget();
		Data4& dat4 = pPlot->GetData();
		dat4.SetSize(iW, iH, iTcCnt, iFoilCnt);

		double *pdDat = new double[iW*iH*iTcCnt];
		for(uint iFoil=0; iFoil<iFoilCnt; ++iFoil)
		{
			const uint* pDat = tof.GetData(iFoil);
			convert(pdDat, pDat, iW*iH*iTcCnt);
			tof.ReleaseData(pDat);

			dat4.SetVals(iFoil, pdDat, 0);
		}
		delete[] pdDat;

		pPlot->plot_manual();
		pPlot->SetLabels("x pixels", "y pixels", "");

		AddSubWindow(pPlotWrapper);
	}
	else 	if(is_equal(strExt, "pad"))
	{
		PadFile pad(strFile.c_str());
		if(!pad.IsOpen())
			return;

		const uint* pDat = pad.GetData();

		const uint iW = pad.GetWidth();
		const uint iH = pad.GetHeight();

		double *pdDat = new double[iW*iH];
		convert(pdDat, pDat, iW*iH);

		std::string strTitle = GetPlotTitle(strFileNoDir);
		Plot2d *pPlot = new Plot2d(m_pmdi, strTitle.c_str(), true);
		pPlot->plot(iW, iH, pdDat);
		pPlot->SetLabels("x pixels", "y pixels", "");

		delete[] pdDat;

		AddSubWindow(pPlot);
	}
	else 	if(is_equal(strExt, "dat") || is_equal(strExt, "sim"))
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

			std::string strTitle = GetPlotTitle(strFileNoDir);

			Plot *pPlot = new Plot(m_pmdi, strTitle.c_str());
			pPlot->plot(pdat1d->GetDim(), pdx, pdy, pdyerr);

			std::string strLabX, strLabY, strPlotTitle;
			pdat1d->GetLabels(strLabX, strLabY);
			pdat1d->GetTitle(strPlotTitle);
			pPlot->SetLabels(strLabX.c_str(), strLabY.c_str());
			pPlot->SetTitle(strPlotTitle.c_str());

			delete pdat1d;

			AddSubWindow(pPlot);
		}
		else if(iArrayDim == 2)
		{
			Data2D* pdat2d = new Data2D(*pdat);

			const uint iW = pdat2d->GetXDim();
			const uint iH = pdat2d->GetYDim();

			double *pDat = new double[iW*iH];
			double *pErr = new double[iW*iH];

			for(uint iY=0; iY<iH; ++iY)
				for(uint iX=0; iX<iW; ++iX)
				{
					pDat[iY*iW + iX] = pdat2d->GetVal(iX, iY);
					pErr[iY*iW + iX] = pdat2d->GetErr(iX, iY);
				}

			std::string strTitle = GetPlotTitle(strFileNoDir);
			Plot2d *pPlot = new Plot2d(m_pmdi, strTitle.c_str(), false);

			pPlot->plot(iW, iH, pDat, pErr);

			std::string strLabX, strLabY, strLabZ, strPlotTitle;
			pdat2d->GetLabels(strLabX, strLabY, strLabZ);
			pdat2d->GetTitle(strPlotTitle);
			pPlot->SetLabels(strLabX.c_str(), strLabY.c_str(), strLabZ.c_str());
			pPlot->SetTitle(strPlotTitle.c_str());

			double dXMin, dXMax, dYMin, dYMax, dZMin, dZMax;
			if(pdat2d->GetLimits(dXMin, dXMax, dYMin, dYMax, dZMin, dZMax))
				pPlot->SetXYMinMax(dXMin, dXMax, dYMin, dYMax);

			delete[] pDat;
			delete[] pErr;
			delete pdat2d;

			AddSubWindow(pPlot);
		}
		else if(iArrayDim == 3)
		{
			Data3D* pdat3d = new Data3D(*pdat);

			const uint iW = pdat3d->GetXDim();
			const uint iH = pdat3d->GetYDim();
			const uint iT = pdat3d->GetTDim();

			double *pDat = new double[iW*iH*iT];
			double *pErr = new double[iW*iH*iT];

			for(uint iZ=0; iZ<iT; ++iZ)
				for(uint iY=0; iY<iH; ++iY)
					for(uint iX=0; iX<iW; ++iX)
					{
						pDat[iZ*iW*iH + iY*iW + iX] = pdat3d->GetVal(iX, iY, iZ);
						pErr[iZ*iW*iH + iY*iW + iX] = pdat3d->GetErr(iX, iY, iZ);
					}


			std::string strTitle = GetPlotTitle(strFileNoDir);
			Plot3dWrapper *pPlotWrapper = new Plot3dWrapper(m_pmdi, strTitle.c_str(), false);
			Plot3d *pPlot = (Plot3d*)pPlotWrapper->GetActualWidget();
			pPlot->plot(iW, iH, iT, pDat, pErr);

			std::string strLabX, strLabY, strLabZ, strPlotTitle;
			pdat3d->GetLabels(strLabX, strLabY, strLabZ);
			pdat3d->GetTitle(strPlotTitle);
			pPlot->SetLabels(strLabX.c_str(), strLabY.c_str(), strLabZ.c_str());
			pPlot->SetTitle(strPlotTitle.c_str());

			double dXMin, dXMax, dYMin, dYMax, dZMin, dZMax;
			if(pdat3d->GetLimits(dXMin, dXMax, dYMin, dYMax, dZMin, dZMax))
				pPlot->SetXYMinMax(dXMin, dXMax, dYMin, dYMax);

			delete[] pDat;
			delete[] pErr;
			delete pdat3d;

			AddSubWindow(pPlotWrapper);
		}

		delete pdat;
	}
}

std::string MiezeMainWnd::GetPlotTitle(const std::string& strFile)
{
	std::ostringstream ostrTitle;
	ostrTitle << "Plot #" << (m_iPlotCnt++) << " - " << strFile;
	return ostrTitle.str();
}

void MiezeMainWnd::FileLoadTriggered()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir", ".").toString();

	QStringList strFiles = QFileDialog::getOpenFileNames(this, "Open data file...", strLastDir,
					"All data files (*.dat *.sim *.pad *.tof);;TOF files (*.tof);;PAD files(*.pad);;DAT files (*.dat *.sim)",
					0, QFileDialog::DontUseNativeDialog);
	if(strFiles.size() == 0)
		return;

	bool bDirSet=false;
	for(const QString& strFile : strFiles)
	{
		if(strFile == "")
			continue;

		std::string strFile1 = strFile.toStdString();
		std::string strExt = get_fileext(strFile1);

		if(!bDirSet)
		{
			pGlobals->setValue("main/lastdir", QString(::get_dir(strFile1).c_str()));
			bDirSet = true;
		}

		LoadFile(strFile1);
	}
}

void MiezeMainWnd::SettingsTriggered()
{
	SettingsDlg dlg(this);
	if(dlg.exec() == QDialog::Accepted)
	{

	}
}

void MiezeMainWnd::AddSubWindow(SubWindowBase* pWnd)
{
	SubWindowBase *pActualWidget = pWnd->GetActualWidget();
	QObject::connect(pActualWidget, SIGNAL(SetStatusMsg(const char*, int)), this, SLOT(SetStatusMsg(const char*, int)));

	m_pmdi->addSubWindow(pWnd);
	pWnd->show();
}

void MiezeMainWnd::keyPressEvent (QKeyEvent * event)
{
	SubWindowBase* pWndBase = 0;
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	if(pWnd && pWnd->widget())
		pWndBase = (SubWindowBase*)pWnd->widget();

	if(event->key()==Qt::Key_L && pWndBase)
	{
		if(pWndBase->GetType() == PLOT_2D || pWndBase->GetType() == PLOT_3D  || pWndBase->GetType() == PLOT_4D)
		{
			Plot2d* plt = (Plot2d*)pWndBase->GetActualWidget();
			plt->SetLog(!plt->GetLog());
		}
	}
	else
		QMainWindow::keyPressEvent(event);
}

void MiezeMainWnd::SetStatusMsg(const char* pcMsg, int iPos)
{
	QLabel* pLabels[] = {m_pStatusLabelLeft, m_pStatusLabelMiddle, m_pStatusLabelRight};
	if(iPos>=3 || iPos<0)
	{
		this->statusBar()->showMessage(pcMsg, 5000);
		return;
	}

	QLabel* pLabel = pLabels[iPos];

	QString strMsg(pcMsg);
	if(pLabel->text() != strMsg)
	{
		pLabel->setText(pcMsg);
		pLabel->repaint();
	}
}

void MiezeMainWnd::ShowListWindowsDlg()
{
	ListGraphsDlg dlg(this);

	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow *pItem : lst)
	{
		SubWindowBase *pWnd = (SubWindowBase *) pItem->widget();
		if(pWnd)
			dlg.AddSubWnd(pWnd);
	}

	int iStatus = dlg.exec();
	/*if(iStatus == QDialog::Accepted)
	{
		std::list<SubWindowBase*> lstWnds = dlg.GetSelectedSubWnds();

		for(auto wnd : lstWnds)
			std::cout << wnd->windowTitle().toStdString() << std::endl;
	}*/
}

void MiezeMainWnd::ShowCombineGraphsDlg()
{
	CombineGraphsDlg dlg(this);

	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow *pItem : lst)
	{
		SubWindowBase *pWnd = (SubWindowBase *) pItem->widget();
		if(pWnd)
			dlg.AddAvailSubWnd(pWnd);
	}

	if(dlg.exec() == QDialog::Accepted)
	{
		Plot *pPlot = dlg.CreatePlot(GetPlotTitle("combined plot"), m_pmdi);
		AddSubWindow(pPlot);
	}
}

#include "mainwnd.moc"
#include "subwnd.moc"

