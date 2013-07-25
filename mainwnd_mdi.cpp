/*
 * mieze-tool
 * main window, mdi specific stuff
 * @author tweber
 * @date 25-jul-2013
 */

#include "mainwnd.h"
#include "settings.h"

#include <QtGui/QMdiSubWindow>

QMdiSubWindow* MiezeMainWnd::FindSubWindow(SubWindowBase* pSWB)
{
	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow* pWnd : lst)
	{
		if(!pWnd) continue;
		SubWindowBase* pCurSWB = (SubWindowBase*)pWnd->widget();
		if(!pCurSWB) continue;

		if(pCurSWB==pSWB || pCurSWB->GetActualWidget()==pSWB)
			return pWnd;
	}

	return 0;
}

std::vector<SubWindowBase*> MiezeMainWnd::GetSubWindows(bool bResolveActualWidget)
{
	std::vector<SubWindowBase*> vec;
	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	vec.reserve(lst.size());

	for(QMdiSubWindow* pWnd : lst)
	{
		if(!pWnd) continue;
		SubWindowBase* pSWB = (SubWindowBase*)pWnd->widget();
		if(!pSWB) continue;

		if(bResolveActualWidget)
			pSWB = pSWB->GetActualWidget();

		vec.push_back(pSWB);
	}

	return vec;
}

void MiezeMainWnd::UpdateSubWndList()
{
	SubWindowBase *pSWBActive = GetActivePlot();

	// remove previous list
	for(QAction* pAction : m_vecSubWndActions)
	{
		pMenuWindows->removeAction(pAction);
		pAction->disconnect();
		delete pAction;
	}
	m_vecSubWndActions.clear();

	std::vector<SubWindowBase*> vec = GetSubWindows(0);

	// add new list
	for(SubWindowBase *pSWB : vec)
	{
		if(pSWB)
		{
			bool bActiveWindow = (pSWB == pSWBActive || pSWB->GetActualWidget() == pSWBActive);
			QString strTitle = pSWB->windowTitle();

			QAction *pAction = new QAction(pMenuWindows);
			pAction->setCheckable(1);
			pAction->setChecked(bActiveWindow);
			pAction->setText(strTitle);

			m_vecSubWndActions.push_back(pAction);
			pMenuWindows->addAction(pAction);

			QObject::connect(pAction, SIGNAL(triggered()), pSWB, SLOT(showNormal()));
			QObject::connect(pAction, SIGNAL(triggered()), pSWB, SLOT(setFocus()));
		}
	}
}

void MiezeMainWnd::SubWindowChanged()
{
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	if(!pWnd)
	{
		pMenuPlot->setEnabled(0);
		return;
	}

	SubWindowBase* pSWB = (SubWindowBase*)pWnd->widget();
	bool bSignal = 1;

	if(pSWB->GetType() == PLOT_1D)
	{
		pMenuPlot->clear();
		pMenuPlot->addActions(m_pMenu1d->actions());
		pMenuPlot->setEnabled(1);
	}
	else if(pSWB->GetType() == PLOT_2D)
	{
		pMenuPlot->clear();
		pMenuPlot->addActions(m_pMenu2d->actions());
		pMenuPlot->setEnabled(1);
	}
	else if(pSWB->GetType() == PLOT_3D)
	{
		pMenuPlot->clear();
		pMenuPlot->addActions(m_pMenu3d->actions());
		pMenuPlot->setEnabled(1);
	}
	else if(pSWB->GetType() == PLOT_4D)
	{
		pMenuPlot->clear();
		pMenuPlot->addActions(m_pMenu4d->actions());
		pMenuPlot->setEnabled(1);
	}
	else
	{
		pMenuPlot->setEnabled(0);
		bSignal = 0;
	}

	if(bSignal)
		emit SubWindowActivated(pSWB);
}

void MiezeMainWnd::SubWindowDestroyed(SubWindowBase *pSWB)
{
	emit SubWindowRemoved(pSWB);
}

void MiezeMainWnd::AddSubWindow(SubWindowBase* pWnd)
{
	if(!pWnd) return;

	pWnd->setParent(m_pmdi);
	SubWindowBase *pActualWidget = pWnd->GetActualWidget();
	QObject::connect(pWnd, SIGNAL(WndDestroyed(SubWindowBase*)), this, SLOT(SubWindowDestroyed(SubWindowBase*)));
	QObject::connect(pActualWidget, SIGNAL(SetStatusMsg(const char*, int)), this, SLOT(SetStatusMsg(const char*, int)));

	m_pmdi->addSubWindow(pWnd);
	emit SubWindowAdded(pWnd);

	pWnd->show();
}

SubWindowBase* MiezeMainWnd::GetActivePlot(bool bResolveWidget)
{
	QMdiSubWindow* pWnd = m_pmdi->activeSubWindow();
	if(pWnd && pWnd->widget())
	{
		SubWindowBase* pWndBase = (SubWindowBase*)pWnd->widget();
		if(!pWndBase)
			return 0;

		if(bResolveWidget)
			pWndBase = pWndBase->GetActualWidget();

		return pWndBase;
	}

	return 0;
}
