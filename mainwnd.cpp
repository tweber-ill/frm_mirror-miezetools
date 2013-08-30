/*
 * mieze-tool
 * main mdi window
 * @author tweber
 * @date 04-mar-2013
 */

#include "mainwnd.h"

#include <QtGui/QLabel>
#include <QtGui/QMenuBar>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtCore/QSignalMapper>

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include "settings.h"

#include "helper/string.h"
#include "helper/file.h"
#include "helper/misc.h"
#include "helper/mieze.hpp"
#include "helper/xml.h"

#include "dialogs/ListDlg.h"
#include "dialogs/SettingsDlg.h"
#include "dialogs/AboutDlg.h"
#include "dialogs/ComboDlg.h"

#include "fitter/models/msin.h"
#include "fitter/models/gauss.h"
#include "fitter/models/interpolation.h"

#include "data/export.h"



MiezeMainWnd::MiezeMainWnd()
					: m_pmdi(new QMdiArea(this)),
					  m_pinfo(new InfoDock(this)),
					  m_iPlotCnt(1),
					  m_pcombinedlg(0), m_pfitdlg(0),
					  m_proidlg(new RoiDlg(this)),
					  m_pantiroidlg(new RoiDlg(this)),
					  m_presdlg(0), m_pphasecorrdlg(0),
					  m_pradialintdlg(0),
					  m_pformuladlg(0),
					  m_pplotpropdlg(0),
					  m_pexportdlg(0)
{
	this->setWindowTitle(WND_TITLE);
	this->addDockWidget(Qt::RightDockWidgetArea, m_pinfo);
	m_proidlg->setWindowTitle("Inclusive ROI");
	m_pantiroidlg->setWindowTitle("Exclusive ROI");

	m_pmdi->setActivationOrder(QMdiArea::StackingOrder);
	//m_pmdi->setViewMode(QMdiArea::TabbedView);
	//m_pmdi->setDocumentMode(1);
	//m_pmdi->setTabPosition(QTabWidget::South);
	m_pmdi->setViewMode(QMdiArea::SubWindowView);
	m_pmdi->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, 1);
	m_pmdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_pmdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

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

	QAction *pSumFoils = new QAction(this);
	pSumFoils->setText("Sum all Foils / Time Channels");

	QAction *pExportPy = new QAction(this);
	pExportPy->setText("Export as Python Script...");

	QAction *pPlotProp = new QAction(this);
	pPlotProp->setText("Plot Properties...");


	m_pMenu1d = new QMenu(this);
	m_pMenu1d->addAction(pExportPy);
	m_pMenu1d->addSeparator();
	m_pMenu1d->addAction(pPlotProp);


	m_pMenu2d = new QMenu(this);
	m_pMenu2d->addAction(pExportPy);
	m_pMenu2d->addSeparator();
	m_pMenu2d->addAction(pPlotProp);


	m_pMenu3d = new QMenu(this);
	m_pMenu3d->addAction(pExportPy);
	m_pMenu3d->addSeparator();
	m_pMenu3d->addAction(pShowT);
	m_pMenu3d->addAction(pSumFoils);
	m_pMenu3d->addSeparator();
	m_pMenu3d->addAction(pPlotProp);


	m_pMenu4d = new QMenu(this);
	m_pMenu4d->addAction(pExportPy);
	m_pMenu4d->addSeparator();

	QAction *pExtractFoils = new QAction(m_pMenu4d);
	pExtractFoils->setText("Extract Foils");

	m_pMenu4d->addAction(pExtractFoils);
	m_pMenu4d->addAction(pShowT);
	m_pMenu4d->addAction(pSumFoils);
	m_pMenu4d->addSeparator();
	m_pMenu4d->addAction(pPlotProp);



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
	pManageROI->setText("Manage Inclusive ROI...");
	pManageROI->setIcon(QIcon::fromTheme("list-add"));
	pMenuROI->addAction(pManageROI);

	QAction *pManageAntiROI = new QAction(this);
	pManageAntiROI->setText("Manage Exclusive ROI...");
	pManageAntiROI->setIcon(QIcon::fromTheme("list-remove"));
	pMenuROI->addAction(pManageAntiROI);



	// Windows
	/*QMenu**/ pMenuWindows = new QMenu(this);
	pMenuWindows->setTitle("Windows");

	QAction *pWndList = new QAction(this);
	pWndList->setText("List Windows...");
	pMenuWindows->addAction(pWndList);

	QAction *pInfoWnd = new QAction(this);
	pInfoWnd->setText("Toggle Info Pane");
	pMenuWindows->addAction(pInfoWnd);

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
	QObject::connect(pSumFoils, SIGNAL(triggered()), this, SLOT(SumFoils()));
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

	QObject::connect(pManageAntiROI, SIGNAL(triggered()), this, SLOT(AntiROIManageTriggered()));
	QObject::connect(m_pantiroidlg, SIGNAL(WantActiveRoi()), this, SLOT(GetActiveAntiROI()));
	QObject::connect(m_pantiroidlg, SIGNAL(SetRoiForActive()), this, SLOT(SetGlobalAntiROIForActive()));
	QObject::connect(m_pantiroidlg, SIGNAL(SetRoiForAll()), this, SLOT(SetGlobalAntiROIForAll()));


	QObject::connect(pInfoWnd, SIGNAL(triggered()), this, SLOT(ToggleInfoWindow()));
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
	if(m_pantiroidlg) delete m_pantiroidlg;
	if(m_presdlg) delete m_presdlg;
	if(m_pphasecorrdlg) delete m_pphasecorrdlg;
	if(m_pradialintdlg) delete m_pradialintdlg;
	if(m_pformuladlg) delete m_pformuladlg;
	if(m_pplotpropdlg) delete m_pplotpropdlg;
	if(m_pexportdlg) delete m_pexportdlg;

	if(m_pinfo) delete m_pinfo;
	if(m_pmdi) delete m_pmdi;
}


void MiezeMainWnd::MakePlot(const Data1& dat, const std::string& strTitle)
{
	Plot *pPlot = new Plot(m_pmdi);
	pPlot->plot(dat);
	pPlot->setWindowTitle(strTitle.c_str());
	AddSubWindow(pPlot);
}

std::string MiezeMainWnd::GetPlotTitle(const std::string& strFile)
{
	std::ostringstream ostrTitle;
	ostrTitle << "Plot #" << (m_iPlotCnt++) << " - " << strFile;
	return ostrTitle.str();
}



// --------------------------------------------------------------------------------
// ROI stuff
void MiezeMainWnd::ROIManageTriggered()
{
	if(!m_proidlg->isVisible())
		m_proidlg->show();
	m_proidlg->activateWindow();
}

void MiezeMainWnd::AntiROIManageTriggered()
{
	if(!m_pantiroidlg->isVisible())
		m_pantiroidlg->show();
	m_pantiroidlg->activateWindow();
}

void MiezeMainWnd::_GetActiveROI(bool bAntiRoi)
{
	RoiDlg *proidlg = bAntiRoi ? m_pantiroidlg : m_proidlg;

	SubWindowBase *pWnd = GetActivePlot();
	if(!pWnd)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	pWnd = pWnd->GetActualWidget();
	Roi* pRoi = pWnd->GetROI(bAntiRoi);
	if(!pRoi)
	{
		QMessageBox::critical(this, "Error", "No active roi defined.");
		return;
	}
	proidlg->SetRoi(pRoi);
}

void MiezeMainWnd::_SetGlobalROIForAll(bool bAntiRoi)
{
	RoiDlg *proidlg = bAntiRoi ? m_pantiroidlg : m_proidlg;
	const Roi* pRoi = proidlg->GetRoi();

	std::vector<SubWindowBase*> vec = GetSubWindows(1);
	for(SubWindowBase *pWnd : vec)
	{
		pWnd->SetROI(pRoi, bAntiRoi);
		pWnd->repaint();
	}
}

void MiezeMainWnd::_SetGlobalROIForActive(bool bAntiRoi)
{
	RoiDlg *proidlg = bAntiRoi ? m_pantiroidlg : m_proidlg;
	const Roi* pRoi = proidlg->GetRoi();

	SubWindowBase *pWnd = GetActivePlot();
	if(!pWnd)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	pWnd = pWnd->GetActualWidget();
	pWnd->SetROI(pRoi, bAntiRoi);

	pWnd->repaint();
}

void MiezeMainWnd::GetActiveROI() { _GetActiveROI(0); }
void MiezeMainWnd::SetGlobalROIForAll() { _SetGlobalROIForAll(0); }
void MiezeMainWnd::SetGlobalROIForActive() { _SetGlobalROIForActive(0); }

void MiezeMainWnd::GetActiveAntiROI() { _GetActiveROI(1); }
void MiezeMainWnd::SetGlobalAntiROIForAll() { _SetGlobalROIForAll(1); }
void MiezeMainWnd::SetGlobalAntiROIForActive() { _SetGlobalROIForActive(1); }
// --------------------------------------------------------------------------------



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

void MiezeMainWnd::ToggleInfoWindow()
{
	if(!m_pinfo->isVisible())
		m_pinfo->show();
	else
		m_pinfo->hide();
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


// --------------------------------------------------------------------------------
// fit
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
// --------------------------------------------------------------------------------



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


// --------------------------------------------------------------------------------
// export
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
					"Python files (*.py)"/*,0, QFileDialog::DontUseNativeDialog*/);
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

void MiezeMainWnd::SumFoils()
{
	SubWindowBase* pSWB = GetActivePlot();
	if(!pSWB) return;
	pSWB = pSWB->GetActualWidget();
	if(pSWB->GetType()!=PLOT_4D && !pSWB->GetType()!=PLOT_3D)
		return;

	Plot2d* pPlot2d = pSWB->ConvertTo2d();
	AddSubWindow(pPlot2d);
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
