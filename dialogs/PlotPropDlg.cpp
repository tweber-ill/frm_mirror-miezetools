/*
 * mieze-tool
 * @author tweber
 * @date 22-jul-2013
 */

#include "PlotPropDlg.h"
#include "../plot/plot.h"
#include "../plot/plot2d.h"
#include "../plot/plot3d.h"
#include "../plot/plot4d.h"

PlotPropDlg::PlotPropDlg(QWidget* pParent)
				: QDialog(pParent), m_pCurPlot(0)
{
	setupUi(this);
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
}

PlotPropDlg::~PlotPropDlg()
{}


void PlotPropDlg::SubWindowActivated(SubWindowBase* pSWB)
{
	m_pCurPlot = pSWB;

	editWindowTitle->setText(m_pCurPlot->windowTitle());

	editTitle->setText(m_pCurPlot->GetLabel(LABEL_TITLE).c_str());
	editXLabel->setText(m_pCurPlot->GetLabel(LABEL_X).c_str());
	editYLabel->setText(m_pCurPlot->GetLabel(LABEL_Y).c_str());
	editZLabel->setText(m_pCurPlot->GetLabel(LABEL_Z).c_str());
	editTLabel->setText(m_pCurPlot->GetLabel(LABEL_T).c_str());
	editFoilLabel->setText(m_pCurPlot->GetLabel(LABEL_F).c_str());

	if(m_pCurPlot->GetType() == PLOT_1D)
	{
		groupXYRanges->setEnabled(false);
		editZLabel->setEnabled(false);
		editTLabel->setEnabled(false);
		editFoilLabel->setEnabled(false);
		checkLogZ->setEnabled(false);
	}
	else if(m_pCurPlot->GetType() == PLOT_2D ||
			m_pCurPlot->GetType() == PLOT_3D ||
			m_pCurPlot->GetType() == PLOT_4D)
	{
		groupXYRanges->setEnabled(true);
		editZLabel->setEnabled(true);
		checkLogZ->setEnabled(true);

		Plot2d *pPlot2d = (Plot2d*)pSWB->GetActualWidget();
		checkLogZ->setChecked(pPlot2d->GetLog());

		if(m_pCurPlot->GetType() == PLOT_2D)
		{
			editTLabel->setEnabled(false);
			editFoilLabel->setEnabled(false);
		}
		else if(m_pCurPlot->GetType() == PLOT_3D)
		{
			editTLabel->setEnabled(true);
			editFoilLabel->setEnabled(false);
		}
		else if(m_pCurPlot->GetType() == PLOT_4D)
		{
			editTLabel->setEnabled(true);
			editFoilLabel->setEnabled(true);
		}

		// TODO: xyrange
	}
}

void PlotPropDlg::SubWindowRemoved(SubWindowBase* pSWB)
{
	if(pSWB == m_pCurPlot)
	{
		m_pCurPlot = 0;

		QLineEdit *pEdits[] = {editTitle, editWindowTitle, editXLabel, editYLabel, editZLabel, editTLabel, editFoilLabel};
		for(QLineEdit *pEdit : pEdits)
			pEdit->clear();

		QDoubleSpinBox *pSpins[] = {spinXMin, spinXMax, spinYMin, spinYMax};
		for(QDoubleSpinBox *pSpin : pSpins)
			pSpin->setValue(0.);

		QCheckBox* pChecks[] = {checkLogZ};
		for(QCheckBox *pCheck : pChecks)
			pCheck->setChecked(false);
	}
}

void PlotPropDlg::SaveSettings()
{
	if(!m_pCurPlot)
		return;

	m_pCurPlot->SetLabel(LABEL_TITLE, editTitle->text().toStdString().c_str());
	m_pCurPlot->SetLabel(LABEL_X, editXLabel->text().toStdString().c_str());
	m_pCurPlot->SetLabel(LABEL_Y, editYLabel->text().toStdString().c_str());
	m_pCurPlot->SetLabel(LABEL_Z, editZLabel->text().toStdString().c_str());
	m_pCurPlot->SetLabel(LABEL_T, editTLabel->text().toStdString().c_str());
	m_pCurPlot->SetLabel(LABEL_F, editFoilLabel->text().toStdString().c_str());

	if(m_pCurPlot->GetType() == PLOT_1D)
	{
	}
	else if(m_pCurPlot->GetType() == PLOT_2D ||
			m_pCurPlot->GetType() == PLOT_3D ||
			m_pCurPlot->GetType() == PLOT_4D)
	{
		Plot2d *pPlot2d = (Plot2d*)m_pCurPlot->GetActualWidget();

		pPlot2d->SetLog(checkLogZ->isChecked());

		// TODO: xyrange
	}
}

void PlotPropDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		SaveSettings();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		QDialog::accept();
	}
}

#include "PlotPropDlg.moc"
