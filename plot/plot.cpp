/**
 * mieze-tool
 * plotter
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 06-mar-2013
 * @license GPLv3
 */

#include "plot.h"
#include <QtCore/QVector>
#include <QtCore/QPoint>
#include <QtGui/QPainter>
#include <QtGui/QGridLayout>
#include <QtGui/QFrame>
#include <limits>
#include <iostream>

#include "tlibs/string/string.h"
#include "tlibs/log/log.h"
#include "fitter/models/freefit.h"

#define PAD_X 18
#define PAD_Y 18

Plot::Plot(QWidget* pParent, const char* pcTitle) : SubWindowBase(pParent), m_pPixmap(0),
	m_dxmin(0.), m_dxmax(0.), m_dymin(0.), m_dymax(0.), m_bXIsLog(0), m_bYIsLog(0)
#ifdef USE_GPL
	, m_pGPLWidget(0), m_pGPLInst(0)
#endif
{
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setWindowTitle(QString(pcTitle));

#ifdef USE_GPL
	GPL_Init();
#else
	this->setMouseTracking(true);
#endif
}

Plot::~Plot()
{
	clear();
	if(m_pPixmap) delete m_pPixmap;

#ifdef USE_GPL
	GPL_Deinit();
#endif
}


#ifdef USE_GPL

void Plot::GPL_Init()
{
	GPL_Deinit();

	m_pGPLWidget = new QtGnuplotWidget(0,0,this);
	//m_pGPLWidget->setGeometry(0,0,320,240);
	QDataStream strIn;
	m_pGPLWidget->processEvent(GEDesactivate, strIn);
	m_pGPLInst = new QtGnuplotInstance(/*m_pGPLWidget*/);

	//connect(m_pGPLInst, SIGNAL(gnuplotOutput(const QString&)), this, SLOT(Coord_Output(const QString&)));
	connect(m_pGPLWidget, SIGNAL(statusTextChanged(const QString&)), this, SLOT(Coord_Output(const QString&)));

	QGridLayout *pLayout = new QGridLayout(this);

	//m_pGPLWidget->processEvent(GESetWidgetSize, dstrSize);
	//m_pGPLWidget->processEvent(GESetSceneSize, dstrSize);
	m_pGPLWidget->processEvent(GEActivate, strIn);

	m_pGPLWidget->resize(298,218);
	m_pGPLInst->setWidget(m_pGPLWidget);
	pLayout->addWidget(m_pGPLWidget,0,0,1,1);

	/*QSize oldsize(-1,-1);
	QSize newsize(320,240);
	QResizeEvent evt(newsize, oldsize);
	resizeEvent(&evt);*/
}

void Plot::GPL_Deinit()
{
        if(m_pGPLWidget) delete m_pGPLWidget;
        if(m_pGPLInst) delete m_pGPLInst;

	m_pGPLWidget = 0;
	m_pGPLInst = 0;
}

#endif


SubWindowBase* Plot::clone() const
{
	Plot *pPlot = new Plot(parentWidget(), windowTitle().toStdString().c_str());

	pPlot->m_vecObjs = this->m_vecObjs;
	pPlot->m_bXIsLog = this->m_bXIsLog;
	pPlot->m_bYIsLog = this->m_bYIsLog;
	pPlot->m_strXAxis = this->m_strXAxis;
	pPlot->m_strYAxis = this->m_strYAxis;
	pPlot->m_strTitle = this->m_strTitle;

	pPlot->m_dxmin = this->m_dxmin;
	pPlot->m_dxmax = this->m_dxmax;
	pPlot->m_dymin = this->m_dymin;
	pPlot->m_dymax = this->m_dymax;

	pPlot->RefreshPlot();
	return pPlot;
}


QSize Plot::minimumSizeHint() const { return QSize(320,240); }

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
#ifndef USE_GPL
	paint();
#endif
	//qDebug() << "plotter resizeEvent: " << pEvent->size();

/*
	QString strW = QString::number(m_pGPLWidget->size().width());
	QString strH = QString::number(m_pGPLWidget->size().height());
	(*m_pGPLInst) << "set term qt widget \"" << m_pGPLWidget->serverName() << "\" size "
		<< strW << "," << strH << "\n";
	//(*m_pGPLInst) << "replot\n";
*/

	QWidget::resizeEvent(pEvent);
}

void Plot::RefreshPlot()
{
	paint();

#ifndef USE_GPL
	update();
#endif
}

#ifdef USE_GPL

inline std::string get_color_string(const QColor& col)
{
	int r,g,b,a;
	col.getRgb(&r,&g,&b,&a);

	char pcCol[16];
	sprintf(pcCol, "%02X%02X%02X%02X", r,g,b,a);
	std::string strCol = pcCol;

	return strCol;
}

void Plot::GPL_Draw()
{
	if(!m_pGPLInst || !m_pGPLWidget)
		GPL_Init();

	QDataStream strIn;
	m_pGPLWidget->processEvent(GEDesactivate, strIn);

	(*m_pGPLInst) << "set grid\n";
	//pg->SetRanges(m_dxmin, m_dxmax, m_dymin, m_dymax);


	// for all plot objects
	for(unsigned int iObj=0; iObj<m_vecObjs.size(); ++iObj)
	{
		const PlotObj& pltobj = m_vecObjs[iObj];
		const Data1& obj = pltobj.dat;
		QColor col = GetColor(iObj);
		std::string strCol = "{x" + get_color_string(col) + "}";

		unsigned int iLen = obj.GetLength();
		QVector<QPointF> qvecPts;
		//qvecPts.reserve(iLen);

		//for all points
		for(unsigned int uiPt=0; uiPt<iLen; ++uiPt)
		{
			qvecPts << QPointF(obj.GetX(uiPt), obj.GetY(uiPt));
			//x_err.a[uiPt] = obj.GetXErr(uiPt);
			//y_err.a[uiPt] = obj.GetYErr(uiPt);
		}

		std::string strStyle = strCol + "r1";

		if(pltobj.plttype == PLOT_DATA)
		{
			strStyle += "#.";
			//pg->Error(x, y, x_err, y_err, strStyle.c_str(), "legend Data Points");
		}
		else if(pltobj.plttype == PLOT_FKT)
		{
			strStyle += "-";
			//std::string strLegend = "legend " + pltobj.strFkt;
			//pg->Plot(x, y, strStyle.c_str(), strLegend.c_str());
		}

		(*m_pGPLInst) << "plot '-'\n";
		(*m_pGPLInst) << qvecPts;
	}

	//pg->Puts(m_dxmin + (m_dxmax-m_dxmin)/2., m_dymax + (m_dymax-m_dymin)/8., m_strTitle.toStdString().c_str(), "b");

	(*m_pGPLInst) << "set title \"" << m_strTitle << "\"\n";
	(*m_pGPLInst) << "set xlabel \"" << m_strXAxis << "\"\n";
	(*m_pGPLInst) << "set ylabel \"" << m_strYAxis << "\"\n";

	if(m_bXIsLog) (*m_pGPLInst) << "set logscale x\n";
	if(m_bYIsLog) (*m_pGPLInst) << "set logscale y\n";

	m_pGPLWidget->processEvent(GEActivate, strIn);
}
#endif


void Plot::paint()
{
	//std::cout << "Creating plot for " << windowTitle().toStdString() << "." << std::endl;

#ifdef USE_GPL

	GPL_Draw();

#else

	QSize size = this->size();
	if(size.width()==0 || size.height()==0)
		return;

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
	painter.setRenderHint(QPainter::TextAntialiasing, true);

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

				// error bars
				if(err.y()!=0.)
					painter.drawRect(QRectF(coord.x()-0.5/dScaleX, coord.y()-1.*err.y(),
						1.5/dScaleX, 2.*err.y()));
				if(err.x()!=0.)
					painter.drawRect(QRectF(coord.x()-1.*err.x(), coord.y()-0.5/dScaleY,
						2.*err.x(), 1.5/dScaleY));
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

			const std::string& strFkt = pltobj.dat.GetParamMapDyn()["function"];
			const std::string& strName = pltobj.dat.GetParamMapDyn()["function_symbolic"];

			QString strToolTip = "";
			if(strFkt != "")
				strToolTip = ("Function: " + strFkt).c_str();
			else if(strName != "")
				strToolTip = strName.c_str();
			this->setToolTip(strToolTip);
		}
	}

	painter.restore();
	painter.end();

#endif
}

void Plot::paintEvent(QPaintEvent *pEvent)
{
	if(!pEvent || pEvent->rect().width()==0 || pEvent->rect().height()==0)
		return;
	//std::cout << "Repaint event for " << windowTitle().toStdString() << "." << std::endl;

#ifndef USE_GPL

	QPainter painter(this);
	if(!m_pPixmap)
		paint();

	painter.setClipping(1);
	painter.setClipRegion(pEvent->region());
	painter.drawPixmap(0, 0, *m_pPixmap);
	painter.end();
	mouseMoveEvent(0);

#endif

	SubWindowBase::paintEvent(pEvent);
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
		pltobj.dat.GetParamMapDyn()["function_symbolic"] = pcLegend;

	m_vecObjs.push_back(pltobj);

	estimate_minmax();
	RefreshStatusMsgs();

#ifdef USE_GPL
	RefreshPlot();
#endif
}

void Plot::plot_param(const tl::FunctionModel_param<>& fkt, int iObj)
{
	const uint iCnt = 512;

	PlotObj pltobj;
	Data1& dat = pltobj.dat;
	pltobj.plttype = PLOT_FKT;
	StringMap& mapStr = dat.GetParamMapDyn();
	mapStr["function_type"] = fkt.GetModelName();


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

	MergeParamMaps();
	emit(ParamsChanged(m_mapMerged));

#ifdef USE_GPL
	RefreshPlot();
#endif
}

void Plot::plot_fkt(const tl::FitterFuncModel<double>& fkt, int iObj, bool bKeepObj)
{
	const uint iCnt = 512;
	PlotObj* pltobj = 0;

	if(bKeepObj)
	{
		if(iObj<0 || iObj>=m_vecObjs.size())
		{
			tl::log_err("Invalid plot index selected.");
			return;
		}
		pltobj = &m_vecObjs[iObj];
	}
	else
	{
		pltobj = new PlotObj;
	}

	pltobj->plttype = PLOT_FKT;
	Data1& dat = pltobj->dat;

	if(!bKeepObj)
	{
		StringMap& mapStr = dat.GetParamMapDyn();
		mapStr["function_type"] = fkt.GetModelName();
		mapStr["function_symbolic"] = fkt.print(0);
		mapStr["function"] = fkt.print(1);

		std::vector<std::string> vecParamNames = fkt.GetParamNames();
		std::vector<double> vecParamVals = fkt.GetParamValues();
		std::vector<double> vecParamErrs = fkt.GetParamErrors();
		for(unsigned int iParam=0; iParam<vecParamNames.size(); ++iParam)
		{
			std::ostringstream ostrParam;
			ostrParam << vecParamVals[iParam] << " +- " << vecParamErrs[iParam];
			mapStr[std::string("param_") + vecParamNames[iParam]] = ostrParam.str();
		}
	}

	double dxmin=m_dxmin, dxmax=m_dxmax;
	bool bFirstLoop = 1;
	for(unsigned int iDat=0; iDat<GetDataCount(); ++iDat)
	{
		const PlotObj& obj1 = GetData(iDat);
		const Data1& dat1 = obj1.dat;
		if(obj1.plttype == PLOT_DATA)
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

	if(!bKeepObj)
	{
		if(iObj<0)
		{
			clearfkt();
			m_vecObjs.push_back(*pltobj);
		}
		else
		{
			if(int(m_vecObjs.size()) < iObj+1)
				m_vecObjs.resize(iObj+1);
			m_vecObjs[iObj] = *pltobj;
		}
	}

	estimate_minmax();
	RefreshStatusMsgs();

	MergeParamMaps();
	emit(ParamsChanged(m_mapMerged));

	if(!bKeepObj)
		delete pltobj;

#ifdef USE_GPL
	RefreshPlot();
#endif
}



void Plot::replot_fkts()
{
#ifndef NO_PARSER
	for(unsigned int iDat=0; iDat<GetDataCount(); ++iDat)
	{
		const PlotObj& obj1 = GetData(iDat);
		if(obj1.plttype == PLOT_FKT)
		{
			const StringMap& mapStr = obj1.dat.GetParamMapDyn();

			const std::string& strFkt = mapStr["function"];
			//std::cout << "--------\n" << mapStr << "--------\n" << std::endl;

			if(strFkt != "")
			{
				FreeFktModel fkt(strFkt.c_str());
				plot_fkt(fkt, iDat, true);
			}
		}
	}

	//MergeParamMaps();
	//emit(ParamsChanged(m_mapMerged));
#else
	tl::log_err("Parser not linked.");
#endif
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
	update();
}

void Plot::MergeParamMaps()
{
	std::vector<const StringMap*> vecMaps;
	vecMaps.reserve(m_vecObjs.size());

	for(const PlotObj& obj : m_vecObjs)
		vecMaps.push_back(&obj.dat.GetParamMapDyn());

	m_mapMerged.MergeFrom(vecMaps);
}

void Plot::RefreshStatusMsgs()
{
	QString strTitle = windowTitle();
	emit SetStatusMsg(strTitle.toAscii().data(), 0);
	emit SetStatusMsg("", 1);
}



void Plot::Coord_Output(const QString& str)
{
	emit SetStatusMsg(str.toStdString().c_str(), 2);
	RefreshStatusMsgs();
}


#ifndef USE_GPL

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

	Coord_Output(ostr.str().c_str());
}
#endif


void Plot::SetROI(const Roi* pROI, bool bAntiRoi)
{
	for(unsigned int iDat=0; iDat<GetDataCount(); ++iDat)
	{
		PlotObj& obj = GetData(iDat);
		DataInterface* pDat = &obj.dat;
		if(!pDat) continue;

		pDat->SetROI(pROI, bAntiRoi);
	}
}

Roi* Plot::GetROI(bool bAntiRoi)
{
	if(GetDataCount()==0) return 0;
	return &GetData(0).dat.GetRoi(bAntiRoi);
}

bool Plot::LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase)
{
	unsigned int iDatCnt = xml.Query<unsigned int>((strBase + "data_count").c_str(), 0);
	for(unsigned int iDat=0; iDat<iDatCnt; ++iDat)
	{
		PlotObj obj;
		std::ostringstream ostrPlotObj;
		ostrPlotObj << strBase << "plot_obj_" << iDat << "/";
		std::string strObjBase = ostrPlotObj.str();

		obj.LoadXML(xml, blob, strObjBase);
		m_vecObjs.push_back(obj);

		//std::cout << m_vecObjs[m_vecObjs.size()-1].dat.GetParamMapDyn() << std::endl;
	}

	std::string strXLab = xml.QueryString((strBase + "x_label").c_str(), "x");
	std::string strYLab = xml.QueryString((strBase + "y_label").c_str(), "y");
	SetLabels(strXLab.c_str(), strYLab.c_str());

	std::string strTitle = xml.QueryString((strBase + "title").c_str(), "");
	SetTitle(strTitle.c_str());

	std::string strWinTitle = xml.QueryString((strBase + "window_title").c_str(), "");
	setWindowTitle(strWinTitle.c_str());

	SetXIsLog(xml.Query<bool>((strBase + "x_log").c_str(), 0));
	SetYIsLog(xml.Query<bool>((strBase + "y_log").c_str(), 0));

	m_dxmin = xml.Query<double>((strBase + "x_min").c_str(), m_dxmin);
	m_dxmax = xml.Query<double>((strBase + "x_max").c_str(), m_dxmax);
	m_dymin = xml.Query<double>((strBase + "y_min").c_str(), m_dymin);
	m_dymax = xml.Query<double>((strBase + "y_max").c_str(), m_dymax);

	//MergeParamMaps();
	replot_fkts();
	//RefreshStatusMsgs();
	//RefreshPlot();
	return 1;
}

bool Plot::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	ostr << "<type> plot_1d </type>\n";

	ostr << "<x_label> " << m_strXAxis.toStdString() << " </x_label>\n";
	ostr << "<y_label> " << m_strYAxis.toStdString() << " </y_label>\n";

	ostr << "<title> " << m_strTitle.toStdString() << " </title>\n";
	ostr << "<window_title> " << windowTitle().toStdString() << " </window_title>\n";

	ostr << "<x_log> " << m_bXIsLog << " </x_log>\n";
	ostr << "<y_log> " << m_bYIsLog << " </y_log>\n";

	ostr << "<x_min> " << m_dxmin << " </x_min>\n";
	ostr << "<x_max> " << m_dxmax << " </x_max>\n";
	ostr << "<y_min> " << m_dymin << " </y_min>\n";
	ostr << "<y_max> " << m_dymax << " </y_max>\n";

	ostr << "<data_count> " << GetDataCount() << " </data_count>\n";

	for(unsigned int iDat=0; iDat<GetDataCount(); ++iDat)
	{
		ostr << "<plot_obj_" << iDat << ">\n";
		const PlotObj& obj = GetData(iDat);
		obj.SaveXML(ostr, ostrBlob);
		ostr << "</plot_obj_" << iDat << ">\n";
	}

	return 1;
}

bool PlotObj::LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase)
{
	bool bOk = dat.LoadXML(xml, blob, strBase + "data/");

	std::string strType = xml.QueryString((strBase + "type").c_str(), "data");
	if(strType == "data")
		plttype = PLOT_DATA;
	else if(strType == "function")
		plttype = PLOT_FKT;
	else
	{
		plttype = PLOT_DATA;
		tl::log_err("Unknown plot object type: \"", strType, "\".");
	}

	std::string strName = xml.QueryString((strBase + "name").c_str(), "");
	tl::trim(strName);
	if(strName != "")
	{
		tl::log_warn("Deprecated field in data file \"name\" overrides settings in data object.");
		dat.GetParamMapDyn()["function_symbolic"] = strName;
	}

	std::string strFkt = xml.QueryString((strBase + "function").c_str(), "");
	tl::trim(strFkt);
	if(strFkt != "")
	{
		tl::log_warn("Deprecated field in data file \"function\" overrides settings in data object.");
		dat.GetParamMapDyn()["function"] = strFkt;
	}

	return bOk;
}

bool PlotObj::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	bool bSaveDat = 1;

	const std::string& strFkt = dat.GetParamMapDyn()["function"];
	const std::string& strName = dat.GetParamMapDyn()["function_symbolic"];

	std::string strType = "unknown";
	switch(plttype)
	{
		case PLOT_DATA:
			strType = "data";
			break;
		case PLOT_FKT:
			if(strFkt != "")
				bSaveDat = 0;
			strType = "function";
			break;
	}

	ostr << "<type> " << strType << " </type>\n";
	//ostr << "<name> " << strName << " </name>\n";
	//ostr << "<function> " << strFkt << " </function>\n";

	ostr << "<data>\n";
	dat.SaveXML(ostr, ostrBlob, bSaveDat);
	ostr << "</data>\n";
	return 1;
}


std::string Plot::GetLabel(LabelType iWhich) const
{
	if(iWhich == LABEL_X)
		return GetXLabel();
	else if(iWhich == LABEL_Y)
		return GetYLabel();
	else if(iWhich == LABEL_TITLE)
		return GetTitle();

	return "";
}
void Plot::SetLabel(LabelType iWhich, const char* pcLab)
{
	if(iWhich == LABEL_X)
		m_strXAxis = pcLab;
	else if(iWhich == LABEL_Y)
		m_strYAxis = pcLab;
	else if(iWhich == LABEL_TITLE)
		SetTitle(pcLab);
	else
		return;

	RefreshPlot();
}


void Plot::SaveImageAs() const
{
#ifdef USE_GPL
	if(m_pGPLWidget)
		m_pGPLWidget->exportToPdf();
#endif
}


#include "plot.moc"
