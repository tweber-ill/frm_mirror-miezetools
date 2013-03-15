/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 11-mar-2013
 */

#include "plot2d.h"
#include <QtGui/QPainter>
#include <iostream>
#include <sstream>
#include "../helper/misc.h"
#include "../helper/string.h"

#define PAD_X 16
#define PAD_Y 16

Plot2d::Plot2d(QWidget* pParent, const char* pcTitle, bool bCountData)
			: SubWindowBase(pParent),
			  m_pImg(0), m_bLog(1), m_bCountData(bCountData)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(QString(pcTitle));
	this->setMouseTracking(true);
}

Plot2d::~Plot2d()
{ clear(); }

void Plot2d::clear()
{
	if(m_pImg)
	{
		delete m_pImg;
		m_pImg = 0;
	}
}

uint Plot2d::GetSpectroColor(double dVal) const
{
	double dMin = m_dat.GetMin();
	double dMax = m_dat.GetMax();

	if(m_bLog)
	{
		dVal = safe_log10(dVal);
		dMin = floor(safe_log10(dMin));
		dMax = ceil(safe_log10(dMax));

		if(m_bCountData)
		{
			if(dMin < -1)
				dMin = -1;
		}
	}

	double dSpec = (dVal-dMin) / (dMax-dMin);

	const uint blue = 0xff0000ff;
	const uint red = 0xffff0000;
	const uint yellow = 0xffffff00;
	const uint cyan = 0xff00ffff;

	const uint col1[] = {blue, cyan, yellow};
	const uint col2[] = {cyan, yellow, red};

	if(dVal < dMin)
		return blue;
	else if(dVal > dMax)
		return red;

	const uint iNumCols = sizeof(col1)/sizeof(col1[0]);
	const double dNumCols = double(iNumCols);
	uint iIdx = uint(dSpec*dNumCols);

	if(iIdx >= iNumCols)
		iIdx = iNumCols-1;
	uint col = lerprgb(col1[iIdx], col2[iIdx], fmod(dSpec, 1./dNumCols)*dNumCols);
	return col;
}

QSize Plot2d::minimumSizeHint() const
{
	return QSize(256,256);
}

void Plot2d::paintEvent (QPaintEvent *pEvent)
{
	if(!m_pImg) return;
	QSize size = this->size();

	QPainter painter(this);
	painter.save();

	// axis labels
	painter.setFont(QFont("Numbus Mono L", 10));
	painter.drawText(QRect(0, 0, size.width(), PAD_Y), Qt::AlignCenter, m_strTitle);
	painter.drawText(QRect(0, size.height() - PAD_Y, size.width(), PAD_Y), Qt::AlignCenter, m_strXAxis);

	painter.save();
	painter.rotate(-90);
	painter.drawText(QRect(0, 0, -size.height(), PAD_X), Qt::AlignCenter, m_strYAxis);
	painter.restore();


	QRect rect(PAD_X,PAD_Y,size.width()-2*PAD_X,size.height()-2*PAD_Y);
	painter.drawImage(rect, *m_pImg);

	painter.restore();
}

void Plot2d::plot(unsigned int iW, unsigned int iH, const double *pdat, const double *perr)
{
	m_dat.SetSize(iW, iH);
	m_dat.SetVals(pdat, perr);

	RefreshPlot();
}

void Plot2d::RefreshPlot()
{
	clear();
	m_pImg = new QImage(m_dat.GetWidth(), m_dat.GetHeight(), QImage::Format_RGB32);

	for(uint iY=0; iY<m_dat.GetHeight(); ++iY)
	{
		QRgb* pline = (QRgb*)m_pImg->scanLine(iY);
		for(uint iX=0; iX<m_dat.GetWidth(); ++iX)
		{
			pline[iX] = GetSpectroColor(m_dat.GetVal(iX, iY));
			//m_pImg->setPixel(iX, iY, GetSpectroColor(m_dat.GetVal(iX, iY)));
		}
	}

	this->repaint(rect());
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
	if(m_bCountData)
	{
		std::ostringstream ostr_total;
		ostr_total << "total counts: " << group_numbers<uint>(m_dat.GetTotal());
		emit SetStatusMsg(ostr_total.str().c_str(), 1);
	}
	else
	{
		emit SetStatusMsg("", 1);
	}

	QString strTitle = this->windowTitle();
	if(m_bLog)
		strTitle += QString(" (log)");
	emit SetStatusMsg(strTitle.toAscii().data(), 0);
}

void Plot2d::mouseMoveEvent(QMouseEvent* pEvent)
{
	const QPoint& pt = pEvent->pos();
	QSize size = this->size();

	double dX = double(pt.x()-PAD_X) / double(size.width()-2*PAD_X) * double(m_dat.GetWidth());
	double dY = double(pt.y()-PAD_Y) / double(size.height()-2*PAD_Y) * double(m_dat.GetHeight());

	if(dX < 0.) dX = 0.;
	if(dX > m_dat.GetWidth()-1) dX = m_dat.GetWidth()-1;
	if(dY < 0.) dY = 0.;
	if(dY > m_dat.GetHeight()-1) dY = m_dat.GetHeight()-1;

	uint iX = uint(dX);
	uint iY = uint(dY);
	double dPixelVal = m_dat.GetVal(iX, iY);
	uint iPixelVal = uint(dPixelVal);

	iY = m_dat.GetHeight()-1-iY;

	std::ostringstream ostr;
	if(m_bCountData)
	{
		ostr << "pixel (" << iX << ", " << iY << "): " << group_numbers<uint>(iPixelVal);
	}
	else
	{
		ostr << "pixel (" << iX << ", " << iY << "): " << dPixelVal;
	}

	emit SetStatusMsg(ostr.str().c_str(), 2);
	RefreshStatusMsgs();
}


#include "plot2d.moc"
