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

#define PAD_X 18
#define PAD_Y 18

Plot::Plot(QWidget* pParent, const char* pcTitle) : SubWindowBase(pParent), m_pPixmap(0),
									m_dxmin(0.), m_dxmax(0.), m_dymin(0.), m_dymax(0.),
									m_bXIsLog(0), m_bYIsLog(0)
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(QString(pcTitle));
	this->setMouseTracking(true);
}

Plot::~Plot()
{
	clear();
	if(m_pPixmap) delete m_pPixmap;
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
	for(const PlotObj& pltobj : m_vecObjs)
	{
		const Data1& obj = pltobj.dat;
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

	const double dPadX = (m_dxmax-m_dxmin) / 12.;
	const double dPadY = (m_dymax-m_dymin) / 12.;
	m_dxmin -= dPadX;
	m_dxmax += dPadX;
	m_dymin -= dPadY;
	m_dymax += dPadY;
}

QColor Plot::GetColor(unsigned int iPlotObj) const
{
	static const std::vector<QColor> cols =
	{
			QColor::fromRgbF(0., 0., 1., 1.),
			QColor::fromRgbF(0., 0.5, 0., 1.),
			QColor::fromRgbF(1., 0., 0., 1.),
			QColor::fromRgbF(0., 0., 0., 1.)
	};

	return cols[iPlotObj % cols.size()];
}

void Plot::resizeEvent(QResizeEvent *pEvent)
{
	paint();
}

void Plot::RefreshPaint()
{
	paint();
	repaint();
}

void Plot::paint()
{
	QSize size = this->size();

	if(m_pPixmap)
	{
		if(size.width()!=m_pPixmap->width() || size.height()!=m_pPixmap->height())
		{
			delete m_pPixmap;
			m_pPixmap=0;
		}
	}
	if(!m_pPixmap)
		m_pPixmap = new QPixmap(size);

	m_pPixmap->fill(Qt::white);

	double dStartX = PAD_X;
	double dStartY = PAD_Y;
	double dCurH = size.height() - PAD_Y*2;
	double dCurW = size.width() - PAD_X*2;


	QPainter painter(m_pPixmap);
	painter.save();
	painter.setRenderHint(QPainter::Antialiasing, true);

	QFont fontMain("Nimbus Sans L", 10);
	painter.setFont(fontMain);
	painter.drawText(QRect(0, 0, size.width(), PAD_Y), Qt::AlignCenter, m_strTitle);
	painter.drawText(QRect(0, size.height()-PAD_Y+2, size.width(), PAD_Y-2), Qt::AlignCenter, m_strXAxis);

	painter.save();
	painter.translate(QPoint(PAD_X-2,0));
	painter.rotate(90.);
	painter.drawText(QRect(0, 0, size.height(), PAD_X-4), Qt::AlignCenter, m_strYAxis);
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

	QFont fontAxis("DejaVu Sans", 7);
	fontAxis.setBold(1);
	painter.setFont(fontAxis);

	const unsigned int iNumGridLines = 8;
	for(unsigned int iGrid=1; iGrid<iNumGridLines+1; ++iGrid)
	{
		double dLineX = dStartX+iGrid*dCurW/double(iNumGridLines+1);
		double dLineY = dStartY+iGrid*dCurH/double(iNumGridLines+1);

		painter.drawLine(QLineF(dStartX, dLineY, dStartX+dCurW, dLineY));
		painter.drawLine(QLineF(dLineX, dStartY, dLineX, dStartY+dCurH));

		if((iGrid-1)%2)
		{
			double dX, dY;
			MapToCoordSys(dLineX, dLineY, dX, dY);
			std::ostringstream ostrX, ostrY;
			ostrX.precision(1); ostrY.precision(1);
			ostrX << std::scientific << dX; ostrY << std::scientific << dY;

			painter.drawText(QPoint(dLineX-24, dStartY+dCurH-2), ostrX.str().c_str());
			painter.drawText(QPoint(dStartX+2, dLineY-1), ostrY.str().c_str());
		}
	}
	painter.setFont(fontMain);


	double dScaleX = dCurW/(m_dxmax-m_dxmin) * 1.0;
	double dScaleY = dCurH/(m_dymax-m_dymin) * 1.0;
	//std::cout << dScaleX << " " << dScaleY << std::endl;

	painter.translate(-m_dxmin*dScaleX+PAD_X, m_dymin*dScaleY+dCurH+PAD_Y);
	painter.scale(dScaleX, -dScaleY);

	// for all plot objects
	for(unsigned int iObj=0; iObj<m_vecObjs.size(); ++iObj)
	{
		const PlotObj& pltobj = m_vecObjs[iObj];
		const Data1& obj = pltobj.dat;
		QColor col = GetColor(iObj);

		if(pltobj.plttype == PLOT_DATA)
		{
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
		else if(pltobj.plttype == PLOT_FKT)
		{
			painter.setPen(/*QColor::fromRgb(0,0,255,255)*/col);

			QVector<QPointF> vecCoords;
			vecCoords.reserve(obj.GetLength()*2);
			for(unsigned int uiPt=0; uiPt<obj.GetLength()-1; ++uiPt)
			{
				const QPointF coord(obj.GetX(uiPt), obj.GetY(uiPt));
				const QPointF coordNext(obj.GetX(uiPt+1), obj.GetY(uiPt+1));

				vecCoords.push_back(coord);
				vecCoords.push_back(coordNext);
			}
			painter.drawLines(vecCoords);
		}
	}

	painter.restore();
}

void Plot::paintEvent (QPaintEvent *pEvent)
{
	if(!m_pPixmap)
		paint();

	QPainter painter(this);
	painter.drawPixmap(0,0,*m_pPixmap);
	mouseMoveEvent(0);
}

void Plot::plot(unsigned int iNum, const double *px, const double *py, const double *pyerr, const double *pxerr,
							PlotType plttype, const char* pcLegend)
{
	plot(Data1(iNum, px, py, pyerr, pxerr), plttype, pcLegend);
}

void Plot::plot(const Data1& dat, PlotType plttype, const char* pcLegend)
{
	PlotObj pltobj;
	pltobj.dat = dat;
	pltobj.plttype = plttype;
	if(pcLegend)
		pltobj.strName = std::string(pcLegend);

	m_vecObjs.push_back(pltobj);

	estimate_minmax();
	RefreshStatusMsgs();
}

void Plot::plot_param(const FunctionModel_param& fkt, int iObj)
{
	const uint iCnt = 512;

	PlotObj pltobj;
	pltobj.plttype = PLOT_FKT;
	pltobj.strName = "interpolation";
	Data1& dat = pltobj.dat;

	dat.SetLength(iCnt);
	for(uint iX=0; iX<iCnt; ++iX)
	{
		double dT = double(iX)/double(iCnt-1);
		boost::numeric::ublas::vector<double> vec = fkt(dT);

		dat.SetX(iX, vec[0]);
		dat.SetY(iX, vec[1]);
	}

	if(iObj<0)
	{
		clearfkt();
		m_vecObjs.push_back(pltobj);
	}
	else
	{
		if(m_vecObjs.size() < iObj+1)
			m_vecObjs.resize(iObj+1);
		m_vecObjs[iObj] = pltobj;
	}

	estimate_minmax();
	RefreshStatusMsgs();
}

void Plot::plot_fkt(const FunctionModel& fkt, int iObj)
{
	const uint iCnt = 512;

	PlotObj pltobj;
	pltobj.plttype = PLOT_FKT;
	pltobj.strName = fkt.print(0);
	Data1& dat = pltobj.dat;


	double dxmin=m_dxmin, dxmax=m_dxmax;
	bool bFirstLoop = 1;
	for(unsigned int iDat=0; iDat<GetDataCount(); ++iDat)
	{
		const PlotObj& obj1 = GetData(iDat);
		const Data1& dat1 = obj1.dat;
		if(dat1.GetType() == DATA_1D)
		{
			double dxmin1, dxmax1;
			dat1.GetXMinMax(dxmin1, dxmax1);

			if(bFirstLoop)
			{
				dxmin = dxmin1;
				dxmax = dxmax1;
				bFirstLoop = 0;
			}
			else
			{
				dxmin = std::min(dxmin, dxmin1);
				dxmax = std::max(dxmax, dxmax1);
			}
		}
	}


	dat.SetLength(iCnt);
	for(uint iX=0; iX<iCnt; ++iX)
	{
		double dX = dxmin + (dxmax-dxmin)*double(iX)/double(iCnt-1);

		dat.SetX(iX, dX);
		dat.SetY(iX, fkt(dX));
	}

	if(iObj<0)
	{
		clearfkt();
		m_vecObjs.push_back(pltobj);
	}
	else
	{
		if(m_vecObjs.size() < iObj+1)
			m_vecObjs.resize(iObj+1);
		m_vecObjs[iObj] = pltobj;
	}

	estimate_minmax();
	RefreshStatusMsgs();
}

void Plot::clearfkt()
{
	for(uint i=0; i<m_vecObjs.size(); ++i)
	{
		if(m_vecObjs[i].plttype == PLOT_FKT)
		{
			m_vecObjs.erase(m_vecObjs.begin()+i);
			--i;
		}
	}
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

void Plot::MapToCoordSys(double dPixelX, double dPixelY, double &dX, double &dY, bool *pbInside)
{
	const QSize size = this->size();

	const double dw = m_dxmax-m_dxmin;
	const double dh = m_dymax-m_dymin;

	// map between [0..1]
	dX = double(dPixelX-PAD_X) / double(size.width()-2*PAD_X);
	dY = 1. - double(dPixelY-PAD_Y) / double(size.height()-2*PAD_Y);

	bool bInside = 0;
	if(dX>=0. && dX<=1. && dY>=0. && dY<=1.)
		bInside = 1;
	else
		bInside = 0;

	if(pbInside) *pbInside = bInside;

	if(m_bXIsLog)
	{
		double dXMin = log10(m_dxmin);
		double dXMax = log10(m_dxmax);
		dX = pow(10., dXMin + dX*(dXMax-dXMin));
	}
	else
	{
		dX = m_dxmin + dX*dw;
		if(dX < m_dxmin) dX = m_dxmin;
		if(dX > m_dxmax) dX = m_dxmax;
	}

	if(m_bYIsLog)
	{
		double dYMin = log10(m_dymin);
		double dYMax = log10(m_dymax);
		dY = pow(10., dYMin + dY*(dYMax-dYMin));
	}
	else
	{
		dY = m_dymin + dY*dh;
		if(dY < m_dymin) dY = m_dymin;
		if(dY > m_dymax) dY = m_dymax;
	}
}

void Plot::mouseMoveEvent(QMouseEvent* pEvent)
{
	QPoint curPt;
	const QPoint* pt;

	if(pEvent)
	{
		pt = &pEvent->pos();
	}
	else
	{
		curPt = mapFromGlobal(QCursor::pos());
		pt = &curPt;
	}

	double dX, dY;
	bool bInside;
	MapToCoordSys(pt->x(), pt->y(), dX, dY, &bInside);

	if(bInside)
		this->setCursor(Qt::CrossCursor);
	else
	{
		if(pEvent==0)  // here, we may not even be in the correct plot window if called externally
			return;

		this->setCursor(Qt::ArrowCursor);
	}

	std::ostringstream ostr;
	ostr << "(" << dX << ", " << dY << ")";

	emit SetStatusMsg(ostr.str().c_str(), 2);
	RefreshStatusMsgs();
}

void Plot::SetGlobalROI(const Roi* pROI, const bool* pbROIActive)
{
	for(unsigned int iDat=0; iDat<GetDataCount(); ++iDat)
	{
		PlotObj& obj = GetData(iDat);
		DataInterface* pDat = &obj.dat;
		if(!pDat) return;

		pDat->SetGlobalROI(pROI, pbROIActive);
	}
}

#include "plot.moc"
