/*
 * mieze-tool
 * main mdi window
 * @author tweber
 * @date 04-mar-2013
 */

#include <QtGui/QMainWindow>
#include <QtGui/QMdiArea>
#include <vector>
#include <string>

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

	virtual void keyPressEvent (QKeyEvent * event);

protected slots:
	void SubWindowChanged();
	void FileLoadTriggered();
	void UpdateSubWndList();

public:
	MiezeMainWnd();
	virtual ~MiezeMainWnd();

	void AddSubWindow(QWidget* pWnd);
	void LoadFile(const std::string& strFile);
};

#endif
