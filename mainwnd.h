/*
 * mieze-tool
 * main mdi window
 * @author tweber
 * @date 04-mar-2013
 */

#include <QtGui/QMainWindow>
#include <QtGui/QMdiArea>

#ifndef __MAINWND_H__
#define __MAINWND_H__

class MiezeMainWnd : public QMainWindow
{ Q_OBJECT
protected:
	QMdiArea *m_pmdi;

protected slots:
	void SubWindowChanged();

public:
	MiezeMainWnd();
	virtual ~MiezeMainWnd();
};

#endif
