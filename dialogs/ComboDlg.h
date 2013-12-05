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

	void SetCurFile(const char* pcStr);
	void SetTitle(const char* pcStr);

	void SetLabel(const char* pcStr);
	void SetLabelY(const char* pcStr);

	void SetValues(const std::vector<std::string>& vec);
	void SetValuesY(const std::vector<std::string>& vec);

	void SelectValue(const std::string& str);
	void SelectValueY(const std::string& str);

	int GetSelectedValue() const;
	int GetSelectedValueY() const;

public slots:
};


#endif
