/*
 * mieze-tool
 * plotter
 * @author tweber
 * @date 06-mar-2013
 */

#include "plot.h"
#include <QtGui/QPainter>

Plot::Plot(QWidget* pParent) : QWidget(pParent)
{
	this->setWindowTitle("Plot");
}

Plot::~Plot() {}

QSize Plot::minimumSizeHint() const
{
	return QSize(320,240);
}

void Plot::estimate_minmax(double& dxmin, double& dxmax, double& dymin, double& dymax)
{
	dxmin = dymin = 10000.;
	dxmax = dymax = -10000.;

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

void Plot::paintEvent (QPaintEvent *pEvent)
{
	QPainter painter(this);
	painter.save();

	double dxmin, dxmax, dymin, dymax;
	estimate_minmax(dxmin, dxmax, dymin, dymax);

	QSize size = this->size();
	double dCurH = size.height();
	double dCurW = size.width();

	double dScaleX = dCurW/(dxmax-dxmin) * 0.9;
	double dScaleY = dCurH/(dymax-dymin) * 0.9;

	painter.translate(-dxmin*dScaleX, -dymin*dScaleY);
	painter.scale(dScaleX, dScaleY);

	painter.setRenderHint(QPainter::Antialiasing, true);
	QPen pen = QColor::fromRgb(0,0,0,255);
	pen.setWidthF(1.);
	painter.setPen(pen);

	// for all plot objects
	for(const PlotObj& obj : m_vecObjs)
	{
		//for all points
		for(unsigned int uiPt=0; uiPt<obj.coord.size(); ++uiPt)
		{
			const QPointF& coord = obj.coord[uiPt];
			const QPointF& err = obj.err[uiPt];

			painter.drawPoint(coord);
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
