/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 11-mar-2013
 */

#include "plot2d.h"
#include <QtGui/QPainter>
#include "../helper/misc.h"


Plot2d::Plot2d(QWidget* pParent, const char* pcTitle) : SubWindowBase(pParent), m_pImg(0)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(QString(pcTitle));
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
	double dSpec = (dVal-dMin) / (dMax-dMin);

	const uint blue = 0xff0000ff;
	const uint red = 0xffff0000;
	const uint yellow = 0xffffff00;
	const uint cyan = 0xff00ffff;

	const uint col1[] = {blue, cyan, yellow};
	const uint col2[] = {cyan, yellow, red};

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

	QPainter painter(this);
	painter.save();

	QSize size = this->size();

	QRect rect(16,16,size.width()-32,size.height()-32);
	painter.drawImage(rect, *m_pImg);

	painter.restore();
}

void Plot2d::plot(unsigned int iW, unsigned int iH, const double *pdat)
{
	m_dat.SetSize(iW, iH);
	m_dat.SetVals(pdat);

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
}


#include "plot2d.moc"
