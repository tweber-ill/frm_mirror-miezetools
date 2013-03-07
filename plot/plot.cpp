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

Plot::Plot(QWidget* pParent) : QWidget(pParent)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle("Plot");
}

Plot::~Plot() {}

QSize Plot::minimumSizeHint() const
{
	return QSize(320,240);
}

void Plot::estimate_minmax(double& dxmin, double& dxmax, double& dymin, double& dymax)
{
	dxmin = dymin = std::numeric_limits<double>::max();
	dxmax = dymax = -dxmin;

	// for all plot objects
	for(const PlotObj& obj : m_vecObjs)
	{
		//for all points
		for(unsigned int uiPt=0; uiPt<obj.coord.size(); ++uiPt)
		{
			const QPointF& coord = obj.coord[uiPt];
			const QPointF& err = obj.err[uiPt];

			if(coord.x() - err.x() < dxmin)
				dxmin = coord.x() - err.x();
			if(coord.x() + err.x() > dxmax)
				dxmax = coord.x() + err.x();
			if(coord.y() - err.y() < dymin)
				dymin = coord.y() - err.y();
			if(coord.y() + err.y() > dymax)
				dymax = coord.y() + err.y();
		}
	}
}

QColor Plot::GetColor(unsigned int iPlotObj)
{
	QColor col = QColor::fromRgbF(0., 0., 0., 1.);
	return col;
}

void Plot::paintEvent (QPaintEvent *pEvent)
{
	QPainter painter(this);
	painter.save();

	QSize size = this->size();
	double dStartX = 16.;
	double dStartY = 16.;
	double dCurH = size.height() - 32.;
	double dCurW = size.width() - 32.;

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



	double dxmin, dxmax, dymin, dymax;
	estimate_minmax(dxmin, dxmax, dymin, dymax);

	double dPadX = (dxmax-dxmin) / 16.;
	double dPadY = (dymax-dymin) / 16.;
	dxmin -= dPadX;
	dxmax += dPadX;
	dymin -= dPadY;
	dymax += dPadY;

	double dScaleX = dCurW/(dxmax-dxmin) * 1.0;
	double dScaleY = dCurH/(dymax-dymin) * 1.0;
	//std::cout << dScaleX << " " << dScaleY << std::endl;

	painter.translate(-dxmin*dScaleX+16., dymin*dScaleY+dCurH+16.);
	painter.scale(dScaleX, -dScaleY);

	painter.setRenderHint(QPainter::Antialiasing, true);

	// for all plot objects
	for(unsigned int iObj=0; iObj<m_vecObjs.size(); ++iObj)
	{
		const PlotObj& obj = m_vecObjs[iObj];
		QColor col = GetColor(iObj);

		//for all points
		for(unsigned int uiPt=0; uiPt<obj.coord.size(); ++uiPt)
		{
			const QPointF& coord = obj.coord[uiPt];
			const QPointF& err = obj.err[uiPt];

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
	PlotObj obj;

	for(unsigned int i=0; i<iNum; ++i)
	{
		obj.coord.push_back(QPointF(px[i], py[i]));
		obj.err.push_back(QPointF(pxerr?pxerr[i]:0., pyerr?pyerr[i]:0.));
	}

	m_vecObjs.push_back(obj);
}

void Plot::clear()
{
	m_vecObjs.clear();
	this->repaint();
}


#include "plot.moc"
