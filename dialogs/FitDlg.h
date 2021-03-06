/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 10-apr-2013
 * @license GPLv3
 */

#ifndef __MIEZE_FIT_DLG__
#define __MIEZE_FIT_DLG__

#include "ui/ui_fit.h"
#include "main/subwnd.h"
#include "plot/plot.h"
#include "plot/plot2d.h"
#include "data/fit_data.h"

#include <string>
#include <map>
#include <QtGui/QMdiArea>


struct SpecialFitResult
{
	bool bOk;
	bool bCreatedNewPlot;
	Plot *pPlot;

	std::string strErr;
};

struct SpecialFitPixelResult
{
	bool bOk;
	Plot2d *pPlot[2];

	std::string strErr;
};

struct FitParams
{
	std::string strMin, strMax;
	std::string strHint, strErr;
	bool bHintActive, bLimitActive;
};

 class FitDlg : public QDialog, Ui::FitDlg
 { Q_OBJECT

 protected:
	 QMdiArea *m_pmdi;

	 typedef std::map<std::string, FitParams> t_mapParams;
	 t_mapParams m_mapParams;
	 void SaveLastParams();
	 void RestoreLastParams();

	 void RemoveDuplicate();
	 void DoFit();
	 void DoSpecialFit();
	 void DoSpecialFitPixelwise();

	 std::string GetTableString(QTableWidget* pTable) const;
	 void UpdateSourceList();
	 void UpdateHint(const std::string& str, double dVal, double dErr);

 protected slots:
	void AddItemSelected();
	void AddActiveItemSelected();
	void RemoveItemSelected();


	void SpecialTypeChanged();
	void SpecialFktChanged(int);
	void FunctionTypeChanged();
	void FunctionChanged(const QString&);

	void ButtonBoxClicked(QAbstractButton*);

 public:
	 FitDlg(QWidget* pParent, QMdiArea *pmdi);
	 virtual ~FitDlg();

	 static SpecialFitResult DoSpecialFit(SubWindowBase* pSWB, int iFkt, int iParam=-1);
	 static SpecialFitPixelResult DoSpecialFitPixel(SubWindowBase* pSWB, int iFoil, int iFkt);

public slots:
	void SubWindowRemoved(SubWindowBase *pSWB);

signals:
	void AddSubWindow(SubWindowBase* pWnd);
 };


#endif
