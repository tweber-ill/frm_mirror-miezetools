/**
 * mieze-tool
 * plotter
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 11-mar-2013
 * @license GPLv3
 */

#include "plot2d.h"

#include <QtGui/QPainter>
#include <QtGui/QGridLayout>
#include <iostream>
#include <sstream>

#include "tlibs/helper/misc.h"
#include "tlibs/math/math.h"
#include "tlibs/string/string.h"


#define PAD_X 24
#define PAD_Y 24

Plot2d::Plot2d(QWidget* pParent, const char* pcTitle, bool bCountData, bool bPhaseData)
			: SubWindowBase(pParent),
			  m_pImg(0),
			  m_bLog(bCountData), m_bCountData(bCountData), m_bCyclicData(0),
			  m_bPhaseData(bPhaseData)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	if(pcTitle) this->setWindowTitle(QString(pcTitle));

	this->setMouseTracking(true);
}

Plot2d::Plot2d(const Plot2d& plot)
			: SubWindowBase(plot.parentWidget()), m_pImg(0)
{
	this->m_bLog = plot.m_bLog;
	this->m_bCountData = plot.m_bCountData;
	this->m_bCyclicData = plot.m_bCyclicData;
	this->m_bPhaseData = plot.m_bPhaseData;

	this->m_strXAxis = plot.m_strXAxis;
	this->m_strYAxis = plot.m_strYAxis;
	this->m_strZAxis = plot.m_strZAxis;
	this->m_strTitle = plot.m_strTitle;

	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(plot.windowTitle());
	this->setMouseTracking(true);

	this->plot(plot.GetData2());
}

SubWindowBase* Plot2d::clone() const
{
	Plot2d* pPlot = new Plot2d(*this);
	return pPlot;
}

Plot2d::~Plot2d()
{
	clear();
}

void Plot2d::ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts)
{
	m_dat.ChangeResolution(iNewWidth, iNewHeight, bKeepTotalCounts);
	RefreshPlot();
}

PlotInfo Plot2d::GetPlotInfo() const
{
	PlotInfo info;
	info.iWidth = m_dat.GetWidth();
	info.iHeight = m_dat.GetHeight();
	return info;
}

void Plot2d::clear()
{
	if(m_pImg)
	{
		delete m_pImg;
		m_pImg = 0;
	}
}

uint Plot2d::GetSpectroColor01(double dVal) const
{
	const uint blue = 0xff0000ff;
	const uint red = 0xffff0000;
	const uint yellow = 0xffffff00;
	const uint cyan = 0xff00ffff;

	const uint col1[] = {blue, cyan, yellow};
	const uint col2[] = {cyan, yellow, red};
	const uint col1_cyc[] = {blue, cyan, yellow, red};
	const uint col2_cyc[] = {cyan, yellow, red, blue};

	const uint iNumCols = (m_bCyclicData ? sizeof(col1_cyc)/sizeof(col1_cyc[0])  : sizeof(col1)/sizeof(col1[0]));
	const double dNumCols = double(iNumCols);
	uint iIdx = uint(dVal*dNumCols);

	const uint *pcol1 = (m_bCyclicData ? col1_cyc : col1);
	const uint *pcol2 = (m_bCyclicData ? col2_cyc : col2);

	if(iIdx >= iNumCols)
		iIdx = iNumCols-1;

	double dLerpVal = fmod(dVal, 1./dNumCols)*dNumCols;
	if(dVal == 0.) dLerpVal = 0.;
	if(dVal == 1.) dLerpVal = 1.;

	if(dLerpVal < 0.) dLerpVal = 0.;
	if(dLerpVal > 1.) dLerpVal = 1.;
	uint col = tl::lerprgb(pcol1[iIdx], pcol2[iIdx], dLerpVal);

	return col;
}

uint Plot2d::GetSpectroColor(double dVal) const
{
	double dMin = m_dat.GetMin();
	double dMax = m_dat.GetMax();

	if(IsPhaseData())
	{
		dMin = 0.;
		dMax = 2.*M_PI;
	}

	if(m_bLog)
	{
		dVal = tl::safe_log10(dVal);
		dMin = floor(tl::safe_log10(dMin));
		dMax = ceil(tl::safe_log10(dMax));

		if(m_bCountData)
		{
			if(dMin < -1)
				dMin = -1;
		}
	}

	if(dVal < dMin) dVal=dMin;
	else if(dVal > dMax) dVal=dMax;

	double dSpec = (dVal-dMin) / (dMax-dMin);
	return GetSpectroColor01(dSpec);
}

QSize Plot2d::minimumSizeHint() const
{
	return QSize(256,256);
}

void Plot2d::resizeEvent(QResizeEvent *pEvent)
{
}

void Plot2d::paintEvent(QPaintEvent *pEvent)
{
	if(!pEvent || pEvent->rect().width()==0 || pEvent->rect().height()==0)
		return;

	if(!m_pImg) return;
	QSize size = this->size();
	if(size.width()==0 || size.height()==0)
		return;

	QPainter painter(this);
	painter.save();

	painter.setClipping(1);
	painter.setClipRegion(pEvent->region());

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);

	// axis labels
	painter.setFont(QFont("Nimbus Sans L", 10));
	painter.drawText(QRect(0, 0, size.width(), PAD_Y), Qt::AlignCenter, m_strTitle);
	painter.drawText(QRect(0, size.height()-PAD_Y+0, size.width(), PAD_Y-0), Qt::AlignCenter, m_strXAxis);

	painter.save();
	painter.rotate(-90);
	painter.drawText(QRect(0, 0, -size.height(), PAD_X-4), Qt::AlignCenter, m_strYAxis);
	painter.restore();


	const int iColorBarWidth = PAD_X / 2;
	const int iColorBarPad = PAD_X / 4;

	// image
	m_rectImage = QRect(PAD_X,PAD_Y,
					size.width()-2*PAD_X, size.height()-2*PAD_Y);

	QRect& rect = m_rectImage;
	painter.drawImage(rect, *m_pImg);

	// frame
	QRect rectFrame = rect;
	rectFrame.setX(rect.x()-1); rect.setY(rect.y()-1);
	painter.drawRect(rectFrame);


	// colorbar frame
	m_rectCB = QRect(rect.right()+iColorBarPad, rect.top()+1, iColorBarWidth, rect.height()-1);
	painter.drawRect(m_rectCB);

	// colorbar
	QPen penOrg = painter.pen();
	for(int iB=0; iB<m_rectCB.height()-1; ++iB)
	{
		double dCBVal = double(iB)/double(m_rectCB.height()-1);

		QPen penCB = penOrg;
		penCB.setColor(GetSpectroColor01(dCBVal));
		painter.setPen(penCB);

		uint iX0 = m_rectCB.left() + 1;
		uint iX1 = m_rectCB.right();
		uint iY = m_rectCB.bottom() - iB;
		painter.drawLine(iX0, iY, iX1, iY);
	}

	painter.translate(m_rectImage.bottomLeft() + QPoint(0., 1.));
	double dScaleX = 1.*double(m_rectImage.width()) / double(m_pImg->width());
	double dScaleY = -1.*double(m_rectImage.height()) / double(m_pImg->height());
	painter.scale(dScaleX, dScaleY);

	QPen penROI = penOrg;
	penROI.setColor(Qt::white);
	penROI.setWidthF(0.75);
	painter.setPen(penROI);
	m_dat.GetRoi(0).DrawRoi(painter, m_dat);

	penROI.setColor(Qt::yellow);
	painter.setPen(penROI);
	m_dat.GetRoi(1).DrawRoi(painter, m_dat);

	painter.setPen(penOrg);


	painter.restore();
	painter.end();
	SubWindowBase::paintEvent(pEvent);
}

void Plot2d::plot(unsigned int iW, unsigned int iH, const double *pdat, const double *perr)
{
	m_dat.SetSize(iW, iH);
	m_dat.SetVals(pdat, perr);

	CheckCyclicData();
	RefreshPlot();
}

void Plot2d::plot(const Data2& dat)
{
	m_dat = dat;

	CheckCyclicData();
	RefreshPlot();
}

void Plot2d::CheckCyclicData()
{
	const double dTolerance = 0.2;

	m_bCyclicData = 0;
	if(IsPhaseData())
	{
		double dMin = m_dat.GetMin();
		double dMax = m_dat.GetMax();

		if(dMax - dMin >= 2.*M_PI-dTolerance)
			m_bCyclicData = 1;
	}
}

void Plot2d::RefreshPlot()
{
	clear();

	m_pImg = new QImage(m_dat.GetWidth(), m_dat.GetHeight(), QImage::Format_RGB32);

	for(uint iY=0; iY<m_dat.GetHeight(); ++iY)
	{
		QRgb* pline = (QRgb*)m_pImg->scanLine(m_dat.GetHeight()-iY-1);
		for(uint iX=0; iX<m_dat.GetWidth(); ++iX)
		{
			pline[iX] = GetSpectroColor(m_dat.GetValRaw(iX, iY));
			//m_pImg->setPixel(iX, iY, GetSpectroColor(m_dat.GetVal(iX, iY)));
		}
	}

	this->update(rect());
	RefreshStatusMsgs();
}

void Plot2d::SetLog(bool bLog)
{
	if(m_bLog == bLog)
		return;

	m_bLog = bLog;

	RefreshPlot();
}

bool Plot2d::GetLog() const
{
	return m_bLog;
}

void Plot2d::RefreshStatusMsgs()
{
	if(this->GetType() == PLOT_2D)
	{
		if(m_bCountData)
		{
			std::ostringstream ostr_total;
			ostr_total << "total counts: " << tl::var_to_str<uint>(m_dat.GetTotal(),10,3);
			emit SetStatusMsg(ostr_total.str().c_str(), 1);
		}
		else
		{
			emit SetStatusMsg("", 1);
		}
	}

	QString strTitle = this->windowTitle();
	if(m_bLog)
		strTitle += QString(" (log10)");
	emit SetStatusMsg(strTitle.toAscii().data(), 0);
}



void Plot2d::mouseMoveEvent(QMouseEvent* pEvent)
{
	const QPoint& pt = pEvent->pos();
	QSize size = this->size();

	// cursor in image
	if(m_rectImage.contains(pt, false))
	{
		this->setCursor(Qt::CrossCursor);

		double dX = double(pt.x()-PAD_X) / double(size.width()-2*PAD_X) * double(m_dat.GetWidth());
		double dY = double(pt.y()-PAD_Y) / double(size.height()-2*PAD_Y) * double(m_dat.GetHeight());

		if(dX < 0.) dX = 0.;
		if(dX > m_dat.GetWidth()-1) dX = m_dat.GetWidth()-1;
		if(dY < 0.) dY = 0.;
		if(dY > m_dat.GetHeight()-1) dY = m_dat.GetHeight()-1;

		uint iX = uint(dX);
		uint iY = uint(dY);

		dY = double(m_dat.GetHeight())-1.-dY;
		iY = m_dat.GetHeight()-1-iY;

		double dPixelVal = m_dat.GetVal(iX, iY);
		uint iPixelVal = uint(dPixelVal);

		std::ostringstream ostr;
		bool bPixelVal = 0;

		double dX_Val = 0., dY_Val = 0.;
		if(m_dat.HasRange())
		{
			dX_Val = m_dat.GetRangeXPos(iX);
			dY_Val = m_dat.GetRangeYPos(iY);

			ostr << "(" << dX_Val << ", " << dY_Val << "): ";
			bPixelVal = 0;
		}
		else
		{
			ostr << "pixel (" << iX << ", " << iY << "): ";
			bPixelVal = 1;
		}

		if(m_bCountData)
			ostr << tl::var_to_str<uint>(iPixelVal,10,3);
		else
			ostr << dPixelVal;


		if(m_dat.IsAnyRoiActive())
		{
			bool bInsideRoi = 0;

			if(bPixelVal)
				bInsideRoi = m_dat.IsInsideRoi(iX, iY);
			else
				bInsideRoi = m_dat.IsInsideRoi(dX_Val, dY_Val);

			ostr << " (" << (bInsideRoi?"in ROI":"not in ROI") << ")";
		}


		emit SetStatusMsg(ostr.str().c_str(), 2);
		RefreshStatusMsgs();
	}
	// cursor in colorbar
	else if(m_rectCB.contains(pt, false))
	{
		this->setCursor(Qt::CrossCursor);

		double dVal01 = 1. - double(pt.y()-m_rectCB.top()+1) / double(m_rectCB.height()-1);
		if(dVal01 > 1.) dVal01 = 1.;
		else if(dVal01 < 0.) dVal01 = 0.;

		double dMin = m_dat.GetMin();
		double dMax = m_dat.GetMax();
		double dVal = 0.;

		if(m_bLog)
		{
			dMin = floor(tl::safe_log10(dMin));
			dMax = ceil(tl::safe_log10(dMax));
			if(m_bCountData)
			{
				if(dMin < -1)
					dMin = -1;
			}

			dVal = pow(10, dMin + dVal01*(dMax-dMin));
		}
		else
			dVal = dMin + dVal01*(dMax-dMin);

		std::ostringstream ostr;
		ostr << "colorbar value: " << (m_bCountData?uint(dVal):dVal);

		emit SetStatusMsg(ostr.str().c_str(), 2);
		RefreshStatusMsgs();
	}
	// cursor outside
	else
	{
		this->setCursor(Qt::ArrowCursor);
	}
}

void Plot2d::SetROI(const Roi* pROI, bool bAntiRoi)
{
	DataInterface* pDat = GetDataInterface();
	if(!pDat) return;

	//if(pDat == &GetData2())
	//	std::cout << "is plot 2d" << std::endl;

	pDat->SetROI(pROI, bAntiRoi);
	GetData2().SetROI(pROI, bAntiRoi);
}

Roi* Plot2d::GetROI(bool bAntiRoi)
{
	DataInterface* pDat = this->GetDataInterface();
	if(!pDat) return 0;

	return &pDat->GetRoi(bAntiRoi);
}


bool Plot2d::LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase)
{
	m_dat.LoadXML(xml, blob, strBase + "data/");

	m_bLog = xml.Query<bool>((strBase+"log").c_str(), 0);
	m_bCountData = xml.Query<bool>((strBase+"count_data").c_str(), 1);
	m_bCyclicData = xml.Query<bool>((strBase+"cyclic_data").c_str(), 0);
	m_bPhaseData = xml.Query<bool>((strBase+"phase_data").c_str(), 0);

	m_strXAxis = xml.QueryString((strBase+"x_label").c_str(), "x").c_str();
	m_strYAxis = xml.QueryString((strBase+"y_label").c_str(), "y").c_str();
	m_strZAxis = xml.QueryString((strBase+"z_label").c_str(), "z").c_str();
	m_strTitle = xml.QueryString((strBase+"title").c_str(), "").c_str();
	setWindowTitle(xml.QueryString((strBase+"window_title").c_str(), "").c_str());

	return 1;
}

bool Plot2d::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	ostr << "<type> plot_2d </type>\n";

	ostr << "<log> " << m_bLog << " </log>\n";
	ostr << "<count_data> " << m_bCountData << " </count_data>\n";
	ostr << "<cyclic_data> " << m_bCyclicData << " </cyclic_data>\n";
	ostr << "<phase_data> " << m_bPhaseData << " </phase_data>\n";

	ostr << "<x_label> " << m_strXAxis.toStdString() << " </x_label>\n";
	ostr << "<y_label> " << m_strYAxis.toStdString() << " </y_label>\n";
	ostr << "<z_label> " << m_strZAxis.toStdString() << " </z_label>\n";
	ostr << "<title> " << m_strTitle.toStdString() << " </title>\n";
	ostr << "<window_title> " << windowTitle().toStdString() << " </window_title>\n";

	ostr << "<data>\n";
	m_dat.SaveXML(ostr, ostrBlob);
	ostr << "</data>\n";

	return 1;
}


std::string Plot2d::GetLabel(LabelType iWhich) const
{
	if(iWhich == LABEL_X)
		return GetXStr().toStdString();
	else if(iWhich == LABEL_Y)
		return GetYStr().toStdString();
	else if(iWhich == LABEL_Z)
		return GetZStr().toStdString();
	else if(iWhich == LABEL_TITLE)
		return GetTitle();

	return "";
}
void Plot2d::SetLabel(LabelType iWhich, const char* pcLab)
{
	if(iWhich == LABEL_X)
		m_strXAxis = pcLab;
	else if(iWhich == LABEL_Y)
		m_strYAxis = pcLab;
	else if(iWhich == LABEL_Z)
		m_strZAxis = pcLab;
	else if(iWhich == LABEL_TITLE)
		SetTitle(pcLab);
	else
		return;

	RefreshPlot();
}


#include "plot2d.moc"
