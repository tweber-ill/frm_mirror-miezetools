/*
 * mieze-tool
 * @author tweber
 * @date 01-may-2013
 */

#ifndef __RESO_DLG_H__
#define __RESO_DLG_H__

#include <QtGui/QDialog>
#include <vector>
#include <string>

#include "../ui/ui_reso.h"


class ResoDlg : public QDialog, Ui::ResoDlg
{Q_OBJECT
protected:
	std::vector<QDoubleSpinBox*> m_vecSpinBoxes;
	std::vector<std::string> m_vecSpinNames;
	std::vector<QRadioButton*> m_vecRadioPlus;
	std::vector<QRadioButton*> m_vecRadioMinus;
	std::vector<std::string> m_vecRadioNames;

	void WriteLastConfig();
	void ReadLastConfig();

public:
	ResoDlg(QWidget* pParent);
	virtual ~ResoDlg();

protected slots:
	void UpdateUI();
	void Calc();

	void ButtonBoxClicked(QAbstractButton*);
};

#endif
