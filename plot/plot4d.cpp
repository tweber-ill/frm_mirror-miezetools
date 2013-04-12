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

#include "../helper/string.h"
#include "../helper/misc.h"

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


Plot4dWrapper::Plot4dWrapper(QWidget* pParent, const char* pcTitle, bool bCountData)
	: SubWindowBase(pParent)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(QString(pcTitle));

	m_pPlot = new Plot4d(this, pcTitle, bCountData);

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

Plot4dWrapper::~Plot4dWrapper()
{
	delete m_pPlot;
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

	/*
	QString strZ = m_pPlot->GetZStr();
	if(strZ=="")
		strZ = "t: ";
	m_pLabel->setText(strZ);
	*/
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

Plot* Plot4d::ConvertTo1d(int iFoil)
{
	const Plot4d* pPlot4d = this;

	if(iFoil<0)
	{
		// TODO
		return ConvertTo1d(0);
	}
	else
	{
		std::string strTitle = pPlot4d->windowTitle().toStdString();
		strTitle += std::string(" -> t channels");

		Data1 dat = pPlot4d->GetData().GetXYSum(iFoil);

		double *pdx = vec_to_array<double>(dat.GetX());
		double *pdy = vec_to_array<double>(dat.GetY());
		double *pdyerr = vec_to_array<double>(dat.GetYErr());
		autodeleter<double> _a0(pdx, 1);
		autodeleter<double> _a1(pdy, 1);
		autodeleter<double> _a2(pdyerr, 1);

		Plot *pPlot = new Plot(0, strTitle.c_str());
		pPlot->plot(dat.GetLength(), pdx, pdy, pdyerr);

		pPlot->SetLabels(pPlot4d->GetZStr().toAscii().data(), "intensity");
		pPlot->SetTitle("");

		return pPlot;
	}
}

#include "plot4d.moc"
