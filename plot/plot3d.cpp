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
#include "../helper/misc.h"

Plot3d::Plot3d(QWidget* pParent, const char* pcTitle,  bool bCountData)
		: Plot2d(pParent, pcTitle, bCountData), m_iCurT(0)
{
	this->m_bLog = false;
}

Plot3d::~Plot3d()
{}

SubWindowBase* Plot3d::clone() const
{
	Plot3d* pPlot = new Plot3d(parentWidget(), windowTitle().toStdString().c_str(), m_bCountData);
	pPlot->m_dat3 = this->m_dat3;

	pPlot->m_strXAxis = this->m_strXAxis;
	pPlot->m_strYAxis = this->m_strYAxis;
	pPlot->m_strZAxis = this->m_strZAxis;
	pPlot->m_strTitle = this->m_strTitle;

	pPlot->RefreshTSlice(this->m_iCurT);

	return pPlot;
}

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

void Plot3d::plot(const Data3& dat3)
{
	m_dat3 = dat3;
	plot_manual();
}

void Plot3d::RefreshTSlice(uint iT)
{
	m_iCurT = iT;
	m_dat = m_dat3.GetVal(iT);
	RefreshPlot();
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

void Plot3d::ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts)
{
	m_dat3.ChangeResolution(iNewWidth, iNewHeight, bKeepTotalCounts);
	RefreshPlot();
}


Plot* Plot3d::ConvertTo1d(int iParam)
{
	const Plot3d* pPlot3d = this;

	std::string strTitle = pPlot3d->windowTitle().toStdString();
	strTitle += std::string(" -> t channels");

	Data1 dat = pPlot3d->GetData().GetXYSum();
	const std::vector<double> *pvecDatX, *pvecDatY, *pvecDatYErr;
	dat.GetData(&pvecDatX, &pvecDatY, &pvecDatYErr);

	double *pdx = vec_to_array<double>(*pvecDatX);
	double *pdy = vec_to_array<double>(*pvecDatY);
	double *pdyerr = vec_to_array<double>(*pvecDatYErr);
	autodeleter<double> _a0(pdx, 1);
	autodeleter<double> _a1(pdy, 1);
	autodeleter<double> _a2(pdyerr, 1);

	Plot *pPlot = new Plot(0, strTitle.c_str());
	pPlot->plot(dat.GetLength(), pdx, pdy, pdyerr);

	pPlot->SetLabels(/*pPlot3d->GetZStr().toAscii().data()*/"t", "I");
	pPlot->SetTitle("");
	pPlot->GetData(0).dat.CopyParamMapsFrom(&GetData());

	return pPlot;
}

Plot2d* Plot3d::ConvertTo2d(int iFoil)
{
	std::ostringstream ostrTitle;
	ostrTitle << windowTitle().toStdString() << " -> ";

	const Data3& dat3 = this->GetData();
	Data2 dat2;
	dat2.CopyParamMapsFrom(&dat3);
	dat2.CopyXYRangeFrom(&dat3);
	dat2.CopyRoiFlagsFrom(&dat3);
	dat2.SetZero();

	ostrTitle << "sum";

	for(unsigned int iTC=0; iTC<dat3.GetDepth(); ++iTC)
		dat2.Add(dat3.GetVal(iFoil));

	Plot2d* pPlot = new Plot2d(0, ostrTitle.str().c_str(), m_bCountData);
	pPlot->plot(dat2);
	pPlot->SetLabels(GetXStr().toStdString().c_str(), GetYStr().toStdString().c_str(), "I");

	return pPlot;
}


bool Plot3d::LoadXML(Xml& xml, Blob& blob, const std::string& strBase)
{
	m_dat3.LoadXML(xml, blob, strBase + "data/");
	Plot2d::LoadXML(xml, blob, strBase+"sub_2d/");

	m_iCurT = xml.Query<unsigned int>((strBase + "cur_t").c_str(), 0);
	setWindowTitle(xml.QueryString((strBase+"window_title").c_str(), "").c_str());

	emit DataLoaded();
	return 1;
}

bool Plot3d::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	ostr << "<type> plot_3d </type>\n";

	ostr << "<window_title> " << windowTitle().toStdString() << " </window_title>\n";
	ostr << "<cur_t> " << m_iCurT << " </cur_t>\n";

	ostr << "<sub_2d>\n";
	Plot2d::SaveXML(ostr, ostrBlob);
	ostr << "</sub_2d>\n";


	ostr << "<data>\n";
	m_dat3.SaveXML(ostr, ostrBlob);
	ostr << "</data>\n";

	return 1;
}





Plot3dWrapper::Plot3dWrapper(QWidget* pParent, const char* pcTitle, bool bCountData)
	: SubWindowBase(pParent)
{
	this->setWindowTitle(QString(pcTitle));
	m_pPlot = new Plot3d(this, pcTitle, bCountData);

	Init();
}

Plot3dWrapper::Plot3dWrapper(Plot3d* pPlot)
				: SubWindowBase(pPlot->parentWidget())
{
	m_pPlot = pPlot;
	setWindowTitle(m_pPlot->windowTitle());

	Init();
	DataLoaded();
}

void Plot3dWrapper::Init()
{
	this->setAttribute(Qt::WA_DeleteOnClose);


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
	emit WndDestroyed(m_pPlot);
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

	QString strZ = /*m_pPlot->GetZStr();
	if(strZ=="")
		strZ =*/ "t: ";
	m_pLabel->setText(strZ);

	// take window title from child widget
	if(windowTitle()=="")
		setWindowTitle(GetActualWidget()->windowTitle());

	m_pPlot->RefreshPlot();
	m_pPlot->RefreshStatusMsgs();
}

void Plot3dWrapper::SliderValueChanged()
{
	uint iVal = (uint)m_pSlider->value();
	m_pPlot->RefreshTSlice(iVal);
}

std::string Plot3dWrapper::GetLabel(LabelType iWhich) const
{
	if(iWhich == LABEL_T)
		return m_pLabel->text().toStdString();
	return m_pPlot->GetLabel(iWhich);
}

void Plot3dWrapper::SetLabel(LabelType iWhich, const char* pcLab)
{
	if(iWhich == LABEL_T)
		m_pLabel->setText(pcLab);
	m_pPlot->SetLabel(iWhich, pcLab);
}


#include "plot3d.moc"
