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

#include "../tools/res/cn.h"
#include "../tools/res/pop.h"

#include "../helper/linalg.h"


class InstLayoutDlg : public QDialog
{Q_OBJECT
protected:
	double m_dMonoTheta, m_dMono2Theta;
	double m_dSampleTheta, m_dSample2Theta;
	double m_dAnaTheta, m_dAna2Theta;

	double m_dDistMonoSample, m_dDistSampleAna, m_dDistAnaDet;

	double m_dPosMonoX, m_dPosMonoY;

	double m_dMonoW, m_dMonoD;
	double m_dAnaW, m_dAnaD;
	double m_dDetW;

	double m_dWidth, m_dHeight;

	virtual void paintEvent (QPaintEvent *pEvent);

public:
	InstLayoutDlg(QWidget* pParent);
	virtual ~InstLayoutDlg();

	void SetParams(const PopParams& pop, const CNResults& res);

protected slots:
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);
};


class ScatterTriagDlg : public QDialog
{Q_OBJECT
protected:
	ublas::vector<double> m_vec_ki, m_vec_kf, m_vec_Q;
	double m_d2Theta, m_dKiQ;

	virtual void paintEvent (QPaintEvent *pEvent);

public:
	ScatterTriagDlg(QWidget* pParent);
	virtual ~ScatterTriagDlg();

	void SetParams(const PopParams& pop, const CNResults& res);

protected slots:
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);
};


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


	InstLayoutDlg *m_pInstDlg;
	ScatterTriagDlg *m_pScatterDlg;

public:
	ResoDlg(QWidget* pParent);
	virtual ~ResoDlg();

protected slots:
	void UpdateUI();
	void Calc();
	void ShowInstrLayout();
	void ShowScatterTriag();

	void ButtonBoxClicked(QAbstractButton*);
	void hideEvent (QHideEvent *event);
	void showEvent(QShowEvent *event);

	void SaveFile();
	void LoadFile();
};

#endif
