/*
 * mieze-tool
 * main mdi window
 * @author tweber
 * @date 04-mar-2013
 */

#include "mainwnd.h"

#include<QtGui/QLabel>
#include<QtGui/QMenuBar>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtCore/QSignalMapper>

#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>

#include "settings.h"

#include "helper/string.h"
#include "helper/file.h"
#include "helper/misc.h"
#include "helper/mieze.hpp"
#include "helper/xml.h"

#include "loader/loadtxt.h"
#include "loader/loadnicos.h"
#include "loader/loadcasc.h"

#include "dialogs/ListDlg.h"
#include "dialogs/SettingsDlg.h"
#include "dialogs/AboutDlg.h"
#include "dialogs/ComboDlg.h"

#include "fitter/models/msin.h"
#include "fitter/models/gauss.h"
#include "fitter/models/interpolation.h"

#include "data/export.h"

#define WND_TITLE "Cattus, a MIEZE toolset"



MiezeMainWnd::MiezeMainWnd()
					: m_iPlotCnt(1),
					  m_pcombinedlg(0), m_pfitdlg(0),
					  m_proidlg(new RoiDlg(this)),
					  m_presdlg(0), m_pphasecorrdlg(0),
					  m_pradialintdlg(0),
					  m_pformuladlg(0),
					  m_pplotpropdlg(0),
					  m_pexportdlg(0)
{
	this->setWindowTitle(WND_TITLE);

	m_pmdi = new QMdiArea(this);
	m_pmdi->setActivationOrder(QMdiArea::StackingOrder);
	//m_pmdi->setViewMode(QMdiArea::TabbedView);
	//m_pmdi->setDocumentMode(1);
	//m_pmdi->setTabPosition(QTabWidget::South);
	m_pmdi->setViewMode(QMdiArea::SubWindowView);
	m_pmdi->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, 1);
	this->setCentralWidget(m_pmdi);


	//--------------------------------------------------------------------------------
	// Menus

	// File
	QMenu* pMenuFile = new QMenu(this);
	pMenuFile->setTitle("File");

	QAction *pLoad = new QAction(this);
	pLoad->setText("Open...");
	pLoad->setIcon(QIcon::fromTheme("document-open"));
	pMenuFile->addAction(pLoad);

	m_pMenuLoadRecent = new QMenu(this);
	m_pMenuLoadRecent->setTitle("Open Recent");
	pMenuFile->addMenu(m_pMenuLoadRecent);

	QAction *pCloseAll = new QAction(this);
	pCloseAll->setText("Close All");
	pCloseAll->setIcon(QIcon::fromTheme("window-close"));
	pMenuFile->addAction(pCloseAll);

	pMenuFile->addSeparator();

	QAction *pLoadSess = new QAction(this);
	pLoadSess->setText("Open Session...");
	pLoadSess->setShortcut(Qt::CTRL + Qt::Key_L);
	pLoadSess->setIcon(QIcon::fromTheme("document-open"));
	pMenuFile->addAction(pLoadSess);

	m_pMenuLoadRecentSession = new QMenu(this);
	m_pMenuLoadRecentSession->setTitle("Open Recent Session");
	pMenuFile->addMenu(m_pMenuLoadRecentSession);


	QAction *pSaveSess = new QAction(this);
	pSaveSess->setText("Save Session");
	pSaveSess->setIcon(QIcon::fromTheme("document-save"));
	pSaveSess->setShortcut(Qt::CTRL + Qt::Key_S);
	pMenuFile->addAction(pSaveSess);

	QAction *pSaveSessAs = new QAction(this);
	pSaveSessAs->setText("Save Session As...");
	pSaveSessAs->setIcon(QIcon::fromTheme("document-save-as"));
	pMenuFile->addAction(pSaveSessAs);


	pMenuFile->addSeparator();

	QAction *pSettings = new QAction(this);
	pSettings->setText("Settings...");
	pSettings->setIcon(QIcon::fromTheme("preferences-system"));
	pMenuFile->addAction(pSettings);

	pMenuFile->addSeparator();

	QAction *pExit = new QAction(this);
	pExit->setText("Exit");
	pExit->setIcon(QIcon::fromTheme("application-exit"));
	pMenuFile->addAction(pExit);



	// Cur. Plot
	/*QMenu**/ pMenuPlot = new QMenu(this);
	pMenuPlot->setTitle("Plot");
	pMenuPlot->setEnabled(0);

	QAction *pShowT = new QAction(this);
	pShowT->setText("Show Time Channels");

	QAction *pExportPy = new QAction(this);
	pExportPy->setText("Export as Python Script...");

	QAction *pPlotProp = new QAction(this);
	pPlotProp->setText("Plot Properties...");

	m_pMenu1d = new QMenu(this);
	m_pMenu1d->addAction(pPlotProp);
	m_pMenu1d->addSeparator();
	m_pMenu1d->addAction(pExportPy);

	m_pMenu2d = new QMenu(this);
	m_pMenu2d->addAction(pPlotProp);
	m_pMenu2d->addSeparator();
	m_pMenu2d->addAction(pExportPy);

	m_pMenu3d = new QMenu(this);
	m_pMenu3d->addAction(pPlotProp);
	m_pMenu3d->addSeparator();
	m_pMenu3d->addAction(pShowT);
	m_pMenu3d->addSeparator();
	m_pMenu3d->addAction(pExportPy);


	m_pMenu4d = new QMenu(this);
	m_pMenu4d->addAction(pPlotProp);
	m_pMenu4d->addSeparator();

	QAction *pExtractFoils = new QAction(m_pMenu4d);
	pExtractFoils->setText("Extract Foils");
	m_pMenu4d->addAction(pExtractFoils);
	m_pMenu4d->addAction(pShowT);

	m_pMenu4d->addSeparator();
	m_pMenu4d->addAction(pExportPy);



	// Fit
	QMenu *pMenuFit = new QMenu(this);
	pMenuFit->setTitle("Fit");

	QAction *pFit = new QAction(this);
	pFit->setText("Fit Function...");
	pFit->setShortcut(Qt::CTRL + Qt::Key_F);
	pMenuFit->addAction(pFit);

	QMenu *pMenuQuickFit = new QMenu(pMenuFit);
	pMenuQuickFit->setTitle("Quick Fit");

	QAction *pQFitMieze = new QAction(this);
	pQFitMieze->setText("MIEZE Sine Signal");
	pQFitMieze->setShortcut(Qt::ALT + Qt::Key_M);
	pMenuQuickFit->addAction(pQFitMieze);

	QAction *pQFitMiezeArea = new QAction(this);
	pQFitMiezeArea->setText("MIEZE Sine Signal (pixel-wise)");
	pQFitMiezeArea->setShortcut(Qt::ALT + Qt::Key_P);
	pMenuQuickFit->addAction(pQFitMiezeArea);

	pMenuQuickFit->addSeparator();

	QAction *pQFitGauss = new QAction(this);
	pQFitGauss->setText("Gaussian");
	pQFitGauss->setShortcut(Qt::ALT + Qt::Key_G);
	pMenuQuickFit->addAction(pQFitGauss);

	QAction *pQFitDGauss = new QAction(this);
	pQFitDGauss->setText("Double Gaussian");
	pQFitDGauss->setShortcut(Qt::ALT + Qt::Key_D);
	pMenuQuickFit->addAction(pQFitDGauss);

	QAction *pQFitTGauss = new QAction(this);
	pQFitTGauss->setText("Triple Gaussian");
	pQFitTGauss->setShortcut(Qt::ALT + Qt::Key_T);
	pMenuQuickFit->addAction(pQFitTGauss);

	pMenuFit->addMenu(pMenuQuickFit);


	pMenuFit->addSeparator();

	QMenu *pMenuInterp = new QMenu(pMenuFit);
	pMenuInterp->setTitle("Interpolation");

	QAction *pInterpBezier = new QAction(this);
	pInterpBezier->setText(QString::fromUtf8("B\303\251zier Curve"));
	pMenuInterp->addAction(pInterpBezier);

	QAction *pInterpBSpline = new QAction(this);
	pInterpBSpline->setText(QString::fromUtf8("B-Spline"));
	pMenuInterp->addAction(pInterpBSpline);

	pMenuFit->addMenu(pMenuInterp);


	// Tools
	QMenu *pMenuTools = new QMenu(this);
	pMenuTools->setTitle("Tools");

	QAction *pCombineGraphs = new QAction(this);
	pCombineGraphs->setText("Plot Counts/Contrasts...");
	pMenuTools->addAction(pCombineGraphs);

	QAction *pPhaseCorr = new QAction(this);
	pPhaseCorr->setText("PSD Phase Correction...");
	pMenuTools->addAction(pPhaseCorr);

	pMenuTools->addSeparator();

	QMenu *pMenuIntegrate = new QMenu(pMenuTools);
	pMenuIntegrate->setTitle("Integrate");

	QAction *pIntAlongY = new QAction(this);
	pIntAlongY->setText("Integrate Y");
	pMenuIntegrate->addAction(pIntAlongY);

	QAction *pIntRad = new QAction(this);
	pIntRad->setText("Integrate Radially...");
	pMenuIntegrate->addAction(pIntRad);

	pMenuTools->addMenu(pMenuIntegrate);

	pMenuTools->addSeparator();

	QAction *pExportPlots = new QAction(this);
	pExportPlots->setText("Export Multiple Plots...");
	pMenuTools->addAction(pExportPlots);


	// ROI
	QMenu *pMenuROI = new QMenu(this);
	pMenuROI->setTitle("ROI");

	QAction *pManageROI = new QAction(this);
	pManageROI->setText("Manage ROI...");
	pManageROI->setIcon(QIcon::fromTheme("list-add"));
	pMenuROI->addAction(pManageROI);



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



	// Calc
	QMenu *pMenuCalc = new QMenu(this);
	pMenuCalc->setTitle("Calculations");

	QAction *pReso = new QAction(this);
	pReso->setText("Resolution...");
	pMenuCalc->addAction(pReso);


	pMenuCalc->addSeparator();


	QAction *pFormulas = new QAction(this);
	pFormulas->setText("Formulas...");
	pMenuCalc->addAction(pFormulas);

	QAction *pPSDPhase = new QAction(this);
	pPSDPhase->setText("Flat PSD Phases...");
	pMenuCalc->addAction(pPSDPhase);




	// Help
	QMenu* pMenuHelp = new QMenu(this);
	pMenuHelp->setTitle("Help");
	QAction* pBrowser = new QAction(this);
	pBrowser->setText("Add Browser Toolbar");
	pBrowser->setIcon(QIcon::fromTheme("applications-internet"));
	pMenuHelp->addAction(pBrowser);

	pMenuHelp->addSeparator();

	QAction* pAbout = new QAction(this);
	pAbout->setText("About...");
	pAbout->setIcon(QIcon::fromTheme("help-about"));
	pMenuHelp->addAction(pAbout);




	QMenuBar *pMenuBar = new QMenuBar(this);
	pMenuBar->addMenu(pMenuFile);
	pMenuBar->addMenu(pMenuPlot);
	pMenuBar->addMenu(pMenuFit);
	pMenuBar->addMenu(pMenuTools);
	pMenuBar->addMenu(pMenuROI);
	pMenuBar->addMenu(pMenuWindows);
	pMenuBar->addMenu(pMenuCalc);
	pMenuBar->addMenu(pMenuHelp);
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

	QObject::connect(pLoadSess, SIGNAL(triggered()), this, SLOT(SessionLoadTriggered()));
	QObject::connect(pSaveSess, SIGNAL(triggered()), this, SLOT(SessionSaveTriggered()));
	QObject::connect(pSaveSessAs, SIGNAL(triggered()), this, SLOT(SessionSaveAsTriggered()));

	QObject::connect(pShowT, SIGNAL(triggered()), this, SLOT(ShowTimeChannels()));
	QObject::connect(pExtractFoils, SIGNAL(triggered()), this, SLOT(ExtractFoils()));
	QObject::connect(pPlotProp, SIGNAL(triggered()), this, SLOT(PlotPropertiesTriggered()));
	QObject::connect(pExportPy, SIGNAL(triggered()), this, SLOT(FileExportPyTriggered()));
	QObject::connect(pExportPlots, SIGNAL(triggered()), this, SLOT(ShowExportDlg()));


	QObject::connect(pCombineGraphs, SIGNAL(triggered()), this, SLOT(ShowCombineGraphsDlg()));
	QObject::connect(pIntAlongY, SIGNAL(triggered()), this, SLOT(IntAlongY()));
	QObject::connect(pIntRad, SIGNAL(triggered()), this, SLOT(IntRad()));

	QObject::connect(pFit, SIGNAL(triggered()), this, SLOT(ShowFitDlg()));
	QObject::connect(pQFitMieze, SIGNAL(triggered()), this, SLOT(QuickFitMIEZE()));
	QObject::connect(pQFitMiezeArea, SIGNAL(triggered()), this, SLOT(QuickFitMIEZEpixel()));
	QObject::connect(pQFitGauss, SIGNAL(triggered()), this, SLOT(QuickFitGauss()));
	QObject::connect(pQFitDGauss, SIGNAL(triggered()), this, SLOT(QuickFitDoubleGauss()));
	QObject::connect(pQFitTGauss, SIGNAL(triggered()), this, SLOT(QuickFitTripleGauss()));
	QObject::connect(pInterpBezier, SIGNAL(triggered()), this, SLOT(BezierInterpolation()));
	QObject::connect(pInterpBSpline, SIGNAL(triggered()), this, SLOT(BSplineInterpolation()));

	QObject::connect(pManageROI, SIGNAL(triggered()), this, SLOT(ROIManageTriggered()));
	QObject::connect(m_proidlg, SIGNAL(WantActiveRoi()), this, SLOT(GetActiveROI()));
	QObject::connect(m_proidlg, SIGNAL(SetRoiForActive()), this, SLOT(SetGlobalROIForActive()));
	QObject::connect(m_proidlg, SIGNAL(SetRoiForAll()), this, SLOT(SetGlobalROIForAll()));

	QObject::connect(pWndList, SIGNAL(triggered()), this, SLOT(ShowListWindowsDlg()));
	QObject::connect(pWndTile, SIGNAL(triggered()), m_pmdi, SLOT(tileSubWindows()));
	QObject::connect(pWndCsc, SIGNAL(triggered()), m_pmdi, SLOT(cascadeSubWindows()));
	QObject::connect(pCloseAll, SIGNAL(triggered()), m_pmdi, SLOT(closeAllSubWindows()));

	QObject::connect(pReso, SIGNAL(triggered()), this, SLOT(ShowReso()));
	QObject::connect(pPhaseCorr, SIGNAL(triggered()), this, SLOT(ShowPSDPhaseCorr()));
	QObject::connect(pPSDPhase, SIGNAL(triggered()), this, SLOT(CalcPSDPhases()));
	QObject::connect(pFormulas, SIGNAL(triggered()), this, SLOT(ShowFormulas()));

	QObject::connect(pAbout, SIGNAL(triggered()), this, SLOT(ShowAbout()));
	QObject::connect(pBrowser, SIGNAL(triggered()), this, SLOT(ShowBrowser()));

	QObject::connect(pMenuWindows, SIGNAL(aboutToShow()), this, SLOT(UpdateSubWndList()));
	QObject::connect(m_pmdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(SubWindowChanged()));
	//--------------------------------------------------------------------------------

	LoadRecentFileList();
	LoadRecentSessionList();
}

MiezeMainWnd::~MiezeMainWnd()
{
	if(m_pcombinedlg) delete m_pcombinedlg;
	if(m_pfitdlg) delete m_pfitdlg;
	if(m_proidlg) delete m_proidlg;
	if(m_presdlg) delete m_presdlg;
	if(m_pphasecorrdlg) delete m_pphasecorrdlg;
	if(m_pradialintdlg) delete m_pradialintdlg;
	if(m_pformuladlg) delete m_pformuladlg;
	if(m_pplotpropdlg) delete m_pplotpropdlg;
	if(m_pexportdlg) delete m_pexportdlg;
}

QMdiSubWindow* MiezeMainWnd::FindSubWindow(SubWindowBase* pSWB)
{
	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow* pWnd : lst)
	{
		if(!pWnd) continue;
		SubWindowBase* pCurSWB = (SubWindowBase*)pWnd->widget();
		if(!pCurSWB) continue;

		if(pCurSWB==pSWB || pCurSWB->GetActualWidget()==pSWB)
			return pWnd;
	}

	return 0;
}

std::vector<SubWindowBase*> MiezeMainWnd::GetSubWindows(bool bResolveActualWidget)
{
	std::vector<SubWindowBase*> vec;
	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	vec.reserve(lst.size());

	for(QMdiSubWindow* pWnd : lst)
	{
		if(!pWnd) continue;
		SubWindowBase* pSWB = (SubWindowBase*)pWnd->widget();
		if(!pSWB) continue;

		if(bResolveActualWidget)
			pSWB = pSWB->GetActualWidget();

		vec.push_back(pSWB);
	}

	return vec;
}

void MiezeMainWnd::UpdateSubWndList()
{
	SubWindowBase *pSWBActive = GetActivePlot();

	// remove previous list
	for(QAction* pAction : m_vecSubWndActions)
	{
		pMenuWindows->removeAction(pAction);
		pAction->disconnect();
		delete pAction;
	}
	m_vecSubWndActions.clear();

	std::vector<SubWindowBase*> vec = GetSubWindows(0);

	// add new list
	for(SubWindowBase *pSWB : vec)
	{
		if(pSWB)
		{
			bool bActiveWindow = (pSWB == pSWBActive || pSWB->GetActualWidget() == pSWBActive);
			QString strTitle = pSWB->windowTitle();

			QAction *pAction = new QAction(pMenuWindows);
			pAction->setCheckable(1);
			pAction->setChecked(bActiveWindow);
			pAction->setText(strTitle);

			m_vecSubWndActions.push_back(pAction);
			pMenuWindows->addAction(pAction);

			QObject::connect(pAction, SIGNAL(triggered()), pSWB, SLOT(showNormal()));
			QObject::connect(pAction, SIGNAL(triggered()), pSWB, SLOT(setFocus()));
		}
	}
}

void MiezeMainWnd::SubWindowChanged()
{
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	if(!pWnd)
	{
		pMenuPlot->setEnabled(0);
		return;
	}

	SubWindowBase* pSWB = (SubWindowBase*)pWnd->widget();
	bool bSignal = 1;

	if(pSWB->GetType() == PLOT_1D)
	{
		pMenuPlot->clear();
		pMenuPlot->addActions(m_pMenu1d->actions());
		pMenuPlot->setEnabled(1);
	}
	else if(pSWB->GetType() == PLOT_2D)
	{
		pMenuPlot->clear();
		pMenuPlot->addActions(m_pMenu2d->actions());
		pMenuPlot->setEnabled(1);
	}
	else if(pSWB->GetType() == PLOT_3D)
	{
		pMenuPlot->clear();
		pMenuPlot->addActions(m_pMenu3d->actions());
		pMenuPlot->setEnabled(1);
	}
	else if(pSWB->GetType() == PLOT_4D)
	{
		pMenuPlot->clear();
		pMenuPlot->addActions(m_pMenu4d->actions());
		pMenuPlot->setEnabled(1);
	}
	else
	{
		pMenuPlot->setEnabled(0);
		bSignal = 0;
	}

	if(bSignal)
		emit SubWindowActivated(pSWB);
}

void MiezeMainWnd::SubWindowDestroyed(SubWindowBase *pSWB)
{
	emit SubWindowRemoved(pSWB);
}

void MiezeMainWnd::AddSubWindow(SubWindowBase* pWnd)
{
	if(!pWnd) return;

	pWnd->setParent(m_pmdi);
	SubWindowBase *pActualWidget = pWnd->GetActualWidget();
	QObject::connect(pWnd, SIGNAL(WndDestroyed(SubWindowBase*)), this, SLOT(SubWindowDestroyed(SubWindowBase*)));
	QObject::connect(pActualWidget, SIGNAL(SetStatusMsg(const char*, int)), this, SLOT(SetStatusMsg(const char*, int)));

	m_pmdi->addSubWindow(pWnd);
	emit SubWindowAdded(pWnd);

	pWnd->show();
}

void MiezeMainWnd::MakePlot(const Data1& dat, const std::string& strTitle)
{
	Plot *pPlot = new Plot(m_pmdi);
	pPlot->plot(dat);
	pPlot->setWindowTitle(strTitle.c_str());
	AddSubWindow(pPlot);
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
		double *pdErr = new double[iW*iH*iTcCnt];
		for(uint iFoil=0; iFoil<iFoilCnt; ++iFoil)
		{
			const uint* pDat = tof.GetData(iFoil);
			if(!pDat)
			{
				std::cerr << "Error: Could not load \"" << strFileNoDir << "\" correctly." << std::endl;
				break;
			}
			convert(pdDat, pDat, iW*iH*iTcCnt);

			//for(unsigned int iTc=0; iTc<iTcCnt; ++iTc)
			//	for(unsigned int iY=0; iY<iH; ++iY)
			//		convert(pdDat+iTc*iW*iH + iY*iW, pDat + iTc*iW*iH + (iH-iY-1)*iW, iW);

			tof.ReleaseData(pDat);

			apply_fkt(pdDat, pdErr, ::sqrt, iW*iH*iTcCnt);
			dat4.SetVals(iFoil, pdDat, pdErr);
		}
		delete[] pdDat;
		delete[] pdErr;

		pPlot->plot_manual();
		pPlot->SetLabels("x pixels", "y pixels", "");

		AddSubWindow(pPlotWrapper);
	}
	else if(is_equal(strExt, "pad"))
	{
		PadFile pad(strFile.c_str());
		if(!pad.IsOpen())
			return;

		const uint* pDat = pad.GetData();
		if(!pDat)
		{
			std::cerr << "Error: Could not load \"" << strFileNoDir << "\"." << std::endl;
			return;
		}

		const uint iW = pad.GetWidth();
		const uint iH = pad.GetHeight();

		double *pdDat = new double[iW*iH];
		convert(pdDat, pDat, iW*iH);
		//for(unsigned int iY=0; iY<iH; ++iY)
		//	convert(pdDat+iY*iW, pDat+(iH-iY-1)*iW, iW);

		std::string strTitle = GetPlotTitle(strFileNoDir);
		Plot2d *pPlot = new Plot2d(m_pmdi, strTitle.c_str(), true);

		pPlot->plot(iW, iH, pdDat);
		pPlot->SetLabels("x pixels", "y pixels", "");

		delete[] pdDat;

		AddSubWindow(pPlot);
	}
	else if(is_equal(strExt, "dat") || is_equal(strExt, "sim"))
	{
		LoadTxt * pdat = new LoadTxt();
		if(!pdat->Load(strFile.c_str()))
		{
			QString strErr = QString("Could not load \"") + QString(strFile.c_str()) +QString("\".");
			QMessageBox::critical(this, "Error", strErr);
			return;
		}

		TxtType dattype = pdat->GetFileType();

		if(dattype == MCSTAS_DATA)
		{
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

				if(Settings::Get<int>("general/sort_x"))
				{
					if(pdyerr)
						::sort_3<double*, double>((double*)pdx,
												(double*)pdx+pdat1d->GetDim(),
												(double*)pdy,
												(double*)pdyerr);
					else
						::sort_2<double*, double>((double*)pdx,
												(double*)pdx+pdat1d->GetDim(),
												(double*)pdy);
				}

				std::string strTitle = GetPlotTitle(strFileNoDir);

				Plot *pPlot = new Plot(m_pmdi, strTitle.c_str());

				pPlot->plot(pdat1d->GetDim(), pdx, pdy, pdyerr);

				std::string strLabX, strLabY, strPlotTitle;
				pdat1d->GetLabels(strLabX, strLabY);
				pdat1d->GetTitle(strPlotTitle);
				pPlot->SetLabels(strLabX.c_str(), strLabY.c_str());
				pPlot->SetTitle(strPlotTitle.c_str());

				bool bXLog=0, bYLog=0;
				pdat1d->GetLogScale(bXLog, bYLog);
				pPlot->SetXIsLog(bXLog);
				pPlot->SetYIsLog(bYLog);

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
				Data2& dat2 = pPlot->GetData2();

				std::string strLabX, strLabY, strLabZ, strPlotTitle;
				pdat2d->GetLabels(strLabX, strLabY, strLabZ);
				pdat2d->GetTitle(strPlotTitle);
				pPlot->SetLabels(strLabX.c_str(), strLabY.c_str(), strLabZ.c_str());
				pPlot->SetTitle(strPlotTitle.c_str());

				double dXMin, dXMax, dYMin, dYMax, dZMin, dZMax;
				if(pdat2d->GetLimits(dXMin, dXMax, dYMin, dYMax, dZMin, dZMax))
				{
					dat2.SetXRange(dXMin, dXMax);
					dat2.SetYRange(dYMin, dYMax);
				}

				bool bXLog=0, bYLog=0;
				pdat2d->GetLogScale(bXLog, bYLog);
				dat2.SetXYLog(bXLog, bYLog);

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
				Data3& dat3 = pPlot->GetData();

				std::string strLabX, strLabY, strLabZ, strPlotTitle;
				pdat3d->GetLabels(strLabX, strLabY, strLabZ);
				pdat3d->GetTitle(strPlotTitle);
				pPlot->SetLabels(strLabX.c_str(), strLabY.c_str(), strLabZ.c_str());
				pPlot->SetTitle(strPlotTitle.c_str());

				double dXMin, dXMax, dYMin, dYMax, dZMin, dZMax;
				if(pdat3d->GetLimits(dXMin, dXMax, dYMin, dYMax, dZMin, dZMax))
				{
					dat3.SetXRange(dXMin, dXMax);
					dat3.SetYRange(dYMin, dYMax);
				}

				bool bXLog=0, bYLog=0;
				pdat3d->GetLogScale(bXLog, bYLog);
				dat3.SetXYLog(bXLog, bYLog);

				delete[] pDat;
				delete[] pErr;
				delete pdat3d;

				AddSubWindow(pPlotWrapper);
			}
		}
		else if(dattype == NICOS_DATA)
		{
			NicosData * pnicosdat = new NicosData(*pdat);
			::autodeleter<NicosData> _a0(pnicosdat);
			const std::string strCtrName = Settings::Get<QString>("nicos/counter_name").toStdString();

			bool bSelectNewXColumn = 0;
			int iX = 0;
			if(m_strLastXColumn.length())
			{
				iX = pnicosdat->GetColIdx(m_strLastXColumn);
				if(iX == -1)
					bSelectNewXColumn = 1;
			}
			else
				bSelectNewXColumn = 1;

			if(bSelectNewXColumn)
			{
				ComboDlg dlg(this);
				dlg.SetCurFile(strFileNoDir.c_str());
				dlg.SetValues(pnicosdat->GetColNames());
				dlg.SetLabel("Select x value: ");

				if(dlg.exec() == QDialog::Accepted)
				{
					iX = dlg.GetSelectedValue();
					m_strLastXColumn = pnicosdat->GetColName(iX);
				}
				else
					return;
			}

			int iY = pnicosdat->GetColIdx(strCtrName);
			const double *pdx = pnicosdat->GetColumn(iX);
			const double *pdy = pnicosdat->GetColumn(iY);
			double *pdyerr = new double[pnicosdat->GetDim()];
			::apply_fkt<double>(pdy, pdyerr, sqrt, pnicosdat->GetDim());

			if(Settings::Get<int>("general/sort_x"))
			{
				if(pdyerr)
					::sort_3<double*, double>((double*)pdx,
											(double*)pdx+pnicosdat->GetDim(),
											(double*)pdy,
											pdyerr);
				else
					::sort_2<double*, double>((double*)pdx,
											(double*)pdx+pnicosdat->GetDim(),
											(double*)pdy);
			}

			std::string strTitle = GetPlotTitle(strFileNoDir);

			Plot *pPlot = new Plot(m_pmdi, strTitle.c_str());
			pPlot->plot(pnicosdat->GetDim(), pdx, pdy, pdyerr);

			delete[] pdyerr;

			std::string strLabX = pnicosdat->GetColName(iX) + std::string(" (") + pnicosdat->GetColUnit(iX) +std::string(")") ;
			std::string strLabY = pnicosdat->GetColName(iY) + std::string(" (") + pnicosdat->GetColUnit(iY) +std::string(")") ;
			std::string strPlotTitle;
			pPlot->SetLabels(strLabX.c_str(), strLabY.c_str());
			pPlot->SetTitle(strPlotTitle.c_str());

			AddSubWindow(pPlot);
		}
		else
		{
			QString strErr = "Unknown data format in file \"";
			strErr += strFile.c_str();
			strErr += "\".";
			QMessageBox::critical(this, "Error", "Unknown data format.");
			delete pdat;
			return;
		}
		delete pdat;
	}

	this->SetStatusMsg("Ok.",0);
	AddRecentFile(QString(strFile.c_str()));
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

	m_strLastXColumn = "";
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

void MiezeMainWnd::ROIManageTriggered()
{
	if(!m_proidlg->isVisible())
		m_proidlg->show();
	m_proidlg->activateWindow();
}

void MiezeMainWnd::GetActiveROI()
{
	SubWindowBase *pWnd = GetActivePlot();
	if(!pWnd)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	pWnd = pWnd->GetActualWidget();
	Roi* pRoi = pWnd->GetROI();
	if(!pRoi)
	{
		QMessageBox::critical(this, "Error", "No active roi defined.");
		return;
	}
	m_proidlg->SetRoi(pRoi);
}

void MiezeMainWnd::SetGlobalROIForAll()
{
	const Roi* pRoi = m_proidlg->GetRoi();

	std::vector<SubWindowBase*> vec = GetSubWindows(1);
	for(SubWindowBase *pWnd : vec)
	{
		pWnd->SetROI(pRoi);
		pWnd->repaint();
	}
}

void MiezeMainWnd::SetGlobalROIForActive()
{
	const Roi* pRoi = m_proidlg->GetRoi();

	SubWindowBase *pWnd = GetActivePlot();
	if(!pWnd)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	pWnd = pWnd->GetActualWidget();
	pWnd->SetROI(pRoi);

	pWnd->repaint();
}

void MiezeMainWnd::SettingsTriggered()
{
	SettingsDlg dlg(this);
	if(dlg.exec() == QDialog::Accepted)
	{

	}
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
		//pLabel->repaint();
	}
}

void MiezeMainWnd::ShowListWindowsDlg()
{
	ListGraphsDlg dlg(this);

	std::vector<SubWindowBase*> vec = GetSubWindows(0);
	for(SubWindowBase *pWnd : vec)
		dlg.AddSubWnd(pWnd);

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
	if(!m_pcombinedlg)
	{
		m_pcombinedlg = new CombineGraphsDlg(this);
		QObject::connect(this, SIGNAL(SubWindowRemoved(SubWindowBase*)), m_pcombinedlg, SLOT(SubWindowRemoved(SubWindowBase*)));
		QObject::connect(this, SIGNAL(SubWindowAdded(SubWindowBase*)), m_pcombinedlg, SLOT(SubWindowAdded(SubWindowBase*)));
		QObject::connect(m_pcombinedlg, SIGNAL(AddSubWindow(SubWindowBase*)), this, SLOT(AddSubWindow(SubWindowBase*)));

		std::vector<SubWindowBase*> vec = GetSubWindows(0);
		for(SubWindowBase *pWnd : vec)
			m_pcombinedlg->SubWindowAdded(pWnd);
	}

	m_pcombinedlg->show();
	m_pcombinedlg->activateWindow();
}

void MiezeMainWnd::IntAlongY()
{
	SubWindowBase* pSWB = GetActivePlot();
	if(!pSWB)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	if(pSWB->GetType() != PLOT_2D)
	{
		QMessageBox::critical(this, "Error", "Plot type mismatch.");
		return;
	}

	Plot2d* pPlot = (Plot2d*)pSWB;
	Data1 dat1 = pPlot->GetData2().SumY();

	MakePlot(dat1, pSWB->windowTitle().toStdString()+std::string(" -> y int"));
}

void MiezeMainWnd::IntRad()
{
	if(!m_pradialintdlg)
	{
		m_pradialintdlg = new RadialIntDlg(this);
		m_pradialintdlg->SetSubWindows(GetSubWindows());

		QObject::connect(this, SIGNAL(SubWindowRemoved(SubWindowBase*)), m_pradialintdlg, SLOT(SubWindowRemoved(SubWindowBase*)));
		QObject::connect(this, SIGNAL(SubWindowAdded(SubWindowBase*)), m_pradialintdlg, SLOT(SubWindowAdded(SubWindowBase*)));
		QObject::connect(m_pradialintdlg, SIGNAL(NewSubWindow(SubWindowBase*)), this, SLOT(AddSubWindow(SubWindowBase*)));
	}

	m_pradialintdlg->show();
	m_pradialintdlg->activateWindow();
}

SubWindowBase* MiezeMainWnd::GetActivePlot(bool bResolveWidget)
{
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	if(pWnd && pWnd->widget())
	{
		SubWindowBase* pWndBase = (SubWindowBase*)pWnd->widget();
		if(!pWndBase)
			return 0;

		if(bResolveWidget)
			pWndBase = pWndBase->GetActualWidget();

		return pWndBase;
	}

	return 0;
}

void MiezeMainWnd::ShowFitDlg()
{
	if(!m_pfitdlg)
	{
		m_pfitdlg = new FitDlg(this, m_pmdi);
		QObject::connect(m_pfitdlg, SIGNAL(AddSubWindow(SubWindowBase*)), this, SLOT(AddSubWindow(SubWindowBase*)));
		QObject::connect(this, SIGNAL(SubWindowRemoved(SubWindowBase*)), m_pfitdlg, SLOT(SubWindowRemoved(SubWindowBase*)));
	}

	m_pfitdlg->show();
	m_pfitdlg->activateWindow();
}

void MiezeMainWnd::QuickFitMIEZE() { QuickFit((SubWindowBase*)GetActivePlot(), FIT_MIEZE_SINE); }
void MiezeMainWnd::QuickFitGauss() { QuickFit((SubWindowBase*)GetActivePlot(), FIT_GAUSSIAN); }
void MiezeMainWnd::QuickFitDoubleGauss() { QuickFit((SubWindowBase*)GetActivePlot(), FIT_MULTI_GAUSSIAN, 2); }
void MiezeMainWnd::QuickFitTripleGauss() { QuickFit((SubWindowBase*)GetActivePlot(), FIT_MULTI_GAUSSIAN, 3); }

void MiezeMainWnd::QuickFit(SubWindowBase* pSWB, int iFkt, int iParam)
{
	if(!pSWB)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	SpecialFitResult res = FitDlg::DoSpecialFit(pSWB, iFkt, iParam);
	if(!res.bOk)
	{
		QMessageBox::critical(this, "Error", res.strErr.c_str());
		if(res.pPlot && res.bCreatedNewPlot) delete res.pPlot;
		return;
	}

	if(res.bCreatedNewPlot)
	{
		res.pPlot->setParent(m_pmdi);
		AddSubWindow(res.pPlot);
	}
}

void MiezeMainWnd::Interpolation(SubWindowBase* pSWB, InterpFkt iFkt)
{
	if(!pSWB)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	if(pSWB->GetType() != PLOT_1D)
	{
		QMessageBox::critical(this, "Error", "Wrong data type, need 1D.");
		return;
	}

	Plot *pPlot = (Plot*)pSWB;
	Data1& dat = pPlot->GetData(0).dat;

	const std::vector<double> *pvecDatX, *pvecDatY;
	dat.GetData(&pvecDatX, &pvecDatY);
	const double *px = ::vec_to_array(*pvecDatX);
	const double *py = ::vec_to_array(*pvecDatY);
	const unsigned int iLen = pvecDatX->size();

	if(iFkt == INTERP_BEZIER)
	{
		Bezier bezier(iLen, px, py);
		pPlot->plot_param(bezier);
	}
	else if(iFkt == INTERP_BSPLINE)
	{
		const int iDegree = Settings::Get<int>("interpolation/spline_degree");

		BSpline spline(iLen, px, py, iDegree);
		pPlot->plot_param(spline);
	}
	else
	{
		QMessageBox::critical(this, "Error", "Unknown interpolation function.");
	}

	delete[] px;
	delete[] py;

	pPlot->RefreshPlot();
}

void MiezeMainWnd::BSplineInterpolation() { Interpolation(GetActivePlot(), INTERP_BSPLINE); }
void MiezeMainWnd::BezierInterpolation() { Interpolation(GetActivePlot(), INTERP_BEZIER); }


void MiezeMainWnd::QuickFitMIEZEpixel()
{
	SubWindowBase *pSWB = this->GetActivePlot();
	if(!pSWB)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	if(pSWB->GetType()!=PLOT_3D && pSWB->GetType()!=PLOT_4D)
	{
		QMessageBox::critical(this, "Error", "Wrong data type.");
		return;
	}

	SpecialFitPixelResult res = FitDlg::DoSpecialFitPixel(pSWB, 0, FIT_MIEZE_SINE_PIXELWISE_FFT);
	if(!res.bOk)
	{
		QMessageBox::critical(this, "Error", res.strErr.c_str());
		if(res.pPlot[0]) delete res.pPlot[0];
		if(res.pPlot[1]) delete res.pPlot[1];
		return;
	}

	AddSubWindow(res.pPlot[0]);
	if(res.pPlot[1]) delete res.pPlot[1];
}

Plot* MiezeMainWnd::Convert3d1d(Plot3d* pPlot3d)
{
	Plot* pPlot = pPlot3d->ConvertTo1d();
	if(pPlot)
	{
		pPlot->setParent(m_pmdi);
		AddSubWindow(pPlot);
	}
	return pPlot;
}

Plot* MiezeMainWnd::Convert4d1d(Plot4d* pPlot4d, int iFoil)
{
	Plot* pPlot = pPlot4d->ConvertTo1d(iFoil);
	if(pPlot)
	{
		pPlot->setParent(m_pmdi);
		AddSubWindow(pPlot);
	}
	return pPlot;
}

void MiezeMainWnd::ShowAbout()
{
	AboutDlg dlg(this);
	dlg.exec();
}

void MiezeMainWnd::ShowBrowser()
{
#ifdef Q_WS_WIN
	QMessageBox::critical(this, "Error", "Too many browser toolbars already installed.\nCannot add more.");
#else
	QMessageBox::information(this, "Unsupported", "This is not the right operating system\nfor such nonsense.");
#endif
}


// --------------------------------------------------------------------------------
// recent files
void MiezeMainWnd::AddRecentFile(const QString& strFile)
{
	m_lstRecentFiles.push_front(strFile);
	m_lstRecentFiles.removeDuplicates();

	if(m_lstRecentFiles.size() > MAX_RECENT_FILES)
	{
		QStringList::iterator iter = m_lstRecentFiles.begin();
		for(unsigned int i=0; i<MAX_RECENT_FILES; ++i) ++iter;
		m_lstRecentFiles.erase(iter, m_lstRecentFiles.end());
	}

	Settings::Set<QStringList>("general/recent_files", m_lstRecentFiles);
	UpdateRecentFileMenu();
}

void MiezeMainWnd::UpdateRecentFileMenu()
{
	m_pMenuLoadRecent->clear();

	for(QStringList::iterator iter=m_lstRecentFiles.begin(); iter!=m_lstRecentFiles.end(); ++iter)
	{
		QAction *pRec = new QAction(this);
		pRec->setText(*iter);
		m_pMenuLoadRecent->addAction(pRec);

		QSignalMapper *pSigs = new QSignalMapper(this);
		pSigs->setMapping(pRec, pRec->text());
		QObject::connect(pRec, SIGNAL(triggered()), pSigs, SLOT(map()));
		QObject::connect(pSigs, SIGNAL(mapped(const QString&)), this, SLOT(LoadFile(const QString&)));
	}
}

void MiezeMainWnd::LoadRecentFileList()
{
	m_lstRecentFiles = Settings::Get<QStringList>("general/recent_files");
	m_lstRecentFiles.removeDuplicates();

	UpdateRecentFileMenu();
}
// --------------------------------------------------------------------------------



// --------------------------------------------------------------------------------
// recent sessions
void MiezeMainWnd::AddRecentSession(const QString& strFile)
{
	m_lstRecentSessions.push_front(strFile);
	m_lstRecentSessions.removeDuplicates();

	if(m_lstRecentSessions.size() > MAX_RECENT_FILES)
	{
		QStringList::iterator iter = m_lstRecentSessions.begin();
		for(unsigned int i=0; i<MAX_RECENT_FILES; ++i) ++iter;
		m_lstRecentSessions.erase(iter, m_lstRecentSessions.end());
	}

	Settings::Set<QStringList>("general/recent_sessions", m_lstRecentSessions);
	UpdateRecentSessionMenu();
}

void MiezeMainWnd::UpdateRecentSessionMenu()
{
	m_pMenuLoadRecentSession->clear();

	for(QStringList::iterator iter=m_lstRecentSessions.begin(); iter!=m_lstRecentSessions.end(); ++iter)
	{
		QAction *pRec = new QAction(this);
		pRec->setText(*iter);
		m_pMenuLoadRecentSession->addAction(pRec);

		QSignalMapper *pSigs = new QSignalMapper(this);
		pSigs->setMapping(pRec, pRec->text());
		QObject::connect(pRec, SIGNAL(triggered()), pSigs, SLOT(map()));
		QObject::connect(pSigs, SIGNAL(mapped(const QString&)), this, SLOT(LoadSession(const QString&)));
	}
}

void MiezeMainWnd::LoadRecentSessionList()
{
	m_lstRecentSessions = Settings::Get<QStringList>("general/recent_sessions");
	m_lstRecentSessions.removeDuplicates();

	UpdateRecentSessionMenu();
}
// --------------------------------------------------------------------------------




void MiezeMainWnd::LoadFile(const QString& strFile)
{
	m_strLastXColumn = "";
	LoadFile(strFile.toStdString());
}

void MiezeMainWnd::LoadSession(const QString& strFile)
{
	LoadSession(strFile.toStdString());
}


void MiezeMainWnd::ShowReso()
{
	if(!m_presdlg)
		m_presdlg = new ResoDlg(this);

	m_presdlg->show();
	m_presdlg->activateWindow();
}

void MiezeMainWnd::CalcPSDPhases()
{
	PsdPhaseDlg dlg(this);
	if(dlg.exec() == QDialog::Accepted)
	{
		std::ostringstream ostr;
		ostr << "PSD Phases for tau=" << dlg.GetTau() << "ps";
		Plot2d *pPlot = new Plot2d(this, ostr.str().c_str(), 0, 1);
		pPlot->SetLabels("x Position (cm)", "y Position (cm)", "Phase (rad)");
		pPlot->plot(dlg.GetData());
		AddSubWindow(pPlot);
	}
}

void MiezeMainWnd::ShowFormulas()
{
	if(!m_pformuladlg)
		m_pformuladlg = new FormulaDlg(this);

	m_pformuladlg->show();
	m_pformuladlg->activateWindow();
}

void MiezeMainWnd::ShowPSDPhaseCorr()
{
	if(!m_pphasecorrdlg)
	{
		m_pphasecorrdlg = new PsdPhaseCorrDlg(this, m_pmdi);
		QObject::connect(this, SIGNAL(SubWindowRemoved(SubWindowBase*)), m_pphasecorrdlg, SLOT(RefreshPhaseCombo()));
		QObject::connect(this, SIGNAL(SubWindowRemoved(SubWindowBase*)), m_pphasecorrdlg, SLOT(SubWindowRemoved(SubWindowBase*)));
		QObject::connect(this, SIGNAL(SubWindowAdded(SubWindowBase*)), m_pphasecorrdlg, SLOT(RefreshPhaseCombo()));

		QObject::connect(m_pphasecorrdlg, SIGNAL(AddNewPlot(SubWindowBase*)), this, SLOT(AddSubWindow(SubWindowBase*)));
	}

	m_pphasecorrdlg->show();
	m_pphasecorrdlg->activateWindow();
}


void MiezeMainWnd::ShowExportDlg()
{
	if(!m_pexportdlg)
	{
		m_pexportdlg = new ExportDlg(this, m_pmdi);
		QObject::connect(this, SIGNAL(SubWindowRemoved(SubWindowBase*)), m_pexportdlg, SLOT(SubWindowRemoved(SubWindowBase*)));
	}

	m_pexportdlg->show();
	m_pexportdlg->activateWindow();
}

void MiezeMainWnd::FileExportPyTriggered()
{
	const SubWindowBase *pSWB = GetActivePlot();
	if(!pSWB)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir_py", ".").toString();

	QString strFile = QFileDialog::getSaveFileName(this, "Save as Python file...", strLastDir,
					"Python files (*.py)",
					0, QFileDialog::DontUseNativeDialog);
	if(strFile == "")
		return;

	std::string strFile1 = strFile.toStdString();
	std::string strExt = get_fileext(strFile1);
	if(strExt != "py")
		strFile1 += ".py";

	if(export_py(strFile1.c_str(), pSWB))
		pGlobals->setValue("main/lastdir_py", QString(::get_dir(strFile1).c_str()));
	else
		QMessageBox::critical(this, "Error", "Export to Python failed.");
}


// --------------------------------------------------------------------------------
// session loading/saving
void MiezeMainWnd::LoadSession(const std::string& strSess)
{
	Xml xml;
	if(!xml.Load(strSess.c_str()))
	{
		QMessageBox::critical(this, "Error", "Failed to load session.");
		return;
	}

	Blob blob((strSess+".blob").c_str());
	bool bHasBlob = blob.IsOpen();

	//m_pmdi->closeAllSubWindows();
	m_strCurSess = strSess;

	std::string strBase = "/cattus_session/";
	m_iPlotCnt = xml.Query<unsigned int>((strBase + "plot_counter").c_str(), 0);
	unsigned int iWndCnt = xml.Query<unsigned int>((strBase + "window_counter").c_str(), 0);

	for(unsigned int iWnd=0; iWnd<iWndCnt; ++iWnd)
	{
		std::ostringstream ostrSWBase;
		ostrSWBase << strBase << "window_" << iWnd << "/";
		std::string strSWBase = ostrSWBase.str();
		std::string strSWType = xml.QueryString((strSWBase + "type").c_str(), "");

		SubWindowBase *pSWB = 0;
		if(strSWType == "plot_1d")
			pSWB = new Plot(m_pmdi);
		else if(strSWType == "plot_2d")
			pSWB = new Plot2d(m_pmdi);
		else if(strSWType == "plot_3d")
			pSWB = new Plot3dWrapper(m_pmdi);
		else if(strSWType == "plot_4d")
			pSWB = new Plot4dWrapper(m_pmdi);
		else
		{
			std::cerr << "Error: Unknown plot type: \"" << strSWType << "\"."
						<< std::endl;
			continue;
		}

		if(pSWB)
		{
			pSWB->LoadXML(xml, blob, strSWBase);
			AddSubWindow(pSWB);

			QMdiSubWindow *pSubWnd = FindSubWindow(pSWB);

			std::string strGeo = xml.QueryString((strSWBase+"geo").c_str(), "");
			if(pSubWnd && strGeo != "")
				pSubWnd->restoreGeometry(QByteArray::fromHex(strGeo.c_str()));

			pSWB->GetActualWidget()->RefreshPlot();
		}
	}

	setWindowTitle((std::string(WND_TITLE) + " - " + get_file(m_strCurSess)).c_str());

	QSettings *pGlobals = Settings::GetGlobals();
	pGlobals->setValue("main/lastdir_session", QString(::get_dir(strSess).c_str()));
	AddRecentSession(QString(strSess.c_str()));
}

void MiezeMainWnd::SessionLoadTriggered()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir_session", ".").toString();
	QString strFile = QFileDialog::getOpenFileName(this, "Load Session...", strLastDir,
					"Session Files (*.cattus)", 0, QFileDialog::DontUseNativeDialog);
	if(strFile == "")
		return;

	m_pmdi->closeAllSubWindows();
	LoadSession(strFile.toStdString());
}

void MiezeMainWnd::SessionSaveTriggered()
{
	if(m_strCurSess=="")
	{
		SessionSaveAsTriggered();
		return;
	}

	std::vector<SubWindowBase*> vecWnd = GetSubWindows(0);

	std::ofstream ofstr(m_strCurSess);
	ofstr << "<cattus_session>\n\n";
	ofstr << "<plot_counter> " << m_iPlotCnt << " </plot_counter>\n";
	ofstr << "<window_counter> " << vecWnd.size() << " </window_counter>\n";

	std::ofstream ofstrBlob(m_strCurSess + ".blob", std::ofstream::binary);

	unsigned int iWnd=0;

	for(SubWindowBase *pWnd : vecWnd)
	{
		std::ostringstream ostrMsg;
		ostrMsg << "Saving \"" << pWnd->windowTitle().toStdString() << "\"...";
		this->SetStatusMsg(ostrMsg.str().c_str(), 0);

		pWnd = pWnd->GetActualWidget();

		ofstr << "<window_" << iWnd << ">\n";
		QMdiSubWindow *pSubWnd = FindSubWindow(pWnd);
		if(pSubWnd)
		{
			std::string strGeo = pSubWnd->saveGeometry().toHex().data();
			ofstr << "<geo> " << strGeo << " </geo>\n";
		}
		pWnd->SaveXML(ofstr, ofstrBlob);
		ofstr << "</window_" << iWnd << ">\n";

		++iWnd;
	}

	ofstr << "\n\n</cattus_session>\n";

	ofstrBlob.close();
	ofstr.close();

	AddRecentSession(QString(m_strCurSess.c_str()));
	this->SetStatusMsg("Ok.", 0);
}

void MiezeMainWnd::SessionSaveAsTriggered()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir_session", ".").toString();
	QString strFile = QFileDialog::getSaveFileName(this, "Save Session as...", strLastDir,
					"Session Files (*.cattus)", 0, QFileDialog::DontUseNativeDialog);
	if(strFile == "")
		return;

	std::string strFile1 = strFile.toStdString();
	std::string strExt = get_fileext(strFile1);
	if(strExt != "cattus")
		strFile1 += ".cattus";

	m_strCurSess = strFile1;
	SessionSaveTriggered();

	setWindowTitle((std::string(WND_TITLE) + " - " + get_file(m_strCurSess)).c_str());
	pGlobals->setValue("main/lastdir_session", QString(::get_dir(strFile1).c_str()));
}
// --------------------------------------------------------------------------------


// --------------------------------------------------------------------------------
// plot specific stuff
void MiezeMainWnd::ExtractFoils()
{
	SubWindowBase* pSWB = GetActivePlot();
	if(!pSWB) return;
	pSWB = pSWB->GetActualWidget();
	if(pSWB->GetType() != PLOT_4D)
		return;

	Plot4d* pPlot = (Plot4d*)pSWB;
	for(unsigned int iFoil=0; iFoil<pPlot->GetNumF(); ++iFoil)
	{
		Plot3d* pPlot3d = pPlot->ConvertTo3d(iFoil);
		Plot3dWrapper *pWrap = new Plot3dWrapper(pPlot3d);

		AddSubWindow(pWrap);
	}
}

void MiezeMainWnd::ShowTimeChannels()
{
	SubWindowBase* pSWB = GetActivePlot();
	if(!pSWB) return;
	pSWB = pSWB->GetActualWidget();

	if(pSWB->GetType()==PLOT_3D)
	{
		Plot3d* pPlot3 = (Plot3d*)pSWB;
		Plot* pPlot = pPlot3->ConvertTo1d(0);
		AddSubWindow(pPlot);
	}
	else if(pSWB->GetType()==PLOT_4D)
	{
		Plot4d* pPlot4 = (Plot4d*)pSWB;
		for(unsigned int iFoil=0; iFoil<pPlot4->GetNumF(); ++iFoil)
		{
			Plot* pPlot = pPlot4->ConvertTo1d(iFoil);
			AddSubWindow(pPlot);
		}
	}
}

void MiezeMainWnd::PlotPropertiesTriggered()
{
	if(!m_pplotpropdlg)
	{
		m_pplotpropdlg = new PlotPropDlg(this);
		QObject::connect(this, SIGNAL(SubWindowActivated(SubWindowBase*)), m_pplotpropdlg, SLOT(SubWindowActivated(SubWindowBase*)));
		QObject::connect(this, SIGNAL(SubWindowRemoved(SubWindowBase*)), m_pplotpropdlg, SLOT(SubWindowRemoved(SubWindowBase*)));

		m_pplotpropdlg->SubWindowActivated(GetActivePlot(0));
	}

	m_pplotpropdlg->show();
	m_pplotpropdlg->activateWindow();
}
// --------------------------------------------------------------------------------


#include "mainwnd.moc"
#include "subwnd.moc"
#include "dialogs/AboutDlg.moc"
