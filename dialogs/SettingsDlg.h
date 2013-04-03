/*
 * mieze-tool
 * @author tweber
 * @date 03-apr-2013
 */

#ifndef __MIEZE_SETTINGS_DLG__
#define __MIEZE_SETTINGS_DLG__


#include <QtGui/QDialog>
#include "../ui/ui_settings.h"


class SettingsDlg : public QDialog, Ui::SettingsDlg
{
	Q_OBJECT
	protected:
		void LoadSettings();
		void SaveSettings();

	public:
		SettingsDlg(QWidget* pParent);
		virtual ~SettingsDlg();

	public slots:
		virtual void accept();
};



#endif
