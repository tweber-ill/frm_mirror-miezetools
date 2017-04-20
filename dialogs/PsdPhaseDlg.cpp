/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 29-may-2013
 * @license GPLv3
 */

#include <QtGui/QMdiSubWindow>
#include <QtGui/QMessageBox>

#include <set>

#include "PsdPhaseDlg.h"
#include "ListDlg.h"
#include "../main/settings.h"
#include "../helper/mieze.h"
#include "../helper/mfourier.h"

#include "../tlibs/phys/mieze.h"
#include "../tlibs/math/math.h"
#include "../tlibs/helper/misc.h"
#include "../tlibs/log/log.h"

#include "../fitter/models/msin.h"

namespace units = boost::units;
namespace co = boost::units::si::constants::codata;

using tl::cm;
using tl::angstrom;
using tl::ps;


PsdPhaseDlg::PsdPhaseDlg(QWidget* pParent)
					: QDialog(pParent), m_bAllowUpdate(0),
					  m_pPlot(new Plot2d(this, "PSD Phase", 0, 1))
{
	setupUi(this);
	spinPhase->setMaximum(2.*M_PI);

	m_vecDSpinBoxes = {spinlx, spinly, spinXCenter, spinYCenter, spinLam, spinTau, spinLs, spinPhase};
	m_vecSpinBoxes = {spinXPix, spinYPix};
	m_vecStrDSpinBoxes = {"lx", "ly", "center_x", "center_y", "lam", "tau", "Ls", "central_phase"};
	m_vecStrSpinBoxes = {"xpix", "ypix"};

	m_pPlot->SetLabels("x Position (cm)", "y Position (cm)", "Phase (rad)");

	QGridLayout *pGrid = new QGridLayout(frame);
	pGrid->addWidget(m_pPlot, 0, 0, 1, 1);

	for(QDoubleSpinBox* pSpin : m_vecDSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(double)), this, SLOT(Update()));
	for(QSpinBox* pSpin : m_vecSpinBoxes)
		QObject::connect(pSpin, SIGNAL(valueChanged(int)), this, SLOT(Update()));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	LoadLastSettings();
	m_bAllowUpdate = 1;
	Update();
}

PsdPhaseDlg::~PsdPhaseDlg()
{
	delete m_pPlot;
}

void PsdPhaseDlg::Update()
{
	if(!m_bAllowUpdate)
		return;

	units::quantity<units::si::length> lx = spinlx->value() *cm;
	units::quantity<units::si::length> ly = spinly->value()*cm;
	units::quantity<units::si::length> Ls = spinLs->value()*cm;
	units::quantity<units::si::time> tau = spinTau->value()*ps;
	units::quantity<units::si::length> lam = spinLam->value()*angstrom;
	unsigned int iXPixels = spinXPix->value();
	unsigned int iYPixels = spinYPix->value();
	units::quantity<units::si::length> xpos = spinXCenter->value()*cm;
	units::quantity<units::si::length> ypos = spinYCenter->value()*cm;
	units::quantity<units::si::plane_angle> pase_offs = spinPhase->value()*units::si::radians;

	ublas::matrix<double> matPhases;
	tl::mieze_reduction_det(lx, ly, xpos, ypos, Ls, tau, lam, pase_offs, iXPixels, iYPixels, &matPhases);

	m_dat.FromMatrix(matPhases);
	m_dat.SetXRange(-lx/2./cm, lx/2./cm);
	m_dat.SetYRange(-ly/2./cm, ly/2./cm);
	m_dat.SetMinMax(0., 2.*M_PI);

	m_pPlot->plot(m_dat);
}

void PsdPhaseDlg::LoadLastSettings()
{
	for(unsigned int i=0; i<m_vecSpinBoxes.size(); ++i)
	{
		QSpinBox* pSpin = m_vecSpinBoxes[i];
		const std::string strName = "phase/" + m_vecStrSpinBoxes[i];

		if(Settings::HasKey(strName.c_str()))
			pSpin->setValue(Settings::Get<int>(strName.c_str()));
	}

	for(unsigned int i=0; i<m_vecDSpinBoxes.size(); ++i)
	{
		QDoubleSpinBox* pSpin = m_vecDSpinBoxes[i];
		const std::string strName = "phase/" + m_vecStrDSpinBoxes[i];

		if(Settings::HasKey(strName.c_str()))
			pSpin->setValue(Settings::Get<double>(strName.c_str()));
	}
}

void PsdPhaseDlg::SaveLastSettings()
{
	for(unsigned int i=0; i<m_vecSpinBoxes.size(); ++i)
	{
		const QSpinBox* pSpin = m_vecSpinBoxes[i];
		const std::string strName = "phase/" + m_vecStrSpinBoxes[i];

		Settings::Set<int>(strName.c_str(), pSpin->value());
	}

	for(unsigned int i=0; i<m_vecDSpinBoxes.size(); ++i)
	{
		const QDoubleSpinBox* pSpin = m_vecDSpinBoxes[i];
		const std::string strName = "phase/" + m_vecStrDSpinBoxes[i];

		Settings::Set<double>(strName.c_str(), pSpin->value());
	}
}

void PsdPhaseDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
		reject();
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		SaveLastSettings();
		accept();
	}
}


// --------------------------------------------------------------------------------



PsdPhaseCorrDlg::PsdPhaseCorrDlg(QWidget* pParent, QMdiArea *pmdi)
				: QDialog(pParent), m_pmdi(pmdi)
{
	setupUi(this);

	connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(AddItemSelected()));
	connect(btnAddActive, SIGNAL(clicked(bool)), this, SLOT(AddActiveItemSelected()));
	connect(btnDel, SIGNAL(clicked(bool)), this, SLOT(RemoveItemSelected()));
	connect(btnPhaseImg, SIGNAL(clicked(bool)), this, SLOT(UsePhaseItemSelected()));
	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	RefreshPhaseCombo();
}

PsdPhaseCorrDlg::~PsdPhaseCorrDlg()
{}


void PsdPhaseCorrDlg::AddActiveItemSelected()
{
	if(!m_pmdi->activeSubWindow()) return;

	SubWindowBase* pWnd = (SubWindowBase*)m_pmdi->activeSubWindow()->widget();
	if(!pWnd || !pWnd->GetActualWidget()) return;

	pWnd = pWnd->GetActualWidget();
	if(pWnd->GetType()!=PLOT_3D && pWnd->GetType()!=PLOT_4D)
	{
		QMessageBox::critical(this, "Error", "Wrong data type, need 3D or 4D.");
		return;
	}

	ListGraphsItem* pItem = new ListGraphsItem(listGraphs);
	pItem->setSubWnd(pWnd);
	pItem->setText(pWnd->windowTitle());

	listGraphs->addItem(pItem);
	RemoveDuplicate();
}

void PsdPhaseCorrDlg::AddItemSelected()
{
	ListGraphsDlg dlg(this);

	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow *pItem : lst)
	{
		SubWindowBase *pWnd = (SubWindowBase *) pItem->widget();
		if(pWnd && (pWnd->GetType()==PLOT_3D || pWnd->GetType()==PLOT_4D))
			dlg.AddSubWnd(pWnd);
	}

	if(dlg.exec() == QDialog::Accepted)
	{
		std::list<SubWindowBase*> lstWnds = dlg.GetSelectedSubWnds();

		for(auto wnd : lstWnds)
		{
			ListGraphsItem* pItem = new ListGraphsItem(listGraphs);
			pItem->setSubWnd(wnd);
			pItem->setText(wnd->windowTitle());

			listGraphs->addItem(pItem);
		}
	}

	RemoveDuplicate();
}

void PsdPhaseCorrDlg::SubWindowRemoved(SubWindowBase *pSWB)
{
	if(!pSWB) return;

	for(int i=0; i<listGraphs->count(); ++i)
	{
		SubWindowBase* pCurItem = ((ListGraphsItem*)listGraphs->item(i))->subWnd();

		if(pCurItem==pSWB)
		{
			delete listGraphs->item(i);
			--i;
		}
	}
}

void PsdPhaseCorrDlg::RemoveItemSelected()
{
	if(listGraphs->selectedItems().size() == 0)
		listGraphs->selectAll();

	for(auto pItem : listGraphs->selectedItems())
		delete pItem;
}

void PsdPhaseCorrDlg::RemoveDuplicate()
{
	std::set<SubWindowBase*> setItems;

	for(int i=0; i<listGraphs->count(); ++i)
	{
		SubWindowBase* pCurItem = ((ListGraphsItem*)listGraphs->item(i))->subWnd();
		if(!setItems.insert(pCurItem).second)	// already in set?
		{
			delete listGraphs->item(i);
			--i;
		}
	}
}

void PsdPhaseCorrDlg::RefreshPhaseCombo()
{
	// get previously selected item
	const int iLastIdx = comboPhaseImg->currentIndex();
	const Plot2d* pLastPlt = 0;
	if(iLastIdx>=0 && iLastIdx<int(m_vecPhaseImgs.size()))
		pLastPlt = m_vecPhaseImgs[iLastIdx];


	comboPhaseImg->clear();
	m_vecPhaseImgs.clear();
	QStringList lstStr;

	QList<QMdiSubWindow*> lst = m_pmdi->subWindowList();
	for(QMdiSubWindow *pItem : lst)
	{
		SubWindowBase *pWnd = (SubWindowBase *) pItem->widget();

		if(pWnd && pWnd->GetType()==PLOT_2D)
		{
			const Plot2d* plt = (Plot2d*) pWnd->GetActualWidget();

			lstStr.push_back(plt->windowTitle());
			m_vecPhaseImgs.push_back(plt);
		}
	}

	comboPhaseImg->addItems(lstStr);


	// select previously selected item
	for(unsigned int iIdx=0; iIdx<m_vecPhaseImgs.size(); ++iIdx)
	{
		if(m_vecPhaseImgs[iIdx] != pLastPlt)
			continue;

		comboPhaseImg->setCurrentIndex(iIdx);
		break;
	}
}

void PsdPhaseCorrDlg::UsePhaseItemSelected()
{
	if(!m_pmdi->activeSubWindow()) return;

	SubWindowBase* pWnd = (SubWindowBase*)m_pmdi->activeSubWindow()->widget();
	if(!pWnd || !pWnd->GetActualWidget()) return;

	pWnd = pWnd->GetActualWidget();
	if(pWnd->GetType()!=PLOT_2D)
	{
		QMessageBox::critical(this, "Error", "Wrong data type, need 2D.");
		return;
	}

	for(unsigned int iIdx=0; iIdx<m_vecPhaseImgs.size(); ++iIdx)
	{
		if(m_vecPhaseImgs[iIdx] == pWnd)
		{
			comboPhaseImg->setCurrentIndex(iIdx);
			break;
		}
	}
}

void PsdPhaseCorrDlg::DoPhaseCorr()
{
	PsdPhaseMethod meth = METH_INVALID;
	if(radioFFT->isChecked())
		meth = METH_FFT;
	else if(radioFit->isChecked())
		meth = METH_FIT;
	else if(radioImage->isChecked())
		meth = METH_THEO;

	const Plot2d* pPhases = 0;
	if(meth == METH_THEO)
	{
		int iPhaseIdx = comboPhaseImg->currentIndex();
		if(iPhaseIdx<0 || iPhaseIdx>=int(m_vecPhaseImgs.size()))
		{
			QMessageBox::critical(this, "Error", "No phase image selected.");
			return;
		}
		pPhases = m_vecPhaseImgs[iPhaseIdx];
	}

	for(int iGraphs=0; iGraphs<listGraphs->count(); ++iGraphs)
	{
		SubWindowBase* pCurItem = ((ListGraphsItem*)listGraphs->item(iGraphs))->subWnd();
		if(!pCurItem) continue;

		SubWindowBase *pCorrectedPlot = 0;

		pCurItem = pCurItem->GetActualWidget();
		if(pCurItem->GetType() == PLOT_3D)
			pCorrectedPlot = new Plot3dWrapper(DoPhaseCorr(pPhases, (Plot3d*)pCurItem, meth));
		else if(pCurItem->GetType() == PLOT_4D)
			pCorrectedPlot = new Plot4dWrapper(DoPhaseCorr(pPhases, (Plot4d*)pCurItem, meth));

		if(!pCorrectedPlot)
			continue;

		emit AddNewPlot(pCorrectedPlot);
	}
}

Plot3d* PsdPhaseCorrDlg::DoPhaseCorr(const Plot2d* pPhasesPlot, const Plot3d* pDatPlot, PsdPhaseMethod meth)
{
	Plot3d* pDatPlot_shifted = (Plot3d*)pDatPlot->clone();
	bool bIsCountData = pDatPlot_shifted->IsCountData();
	pDatPlot_shifted->setWindowTitle(pDatPlot_shifted->windowTitle() + " (psd corr)");

	const Data3* pDat = &pDatPlot->GetData();
	const double dMinCounts = Settings::Get<int>("misc/min_counts");

	Data3* pDat_shifted = &pDatPlot_shifted->GetData();
	const Data2* pPhases = 0;
	if(meth == METH_THEO)
	{
		pPhases = &pPhasesPlot->GetData2();
		if(pDat->GetWidth()!=pPhases->GetWidth() || pDat->GetHeight()!=pPhases->GetHeight())
			tl::log_warn("Pixel sizes of \"", pDatPlot->windowTitle().toStdString(),
					"\" and \"", pPhasesPlot->windowTitle().toStdString(),
					"\" do not match.");
	}

	const double dNumOsc = Settings::Get<double>("mieze/num_osc");

	MFourier fourier(pDat->GetDepth());
	double *pdMem = new double[pDat->GetDepth()*5];
	double *pdY = pdMem;
	double *pdY_shift = pdMem + 1*pDat->GetDepth();
	double *pdYErr = pdMem + 2*pDat->GetDepth();
	double *pdYErr_shift = pdMem + 3*pDat->GetDepth();
	double *pdX = pdMem + 4*pDat->GetDepth();

	unsigned int iUnfittedPixels=0;
	for(unsigned int iY=0; iY<pDat->GetHeight(); ++iY)
		for(unsigned int iX=0; iX<pDat->GetWidth(); ++iX)
		{
			Data1 dat = pDat->GetXY(iX, iY);
			for(unsigned int iT=0; iT<dat.GetLength(); ++iT)
			{
				pdY[iT] = dat.GetY(iT);
				pdYErr[iT] = dat.GetYErr(iT);
				pdX[iT] = dat.GetX(iT);
			}

			if(pDatPlot->IsCountData() && dat.SumY()<dMinCounts)
			{
				for(unsigned int iT=0; iT<dat.GetLength(); ++iT)
				{
					pdY_shift[iT] = pdY[iT];
					pdYErr_shift[iT] = pdYErr[iT];
				}
			}
			else
			{
				double dPhase = 0.;

				if(meth == METH_THEO)
					dPhase = pPhases->GetVal(iX, iY);
				else if(meth == METH_FFT)
				{
					double dC;
					fourier.get_contrast(dNumOsc, pdY, dC, dPhase);
					//dPhase *= dNumOsc;
				}
				else if(meth == METH_FIT)
				{
					double dFreq = ::get_mieze_freq(pdX, dat.GetLength(), dNumOsc);
					double dThisNumOsc = dNumOsc;

					MiezeSinModel* pModel = 0;
					bool bOk = ::get_mieze_contrast(dFreq, dThisNumOsc, dat.GetLength(), pdX, pdY, pdYErr, &pModel);
					if(bOk && pModel)
						dPhase = pModel->GetPhase();
					else
						++iUnfittedPixels;

					delete pModel;
				}

				fourier.phase_correction_0(pdY, pdY_shift, dPhase/dNumOsc);
				fourier.phase_correction_0(pdYErr, pdYErr_shift, dPhase/dNumOsc);
			}

			for(unsigned int iT=0; iT<pDat->GetDepth(); ++iT)
			{
				if(pdY_shift[iT] < 0.) pdY_shift[iT] = 0.;
				pdYErr_shift[iT] = std::fabs(pdYErr_shift[iT]);

				pDat_shifted->SetVal(iX, iY, iT, pdY_shift[iT]);
				pDat_shifted->SetErr(iX, iY, iT, pdYErr_shift[iT]);
			}
		}

	if(iUnfittedPixels)
	{
		tl::log_err("PSD phase correction: Could not fit ", iUnfittedPixels, " pixels.");
	}

	delete[] pdMem;

	pDat_shifted->RecalcMinMaxTotal();
	pDatPlot_shifted->RefreshTSlice(0);
	return pDatPlot_shifted;
}

Plot4d* PsdPhaseCorrDlg::DoPhaseCorr(const Plot2d* pPhasesPlot, const Plot4d* pDatPlot, PsdPhaseMethod meth)
{
	Plot4d* pDatPlot_shifted = (Plot4d*)pDatPlot->clone();
	bool bIsCountData = pDatPlot_shifted->IsCountData();
	pDatPlot_shifted->setWindowTitle(pDatPlot_shifted->windowTitle() + " (psd corr)");
	const double dMinCounts = Settings::Get<int>("misc/min_counts");

	const Data4* pDat = &pDatPlot->GetData();
	Data4* pDat_shifted = &pDatPlot_shifted->GetData();
	const Data2* pPhases = 0;
	if(meth == METH_THEO)
	{
		pPhases = &pPhasesPlot->GetData2();
		if(pDat->GetWidth()!=pPhases->GetWidth() || pDat->GetHeight()!=pPhases->GetHeight())
			tl::log_warn("Pixel sizes of \"", pDatPlot->windowTitle().toStdString(),
					"\" and \"", pPhasesPlot->windowTitle().toStdString(),
					"\" do not match.");
	}

	const double dNumOsc = Settings::Get<double>("mieze/num_osc");

	MFourier fourier(pDat->GetDepth());
	double *pdMem = new double[pDat->GetDepth()*5];
	double *pdY = pdMem;
	double *pdY_shift = pdMem + 1*pDat->GetDepth();
	double *pdYErr = pdMem + 2*pDat->GetDepth();
	double *pdYErr_shift = pdMem + 3*pDat->GetDepth();
	double *pdX = pdMem + 4*pDat->GetDepth();

	unsigned int iUnfittedPixels=0;
	for(unsigned int iFoil=0; iFoil<pDat->GetDepth2(); ++iFoil)
		for(unsigned int iY=0; iY<pDat->GetHeight(); ++iY)
			for(unsigned int iX=0; iX<pDat->GetWidth(); ++iX)
			{
				Data1 dat = pDat->GetXYD2(iX, iY, iFoil);
				for(unsigned int iT=0; iT<dat.GetLength(); ++iT)
				{
					pdY[iT] = dat.GetY(iT);
					pdYErr[iT] = dat.GetYErr(iT);
					pdX[iT] = dat.GetX(iT);
				}

				if(pDatPlot->IsCountData() && dat.SumY()<dMinCounts)
				{
					for(unsigned int iT=0; iT<dat.GetLength(); ++iT)
					{
						pdY_shift[iT] = pdY[iT];
						pdYErr_shift[iT] = pdYErr[iT];
					}
				}
				else
				{
					double dPhase = 0.;

					if(meth == METH_THEO)
						dPhase = pPhases->GetVal(iX, iY);
					else if(meth == METH_FFT)
					{
						double dC;
						fourier.get_contrast(dNumOsc, pdY, dC, dPhase);
						//dPhase *= dNumOsc;
					}
					else if(meth == METH_FIT)
					{
						double dFreq = ::get_mieze_freq(pdX, dat.GetLength(), dNumOsc);
						double dThisNumOsc = dNumOsc;

						MiezeSinModel* pModel = 0;
						bool bOk = ::get_mieze_contrast(dFreq, dThisNumOsc, dat.GetLength(), pdX, pdY, pdYErr, &pModel);
						if(bOk && pModel)
							dPhase = pModel->GetPhase();
						else
							++iUnfittedPixels;

						delete pModel;
					}

					fourier.phase_correction_0(pdY, pdY_shift, dPhase/dNumOsc);
					fourier.phase_correction_0(pdYErr, pdYErr_shift, dPhase/dNumOsc);
				}

				for(unsigned int iT=0; iT<pDat->GetDepth(); ++iT)
				{
					if(pdY_shift[iT] < 0.) pdY_shift[iT] = 0.;
					pdYErr_shift[iT] = std::fabs(pdYErr_shift[iT]);

					pDat_shifted->SetVal(iX, iY, iT, iFoil, pdY_shift[iT]);
					pDat_shifted->SetErr(iX, iY, iT, iFoil, pdYErr_shift[iT]);
				}
			}

	if(iUnfittedPixels)
	{
		tl::log_err("PSD phase correction: Could not fit ", iUnfittedPixels, " pixels.");
	}

	delete[] pdMem;

	pDat_shifted->RecalcMinMaxTotal();
	pDatPlot_shifted->RefreshTFSlice(0,0);
	return pDatPlot_shifted;
}

void PsdPhaseCorrDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
	   buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		DoPhaseCorr();
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
		reject();
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
		QDialog::accept();
}


#include "PsdPhaseDlg.moc"
