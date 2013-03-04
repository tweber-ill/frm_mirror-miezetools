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
#include <iostream>

MiezeMainWnd::MiezeMainWnd()
{
	this->setWindowTitle("mieze tool");

	m_pmdi = new QMdiArea(this);
	this->setCentralWidget(m_pmdi);

	//--------------------------------------------------------------------------------
	// Menus

	// File
	QMenu* pMenuFile = new QMenu(this);
	pMenuFile->setTitle("File");

	QAction *pExit = new QAction(this);
	pExit->setText("Exit");
	pMenuFile->addAction(pExit);


	// Windows
	QMenu* pMenuWindows = new QMenu(this);
	pMenuWindows->setTitle("Windows");

	QAction *pWndTile = new QAction(this);
	pWndTile->setText("Tile");
	pMenuWindows->addAction(pWndTile);

	QAction *pWndCsc = new QAction(this);
	pWndCsc->setText("Cascade");
	pMenuWindows->addAction(pWndCsc);


	QMenuBar *pMenuBar = new QMenuBar(this);
	pMenuBar->addMenu(pMenuFile);
	pMenuBar->addMenu(pMenuWindows);
	this->setMenuBar(pMenuBar);
	//--------------------------------------------------------------------------------





	m_pmdi->addSubWindow(new QLabel("test"));
	m_pmdi->addSubWindow(new QLabel("test 2"));
	m_pmdi->addSubWindow(new QLabel("test 3"));




	//--------------------------------------------------------------------------------
	// Connections

	QObject::connect(pExit, SIGNAL(triggered()), this, SLOT(close()));
	QObject::connect(pWndTile, SIGNAL(triggered()), m_pmdi, SLOT(tileSubWindows()));
	QObject::connect(pWndCsc, SIGNAL(triggered()), m_pmdi, SLOT(cascadeSubWindows()));

	QObject::connect(m_pmdi, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(SubWindowChanged()));


	//--------------------------------------------------------------------------------
}

MiezeMainWnd::~MiezeMainWnd()
{

}


void MiezeMainWnd::SubWindowChanged()
{
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	QWidget *pWdg = pWnd->widget();

	// ...
}

#include "mainwnd.moc"

