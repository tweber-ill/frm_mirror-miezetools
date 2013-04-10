/*
 * mieze-tool
 * @author tweber
 * @date 10-apr-2013
 */

#ifndef __MIEZE_FIT_DLG__
#define __MIEZE_FIT_DLG__

#include "../ui/ui_fit.h"
#include "../subwnd.h"

#include <QtGui/QMdiArea>

 class FitDlg : public QDialog, Ui::FitDlg
 { Q_OBJECT

 protected:
	 QMdiArea *m_pmdi;

	 void RemoveDuplicate();
	 void DoFit();

 protected slots:
	void AddItemSelected();
	void AddActiveItemSelected();
	void RemoveItemSelected();

 public:
	 FitDlg(QWidget* pParent, QMdiArea *pmdi);
	 virtual ~FitDlg();

		public slots:
			virtual void accept();
 };


#endif
