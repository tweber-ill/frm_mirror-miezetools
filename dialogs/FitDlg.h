/*
 * mieze-tool
 * @author tweber
 * @date 10-apr-2013
 */

#ifndef __MIEZE_FIT_DLG__
#define __MIEZE_FIT_DLG__

#include "../ui/ui_fit.h"
#include "../subwnd.h"

#include <string>
#include <map>
#include <QtGui/QMdiArea>

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

	 std::string GetTableString(QTableWidget* pTable) const;
	 void UpdateSourceList();
	 void UpdateHint(const std::string& str, double dVal, double dErr);

 protected slots:
	void AddItemSelected();
	void AddActiveItemSelected();
	void RemoveItemSelected();

	void FunctionTypeChanged();
	void FunctionChanged(const QString&);

	void ButtonBoxClicked(QAbstractButton*);

 public:
	 FitDlg(QWidget* pParent, QMdiArea *pmdi);
	 virtual ~FitDlg();

public slots:

signals:
	void AddSubWindow(SubWindowBase* pWnd);
 };


#endif
