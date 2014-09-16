/*
 * mieze-tool
 * main window, file management
 * @author tweber
 * @date 25-jul-2013
 */

#include "mainwnd.h"
#include "../dialogs/ComboDlg.h"
#include "../helper/string.h"
#include "../helper/misc.h"
#include "../helper/comp.h"
#include "../helper/file.h"
#include "../helper/log.h"
#include "settings.h"
#include "../loader/loadtxt.h"
#include "../loader/loadnicos.h"
#include "../loader/loadcasc.h"

#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtCore/QSignalMapper>
#include <QtCore/QTemporaryFile>

#include "../data/export.h"

#include <fstream>


void MiezeMainWnd::LoadFile(const std::string& _strFile)
{
	TmpFile tmp;

	std::string strFile = _strFile;
	std::string strExt = get_fileext(strFile);
	if(strExt == "gz" || strExt == "bz2")
	{
		strExt = get_fileext2(strFile);

		if(!tmp.open())
		{
			log_err("Cannot create temporary file for \"", strFile, "\".");
			return;
		}

		strFile = tmp.GetFileName();
		if(!decomp_file_to_file(_strFile.c_str(), strFile.c_str()))
		{
			log_err("Cannot decompress file \"", strFile, "\".");
			return;
		}
	}

	std::string strFileNoDir = ::get_file(_strFile);

	if(is_equal(strExt, std::string("tof")))
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
				log_err("Could not load \"", strFileNoDir, "\" correctly.");
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

		pPlot->GetData().SetParamMapStat(tof.GetParamMap());

		AddSubWindow(pPlotWrapper);
		pPlotWrapper->GetActualWidget()->RefreshPlot();


		/*SubWindowBase *pSWB = pPlot->clone();
		pSWB->ChangeResolution(512,512,1);
		AddSubWindow(pSWB);
		pSWB->GetActualWidget()->RefreshPlot();*/
	}
	else if(is_equal(strExt, std::string("pad")))
	{
		PadFile pad(strFile.c_str());
		if(!pad.IsOpen())
			return;

		const uint* pDat = pad.GetData();
		if(!pDat)
		{
			log_err("Could not load \"", strFileNoDir, "\".");
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

		pPlot->GetData2().SetParamMapStat(pad.GetParamMap());

		delete[] pdDat;

		AddSubWindow(pPlot);
		pPlot->GetActualWidget()->RefreshPlot();
	}
	else if(is_equal(strExt, std::string("dat")) || is_equal(strExt, std::string("sim")))
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
			StringMap mapStr;
			mapStr.SetMap(vecmap_to_map<std::string>(pdat->GetCommMap()));

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

				if(Settings::Get<int>("misc/sort_x"))
				{
					if(pdyerr)
						::sort_3<double*>((double*)pdx,
								(double*)pdx+pdat1d->GetDim(),
								(double*)pdy,
								(double*)pdyerr);
					else
						::sort_2<double*>((double*)pdx,
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

				if(pPlot->GetDataCount()>=1)
					pPlot->GetData(0).dat.SetParamMapStat(mapStr);

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

				pPlot->GetData2().SetParamMapStat(mapStr);

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

				pPlot->GetData().SetParamMapStat(mapStr);

				delete[] pDat;
				delete[] pErr;
				delete pdat3d;

				AddSubWindow(pPlotWrapper);
			}
		}
		else if(dattype == NICOS_DATA)
		{
			NicosData * pnicosdat = new NicosData(*pdat);
			StringMap mapStr;
			mapStr.SetMap(vecmap_to_map<std::string>(pdat->GetCommMap()));

			::autodeleter<NicosData> _a0(pnicosdat);
			std::string strCtrName = Settings::Get<QString>("nicos/counter_name").toStdString();

			bool bSelectNewXColumn = 0;
			int iX = 0, iY = 0;
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
				dlg.SetValuesY(pnicosdat->GetColNames());

				dlg.SelectValue(pnicosdat->TryFindScanVar(pdat->GetCommMap()));
				dlg.SelectValueY(strCtrName);

				dlg.SetLabel("Select x value: ");
				dlg.SetLabelY("Select y value: ");

				if(dlg.exec() == QDialog::Accepted)
				{
					iX = dlg.GetSelectedValue();
					iY = dlg.GetSelectedValueY();
					m_strLastXColumn = pnicosdat->GetColName(iX);
					m_strLastYColumn = pnicosdat->GetColName(iY);
					//strCtrName = m_strLastYColumn;
				}
				else
					return;
			}

			if(m_strLastYColumn.length())
				iY = pnicosdat->GetColIdx(m_strLastYColumn);
			if(iY < 0)
				iY = pnicosdat->GetColIdx(strCtrName);
			const double *pdx = pnicosdat->GetColumn(iX);
			const double *pdy = pnicosdat->GetColumn(iY);
			double *pdyerr = new double[pnicosdat->GetDim()];
			::apply_fkt<double>(pdy, pdyerr, sqrt, pnicosdat->GetDim());

			if(Settings::Get<int>("misc/sort_x"))
			{
				if(pdyerr)
					::sort_3<double*>((double*)pdx,
							(double*)pdx+pnicosdat->GetDim(),
							(double*)pdy,
							pdyerr);
				else
					::sort_2<double*>((double*)pdx,
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

			if(pPlot->GetDataCount()>=1)
				pPlot->GetData(0).dat.SetParamMapStat(mapStr);

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

	AddRecentFile(QString(_strFile.c_str()));
}

void MiezeMainWnd::FileLoadTriggered()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir", ".").toString();

	QStringList strFiles = QFileDialog::getOpenFileNames(this, "Open data file...", strLastDir,
					"All data files (*.dat *.sim *.pad *.tof *.dat.gz *.dat.bz2 *.pad.gz *.pad.bz2 *.tof.gz *.tof.bz2);;TOF files (*.tof *.tof.gz *.tof.bz2);;PAD files(*.pad *.pad.gz *.pad.bz2);;DAT files (*.dat *.sim *.dat.gz *.dat.bz2)"
					/*,0, QFileDialog::DontUseNativeDialog*/);
	if(strFiles.size() == 0)
		return;

	m_strLastXColumn = "";
	m_strLastYColumn = "";
	bool bDirSet=false;

	unsigned int iFile = 0;
	for(const QString& strFile : strFiles)
	{
		std::string strFile1 = strFile.toStdString();
		std::string strFileNoDir = ::get_file(strFile1);

		std::ostringstream ostrMsg;
		ostrMsg << "Loading " << (iFile+1) << " of " << strFiles.size() << ": "
				<< strFileNoDir << ".";
		this->SetStatusMsg(ostrMsg.str().c_str(), 2);

		if(strFile == "")
			continue;

		std::string strExt = get_fileext(strFile1);

		if(!bDirSet)
		{
			pGlobals->setValue("main/lastdir", QString(::get_dir(strFile1).c_str()));
			bDirSet = true;
		}

		LoadFile(strFile1);
		++iFile;
	}
}

void MiezeMainWnd::CloseAllTriggered()
{
	// safety check
	for(SubWindowBase *pSWB : GetSubWindows(0))
		emit SubWindowRemoved(pSWB);

	// TODO: save dialog
	m_pmdi->closeAllSubWindows();
}

void MiezeMainWnd::CloseAllTriggeredWithRetain()
{
	if(!m_pRetainSession->isChecked())
		CloseAllTriggered();
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

	Settings::Set<QStringList>("misc/recent_files", m_lstRecentFiles);
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
	m_lstRecentFiles = Settings::Get<QStringList>("misc/recent_files");
	m_lstRecentFiles.removeDuplicates();

	UpdateRecentFileMenu();
}
// --------------------------------------------------------------------------------


void MiezeMainWnd::LoadFile(const QString& strFile)
{
	m_strLastXColumn = "";
	m_strLastYColumn = "";
	LoadFile(strFile.toStdString());
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

void MiezeMainWnd::FileExportTriggered()
{
	const SubWindowBase *pSWB = GetActivePlot();
        if(!pSWB)  
        {
                QMessageBox::critical(this, "Error", "No active plot.");
                return;
        }

	pSWB->SaveImageAs();
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
