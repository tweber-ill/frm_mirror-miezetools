/*
 * mieze-tool
 * main mdi window
 * @author tweber
 * @date 04-mar-2013
 */

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

#include "dialogs/FitDlg.h"


#ifndef __MAINWND_H__
#define __MAINWND_H__

class MiezeMainWnd : public QMainWindow
{ Q_OBJECT
protected:
	QMdiArea *m_pmdi;
	FitDlg *m_pfitdlg;

	unsigned int m_iPlotCnt;
	std::string GetPlotTitle(const std::string& strFile);

	QMenu* pMenuWindows;
	std::vector<QAction*> m_vecSubWndActions;

	QLabel *m_pStatusLabelLeft, *m_pStatusLabelMiddle, *m_pStatusLabelRight;

	virtual void keyPressEvent (QKeyEvent * event);

	SubWindowBase* GetActivePlot();

	Plot* Convert3d1d(Plot3d* pPlot3d);
	Plot* Convert4d1d(Plot4d* pPlot4d, int iFoil=-1);

protected slots:
	void SubWindowChanged();
	void FileLoadTriggered();
	void SettingsTriggered();
	void UpdateSubWndList();
	void ShowListWindowsDlg();
	void ShowCombineGraphsDlg();

	void ShowFitDlg();
	void QuickFit(SubWindowBase* pSWB, int iFkt);
	void QuickFitMIEZE();
	void QuickFitGauss();

	void ShowAbout();
	void ShowBrowser();

public:
	MiezeMainWnd();
	virtual ~MiezeMainWnd();

	void LoadFile(const std::string& strFile);

public slots:
	void SetStatusMsg(const char* pcMsg, int iPos);
	void AddSubWindow(SubWindowBase* pWnd);
};

#endif
