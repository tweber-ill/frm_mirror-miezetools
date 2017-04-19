/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 31-oct-2013
 * @license GPLv3
 */

#include "RebinDlg.h"
#include "../plot/plot.h"
#include "../helper/mfourier.h"


RebinDlg::RebinDlg(QWidget* pParent) : QDialog(pParent), m_pCurPlot(0)
{
	setupUi(this);
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
}

RebinDlg::~RebinDlg()
{
}

void RebinDlg::SubWindowActivated(SubWindowBase* pSWB)
{
	if(!pSWB)
		return;

	m_pCurPlot = pSWB;
	const StringMap* pmapParams = m_pCurPlot->GetParamMapDyn();

	if(m_pCurPlot->GetType() == PLOT_1D)
	{
		labelStatus->setText(QString("Plot: ") + m_pCurPlot->windowTitle());
	}
	else
	{
		m_pCurPlot = 0;
		labelStatus->setText("Plot type not supported");
	}
}

void RebinDlg::SubWindowRemoved(SubWindowBase* pSWB)
{
	if(pSWB == m_pCurPlot)
	{
		m_pCurPlot = 0;
		labelStatus->setText("");
	}
}

void RebinDlg::ApplyChanges()
{
	if(!m_pCurPlot)
		return;

	double dShift = spinShift->value() / 100. * 2.*M_PI;

	Plot* pPlot = (Plot*)m_pCurPlot;
	unsigned int iDatLen = pPlot->GetData(0).dat.GetLength();

	//const double *pdXOrg = pPlot->GetData(0).dat.GetXPtr();
	const double *pdYOrg = pPlot->GetData(0).dat.GetYPtr();

	double *pdYNew = new double[iDatLen];

	MFourier fourier(iDatLen);
	fourier.phase_correction_0(pdYOrg, pdYNew, -dShift);


	for(unsigned int i=0; i<iDatLen; ++i)
	{
		pPlot->GetData(0).dat.SetY(i, pdYNew[i]);
		pPlot->GetData(0).dat.SetYErr(i, /*std::sqrt(pdYNew[i])*/ 0.);
	}

	delete[] pdYNew;

	pPlot->RefreshPlot();
}

void RebinDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		ApplyChanges();
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


#include "RebinDlg.moc"
