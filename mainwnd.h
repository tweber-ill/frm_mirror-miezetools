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

#include <vector>
#include <string>

#include "subwnd.h"
#include "plot/plot.h"
#include "plot/plot2d.h"
#include "plot/plot3d.h"
#include "plot/plot4d.h"

#include "dialogs/RoiDlg.h"
#include "dialogs/FitDlg.h"
#include "dialogs/ResoDlg.h"

#define MAX_RECENT_FILES 16

enum InterpFkt
{
	INTERP_BEZIER = 0,
	INTERP_BSPLINE
};

class MiezeMainWnd : public QMainWindow
{ Q_OBJECT
protected:
	QMdiArea *m_pmdi;
	FitDlg *m_pfitdlg;
	RoiDlg *m_proidlg;
	ResoDlg *m_presdlg;

	unsigned int m_iPlotCnt;
	std::string GetPlotTitle(const std::string& strFile);

	QMenu* pMenuWindows;
	std::vector<QAction*> m_vecSubWndActions;

	QLabel *m_pStatusLabelLeft, *m_pStatusLabelMiddle, *m_pStatusLabelRight;

	virtual void keyPressEvent (QKeyEvent * event);

	SubWindowBase* GetActivePlot();

	Plot* Convert3d1d(Plot3d* pPlot3d);
	Plot* Convert4d1d(Plot4d* pPlot4d, int iFoil=-1);

	bool m_bmainROIActive;
	Roi m_mainROI;

	std::string m_strLastXColumn;

	QStringList m_lstRecentFiles;
	QMenu *m_pMenuLoadRecent;
	void LoadRecentFileList();
	void UpdateRecentFileMenu();
	void AddRecentFile(const QString& strFile);

protected slots:
	void SubWindowChanged();
	void FileLoadTriggered();

	void UpdateSubWndList();
	void ShowListWindowsDlg();
	void ShowCombineGraphsDlg();

	void SettingsTriggered();

	void ROIManageTriggered();
	void ROILoadTriggered();
	void ROISaveTriggered();

	void ShowFitDlg();
	void QuickFit(SubWindowBase* pSWB, int iFkt);
	void QuickFitMIEZE();
	void QuickFitMIEZEpixel();
	void QuickFitGauss();
	void QuickFitDoubleGauss();

	void Interpolation(SubWindowBase* pSWB, InterpFkt iFkt);
	void BezierInterpolation();
	void BSplineInterpolation();

	void ShowReso();

	void ShowAbout();
	void ShowBrowser();

	void NewRoiAvailable(const Roi* pROI);
	void SetGlobalROI(bool bSet);

public:
	MiezeMainWnd();
	virtual ~MiezeMainWnd();

	void LoadFile(const std::string& strFile);

public slots:
	void SetStatusMsg(const char* pcMsg, int iPos);
	void AddSubWindow(SubWindowBase* pWnd);
	void LoadFile(const QString& strFile);
};

#endif
