/*
 * mieze-tool
 * main window, session management
 * @author tweber
 * @date 25-jul-2013
 */

#include "mainwnd.h"
#include "../helper/string.h"
#include "settings.h"

#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtCore/QSignalMapper>

#include <fstream>
#include <algorithm>
#include <thread>
//#include <chrono>
#include <unistd.h>


// --------------------------------------------------------------------------------

static void session_load_progress_thread(std::vector<int>* pvecProgress, bool* pbRunning, MiezeMainWnd *pWnd)
{
	while(*pbRunning)
	{
		unsigned int iLoaded = std::count(pvecProgress->begin(), pvecProgress->end(), 1);
		unsigned int iTotal = pvecProgress->size();

		std::ostringstream ostrLoading;
		ostrLoading << "Loaded " << iLoaded << " of " << iTotal << ".";
		pWnd->SetStatusMsg(ostrLoading.str().c_str(), 2);

		// all plots loaded
		if(iLoaded >= iTotal)
			break;

		usleep(500);

		//std::chrono::duration<int, std::milli> sleeptime(250);
		//std::this_thread::sleep_for(sleeptime);
	}
}

static void session_load_thread(Xml* xml, Blob* blob,
								const std::vector<std::string>* vecBase,
								const std::vector<unsigned int>* vecWndIdx,
								SubWindowBase **pSWBs, int *piProgress)
{
	for(unsigned int iWnd : *vecWndIdx)
	{
		pSWBs[iWnd]->LoadXML(*xml, *blob, (*vecBase)[iWnd]);
		piProgress[iWnd] = 1;
	}
}

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

	//CloseAllTriggeredWithRetain();
	if(!m_pRetainSession->isChecked())
		m_strCurSess = strSess;

	std::string strBase = "/cattus_session/";
	m_iPlotCnt = xml.Query<unsigned int>((strBase + "plot_counter").c_str(), 0);
	unsigned int iWndCnt = xml.Query<unsigned int>((strBase + "window_counter").c_str(), 0);

	/*std::string strGeo = xml.QueryString((strBase+"viewport_geo").c_str(), "");
	::trim(strGeo);
	if(strGeo != "")
		m_pmdi->viewport()->restoreGeometry(QByteArray::fromHex(strGeo.c_str()));*/


	std::vector<std::string> vecSWBase, vecSWType;
	SubWindowBase** pSWBs = new SubWindowBase*[iWndCnt];

	unsigned int iNumThreads = std::thread::hardware_concurrency();
	if(iNumThreads==0) iNumThreads = 1;
	std::vector<std::vector<unsigned int> > vecWndForThreads;
	vecWndForThreads.resize(iNumThreads);


	unsigned int iCurThread = 0;
	for(unsigned int iWnd=0; iWnd<iWndCnt; ++iWnd)
	{
		std::ostringstream ostrSWBase;
		ostrSWBase << strBase << "window_" << iWnd << "/";
		std::string strSWBase = ostrSWBase.str();
		std::string strSWType = xml.QueryString((strSWBase + "type").c_str(), "");
		vecSWBase.push_back(strSWBase);
		vecSWType.push_back(strSWType);

		QMdiArea *pMdi = GetMdiArea();
		SubWindowBase *pSWB = 0;
		if(strSWType == "plot_1d")
			pSWB = new Plot(pMdi);
		else if(strSWType == "plot_2d")
			pSWB = new Plot2d(pMdi);
		else if(strSWType == "plot_3d")
			pSWB = new Plot3dWrapper(pMdi);
		else if(strSWType == "plot_4d")
			pSWB = new Plot4dWrapper(pMdi);
		else
		{
			std::cerr << "Error: Unknown plot type: \"" << strSWType << "\"."
						<< std::endl;
			continue;
		}
		pSWBs[iWnd] = pSWB;

		vecWndForThreads[iCurThread].push_back(iWnd);
		if(++iCurThread >= iNumThreads)
			iCurThread = 0;
	}


	std::vector<std::thread*> vecThreads;
	std::vector<int> vecProgress(iWndCnt);

	bool bProgressRunning = 1;
	std::thread thProgress = std::thread(session_load_progress_thread,
											&vecProgress, &bProgressRunning,
											this);

	for(unsigned int iTh=0; iTh<iNumThreads; ++iTh)
	{
		if(vecWndForThreads[iTh].size() != 0)
		{
			std::thread *pTh = new std::thread(session_load_thread, &xml, &blob,
												&vecSWBase, &vecWndForThreads[iTh],
												pSWBs, vecProgress.data());
			vecThreads.push_back(pTh);
		}
	}

	for(unsigned int iTh=0; iTh<vecThreads.size(); ++iTh)
	{
		vecThreads[iTh]->join();
		delete vecThreads[iTh];
	}

	bProgressRunning = 0;
	thProgress.detach();
	thProgress.~thread();


	for(unsigned int iWnd=0; iWnd<iWndCnt; ++iWnd)
	{
		SubWindowBase *pSWB = pSWBs[iWnd];
		if(!pSWB) continue;

		AddSubWindow(pSWB, 0);
		QMdiSubWindow *pSubWnd = FindSubWindow(pSWB);

		if(pSubWnd)
		{
			const std::string& strSWBase = vecSWBase[iWnd];
			std::string strGeo = xml.QueryString((strSWBase+"geo").c_str(), "");
			::trim(strGeo);
			if(strGeo != "")
				pSubWnd->restoreGeometry(QByteArray::fromHex(strGeo.c_str()));

			// hack to position subwindows correctly in scroll area
			bool bOkX = 0, bOkY = 0;
			int iGeoX = xml.Query<int>((strSWBase+"geo_x").c_str(), 0, &bOkX);
			int iGeoY = xml.Query<int>((strSWBase+"geo_y").c_str(), 0, &bOkY);

			if(bOkX && bOkY)
				pSubWnd->move(iGeoX, iGeoY);
		}

		pSWB->GetActualWidget()->RefreshPlot();
		pSWB->show();
	}

	delete[] pSWBs;

	SetStatusMsg("Session loaded.", 2);
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
					"Session Files (*.cattus)"/*, 0, QFileDialog::DontUseNativeDialog*/);
	if(strFile == "")
		return;

	CloseAllTriggeredWithRetain();
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
	//ofstr << "<viewport_geo> " << m_pmdi->viewport()->saveGeometry().toHex().data()
	//		<< " </viewport_geo>\n";

	std::ofstream ofstrBlob(m_strCurSess + ".blob", std::ofstream::binary);

	unsigned int iWnd=0;
	for(SubWindowBase *pWnd : vecWnd)
	{
		std::ostringstream ostrSaving;
		ostrSaving << "Saving " << (iWnd+1) << " of " << vecWnd.size() << ": "
				<< pWnd->windowTitle().toStdString() << ".";
		SetStatusMsg(ostrSaving.str().c_str(), 2);

		pWnd = pWnd->GetActualWidget();
		ofstr << "\n\n";
		ofstr << "<window_" << iWnd << ">\n";
		QMdiSubWindow *pSubWnd = FindSubWindow(pWnd);
		if(pSubWnd)
		{
			std::string strGeo = pSubWnd->saveGeometry().toHex().data();
			ofstr << "<geo> " << strGeo << " </geo>\n";

			// hack
			ofstr << "<geo_x> " << pSubWnd->pos().x() << "</geo_x>\n";
			ofstr << "<geo_y> " << pSubWnd->pos().y() << "</geo_y>\n";
		}
		pWnd->SaveXML(ofstr, ofstrBlob);
		ofstr << "</window_" << iWnd << ">\n";
		ofstr << "\n\n";

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
					"Session Files (*.cattus)"/*, 0, QFileDialog::DontUseNativeDialog*/);
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

		QObject::connect(pSigs, SIGNAL(mapped(const QString&)), this, SLOT(CloseAllTriggeredWithRetain()));
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


void MiezeMainWnd::LoadSession(const QString& strFile)
{
	LoadSession(strFile.toStdString());
}
