/*
 * mieze-tool
 * @author tweber
 * @date 29-may-2013
 */

#ifndef __PSD_PHASE_DLG__
#define __PSD_PHASE_DLG__

#include <QtGui/QDialog>
#include <QtGui/QWidget>
#include "../ui/ui_psd_phase.h"
#include "../plot/plot2d.h"

class PsdPhaseDlg : public QDialog, Ui::PsdPhaseDlg
{Q_OBJECT
protected:
	Plot2d *m_pPlot;
	Data2 m_dat;
	bool m_bAllowUpdate;

protected slots:
	void Update();

public:
	PsdPhaseDlg(QWidget* pParent);
	virtual ~PsdPhaseDlg();

	const Data2& GetData() const { return m_dat; }
	double GetTau() const { return spinTau->value(); }
};

#endif
