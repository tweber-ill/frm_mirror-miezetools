/*
 * mieze-tool
 * @author tweber
 * @date 29-may-2013
 * @license GPLv3
 */

#ifndef __PSD_PHASE_DLG__
#define __PSD_PHASE_DLG__

#include <QtGui/QDialog>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>

#include <vector>
#include <string>

#include "../main/subwnd.h"
#include "../ui/ui_psd_phase.h"
#include "../ui/ui_psd_phase_corr.h"
#include "../plot/plot2d.h"
#include "../plot/plot3d.h"
#include "../plot/plot4d.h"

class PsdPhaseDlg : public QDialog, Ui::PsdPhaseDlg
{Q_OBJECT
protected:
	Plot2d *m_pPlot;
	Data2 m_dat;
	bool m_bAllowUpdate;

	std::vector<QDoubleSpinBox*> m_vecDSpinBoxes;
	std::vector<std::string> m_vecStrDSpinBoxes;
	std::vector<QSpinBox*> m_vecSpinBoxes;
	std::vector<std::string> m_vecStrSpinBoxes;


protected slots:
	void Update();
	void ButtonBoxClicked(QAbstractButton* pBtn);

public:
	PsdPhaseDlg(QWidget* pParent);
	virtual ~PsdPhaseDlg();

	const Data2& GetData() const { return m_dat; }
	double GetTau() const { return spinTau->value(); }

protected:
	void LoadLastSettings();
	void SaveLastSettings();
};


enum PsdPhaseMethod
{
	METH_INVALID,

	METH_FIT,
	METH_FFT,
	METH_THEO
};

class PsdPhaseCorrDlg : public QDialog, Ui::PsdPhaseCorrDlg
{Q_OBJECT
protected:
	QMdiArea *m_pmdi;
	std::vector<const Plot2d*> m_vecPhaseImgs;

	void RemoveDuplicate();
	void DoPhaseCorr();

protected slots:
	void UsePhaseItemSelected();

	void AddItemSelected();
	void AddActiveItemSelected();
	void RemoveItemSelected();
	void ButtonBoxClicked(QAbstractButton* pBtn);

public slots:
	void RefreshPhaseCombo();
	void SubWindowRemoved(SubWindowBase *pSWB);

public:
	PsdPhaseCorrDlg(QWidget* pParent, QMdiArea *pmdi);
	virtual ~PsdPhaseCorrDlg();

	static Plot3d* DoPhaseCorr(const Plot2d* pPhasesPlot, const Plot3d* pDatPlot, PsdPhaseMethod meth=METH_THEO);
	static Plot4d* DoPhaseCorr(const Plot2d* pPhasesPlot, const Plot4d* pDatPlot, PsdPhaseMethod meth=METH_THEO);

signals:
	void AddNewPlot(SubWindowBase *pSWB);
};

#endif
