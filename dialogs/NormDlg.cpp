/*
 * mieze-tool
 * @author tweber
 * @date 30-sep-2013
 * @license GPLv3
 */

#include "NormDlg.h"
#include "../plot/plot.h"
#include <QtGui/QMessageBox>


NormDlg::NormDlg(QWidget* pParent) : QDialog(pParent)
{
	this->setupUi(this);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));
}

NormDlg::~NormDlg()
{}


void NormDlg::SubWindowRemoved(SubWindowBase* pSWB)
{
	if(!pSWB) return;

	auto iter = std::find(m_allSubWnds.begin(), m_allSubWnds.end(), pSWB);
	if(iter == m_allSubWnds.end())
		return;

	int iWndIdx = std::distance(m_allSubWnds.begin(), iter);

	QComboBox* pCombos[] = {comboPlot, comboRef};
	for(QComboBox* pCombo : pCombos)
	{
		int iIdx = pCombo->currentIndex();

		int iNewIdx = iIdx;
		if(iIdx == iWndIdx)
			iNewIdx = 0;
		else if(iIdx > iWndIdx)
			--iNewIdx;

		pCombo->removeItem(iWndIdx);
		pCombo->setCurrentIndex(iNewIdx);
	}

	m_allSubWnds.erase(iter);
}

void NormDlg::SubWindowAdded(SubWindowBase* pSWB)
{
	if(!pSWB) return;

	// TODO: support the other plot types
	if(pSWB->GetType() != PLOT_1D)
		return;

	m_allSubWnds.push_back(pSWB);

	QComboBox* pCombos[] = {comboPlot, comboRef};
	for(QComboBox* pCombo : pCombos)
		pCombo->addItem(pSWB->windowTitle());
}


Plot* NormDlg::CreatePlot(const std::string& strTitle, QWidget* pPlotParent) const
{
	SubWindowBase *pSWBOrg = m_allSubWnds[comboPlot->currentIndex()];
	SubWindowBase *pSWBRef = m_allSubWnds[comboRef->currentIndex()];

	Plot* pPlotOrg = pSWBOrg->ConvertTo1d();
	Plot* pPlotRef = pSWBRef->ConvertTo1d();

	const unsigned int iCnt = pPlotOrg->GetData(0).dat.GetLength();
	if(iCnt != pPlotRef->GetData(0).dat.GetLength())
	{
		QMessageBox::critical((QWidget*)this, "Error", "Number of data points in graphs differ");
		return 0;
	}


	std::string strLabX = pPlotOrg->GetLabel(LABEL_X);
	std::string strLabY = pPlotOrg->GetLabel(LABEL_Y);



	double *pdX = new double[iCnt];
	double *pdY = new double[iCnt];
	double *pdYErr = new double[iCnt];

	for(unsigned int iPt=0; iPt<iCnt; ++iPt)
	{
		double dX = pPlotOrg->GetData(0).dat.GetX(iPt);
		double dY = pPlotOrg->GetData(0).dat.GetY(iPt);
		double dYErr = pPlotOrg->GetData(0).dat.GetYErr(iPt);

		double dX_ref = pPlotRef->GetData(0).dat.GetX(iPt);
		double dY_ref = pPlotRef->GetData(0).dat.GetY(iPt);
		double dYErr_ref = pPlotRef->GetData(0).dat.GetYErr(iPt);

		pdX[iPt] = dX;
		pdY[iPt] = dY / dY_ref;
		pdYErr[iPt] = std::sqrt((dYErr/dY_ref)*(dYErr/dY_ref) +
							dY*dYErr_ref/(dY_ref*dY_ref)*dY*dYErr_ref/(dY_ref*dY_ref));
	}


	Plot *pPlot = new Plot(pPlotParent, strTitle.c_str());
	pPlot->plot(iCnt, pdX, pdY, pdYErr);
	pPlot->SetLabels(strLabX.c_str(), strLabY.c_str());


	delete[] pdX; delete[] pdY; delete[] pdYErr;
	return pPlot;
}

void NormDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		Plot* pPlot = CreatePlot("Normalized Graph", this->parentWidget());
		if(pPlot)
			emit AddSubWindow((SubWindowBase*)pPlot);
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


#include "NormDlg.moc"
