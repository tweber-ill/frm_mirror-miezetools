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
#include <QtGui/QMdiArea>

 class FitDlg : public QDialog, Ui::FitDlg
 { Q_OBJECT

 protected:
	 QMdiArea *m_pmdi;

	 void RemoveDuplicate();
	 void DoFit();

	 std::string GetTableString(QTableWidget* pTable) const;
	 void UpdateSourceList();

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
