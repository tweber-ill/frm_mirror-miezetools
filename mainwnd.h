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

#ifndef __MAINWND_H__
#define __MAINWND_H__

class MiezeMainWnd : public QMainWindow
{ Q_OBJECT
protected:
	QMdiArea *m_pmdi;

	unsigned int m_iPlotCnt;
	std::string GetPlotTitle(const std::string& strFile);

	QMenu* pMenuWindows;
	std::vector<QAction*> m_vecSubWndActions;

	QLabel *m_pStatusLabelLeft, *m_pStatusLabelMiddle, *m_pStatusLabelRight;

	virtual void keyPressEvent (QKeyEvent * event);

protected slots:
	void SubWindowChanged();
	void FileLoadTriggered();
	void UpdateSubWndList();

public:
	MiezeMainWnd();
	virtual ~MiezeMainWnd();

	void AddSubWindow(SubWindowBase* pWnd);
	void LoadFile(const std::string& strFile);

public slots:
	void SetStatusMsg(const char* pcMsg, int iPos);
};

#endif
