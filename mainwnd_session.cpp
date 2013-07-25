/*
 * mieze-tool
 * main window, session management
 * @author tweber
 * @date 25-jul-2013
 */

#include "mainwnd.h"
#include "helper/string.h"
#include "settings.h"

#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtCore/QSignalMapper>

#include <fstream>

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
		ofstr << "\n\n";
		ofstr << "<window_" << iWnd << ">\n";
		QMdiSubWindow *pSubWnd = FindSubWindow(pWnd);
		if(pSubWnd)
		{
			std::string strGeo = pSubWnd->saveGeometry().toHex().data();
			ofstr << "<geo> " << strGeo << " </geo>\n";
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


void MiezeMainWnd::LoadSession(const QString& strFile)
{
	LoadSession(strFile.toStdString());
}
