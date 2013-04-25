/*
 * mieze-tool
 * @author tweber
 * @date 25-apr-2013
 */

#ifndef __COMBO_DLG__
#define __COMBO_DLG__

#include <QtGui/QDialog>
#include "../ui/ui_combo.h"

#include <vector>
#include <string>

class ComboDlg : public QDialog, Ui::ComboDlg
{Q_OBJECT
public:
	ComboDlg(QWidget* pParent) : QDialog(pParent)
	{
		this->setupUi(this);
	}

	virtual ~ComboDlg()
	{}

	void SetLabel(const char* pcStr);
	void SetTitle(const char* pcStr);
	void SetValues(const std::vector<std::string>& vec);

	int GetSelectedValue() const;

public slots:
};


#endif
