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
#include "../tlibs/string/string.h"

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


	const StringMap* pmapParams = m_pCurPlot->GetParamMapDyn();
	if(pmapParams)
		editTau->setText((*pmapParams)["param_tau"].c_str());


	XYRange *pRange = 0;

	if(m_pCurPlot->GetType() == PLOT_1D)
	{
		groupXYRanges->setEnabled(false);
		editZLabel->setEnabled(false);
		editTLabel->setEnabled(false);
		editFoilLabel->setEnabled(false);
		checkLogZ->setEnabled(false);
		groupMIEZE->setEnabled(false);
	}
	else if(m_pCurPlot->GetType() == PLOT_2D ||
			m_pCurPlot->GetType() == PLOT_3D ||
			m_pCurPlot->GetType() == PLOT_4D)
	{
		groupXYRanges->setEnabled(true);
		editZLabel->setEnabled(true);
		checkLogZ->setEnabled(true);
		groupMIEZE->setEnabled(false);

		Plot2d *pPlot2d = (Plot2d*)pSWB->GetActualWidget();
		checkLogZ->setChecked(pPlot2d->GetLog());

		if(m_pCurPlot->GetType() == PLOT_2D)
		{
			editTLabel->setEnabled(false);
			editFoilLabel->setEnabled(false);

			pRange = &pPlot2d->GetData2();
		}
		else if(m_pCurPlot->GetType() == PLOT_3D)
		{
			editTLabel->setEnabled(true);
			editFoilLabel->setEnabled(false);
			groupMIEZE->setEnabled(true);

			pRange = &((Plot3d*)pPlot2d)->GetData();
		}
		else if(m_pCurPlot->GetType() == PLOT_4D)
		{
			editTLabel->setEnabled(true);
			editFoilLabel->setEnabled(true);
			groupMIEZE->setEnabled(true);

			const Data4& dat4 = ((Plot4d*)pPlot2d)->GetData();
			pRange = &((Plot4d*)pPlot2d)->GetData();

			// TODO: Get foil phases shifts automatically from elastic reference TOF
			if(dat4.HasPhases())
			{
				std::ostringstream ostrPhases;
				for(unsigned int iPhase=0; iPhase<dat4.GetDepth2(); ++iPhase)
				{
					ostrPhases << dat4.GetPhases()[iPhase];
					if(iPhase < dat4.GetDepth2()-1)
						ostrPhases << ", ";
				}

				editPhases->setText(ostrPhases.str().c_str());
			}
			else
			{
				editPhases->setText("");
			}
		}
	}

	if(pRange)
	{
		spinXMin->setValue(pRange->GetXRangeMin());
		spinXMax->setValue(pRange->GetXRangeMax());
		spinYMin->setValue(pRange->GetYRangeMin());
		spinYMax->setValue(pRange->GetYRangeMax());
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
	m_pCurPlot->setWindowTitle(editWindowTitle->text());



	std::string strTau = editTau->text().trimmed().toStdString();
	StringMap *pmapParams = const_cast<StringMap*>(m_pCurPlot->GetParamMapDyn());

	if(pmapParams && strTau != "")
		(*pmapParams)["param_tau"] = strTau;



	if(m_pCurPlot->GetType() == PLOT_1D)
	{
	}
	else if(m_pCurPlot->GetType() == PLOT_2D ||
			m_pCurPlot->GetType() == PLOT_3D ||
			m_pCurPlot->GetType() == PLOT_4D)
	{
		XYRange *pRange = 0;
		Plot2d *pPlot2d = (Plot2d*)m_pCurPlot->GetActualWidget();

		pPlot2d->SetLog(checkLogZ->isChecked());

		if(m_pCurPlot->GetType() == PLOT_2D)
		{
			pRange = &pPlot2d->GetData2();
		}
		else if(m_pCurPlot->GetType() == PLOT_3D)
		{
			pRange = &((Plot3d*)pPlot2d)->GetData();
		}
		else if(m_pCurPlot->GetType() == PLOT_4D)
		{
			Data4& dat4 = ((Plot4d*)pPlot2d)->GetData();
			pRange = &((Plot4d*)pPlot2d)->GetData();

			std::string strPhases = editPhases->text().toStdString();
			std::vector<double> vecPhases;
			tl::get_tokens<double>(strPhases, std::string(",; "), vecPhases);

			bool bHasPhases = (vecPhases.size() != 0);
			dat4.SetHasPhases(bHasPhases);
			if(bHasPhases)
			{
				while(vecPhases.size() < dat4.GetDepth2())
					vecPhases.push_back(0.);

				dat4.SetPhases(vecPhases);
			}
		}


		if(pRange)
		{
			double dXMin = spinXMin->value();
			double dXMax = spinXMax->value();
			double dYMin = spinYMin->value();
			double dYMax = spinYMax->value();

			pRange->SetXRange(dXMin, dXMax);
			pRange->SetYRange(dYMin, dYMax);

			if(m_pCurPlot->GetType() == PLOT_3D)
				((Plot3d*)pPlot2d)->RefreshTSlice(((Plot3d*)pPlot2d)->GetCurT());
			else if(m_pCurPlot->GetType() == PLOT_4D)
				((Plot4d*)pPlot2d)->RefreshTFSlice(((Plot4d*)pPlot2d)->GetCurT(), ((Plot4d*)pPlot2d)->GetCurF());
		}
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
