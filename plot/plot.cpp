/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 06-mar-2013
 */

#include "plot.h"
#include <QtGui/QPainter>
#include <limits>
#include <iostream>
#include "../helper/string.h"

#define PAD_X 16
#define PAD_Y 16

Plot::Plot(QWidget* pParent, const char* pcTitle) : SubWindowBase(pParent),
									m_dxmin(0.), m_dxmax(0.), m_dymin(0.), m_dymax(0.)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(QString(pcTitle));
	this->setMouseTracking(true);
}

Plot::~Plot()
{
	clear();
}

QSize Plot::minimumSizeHint() const
{
	return QSize(320,240);
}

void Plot::estimate_minmax()
{
	m_dxmin = m_dymin = std::numeric_limits<double>::max();
	m_dxmax = m_dymax = -m_dxmin;

	// for all plot objects
	for(const Data1& obj : m_vecObjs)
	{
		//for all points
		for(unsigned int uiPt=0; uiPt<obj.GetLength(); ++uiPt)
		{
			double x = obj.GetX(uiPt);
			double y = obj.GetY(uiPt);
			double xerr = obj.GetXErr(uiPt);
			double yerr = obj.GetYErr(uiPt);

			if(x - xerr < m_dxmin)
				m_dxmin = x - xerr;
			if(x + xerr > m_dxmax)
				m_dxmax = x + xerr;
			if(y - yerr < m_dymin)
				m_dymin = y - yerr;
			if(y + yerr > m_dymax)
				m_dymax = y + yerr;
		}
	}

	const double dPadX = (m_dxmax-m_dxmin) / 16.;
	const double dPadY = (m_dymax-m_dymin) / 16.;
	m_dxmin -= dPadX;
	m_dxmax += dPadX;
	m_dymin -= dPadY;
	m_dymax += dPadY;
}

QColor Plot::GetColor(unsigned int iPlotObj)
{
	QColor col = QColor::fromRgbF(0., 0., 0., 1.);
	return col;
}

void Plot::paintEvent (QPaintEvent *pEvent)
{
	QSize size = this->size();
	double dStartX = PAD_X;
	double dStartY = PAD_Y;
	double dCurH = size.height() - PAD_Y*2;
	double dCurW = size.width() - PAD_X*2;


	QPainter painter(this);
	painter.save();

	painter.setFont(QFont("Numbus Mono L", 10));
	painter.drawText(QRect(0, 0, size.width(), PAD_Y), Qt::AlignCenter, m_strTitle);
	painter.drawText(QRect(0, size.height() - PAD_Y, size.width(), PAD_Y), Qt::AlignCenter, m_strXAxis);

	painter.save();
	painter.rotate(-90);
	painter.drawText(QRect(0, 0, -size.height(), PAD_X), Qt::AlignCenter, m_strYAxis);
	painter.restore();

	QPen pen = QColor::fromRgb(0,0,0,255);
	pen.setStyle(Qt::SolidLine);
	pen.setWidthF(1.);
	painter.setPen(pen);
	painter.fillRect(dStartX, dStartY, dCurW, dCurH, QColor::fromRgbF(1.,1.,1.,1.));
	painter.drawRect(dStartX, dStartY, dCurW, dCurH);

	// grid
	pen.setColor(QColor::fromRgbF(0., 0., 0., 0.5));
	pen.setStyle(Qt::DotLine);
	painter.setPen(pen);

	const unsigned int iNumGridLines = 10;
	for(unsigned int iGrid=1; iGrid<iNumGridLines+1; ++iGrid)
	{
		// x line
		painter.drawLine(QLineF(dStartX, dStartY+iGrid*dCurH/double(iNumGridLines+1),
											dStartX+dCurW, dStartY+iGrid*dCurH/double(iNumGridLines+1)));

		// y line
		painter.drawLine(QLineF(dStartX+iGrid*dCurW/double(iNumGridLines+1), dStartY,
											dStartX+iGrid*dCurW/double(iNumGridLines+1), dStartY+dCurH));
	}

	double dScaleX = dCurW/(m_dxmax-m_dxmin) * 1.0;
	double dScaleY = dCurH/(m_dymax-m_dymin) * 1.0;
	//std::cout << dScaleX << " " << dScaleY << std::endl;

	painter.translate(-m_dxmin*dScaleX+PAD_X, m_dymin*dScaleY+dCurH+PAD_Y);
	painter.scale(dScaleX, -dScaleY);

	painter.setRenderHint(QPainter::Antialiasing, true);

	// for all plot objects
	for(unsigned int iObj=0; iObj<m_vecObjs.size(); ++iObj)
	{
		const Data1& obj = m_vecObjs[iObj];
		QColor col = GetColor(iObj);

		//for all points
		for(unsigned int uiPt=0; uiPt<obj.GetLength(); ++uiPt)
		{
			const QPointF coord(obj.GetX(uiPt), obj.GetY(uiPt));
			const QPointF err(obj.GetXErr(uiPt), obj.GetYErr(uiPt));

			// point
			painter.setPen(Qt::NoPen);
			QBrush brush(Qt::SolidPattern);
			brush.setColor(col);
			painter.setBrush(brush);
			painter.drawEllipse(coord, 2./dScaleX, 2./dScaleY);

			// y error bar
			if(err.y() != 0.)
			{
				QPen penerr(col);
				penerr.setWidthF(1.5/dScaleX);
				painter.setPen(penerr);
				painter.drawLine(QLineF(coord.x(), coord.y()-err.y()/2.,
													coord.x(), coord.y()+err.y()/2.));
			}

			// x error bar
			if(err.y() != 0.)
			{
				QPen penerr(col);
				penerr.setWidthF(1.5/dScaleX);
				painter.setPen(penerr);
				painter.drawLine(QLineF(coord.x()-err.x()/2., coord.y(),
													coord.x()+err.x()/2., coord.y()));
			}
		}
	}

	painter.restore();
}

void Plot::plot(unsigned int iNum, const double *px, const double *py, const double *pyerr, const double *pxerr)
{
	Data1 obj(iNum, px, py, pyerr, pxerr);
	m_vecObjs.push_back(obj);

	estimate_minmax();
	RefreshStatusMsgs();
}

void Plot::clear()
{
	m_vecObjs.clear();
	this->repaint();
}

void Plot::RefreshStatusMsgs()
{
	QString strTitle = this->windowTitle();
	emit SetStatusMsg(strTitle.toAscii().data(), 0);
	emit SetStatusMsg("", 1);
}

void Plot::mouseMoveEvent(QMouseEvent* pEvent)
{
	const QPoint& pt = pEvent->pos();
	const QSize size = this->size();

	const double dw = m_dxmax-m_dxmin;
	const double dh = m_dymax-m_dymin;

	// map between [0..1]
	double dX = double(pt.x()-PAD_X) / double(size.width()-2*PAD_X);
	double dY = 1. - double(pt.y()-PAD_Y) / double(size.height()-2*PAD_Y);

	// transform to plot ranges
	dX = m_dxmin + dX*dw;
	dY = m_dymin + dY*dh;

	if(dX < m_dxmin) dX = m_dxmin;
	if(dX > m_dxmax) dX = m_dxmax;
	if(dY < m_dymin) dY = m_dymin;
	if(dY > m_dymax) dY = m_dymax;

	std::ostringstream ostr;
	ostr << "(" << dX << ", " << dY << ")";

	emit SetStatusMsg(ostr.str().c_str(), 2);
	RefreshStatusMsgs();
}

#include "plot.moc"
