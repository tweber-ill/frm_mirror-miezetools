/*
 * mieze-tool
 * main mdi window
 * @author tweber
 * @date 04-mar-2013
 */

#ifndef __MAINWND_H__
#define __MAINWND_H__

#include <QtGui/QMainWindow>
#include <QtGui/QMdiArea>
#include <QtGui/QLabel>
#include <QtGui/QMenu>

#include <vector>
#include <string>

#include "subwnd.h"
#include "plot/plot.h"
#include "plot/plot2d.h"
#include "plot/plot3d.h"
#include "plot/plot4d.h"

#include "dialogs/CombineDlg.h"
#include "dialogs/RoiDlg.h"
#include "dialogs/FitDlg.h"
#include "dialogs/ResoDlg.h"
#include "dialogs/PsdPhaseDlg.h"
#include "dialogs/RadialIntDlg.h"
#include "dialogs/FormulaDlg.h"
#include "dialogs/PlotPropDlg.h"
#include "dialogs/ExportDlg.h"
#include "dialogs/InfoDock.h"
#include "dialogs/NormDlg.h"

#define MAX_RECENT_FILES 32
#define WND_TITLE "Cattus, a MIEZE toolset"

enum InterpFkt
{
	INTERP_BEZIER = 0,
	INTERP_BSPLINE
};

class MiezeMainWnd : public QMainWindow
{ Q_OBJECT
protected:
	QMdiArea *m_pmdi;
	InfoDock *m_pinfo;

	QAction *m_pRetainSession;

	CombineGraphsDlg *m_pcombinedlg;
	FitDlg *m_pfitdlg;
	RoiDlg *m_proidlg, *m_pantiroidlg;
	ResoDlg *m_presdlg;
	RadialIntDlg *m_pradialintdlg;
	PsdPhaseCorrDlg *m_pphasecorrdlg;
	FormulaDlg *m_pformuladlg;
	PlotPropDlg *m_pplotpropdlg;
	ExportDlg *m_pexportdlg;
	NormDlg *m_pnormdlg;

	unsigned int m_iPlotCnt;
	std::string GetPlotTitle(const std::string& strFile);

	QMenu* pMenuWindows, *pMenuPlot;
	QMenu *m_pMenu1d, *m_pMenu2d, *m_pMenu3d, *m_pMenu4d;
	std::vector<QAction*> m_vecSubWndActions;

	QLabel *m_pStatusLabelLeft, *m_pStatusLabelMiddle, *m_pStatusLabelRight;

	virtual void keyPressEvent (QKeyEvent * event);

	SubWindowBase* GetActivePlot(bool bResolveWidget=1);

	Plot* Convert3d1d(Plot3d* pPlot3d);
	Plot* Convert4d1d(Plot4d* pPlot4d, int iFoil=-1);

	std::string m_strLastXColumn;

	// recent files
	QStringList m_lstRecentFiles;
	QMenu *m_pMenuLoadRecent;
	void LoadRecentFileList();
	void UpdateRecentFileMenu();
	void AddRecentFile(const QString& strFile);

	// recent sessions
	QStringList m_lstRecentSessions;
	QMenu *m_pMenuLoadRecentSession;
	void LoadRecentSessionList();
	void UpdateRecentSessionMenu();
	void AddRecentSession(const QString& strSession);


	std::string m_strCurSess;

	QMdiSubWindow* FindSubWindow(SubWindowBase* pSWB);
	std::vector<SubWindowBase*> GetSubWindows(bool bResolveActualWidget=1);


	void _GetActiveROI(bool bAntiRoi=0);
	void _SetGlobalROIForAll(bool bAntiRoi=0);
	void _SetGlobalROIForActive(bool bAntiRoi=0);


protected slots:
	void SubWindowChanged();
	void FileLoadTriggered();

	void CloseAllTriggeredWithRetain();
	void CloseAllTriggered();

	void SessionLoadTriggered();
	void SessionSaveTriggered();
	void SessionSaveAsTriggered();

	void FileExportPyTriggered();
	void ShowExportDlg();

	void UpdateSubWndList();
	void ShowListWindowsDlg();
	void ToggleInfoWindow();

	void ShowCombineGraphsDlg();
	void IntAlongY();
	void IntRad();

	void SettingsTriggered();
	void ROIManageTriggered();
	void AntiROIManageTriggered();

	void ShowFitDlg();
	void QuickFit(SubWindowBase* pSWB, int iFkt, int iParam=-1);
	void QuickFitMIEZE();
	void QuickFitMIEZEpixel();
	void QuickFitGauss();
	void QuickFitDoubleGauss();
	void QuickFitTripleGauss();

	void Interpolation(SubWindowBase* pSWB, InterpFkt iFkt);
	void BezierInterpolation();
	void BSplineInterpolation();

	void ShowReso();
	void ShowPSDPhaseCorr();
	void CalcPSDPhases();
	void ShowFormulas();

	void ShowAbout();
	void ShowBrowser();


	void GetActiveROI();
	void SetGlobalROIForAll();
	void SetGlobalROIForActive();

	void GetActiveAntiROI();
	void SetGlobalAntiROIForAll();
	void SetGlobalAntiROIForActive();


	void ShowTimeChannels();
	void ExtractFoils();
	void SumFoils();
	void PlotPropertiesTriggered();
	void NormalizeTriggered();

	void PlotParamsDynChanged(const StringMap&);

public:
	MiezeMainWnd();
	virtual ~MiezeMainWnd();

	void LoadFile(const std::string& strFile);
	void LoadSession(const std::string& strFile);
	void MakePlot(const Data1& dat, const std::string& strTitle);

public slots:
	void SetStatusMsg(const char* pcMsg, int iPos);
	void AddSubWindow(SubWindowBase* pWnd, bool bShow=1);
	void LoadFile(const QString& strFile);
	void LoadSession(const QString& strFile);

protected slots:
	void SubWindowDestroyed(SubWindowBase *pSWB);

signals:
	void SubWindowRemoved(SubWindowBase *pSWB);
	void SubWindowAdded(SubWindowBase *pSWB);
	void SubWindowActivated(SubWindowBase *pSWB);
};

#endif
