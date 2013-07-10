/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 15-mar-2013
 */
#include "plot4d.h"

#include <QtGui/QGridLayout>
#include <iostream>
#include <sstream>
#include <math.h>

#include "../helper/string.h"
#include "../helper/misc.h"
#include "../helper/fourier.h"
#include "../helper/mieze.hpp"

#include "../fitter/models/msin.h"

#include "../settings.h"


Plot4d::Plot4d(QWidget* pParent, const char* pcTitle,  bool bCountData)
		: Plot2d(pParent, pcTitle, bCountData), m_iCurT(0), m_iCurF(0)
{
	this->m_bLog = false;
}

Plot4d::~Plot4d() {}

void Plot4d::plot_manual()
{
	RefreshTFSlice(0,0);
	emit DataLoaded();
}

void Plot4d::plot(uint iW, uint iH, uint iT, uint iF, const double *pdat, const double *perr)
{
	m_dat4.SetSize(iW, iH, iT, iF);
	m_dat4.SetVals(pdat, perr);

	plot_manual();
}

void Plot4d::RefreshTFSlice(uint iT, uint iF)
{
	m_iCurT = iT;
	m_iCurF = iF;
	m_dat = m_dat4.GetVal(iT, iF);
	RefreshPlot();
}


SubWindowBase* Plot4d::clone() const
{
	Plot4d* pPlot = new Plot4d(parentWidget(), windowTitle().toStdString().c_str(), m_bCountData);
	pPlot->m_dat4 = this->m_dat4;

	pPlot->m_strXAxis = this->m_strXAxis;
	pPlot->m_strYAxis = this->m_strYAxis;
	pPlot->m_strZAxis = this->m_strZAxis;
	pPlot->m_strTitle = this->m_strTitle;

	pPlot->RefreshTFSlice(this->m_iCurT, this->m_iCurF);

	return pPlot;
}

void Plot4dWrapper::DataLoaded()
{
	const Data4 &dat = m_pPlot->GetData();
	uint iMaxT = dat.GetDepth();
	uint iCurT = m_pPlot->GetCurT();
	m_pSliderT->setMinimum(0);
	m_pSliderT->setMaximum(iMaxT-1);
	m_pSliderT->setValue(iCurT);

	uint iMaxF = dat.GetDepth2();
	uint iCurF = m_pPlot->GetCurF();
	m_pSliderF->setMinimum(0);
	m_pSliderF->setMaximum(iMaxF-1);
	m_pSliderF->setValue(iCurF);

	// take window title from child widget
	if(windowTitle()=="")
		setWindowTitle(GetActualWidget()->windowTitle());

	/*
	QString strZ = m_pPlot->GetZStr();
	if(strZ=="")
		strZ = "t: ";
	m_pLabel->setText(strZ);
	*/

	m_pPlot->RefreshPlot();
	m_pPlot->RefreshStatusMsgs();
}

void Plot4dWrapper::SliderValueChanged()
{
	uint iValT = (uint)m_pSliderT->value();
	uint iValF = (uint)m_pSliderF->value();
	m_pPlot->RefreshTFSlice(iValT, iValF);
}

void Plot4d::RefreshStatusMsgs()
{
	Plot2d::RefreshStatusMsgs();

	if(m_bCountData)
	{
		std::ostringstream ostr_total;
		ostr_total << "total counts: " << group_numbers<uint>(GetData().GetTotal())
					  << ", counts: " << group_numbers<uint>(GetData2().GetTotal());
		emit SetStatusMsg(ostr_total.str().c_str(), 1);
	}
	else
	{
		emit SetStatusMsg("", 1);
	}
}

void Plot4d::ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts)
{
	m_dat4.ChangeResolution(iNewWidth, iNewHeight, bKeepTotalCounts);
	RefreshPlot();
}



Plot* Plot4d::ConvertTo1d(int iFoil)
{
	const double dNumOsc = Settings::Get<double>("mieze/num_osc");
	const Plot4d* pPlot4d = this;

	if(iFoil<0)
	{	// total, corrected MIEZE signal
		const Data4& dat4 = pPlot4d->GetData();
		const unsigned int iNumFoils = dat4.GetDepth2();
		Fourier fourier(dat4.GetDepth());

		double *pdyTotal = new double[dat4.GetDepth()];
		double *pdyerrTotal = new double[dat4.GetDepth()];
		autodeleter<double> _a1(pdyTotal, 1);
		autodeleter<double> _a2(pdyerrTotal, 1);
		for(unsigned int iTc=0; iTc<dat4.GetDepth(); ++iTc)
			pdyTotal[iTc] = pdyerrTotal[iTc] = 0.;

		std::vector<double*> vecXFoil; vecXFoil.resize(iNumFoils);
		std::vector<double*> vecYFoil; vecYFoil.resize(iNumFoils);
		std::vector<double*> vecYErrFoil; vecYErrFoil.resize(iNumFoils);
		std::vector<double*> vecYCorrFoil; vecYCorrFoil.resize(iNumFoils);
		std::vector<std::vector<double*>*> all_vecs = {&vecXFoil, &vecYFoil, &vecYErrFoil, &vecYCorrFoil};

		double *pdPhases = new double[dat4.GetDepth()];
		autodeleter<double> _a7(pdPhases, 1);

		double dMeanPhase=0.;
		double dTotalCnts = 0.;
		for(unsigned int iFoil=0; iFoil<iNumFoils; ++iFoil)
		{
			Data1 dat = dat4.GetXYSum(iFoil);
			const std::vector<double> *pvecDatX, *pvecDatY, *pvecDatYErr;
			dat.GetData(&pvecDatX, &pvecDatY, &pvecDatYErr);
			// TODO: case for dat4.GetDepth() != pvecDatX->size() (ROI)

			const std::vector<double> &vecX = *pvecDatX,
									&vecY = *pvecDatY,
									&vecYErr = *pvecDatYErr;

			double *pdxFoil = new double[dat4.GetDepth()];
			double *pdyFoil = new double[dat4.GetDepth()];
			double *pdyerrFoil = new double[dat4.GetDepth()];
			vecXFoil[iFoil] = pdxFoil;
			vecYFoil[iFoil] = pdyFoil;
			vecYErrFoil[iFoil] = pdyerrFoil;

			for(unsigned int iTc=0; iTc<dat4.GetDepth(); ++iTc)
			{
				pdxFoil[iTc] = vecX[iTc];
				pdyFoil[iTc] = vecY[iTc];
				pdyerrFoil[iTc] = vecYErr[iTc];
			}

			MiezeSinModel *pModel = 0;
			double dThisNumOsc = dNumOsc;
			double dFreq = get_mieze_freq(pdxFoil, dat.GetLength(), dNumOsc);
			bool bOk = ::get_mieze_contrast(dFreq, dThisNumOsc, dat.GetLength(), pdxFoil, pdyFoil, pdyerrFoil, &pModel);
			pdPhases[iFoil] = pModel->GetPhase();

			/*Fourier fourier(dat4.GetDepth());
			double dCont, dPhi;
			bool bOk = fourier.get_contrast(dThisNumOsc, pdyFoil, dCont, dPhi);
			pdPhases[iFoil] = dPhi;*/

			//std::cout << "fit: " << pModel->print(1) << std::endl;
			double dCnts = sum_vec(vecY);
			dTotalCnts += dCnts;
			dMeanPhase += pdPhases[iFoil] * dCnts;

			if(pModel) delete pModel;
		}
		dMeanPhase /= dTotalCnts;
		dMeanPhase = fmod(dMeanPhase, 2.*M_PI);


		for(unsigned int iFoil=0; iFoil<iNumFoils; ++iFoil)
		{
			double *pdxFoil = vecXFoil[iFoil];
			double *pdyFoil = vecYFoil[iFoil];
			double *pdyerrFoil = vecYErrFoil[iFoil];

			double *pdyFoilCorr = new double[dat4.GetDepth()];
			vecYCorrFoil[iFoil] = pdyFoilCorr;

			fourier.phase_correction_0(pdyFoil, pdyFoilCorr, (pdPhases[iFoil]-dMeanPhase)/dNumOsc);

			for(unsigned int iTc=0; iTc<dat4.GetDepth(); ++iTc)
				pdyTotal[iTc] += pdyFoilCorr[iTc];
		}
		//std::cout << "mean phase: " << dMeanPhase/M_PI*180. << std::endl;

		for(unsigned int iTc=0; iTc<dat4.GetDepth(); ++iTc)
			pdyerrTotal[iTc] = sqrt(pdyTotal[iTc]);

		std::string strTitle = pPlot4d->windowTitle().toStdString();
		strTitle += std::string(" -> t channels (corr)");

		Plot *pPlot = new Plot(0, strTitle.c_str());
		pPlot->plot(dat4.GetDepth(), vecXFoil[0], pdyTotal, pdyerrTotal);

		pPlot->SetLabels(/*pPlot4d->GetZStr().toAscii().data()*/"t", "I");
		pPlot->SetTitle("");


		// cleanup
		for(std::vector<double*>* pVec : all_vecs)
			for(double *pArr : *pVec)
				delete[] pArr;

		return pPlot;
	}
	else
	{
		std::string strTitle = pPlot4d->windowTitle().toStdString();
		strTitle += std::string(" -> t channels");

		Data1 dat = pPlot4d->GetData().GetXYSum(iFoil);
		const std::vector<double> *pvecDatX, *pvecDatY, *pvecDatYErr;
		dat.GetData(&pvecDatX, &pvecDatY, &pvecDatYErr);
		// TODO

		double *pdx = vec_to_array<double>(*pvecDatX);
		double *pdy = vec_to_array<double>(*pvecDatY);
		double *pdyerr = vec_to_array<double>(*pvecDatYErr);
		autodeleter<double> _a0(pdx, 1);
		autodeleter<double> _a1(pdy, 1);
		autodeleter<double> _a2(pdyerr, 1);

		Plot *pPlot = new Plot(0, strTitle.c_str());
		pPlot->plot(dat.GetLength(), pdx, pdy, pdyerr);

		pPlot->SetLabels(/*pPlot4d->GetZStr().toAscii().data()*/"t", "I");
		pPlot->SetTitle("");

		return pPlot;
	}
}

Plot3d* Plot4d::ConvertTo3d(int iFoil)
{
	std::ostringstream ostrTitle;
	ostrTitle << windowTitle().toStdString() << " -> ";

	const Data4& dat4 = this->GetData();
	Data3 dat3;
	dat3.CopyRoiFlagsFrom(&dat4);

	if(iFoil<0)
	{
		ostrTitle << "foil sum";

		dat3.SetZero();
		for(unsigned int iFoil=0; iFoil<dat4.GetDepth2(); ++iFoil)
			dat3.Add(dat4.GetVal(iFoil));
	}
	else
	{
		ostrTitle << "foil " << iFoil;

		dat3 = dat4.GetVal(iFoil);
	}

	Plot3d* pPlot = new Plot3d(0, ostrTitle.str().c_str(), m_bCountData);
	pPlot->plot(dat3);
	pPlot->SetLabels(GetXStr().toStdString().c_str(), GetYStr().toStdString().c_str(), "I");

	return pPlot;
}


bool Plot4d::LoadXML(Xml& xml, Blob& blob, const std::string& strBase)
{
	m_dat4.LoadXML(xml, blob, strBase + "data/");
	Plot2d::LoadXML(xml, blob, strBase+"sub_2d/");

	m_iCurT = xml.Query<unsigned int>((strBase + "cur_t").c_str(), 0);
	m_iCurF = xml.Query<unsigned int>((strBase + "cur_f").c_str(), 0);
	setWindowTitle(xml.QueryString((strBase+"window_title").c_str(), "").c_str());

	emit DataLoaded();
	return 1;
}

bool Plot4d::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	ostr << "<type> plot_4d </type>\n";

	ostr << "<window_title> " << windowTitle().toStdString() << " </window_title>\n";
	ostr << "<cur_t> " << m_iCurT << " </cur_t>\n";
	ostr << "<cur_f> " << m_iCurF << " </cur_f>\n";

	ostr << "<sub_2d>\n";
	Plot2d::SaveXML(ostr, ostrBlob);
	ostr << "</sub_2d>\n";


	ostr << "<data>\n";
	m_dat4.SaveXML(ostr, ostrBlob);
	ostr << "</data>\n";

	return 1;
}



Plot4dWrapper::Plot4dWrapper(QWidget* pParent, const char* pcTitle, bool bCountData)
	: SubWindowBase(pParent)
{
	this->setWindowTitle(QString(pcTitle));
	m_pPlot = new Plot4d(this, pcTitle, bCountData);

	Init();
}

Plot4dWrapper::Plot4dWrapper(Plot4d* pPlot)
{
	m_pPlot = pPlot;
	setWindowTitle(m_pPlot->windowTitle());

	Init();
	DataLoaded();
}


Plot4dWrapper::~Plot4dWrapper()
{
	delete m_pPlot;
}

void Plot4dWrapper::Init()
{
	this->setAttribute(Qt::WA_DeleteOnClose);


	QGridLayout *pLayout = new QGridLayout(this);
	pLayout->addWidget(m_pPlot, 0, 0, 1, 2);

	m_pLabelF = new QLabel(this);
	m_pLabelF->setText("foil: ");
	pLayout->addWidget(m_pLabelF, 1,0,1,1);

	m_pSliderF = new QSlider(this);
	m_pSliderF->setOrientation(Qt::Horizontal);
	m_pSliderF->setTracking(1);
	pLayout->addWidget(m_pSliderF, 1,1,1,1);

	m_pLabelT = new QLabel(this);
	m_pLabelT->setText("t: ");
	pLayout->addWidget(m_pLabelT, 2,0,1,1);

	m_pSliderT = new QSlider(this);
	m_pSliderT->setOrientation(Qt::Horizontal);
	m_pSliderT->setTracking(1);
	pLayout->addWidget(m_pSliderT, 2,1,1,1);


	QObject::connect(m_pPlot, SIGNAL(DataLoaded()), this, SLOT(DataLoaded()));
	QObject::connect(m_pSliderF, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged()));
	QObject::connect(m_pSliderT, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged()));
}


#include "plot4d.moc"
