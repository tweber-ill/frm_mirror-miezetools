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

#include "settings.h"

#include "helper/string.h"
#include "helper/file.h"
#include "helper/misc.h"

#include "loader/loadtxt.h"
#include "loader/loadnicos.h"
#include "loader/loadcasc.h"

#include "dialogs/ListDlg.h"
#include "dialogs/CombineDlg.h"
#include "dialogs/SettingsDlg.h"
#include "dialogs/AboutDlg.h"
#include "dialogs/ComboDlg.h"

#include "fitter/models/msin.h"
#include "fitter/models/gauss.h"


MiezeMainWnd::MiezeMainWnd()
					: m_iPlotCnt(1), m_pfitdlg(0),
					  m_proidlg(new RoiDlg(this)), m_bmainROIActive(0)
{
	this->setWindowTitle("Cattus, a MIEZE toolset");

	m_pmdi = new QMdiArea(this);
	this->setCentralWidget(m_pmdi);


	//--------------------------------------------------------------------------------
	// Menus

	// File
	QMenu* pMenuFile = new QMenu(this);
	pMenuFile->setTitle("File");

	QAction *pLoad = new QAction(this);
	pLoad->setText("Open...");
	pLoad->setIcon(QIcon::fromTheme("document-open"));
	pLoad->setShortcut(Qt::CTRL + Qt::Key_L);
	pMenuFile->addAction(pLoad);

	QAction *pCloseAll = new QAction(this);
	pCloseAll->setText("Close All");
	pCloseAll->setIcon(QIcon::fromTheme("window-close"));
	pMenuFile->addAction(pCloseAll);

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



	// Fit
	QMenu *pMenuFit = new QMenu(this);
	pMenuFit->setTitle("Fit");

	QAction *pFit = new QAction(this);
	pFit->setText("Fit Function...");
	pMenuFit->addAction(pFit);

	QMenu *pMenuQuickFit = new QMenu(pMenuFit);
	pMenuQuickFit->setTitle("Quick Fit");

	QAction *pQFitMieze = new QAction(this);
	pQFitMieze->setText("MIEZE Sine Signal");
	pMenuQuickFit->addAction(pQFitMieze);

	QAction *pQFitMiezeArea = new QAction(this);
	pQFitMiezeArea->setText("MIEZE Sine Signal (pixel-wise)");
	pMenuQuickFit->addAction(pQFitMiezeArea);

	QAction *pQFitGauss = new QAction(this);
	pQFitGauss->setText("Gaussian");
	pMenuQuickFit->addAction(pQFitGauss);

	pMenuFit->addMenu(pMenuQuickFit);



	// Tools
	QMenu *pMenuTools = new QMenu(this);
	pMenuTools->setTitle("Tools");

	QAction *pCombineGraphs = new QAction(this);
	pCombineGraphs->setText("Plot Counts/Contrasts...");
	pMenuTools->addAction(pCombineGraphs);


	// ROI
	QMenu *pMenuROI = new QMenu(this);
	pMenuROI->setTitle("ROI");

	QAction *pManageROI = new QAction(this);
	pManageROI->setText("Manage ROI...");
	pManageROI->setIcon(QIcon::fromTheme("list-add"));
	pMenuROI->addAction(pManageROI);

	pMenuROI->addSeparator();

	QAction *pLoadROI = new QAction(this);
	pLoadROI->setText("Load ROI...");
	pLoadROI->setIcon(QIcon::fromTheme("document-open"));
	pMenuROI->addAction(pLoadROI);

	QAction *pSaveROI = new QAction(this);
	pSaveROI->setText("Save ROI...");
	pSaveROI->setIcon(QIcon::fromTheme("document-save-as"));
	pMenuROI->addAction(pSaveROI);

	pMenuROI->addSeparator();

	QAction *pActiveROI = new QAction(this);
	pActiveROI->setText("Global ROI Active");
	pActiveROI->setCheckable(1);
	pActiveROI->setChecked(m_bmainROIActive);
	pMenuROI->addAction(pActiveROI);



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
	pMenuBar->addMenu(pMenuFit);
	pMenuBar->addMenu(pMenuTools);
	pMenuBar->addMenu(pMenuROI);
	pMenuBar->addMenu(pMenuWindows);
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

	QObject::connect(pCombineGraphs, SIGNAL(triggered()), this, SLOT(ShowCombineGraphsDlg()));
	QObject::connect(pFit, SIGNAL(triggered()), this, SLOT(ShowFitDlg()));
	QObject::connect(pQFitMieze, SIGNAL(triggered()), this, SLOT(QuickFitMIEZE()));
	QObject::connect(pQFitMiezeArea, SIGNAL(triggered()), this, SLOT(QuickFitMIEZEpixel()));
	QObject::connect(pQFitGauss, SIGNAL(triggered()), this, SLOT(QuickFitGauss()));

	QObject::connect(pManageROI, SIGNAL(triggered()), this, SLOT(ROIManageTriggered()));
	QObject::connect(pLoadROI, SIGNAL(triggered()), this, SLOT(ROILoadTriggered()));
	QObject::connect(pSaveROI, SIGNAL(triggered()), this, SLOT(ROISaveTriggered()));
	QObject::connect(pActiveROI, SIGNAL(toggled(bool)), this, SLOT(SetGlobalROI(bool)));

	QObject::connect(pWndList, SIGNAL(triggered()), this, SLOT(ShowListWindowsDlg()));
	QObject::connect(pWndTile, SIGNAL(triggered()), m_pmdi, SLOT(tileSubWindows()));
	QObject::connect(pWndCsc, SIGNAL(triggered()), m_pmdi, SLOT(cascadeSubWindows()));
	QObject::connect(pCloseAll, SIGNAL(triggered()), m_pmdi, SLOT(closeAllSubWindows()));

	QObject::connect(pAbout, SIGNAL(triggered()), this, SLOT(ShowAbout()));
	QObject::connect(pBrowser, SIGNAL(triggered()), this, SLOT(ShowBrowser()));

	QObject::connect(pMenuWindows, SIGNAL(aboutToShow()), this, SLOT(UpdateSubWndList()));
	QObject::connect(m_pmdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(SubWindowChanged()));

	QObject::connect(m_proidlg, SIGNAL(NewRoiAvailable(const Roi*)), this, SLOT(NewRoiAvailable(const Roi*)));
	//--------------------------------------------------------------------------------
}

MiezeMainWnd::~MiezeMainWnd()
{
	if(m_pfitdlg) delete m_pfitdlg;
	if(m_proidlg) delete m_proidlg;
}

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
		pPlot->SetGlobalROI(&m_mainROI, &m_bmainROIActive);
		Data4& dat4 = pPlot->GetData();
		dat4.SetSize(iW, iH, iTcCnt, iFoilCnt);

		double *pdDat = new double[iW*iH*iTcCnt];
		double *pdErr = new double[iW*iH*iTcCnt];
		for(uint iFoil=0; iFoil<iFoilCnt; ++iFoil)
		{
			const uint* pDat = tof.GetData(iFoil);
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
		//for(unsigned int iY=0; iY<iH; ++iY)
		//	convert(pdDat+iY*iW, pDat+(iH-iY-1)*iW, iW);

		std::string strTitle = GetPlotTitle(strFileNoDir);
		Plot2d *pPlot = new Plot2d(m_pmdi, strTitle.c_str(), true);
		pPlot->SetGlobalROI(&m_mainROI, &m_bmainROIActive);
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
				pPlot->SetGlobalROI(&m_mainROI, &m_bmainROIActive);

				pPlot->plot(iW, iH, pDat, pErr);

				std::string strLabX, strLabY, strLabZ, strPlotTitle;
				pdat2d->GetLabels(strLabX, strLabY, strLabZ);
				pdat2d->GetTitle(strPlotTitle);
				pPlot->SetLabels(strLabX.c_str(), strLabY.c_str(), strLabZ.c_str());
				pPlot->SetTitle(strPlotTitle.c_str());

				double dXMin, dXMax, dYMin, dYMax, dZMin, dZMax;
				if(pdat2d->GetLimits(dXMin, dXMax, dYMin, dYMax, dZMin, dZMax))
					pPlot->SetXYMinMax(dXMin, dXMax, dYMin, dYMax);

				bool bXLog=0, bYLog=0;
				pdat2d->GetLogScale(bXLog, bYLog);
				pPlot->SetXIsLog(bXLog);
				pPlot->SetYIsLog(bYLog);

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
				pPlot->SetGlobalROI(&m_mainROI, &m_bmainROIActive);
				pPlot->plot(iW, iH, iT, pDat, pErr);

				std::string strLabX, strLabY, strLabZ, strPlotTitle;
				pdat3d->GetLabels(strLabX, strLabY, strLabZ);
				pdat3d->GetTitle(strPlotTitle);
				pPlot->SetLabels(strLabX.c_str(), strLabY.c_str(), strLabZ.c_str());
				pPlot->SetTitle(strPlotTitle.c_str());

				double dXMin, dXMax, dYMin, dYMax, dZMin, dZMax;
				if(pdat3d->GetLimits(dXMin, dXMax, dYMin, dYMax, dZMin, dZMax))
					pPlot->SetXYMinMax(dXMin, dXMax, dYMin, dYMax);

				bool bXLog=0, bYLog=0;
				pdat3d->GetLogScale(bXLog, bYLog);
				pPlot->SetXIsLog(bXLog);
				pPlot->SetYIsLog(bYLog);

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
				dlg.SetValues(pnicosdat->GetColNames());
				dlg.SetLabel("Select x values: ");

				if(dlg.exec() == QDialog::Accepted)
					iX = dlg.GetSelectedValue();
				else
					return;
			}

			int iY = pnicosdat->GetColIdx(strCtrName);
			const double *pdx = pnicosdat->GetColumn(iX);
			const double *pdy = pnicosdat->GetColumn(iY);
			double *pdyerr = new double[pnicosdat->GetDim()];
			::apply_fkt<double>(pdy, pdyerr, sqrt, pnicosdat->GetDim());

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

void MiezeMainWnd::ROIManageTriggered()
{
	if(!m_proidlg->isVisible())
	{
		m_proidlg->SetRoi(&m_mainROI);
		m_proidlg->show();
	}
	m_proidlg->activateWindow();
}

void MiezeMainWnd::ROILoadTriggered()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir_roi", ".").toString();

	QString strFile = QFileDialog::getOpenFileName(this, "Open ROI file...", strLastDir,
					"ROI files (*.roi *.ROI);;All files (*.*)",
					0, QFileDialog::DontUseNativeDialog);
	if(strFile.length() == 0)
		return;


	bool bDirSet=false;
	std::string strFile1 = strFile.toStdString();

	if(!bDirSet)
	{
		pGlobals->setValue("main/lastdir_roi", QString(::get_dir(strFile1).c_str()));
		bDirSet = true;
	}

	if(!m_mainROI.Load(strFile1.c_str()))
	{
		QMessageBox::critical(this, "Error", "Could not load ROI.");
		return;
	}

	m_proidlg->SetRoi(&m_mainROI);
}

void MiezeMainWnd::ROISaveTriggered()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir_roi", ".").toString();

	QString strFile = QFileDialog::getSaveFileName(this, "Save ROI file...", strLastDir,
					"ROI files (*.roi *.ROI);;All files (*.*)",
					0, QFileDialog::DontUseNativeDialog);
	if(strFile.length() == 0)
		return;


	bool bDirSet=false;
	std::string strFile1 = strFile.toStdString();

	if(!bDirSet)
	{
		pGlobals->setValue("main/lastdir_roi", QString(::get_dir(strFile1).c_str()));
		bDirSet = true;
	}

	if(!m_mainROI.Save(strFile1.c_str()))
		QMessageBox::critical(this, "Error", "Could not save ROI.");
}

void MiezeMainWnd::NewRoiAvailable(const Roi* pROI)
{
	m_mainROI = *pROI;
}

void MiezeMainWnd::SetGlobalROI(bool bSet)
{
	m_bmainROIActive = bSet;
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
	pWnd->setParent(m_pmdi);
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
		//std::cout << "old: " << pLabel->text().toStdString() << ", new: " << strMsg.toStdString() << std::endl;

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

SubWindowBase* MiezeMainWnd::GetActivePlot()
{
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	if(pWnd && pWnd->widget())
	{
		SubWindowBase* pWndBase = (SubWindowBase*)pWnd->widget();
		if(!pWndBase)
			return 0;

		return pWndBase->GetActualWidget();
	}

	return 0;
}

void MiezeMainWnd::ShowFitDlg()
{
	if(!m_pfitdlg)
	{
		m_pfitdlg = new FitDlg(this, m_pmdi);
		QObject::connect(m_pfitdlg, SIGNAL(AddSubWindow(SubWindowBase*)), this, SLOT(AddSubWindow(SubWindowBase*)));
	}

	m_pfitdlg->show();
	m_pfitdlg->activateWindow();
}

void MiezeMainWnd::QuickFitMIEZE() { QuickFit((SubWindowBase*)GetActivePlot(), FIT_MIEZE_SINE); }
void MiezeMainWnd::QuickFitGauss() { QuickFit((SubWindowBase*)GetActivePlot(), FIT_GAUSSIAN); }

void MiezeMainWnd::QuickFit(SubWindowBase* pSWB, int iFkt)
{
	if(!pSWB)
	{
		QMessageBox::critical(this, "Error", "No active plot.");
		return;
	}

	SpecialFitResult res = FitDlg::DoSpecialFit(pSWB, iFkt);
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

#include "mainwnd.moc"
#include "subwnd.moc"
#include "dialogs/AboutDlg.moc"
