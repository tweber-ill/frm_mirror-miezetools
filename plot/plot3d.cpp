/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 15-mar-2013
 */
#include "plot3d.h"

#include <QtGui/QGridLayout>
#include <iostream>
#include <sstream>

#include "../helper/string.h"

Plot3d::Plot3d(QWidget* pParent, const char* pcTitle,  bool bCountData)
		: Plot2d(pParent, pcTitle, bCountData), m_iCurT(0)
{
	this->m_bLog = false;
}

Plot3d::~Plot3d()
{}

void Plot3d::plot_manual()
{
	RefreshTSlice(0);
	emit DataLoaded();
}

void Plot3d::plot(uint iW, uint iH, uint iT, const double *pdat, const double *perr)
{
	m_dat3.SetSize(iW, iH, iT);
	m_dat3.SetVals(pdat, perr);

	plot_manual();
}

void Plot3d::RefreshTSlice(uint iT)
{
	m_iCurT = iT;
	m_dat = m_dat3.GetVal(iT);
	RefreshPlot();
}


Plot3dWrapper::Plot3dWrapper(QWidget* pParent, const char* pcTitle, bool bCountData)
	: SubWindowBase(pParent)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(QString(pcTitle));

	m_pPlot = new Plot3d(this, pcTitle, bCountData);

	QGridLayout *pLayout = new QGridLayout(this);
	pLayout->addWidget(m_pPlot, 0, 0, 1, 2);

	m_pLabel = new QLabel(this);
	m_pLabel->setText("t: ");
	pLayout->addWidget(m_pLabel, 1,0,1,1);

	m_pSlider = new QSlider(this);
	m_pSlider->setOrientation(Qt::Horizontal);
	m_pSlider->setTracking(1);
	pLayout->addWidget(m_pSlider, 1,1,1,1);


	QObject::connect(m_pPlot, SIGNAL(DataLoaded()), this, SLOT(DataLoaded()));
	QObject::connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged()));
}

Plot3dWrapper::~Plot3dWrapper()
{
	delete m_pPlot;
}

void Plot3dWrapper::DataLoaded()
{
	const Data3 &dat = m_pPlot->GetData();
	uint iMaxT = dat.GetDepth();
	uint iCurT = m_pPlot->GetCurT();

	m_pSlider->setMinimum(0);
	m_pSlider->setMaximum(iMaxT-1);
	m_pSlider->setValue(iCurT);

	QString strZ = m_pPlot->GetZStr();
	if(strZ=="")
		strZ = "t: ";
	m_pLabel->setText(strZ);
}

void Plot3dWrapper::SliderValueChanged()
{
	uint iVal = (uint)m_pSlider->value();
	m_pPlot->RefreshTSlice(iVal);
}


void Plot3d::RefreshStatusMsgs()
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

#include "plot3d.moc"
