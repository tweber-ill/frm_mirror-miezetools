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

#define PAD_X 24
#define PAD_Y 24

Plot2d::Plot2d(QWidget* pParent, const char* pcTitle, bool bCountData)
			: SubWindowBase(pParent),
			  m_pImg(0), m_bLog(bCountData), m_bCountData(bCountData),
			  m_bHasXYMinMax(0), m_bXIsLog(0), m_bYIsLog(0)
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

void Plot2d::SetXYMinMax(double dXMin, double dXMax, double dYMin, double dYMax)
{
	this->m_bHasXYMinMax = 1;
	this->m_dXMin = dXMin;
	this->m_dXMax = dXMax;
	this->m_dYMin = dYMin;
	this->m_dYMax = dYMax;
}

uint Plot2d::GetSpectroColor01(double dVal) const
{
	const uint blue = 0xff0000ff;
	const uint red = 0xffff0000;
	const uint yellow = 0xffffff00;
	const uint cyan = 0xff00ffff;

	const uint col1[] = {blue, cyan, yellow};
	const uint col2[] = {cyan, yellow, red};

	const uint iNumCols = sizeof(col1)/sizeof(col1[0]);
	const double dNumCols = double(iNumCols);
	uint iIdx = uint(dVal*dNumCols);

	if(iIdx >= iNumCols)
		iIdx = iNumCols-1;
	uint col = lerprgb(col1[iIdx], col2[iIdx], fmod(dVal, 1./dNumCols)*dNumCols);
	return col;
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

	const uint blue = 0xff0000ff;
	const uint red = 0xffff0000;

	if(dVal <= dMin)
		return blue;
	else if(dVal >= dMax)
		return red;

	double dSpec = (dVal-dMin) / (dMax-dMin);
	return GetSpectroColor01(dSpec);
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
	painter.setRenderHint(QPainter::Antialiasing, true);

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
	painter.setPen(penOrg);

	painter.restore();
}

void Plot2d::plot(unsigned int iW, unsigned int iH, const double *pdat, const double *perr)
{
	m_dat.SetSize(iW, iH);
	m_dat.SetVals(pdat, perr);

	RefreshPlot();
}

void Plot2d::plot(const Data2& dat)
{
	m_dat = dat;
	RefreshPlot();
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
	if(this->GetType() == PLOT_2D)
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
		if(m_bHasXYMinMax)
		{
			// range 0..1
			dX_Val = dX / double(m_dat.GetWidth()-1);
			dY_Val = dY / double(m_dat.GetHeight()-1);

			if(m_bXIsLog)
			{
				double dXMin = log10(m_dXMin);
				double dXMax = log10(m_dXMax);
				dX_Val = pow(10., dXMin + dX_Val*(dXMax-dXMin));
			}
			else
				dX_Val = dX_Val * (m_dXMax-m_dXMin) + m_dXMin;

			if(m_bYIsLog)
			{
				double dYMin = log10(m_dYMin);
				double dYMax = log10(m_dYMax);
				dY_Val = pow(10., dYMin + dY_Val*(dYMax-dYMin));
			}
			else
				dY_Val = dY_Val * (m_dYMax-m_dYMin) + m_dYMin;

			ostr << "(" << dX_Val << ", " << dY_Val << "): ";
			bPixelVal = 0;
		}
		else
		{
			ostr << "pixel (" << iX << ", " << iY << "): ";
			bPixelVal = 1;
		}

		if(m_bCountData)
			ostr << group_numbers<uint>(iPixelVal);
		else
			ostr << dPixelVal;


		if(m_dat.IsRoiActive())
		{
			bool bInsideRoi = 0;
			const Roi *pRoi = m_dat.GetRoi();

			if(bPixelVal)
				bInsideRoi = pRoi->IsInside(iX, iY);
			else
				bInsideRoi = pRoi->IsInside(dX_Val, dY_Val);

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
			dMin = floor(safe_log10(dMin));
			dMax = ceil(safe_log10(dMax));
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


#include "plot2d.moc"
