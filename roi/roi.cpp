/*
 * mieze-tool
 * roi stuff
 * @author tweber
 * @date 2011, 23-apr-2013
 */

#include <math.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <limits>

#include "roi.h"
#include "pnpoly.h"
#include "../helper/math.h"
#include "../helper/xml.h"
#include "../data/data.h"

//------------------------------------------------------------------------------
// Bounding Rectangle

void BoundingRect::SetInvalidBounds()
{
	bottomleft[0] = std::numeric_limits<double>::max();
	bottomleft[1] = std::numeric_limits<double>::max();
	topright[0] = std::numeric_limits<double>::min();
	topright[1] = std::numeric_limits<double>::min();
}

void BoundingRect::AddVertex(const ublas::vector<double>& vertex)
{
	bottomleft[0] = std::min(bottomleft[0], vertex[0]);
	bottomleft[1] = std::min(bottomleft[1], vertex[1]);

	topright[0] = std::max(topright[0], vertex[0]);
	topright[1] = std::max(topright[1], vertex[1]);
}


//------------------------------------------------------------------------------
// RoiElement

RoiElement::RoiElement()
{}

void RoiElement::CalculateBoundingRect()
{
	BoundingRect& rect = m_boundingrect;
	rect.SetInvalidBounds();

	for(unsigned int i=0; i<GetVertexCount(); ++i)
	{
		ublas::vector<double> vert = GetVertex(i);
		rect.AddVertex(vert);
	}
}

const BoundingRect& RoiElement::GetBoundingRect() const
{
	return m_boundingrect;
}

RoiElement& RoiElement::operator=(const RoiElement& elem)
{
	this->m_boundingrect = elem.m_boundingrect;
	return *this;
}

bool RoiElement::IsInBoundingRect(double dX, double dY) const
{
	const BoundingRect& rect = m_boundingrect;

	if(dX >= rect.bottomleft[0] && dX < rect.topright[0] &&
	   dY >= rect.bottomleft[1] && dY < rect.topright[1])
	   return true;

	return false;
}

/// \todo directly calculate fraction inside roi element
///       in the respective child classes; this general
///       method is horribly inefficient
double RoiElement::HowMuchInside(int iX, int iY) const
{
	const double dInc = 0.2;
	const double dTotal = (1./dInc) * (1./dInc);

	double dFraction = 0.;

	for(double dY=iY; dY<iY+1; dY+=dInc)
		for(double dX=iX; dX<iX+1; dX+=dInc)
		{
			if(IsInside(dX, dY))
				dFraction += 1./dTotal;
		}

	return dFraction;
}

//------------------------------------------------------------------------------
// rect

RoiRect::RoiRect(const ublas::vector<double>& bottomleft,
				 const ublas::vector<double>& topright, double dAngle)
		: m_bottomleft(bottomleft), m_topright(topright), m_dAngle(dAngle)
{
	if(m_bottomleft[0] > m_topright[0])
		std::swap(m_bottomleft[0], m_topright[0]);
	if(m_bottomleft[1] > m_topright[1])
		std::swap(m_bottomleft[1], m_topright[1]);

	CalculateBoundingRect();
}

RoiRect::RoiRect(double dX1, double dY1, double dX2, double dY2, double dAngle)
{
	ublas::vector<double> vec1(2);
	ublas::vector<double> vec2(2);

	vec1[0] = dX1;
	vec1[1] = dY1;
	vec2[0] = dX2;
	vec2[1] = dY2;

	*this = RoiRect(vec1, vec2, dAngle);
}

RoiRect::RoiRect() : m_bottomleft(2), m_topright(2), m_dAngle(0.)
{
	m_bottomleft[0] = m_bottomleft[1] = 0.;
	m_topright[0] = m_topright[1] = 0.;
}

RoiRect::RoiRect(const RoiRect& rect)
{
	*this = rect;
}

bool RoiRect::IsInside(double dX, double dY) const
{
	ublas::vector<double> vecCenter = m_bottomleft + (m_topright-m_bottomleft)*.5;
	ublas::matrix<double> matRot_inv = ::rotation_matrix_2d<double>(-m_dAngle/180.*M_PI);

	ublas::vector<double> vecPoint(2);
	vecPoint[0] = dX;
	vecPoint[1] = dY;
	vecPoint = ublas::prod(matRot_inv, (vecPoint-vecCenter)) + vecCenter;

	if(vecPoint[0]>=m_bottomleft[0] && vecPoint[0]<m_topright[0] &&
	   vecPoint[1]>=m_bottomleft[1] && vecPoint[1]<m_topright[1])
	   return true;
	return false;
}

std::string RoiRect::GetName() const { return "rectangle"; }

int RoiRect::GetParamCount() const
{
	return 5;
}

std::string RoiRect::GetParamName(int iParam) const
{
	std::string strRet;

	switch(iParam)
	{
		case 0: strRet="bottomleft_x"; break;
		case 1: strRet="bottomleft_y"; break;
		case 2: strRet="topright_x"; break;
		case 3: strRet="topright_y"; break;
		case 4: strRet="angle"; break;
		default: strRet="unknown"; break;
	}

	return strRet;
}

double RoiRect::GetParam(int iParam) const
{
	switch(iParam)
	{
		case 0: return m_bottomleft[0];
		case 1: return m_bottomleft[1];
		case 2: return m_topright[0];
		case 3: return m_topright[1];
		case 4: return m_dAngle;
	}
	return 0.;
}

void RoiRect::SetParam(int iParam, double dVal)
{
	switch(iParam)
	{
		case 0: m_bottomleft[0] = dVal; break;
		case 1: m_bottomleft[1] = dVal; break;
		case 2: m_topright[0] = dVal; break;
		case 3: m_topright[1] = dVal; break;
		case 4: m_dAngle = dVal; break;
	}

	CalculateBoundingRect();
}

unsigned int RoiRect::GetVertexCount() const
{
	return 4;
}

ublas::vector<double> RoiRect::GetVertex(unsigned int i) const
{
	ublas::vector<double> topleft(2), bottomright(2);
	topleft[0] = m_bottomleft[0];
	topleft[1] = m_topright[1];
	bottomright[0] = m_topright[0];
	bottomright[1] = m_bottomleft[1];

	const ublas::vector<double>& bottomleft = m_bottomleft;
	const ublas::vector<double>& topright = m_topright;

	ublas::vector<double> vecRet(2);
	ublas::vector<double> vecNull(2);
	vecNull[0] = vecNull[1] = 0.;

	switch(i)
	{
		case 0: vecRet = bottomleft; break;
		case 1: vecRet = topleft; break;
		case 2: vecRet = topright; break;
		case 3: vecRet = bottomright; break;
		default: return vecNull;
	}

	ublas::vector<double> vecCenter = bottomleft + (topright-bottomleft)*.5;
	ublas::matrix<double> matRot = ::rotation_matrix_2d<double>(m_dAngle / 180. * M_PI);

	vecRet = ublas::prod(matRot, (vecRet-vecCenter)) + vecCenter;
	return vecRet;
}

RoiRect& RoiRect::operator=(const RoiRect& elem)
{
	RoiElement::operator=(elem);

	this->m_bottomleft = elem.m_bottomleft;
	this->m_topright = elem.m_topright;
	this->m_dAngle = elem.m_dAngle;

	return *this;
}

RoiElement* RoiRect::copy() const
{
	RoiRect* pElem = new RoiRect;
	pElem->operator=(*this);
	return pElem;
}




//------------------------------------------------------------------------------
// circle

RoiCircle::RoiCircle(const ublas::vector<double>& vecCenter, double dRadius)
		 : m_vecCenter(vecCenter), m_dRadius(dRadius)
{
	if(m_dRadius < 0.) m_dRadius = -m_dRadius;

	CalculateBoundingRect();
}

RoiCircle::RoiCircle() : m_vecCenter(2), m_dRadius(0.)
{
	m_vecCenter[0] = m_vecCenter[1] = 0.;
}

RoiCircle::RoiCircle(const RoiCircle& elem)
{
	*this = elem;
}

bool RoiCircle::IsInside(double dX, double dY) const
{
	double dX_0 = dX - m_vecCenter[0];
	double dY_0 = dY - m_vecCenter[1];

	double dLen = sqrt(dX_0*dX_0 + dY_0*dY_0);
	return dLen <= m_dRadius;
}

std::string RoiCircle::GetName() const { return "circle"; }

int RoiCircle::GetParamCount() const
{
	// 0: m_dCenter[0]
	// 1: m_dCenter[1]
	// 2: m_dRadius
	return 3;
}

std::string RoiCircle::GetParamName(int iParam) const
{
	std::string strRet;

	switch(iParam)
	{
		case 0: strRet="center_x"; break;
		case 1: strRet="center_y"; break;
		case 2: strRet="radius"; break;
		default: strRet="unknown"; break;
	}

	return strRet;
}

double RoiCircle::GetParam(int iParam) const
{
	switch(iParam)
	{
		case 0: return m_vecCenter[0];
		case 1: return m_vecCenter[1];
		case 2: return m_dRadius;
	}
	return 0.;
}

void RoiCircle::SetParam(int iParam, double dVal)
{
	switch(iParam)
	{
		case 0: m_vecCenter[0] = dVal; break;
		case 1: m_vecCenter[1] = dVal; break;
		case 2: m_dRadius = dVal; break;
	}

	CalculateBoundingRect();
}

unsigned int RoiCircle::GetVertexCount() const
{
	return CIRCLE_VERTICES;
}

ublas::vector<double> RoiCircle::GetVertex(unsigned int i) const
{
	double dAngle = 2*M_PI * double(i)/double(GetVertexCount()-1);

	ublas::vector<double> vecRet(2);
	vecRet[0] = m_dRadius*cos(dAngle);
	vecRet[1] = m_dRadius*sin(dAngle);
	vecRet = vecRet + m_vecCenter;

	return vecRet;
}

void RoiCircle::CalculateBoundingRect()
{
	BoundingRect& rect = m_boundingrect;

	rect.bottomleft = m_vecCenter;
	rect.bottomleft[0] -= m_dRadius;
	rect.bottomleft[1] -= m_dRadius;

	rect.topright = m_vecCenter;
	rect.topright[0] += m_dRadius;
	rect.topright[1] += m_dRadius;
}

RoiCircle& RoiCircle::operator=(const RoiCircle& elem)
{
	RoiElement::operator=(elem);

	this->m_vecCenter = elem.m_vecCenter;
	this->m_dRadius = elem.m_dRadius;

	return *this;
}

RoiElement* RoiCircle::copy() const
{
	RoiCircle *pElem = new RoiCircle;
	pElem->operator=(*this);
	return pElem;
}



//------------------------------------------------------------------------------
// ellipse

RoiEllipse::RoiEllipse(const ublas::vector<double>& vecCenter,
					   double dRadiusX, double dRadiusY)
		 : m_vecCenter(vecCenter), m_dRadiusX(dRadiusX), m_dRadiusY(dRadiusY)
{
	if(m_dRadiusX < 0.) m_dRadiusX = -m_dRadiusX;
	if(m_dRadiusY < 0.) m_dRadiusY = -m_dRadiusY;

	CalculateBoundingRect();
}

RoiEllipse::RoiEllipse() : m_vecCenter(2), m_dRadiusX(0.), m_dRadiusY(0.)
{
	m_vecCenter[0] = m_vecCenter[1] = 0.;
}

RoiEllipse::RoiEllipse(const RoiEllipse& elem)
{
	*this = elem;
}

bool RoiEllipse::IsInside(double dX, double dY) const
{
	ublas::vector<double> vecVertex(2);
	vecVertex[0] = dX;
	vecVertex[1] = dY;

	double dX0 = dX - m_vecCenter[0];
	double dY0 = dY - m_vecCenter[1];

	bool bInside = ((dX0*dX0/(m_dRadiusX*m_dRadiusX) +
				     dY0*dY0/(m_dRadiusY*m_dRadiusY)) <= 1.);

	return bInside;
}

std::string RoiEllipse::GetName() const { return "ellipse"; }

int RoiEllipse::GetParamCount() const
{
	// 0: m_dCenter[0]
	// 1: m_dCenter[1]
	// 2: m_dRadiusX
	// 3: m_dRadiusY
	return 4;
}

std::string RoiEllipse::GetParamName(int iParam) const
{
	std::string strRet;

	switch(iParam)
	{
		case 0: strRet="center_x"; break;
		case 1: strRet="center_y"; break;
		case 2: strRet="radius_x"; break;
		case 3: strRet="radius_y"; break;
		default: strRet="unknown"; break;
	}

	return strRet;
}

double RoiEllipse::GetParam(int iParam) const
{
	switch(iParam)
	{
		case 0: return m_vecCenter[0];
		case 1: return m_vecCenter[1];
		case 2: return m_dRadiusX;
		case 3: return m_dRadiusY;
	}
	return 0.;
}

void RoiEllipse::SetParam(int iParam, double dVal)
{
	switch(iParam)
	{
		case 0: m_vecCenter[0] = dVal; break;
		case 1: m_vecCenter[1] = dVal; break;
		case 2: m_dRadiusX = dVal; break;
		case 3: m_dRadiusY = dVal; break;
	}

	CalculateBoundingRect();
}

unsigned int RoiEllipse::GetVertexCount() const
{
	return CIRCLE_VERTICES;
}

ublas::vector<double> RoiEllipse::GetVertex(unsigned int i) const
{
	double dAngle = 2*M_PI * double(i)/double(GetVertexCount()-1);

	ublas::vector<double> vecRet(2);
	vecRet[0] = m_dRadiusX*cos(dAngle);
	vecRet[1] = m_dRadiusY*sin(dAngle);
	vecRet = vecRet + m_vecCenter;

	return vecRet;
}

void RoiEllipse::CalculateBoundingRect()
{
	BoundingRect &rect = m_boundingrect;

	rect.bottomleft = m_vecCenter;
	rect.bottomleft[0] -= m_dRadiusX;
	rect.bottomleft[1] -= m_dRadiusY;

	rect.topright = m_vecCenter;
	rect.topright[0] += m_dRadiusX;
	rect.topright[1] += m_dRadiusY;
}

RoiEllipse& RoiEllipse::operator=(const RoiEllipse& elem)
{
	RoiElement::operator=(elem);

	this->m_vecCenter = elem.m_vecCenter;
	this->m_dRadiusX = elem.m_dRadiusX;
	this->m_dRadiusY = elem.m_dRadiusY;

	return *this;
}

RoiElement* RoiEllipse::copy() const
{
	RoiEllipse *pElem = new RoiEllipse;
	pElem->operator=(*this);
	return pElem;
}




//------------------------------------------------------------------------------
// circle ring

RoiCircleRing::RoiCircleRing(const ublas::vector<double>& vecCenter,
								   double dInnerRadius, double dOuterRadius)
				: m_vecCenter(vecCenter),
				  m_dInnerRadius(dInnerRadius), m_dOuterRadius(dOuterRadius)
{
	if(m_dInnerRadius < 0.) m_dInnerRadius = -m_dInnerRadius;
	if(m_dOuterRadius < 0.) m_dOuterRadius = -m_dOuterRadius;

	if(m_dOuterRadius < m_dInnerRadius)
		std::swap(m_dOuterRadius, m_dInnerRadius);

	CalculateBoundingRect();
}

RoiCircleRing::RoiCircleRing()
				: m_vecCenter(2), m_dInnerRadius(0.), m_dOuterRadius(0.)
{
	m_vecCenter[0] = m_vecCenter[1] = 0.;
}

RoiCircleRing::RoiCircleRing(const RoiCircleRing& elem)
{
	*this = elem;
}

bool RoiCircleRing::IsInside(double dX, double dY) const
{
	ublas::vector<double> vecVertex(2);
	vecVertex[0] = dX;
	vecVertex[1] = dY;

	double dX0 = dX - m_vecCenter[0];
	double dY0 = dY - m_vecCenter[1];

	// outside inner radius?
	bool bOutsideInnerRad = ((dX0*dX0/(m_dInnerRadius*m_dInnerRadius) +
						      dY0*dY0/(m_dInnerRadius*m_dInnerRadius)) >= 1.);
	if(!bOutsideInnerRad)
		return false;

	// inside outer radius?
	bool bInsideOuterRad = ((dX0*dX0/(m_dOuterRadius*m_dOuterRadius) +
						     dY0*dY0/(m_dOuterRadius*m_dOuterRadius)) < 1.);
	if(!bInsideOuterRad)
		return false;

	return true;
}

std::string RoiCircleRing::GetName() const
{
	return "circle_ring";
}

int RoiCircleRing::GetParamCount() const
{
	return 4;
}

std::string RoiCircleRing::GetParamName(int iParam) const
{
	std::string strRet;

	switch(iParam)
	{
		case 0: strRet="center_x"; break;
		case 1: strRet="center_y"; break;
		case 2: strRet="inner_radius"; break;
		case 3: strRet="outer_radius"; break;
		default: strRet="unknown"; break;
	}

	return strRet;
}

double RoiCircleRing::GetParam(int iParam) const
{
	switch(iParam)
	{
		case 0: return m_vecCenter[0];
		case 1: return m_vecCenter[1];
		case 2: return m_dInnerRadius;
		case 3: return m_dOuterRadius;
	}
	return 0.;
}

void RoiCircleRing::SetParam(int iParam, double dVal)
{
	switch(iParam)
	{
		case 0: m_vecCenter[0] = dVal; break;
		case 1: m_vecCenter[1] = dVal; break;
		case 2: m_dInnerRadius = dVal; break;
		case 3: m_dOuterRadius = dVal; break;
	}

	CalculateBoundingRect();
}

unsigned int RoiCircleRing::GetVertexCount() const
{
	return CIRCLE_VERTICES;
}

ublas::vector<double> RoiCircleRing::GetVertex(unsigned int i) const
{
	ublas::vector<double> vecRet(2);
	const unsigned int iVerticesPerArc = (GetVertexCount())/2;
	const double dAngleRange = 2. * M_PI;

	// inner circle
	if(i<iVerticesPerArc)
	{
		double dAngle = dAngleRange*double(i)/double(iVerticesPerArc-1);

		vecRet[0] = m_dInnerRadius*cos(dAngle);
		vecRet[1] = m_dInnerRadius*sin(dAngle);

		vecRet = vecRet + m_vecCenter;
	}
	// outer circle
	else if(i>=iVerticesPerArc)
	{
		const int iIdx = 2*iVerticesPerArc - i - 1;

		double dAngle = dAngleRange*double(iIdx)/double(iVerticesPerArc-1);

		vecRet[0] = m_dOuterRadius*cos(dAngle);
		vecRet[1] = m_dOuterRadius*sin(dAngle);

		vecRet = vecRet + m_vecCenter;
	}

	return vecRet;
}

void RoiCircleRing::CalculateBoundingRect()
{
	BoundingRect& rect = m_boundingrect;

	rect.bottomleft = m_vecCenter;
	rect.bottomleft[0] -= m_dOuterRadius;
	rect.bottomleft[1] -= m_dOuterRadius;

	rect.topright = m_vecCenter;
	rect.topright[0] += m_dOuterRadius;
	rect.topright[1] += m_dOuterRadius;
}

RoiCircleRing& RoiCircleRing::operator=(const RoiCircleRing& elem)
{
	RoiElement::operator=(elem);

	this->m_vecCenter = elem.m_vecCenter;
	this->m_dInnerRadius = elem.m_dInnerRadius;
	this->m_dOuterRadius = elem.m_dOuterRadius;

	return *this;
}

RoiElement* RoiCircleRing::copy() const
{
	RoiCircleRing *pElem = new RoiCircleRing;
	pElem->operator=(*this);
	return pElem;
}



//------------------------------------------------------------------------------
// circle segment

RoiCircleSegment::RoiCircleSegment(const ublas::vector<double>& vecCenter,
								   double dInnerRadius, double dOuterRadius,
								   double dBeginAngle, double dEndAngle)
				: RoiCircleRing(vecCenter, dInnerRadius, dOuterRadius),
				  m_dBeginAngle(dBeginAngle), m_dEndAngle(dEndAngle)
{
	CalculateBoundingRect();
}

RoiCircleSegment::RoiCircleSegment()
				: RoiCircleRing(),
				  m_dBeginAngle(0.), m_dEndAngle(0.)
{}

RoiCircleSegment::RoiCircleSegment(const RoiCircleSegment& elem)
				: RoiCircleRing()
{
	*this = elem;
}

bool RoiCircleSegment::IsInside(double dX, double dY) const
{
	if(!RoiCircleRing::IsInside(dX, dY))
		return false;

	// test if point is between the two angles?

	ublas::vector<double> vecVertex(2);
	vecVertex[0] = dX;
	vecVertex[1] = dY;
	vecVertex = vecVertex - m_vecCenter;
	//vecVertex.normalize();

	const double dAngle1Rad = m_dBeginAngle / 180. * M_PI;
	const double dAngle2Rad = m_dEndAngle / 180. * M_PI;

	ublas::vector<double> vecNormal1(2);
	vecNormal1[0] = -sin(dAngle1Rad);
	vecNormal1[1] = cos(dAngle1Rad);

	ublas::vector<double> vecNormal2(2);
	vecNormal2[0] = -sin(dAngle2Rad);
	vecNormal2[1] = cos(dAngle2Rad);

	bool bBetweenAngles = false;
	double dDot1 = ublas::inner_prod(vecVertex, vecNormal1);
	double dDot2 = ublas::inner_prod(vecVertex, vecNormal2);

	if(dDot1>=0. && dDot2<=0.)
		bBetweenAngles=true;

	if(!bBetweenAngles)
		return false;

	return true;
}

std::string RoiCircleSegment::GetName() const
{
	return "circle_segment";
}

int RoiCircleSegment::GetParamCount() const
{
	return 6;
}

std::string RoiCircleSegment::GetParamName(int iParam) const
{
	std::string strRet;

	if(iParam>=0 && iParam<4)
		return RoiCircleRing::GetParamName(iParam);

	switch(iParam)
	{
		case 4: strRet="begin_angle"; break;
		case 5: strRet="end_angle"; break;
		default: strRet="unknown"; break;
	}

	return strRet;
}

double RoiCircleSegment::GetParam(int iParam) const
{
	if(iParam>=0 && iParam<4)
		return RoiCircleRing::GetParam(iParam);

	switch(iParam)
	{
		case 4: return m_dBeginAngle;
		case 5: return m_dEndAngle;
	}
	return 0.;
}

void RoiCircleSegment::SetParam(int iParam, double dVal)
{
	if(iParam>=0 && iParam<4)
		RoiCircleRing::SetParam(iParam, dVal);

	switch(iParam)
	{
		case 4: m_dBeginAngle = dVal; break;
		case 5: m_dEndAngle = dVal; break;
	}

	CalculateBoundingRect();
}

unsigned int RoiCircleSegment::GetVertexCount() const
{
	return RoiCircleRing::GetVertexCount();
}

ublas::vector<double> RoiCircleSegment::GetVertex(unsigned int i) const
{
	ublas::vector<double> vecRet(2);
	const unsigned int iVerticesPerArc = (GetVertexCount())/2;
	const double dAngleRange = (m_dEndAngle-m_dBeginAngle) / 180. * M_PI;

	// inner circle
	if(i<iVerticesPerArc)
	{
		double dAngle = dAngleRange*double(i)/double(iVerticesPerArc-1);
		dAngle += m_dBeginAngle / 180. * M_PI;

		vecRet[0] = m_dInnerRadius*cos(dAngle);
		vecRet[1] = m_dInnerRadius*sin(dAngle);

		vecRet = vecRet + m_vecCenter;
	}
	// outer circle
	else if(i>=iVerticesPerArc)
	{
		const int iIdx = 2*iVerticesPerArc - i - 1;

		double dAngle = dAngleRange*double(iIdx)/double(iVerticesPerArc-1);
		dAngle += m_dBeginAngle / 180. * M_PI;

		vecRet[0] = m_dOuterRadius*cos(dAngle);
		vecRet[1] = m_dOuterRadius*sin(dAngle);

		vecRet = vecRet + m_vecCenter;
	}

	return vecRet;
}

void RoiCircleSegment::CalculateBoundingRect()
{
	/// \todo
	RoiCircleRing::CalculateBoundingRect();
}

RoiCircleSegment& RoiCircleSegment::operator=(const RoiCircleSegment& elem)
{
	RoiCircleRing::operator=(elem);

	this->m_dBeginAngle = elem.m_dBeginAngle;
	this->m_dEndAngle = elem.m_dEndAngle;

	return *this;
}

RoiElement* RoiCircleSegment::copy() const
{
	RoiCircleSegment *pElem = new RoiCircleSegment;
	pElem->operator=(*this);
	return pElem;
}



//------------------------------------------------------------------------------
// polygon

RoiPolygon::RoiPolygon()
{}

RoiPolygon::RoiPolygon(const RoiPolygon& elem)
{
	*this = elem;
}

// Adaptor to use external pnpoly function more efficiently
class RoiPolygonArrayAdaptor
{
	protected:
		const RoiPolygon* m_pPoly;
		int m_iCoord;

	public:
		RoiPolygonArrayAdaptor(const RoiPolygon* pPoly, int iCoord)
					: m_pPoly(pPoly), m_iCoord(iCoord)
		{}

		double operator[](unsigned int i) const
		{
			// repeat first vertex
			if(i==m_pPoly->GetVertexCount())
				i=0;

			return m_pPoly->GetVertex(i)[m_iCoord];
		}
};

bool RoiPolygon::IsInside(double dX, double dY) const
{
	const unsigned int iVertCnt = GetVertexCount();

	RoiPolygonArrayAdaptor adaptor_x(this,0);
	RoiPolygonArrayAdaptor adaptor_y(this,1);

	return (pnpoly(iVertCnt+1, adaptor_x, adaptor_y, dX, dY) != 0);
}

std::string RoiPolygon::GetName() const
{
	return "polygon";
}

int RoiPolygon::GetParamCount() const
{
	return GetVertexCount()*2;
}

std::string RoiPolygon::GetParamName(int iParam) const
{
	int iVertex = iParam/2;
	int iCoord = iParam%2;

	std::ostringstream ostr;
	ostr << "vertex_" << iVertex << "_" << (iCoord==0?"x":"y");

	return ostr.str();
}

double RoiPolygon::GetParam(int iParam) const
{
	int iVertex = iParam/2;
	int iCoord = iParam%2;

	return GetVertex(iVertex)[iCoord];
}

void RoiPolygon::SetParam(int iParam, double dVal)
{
	int iVertex = iParam/2;
	int iCoord = iParam%2;

	if(iVertex < GetVertexCount())
		m_vertices[iVertex][iCoord] = dVal;
	else
	{
		ublas::vector<double> vec(2);
		vec[iCoord] = dVal;
		m_vertices.push_back(vec);
	}

	CalculateBoundingRect();
}

unsigned int RoiPolygon::GetVertexCount() const
{
	return m_vertices.size();
}

ublas::vector<double> RoiPolygon::GetVertex(unsigned int i) const
{
	return m_vertices[i];
}

void RoiPolygon::AddVertex(const ublas::vector<double>& vertex)
{
	//std::cout << vertex[0] << " " << vertex[1] << std::endl;
	m_vertices.push_back(vertex);

	CalculateBoundingRect();
}

RoiPolygon& RoiPolygon::operator=(const RoiPolygon& elem)
{
	RoiElement::operator=(elem);

	this->m_vertices = elem.m_vertices;

	return *this;
}

RoiElement* RoiPolygon::copy() const
{
	RoiPolygon *pElem = new RoiPolygon;
	pElem->operator=(*this);
	return pElem;
}


//------------------------------------------------------------------------------
// roi

Roi::Roi() : m_bActive(0)
{}

Roi::Roi(const Roi& roi)
{
	operator=(roi);
}

Roi& Roi::operator=(const Roi& roi)
{
	clear();

	for(int i=0; i<roi.GetNumElements(); ++i)
	{
		const RoiElement& elem = roi.GetElement(i);
		RoiElement* pNewElem = elem.copy();
		add(pNewElem);
	}

	this->m_bActive = roi.m_bActive;
	return *this;
}

Roi::~Roi()
{
	clear();
}

int Roi::add(RoiElement* elem)
{
	m_vecRoi.push_back(elem);
	return m_vecRoi.size()-1;
}

void Roi::clear()
{
	for(unsigned int i=0; i<m_vecRoi.size(); ++i)
	{
		if(m_vecRoi[i])
		{
			delete m_vecRoi[i];
			m_vecRoi[i] = 0;
		}
	}
	m_vecRoi.clear();
	m_bActive = 0;
}

bool Roi::IsInside(double dX, double dY) const
{
	if(!m_bActive) return 1;

	// check bounding rects
	bool bInBoundingRect = false;
	for(unsigned int i=0; i<m_vecRoi.size(); ++i)
	{
		if(m_vecRoi[i]->IsInBoundingRect(dX, dY))
		{
			bInBoundingRect = true;
			break;
		}
	}
	if(!bInBoundingRect)
		return false;


	// check actual elements
	for(unsigned int i=0; i<m_vecRoi.size(); ++i)
	{
		if(m_vecRoi[i]->IsInside(dX, dY))
			return true;
	}
	return false;
}

// TODO: consider overlapping roi elements
double Roi::HowMuchInside(int iX, int iY) const
{
	if(!m_bActive) return 1.;

	double dFraction = 0.;
	for(unsigned int i=0; i<m_vecRoi.size(); ++i)
	{
		dFraction += m_vecRoi[i]->HowMuchInside(iX, iY);
		if(dFraction >= 1.)
			return 1.;
	}
	return dFraction;
}

RoiElement& Roi::GetElement(unsigned int iElement)
{
	return *m_vecRoi[iElement];
}

const RoiElement& Roi::GetElement(unsigned int iElement) const
{
	return *m_vecRoi[iElement];
}

void Roi::DeleteElement(int iElement)
{
	if(m_vecRoi[iElement])
		delete m_vecRoi[iElement];
	m_vecRoi.erase(m_vecRoi.begin()+iElement);
}

unsigned int Roi::GetNumElements() const
{
	return m_vecRoi.size();
}

BoundingRect Roi::GetBoundingRect() const
{
	BoundingRect totalrect;
	totalrect.SetInvalidBounds();

	for(int i=0; i<GetNumElements(); ++i)
	{
		const BoundingRect& rect = GetElement(i).GetBoundingRect();

		totalrect.AddVertex(rect.bottomleft);
		totalrect.AddVertex(rect.topright);
	}

	return totalrect;
}

bool Roi::Load(const char* pcFile)
{
	clear();

	Xml xml;
	if(!xml.Load(pcFile))
	{
		std::cerr << "Roi: Cannot load \"" << pcFile << "\"." << std::endl;
		return false;
	}

	bool bOKActive=false;
	SetRoiActive(xml.Query<bool>("/roi/active", 0, &bOKActive));

	const std::string strPaths[] =
	{
			"/roi_elements/element_",		// old format
			"/roi/elements/element_"			// new format
	};

	for(const std::string& strPath : strPaths)
		for(int iElem=0; true; ++iElem)
		{
			std::ostringstream ostr;
			ostr << strPath;
			ostr << iElem;
			ostr << "/";

			std::string strQueryBase = ostr.str();
			std::string strQueryType = strQueryBase + std::string("type");

			bool bOK=false;
			std::string strType = xml.QueryString(strQueryType.c_str(), "", &bOK);
			if(!bOK)
				break;

			bool bUndeterminedParamCount=false;

			RoiElement *pElem = 0;
			if(strType == std::string("rectangle"))
				pElem = new RoiRect;
			else if(strType == std::string("circle"))
				pElem = new RoiCircle;
			else if(strType == std::string("circle_ring"))
				pElem = new RoiCircleRing;
			else if(strType == std::string("circle_segment"))
				pElem = new RoiCircleSegment;
			else if(strType == std::string("ellipse"))
				pElem = new RoiEllipse;
			else if(strType == std::string("polygon"))
			{
				bUndeterminedParamCount = true;
				pElem = new RoiPolygon;
			}
			else
			{
				std::cerr << "Roi: Unknown element \"" << strType << "\"." << std::endl;
				continue;
			}

			int iParamCount = pElem->GetParamCount();
			if(bUndeterminedParamCount)
				iParamCount = std::numeric_limits<int>::max();

			for(int iParam=0; iParam<iParamCount; ++iParam)
			{
				bool bOk=false;

				std::string strQueryParam = pElem->GetParamName(iParam);
				double dVal = xml.Query<double>((strQueryBase + strQueryParam).c_str(),
												0., &bOk);
				if(!bOk)
					break;
				pElem->SetParam(iParam, dVal);
			}

			add(pElem);
		}

	return true;
}

bool Roi::Save(const char* pcFile) const
{
	std::ofstream ofstr(pcFile);
	if(!ofstr.is_open())
	{
		std::cerr << "Roi: Cannot save \"" << pcFile << "\"." << std::endl;
		return false;
	}

	ofstr << "<?xml version=\"1.0\"?>\n\n";
	ofstr << "<!-- ROI element configuration for Cattus -->\n\n";

	ofstr << "<roi>\n\n";
	ofstr << "<active> " << IsRoiActive() << " </active>\n\n";
	ofstr << "<elements>\n\n";

	for(int i=0; i<GetNumElements(); ++i)
	{
		const RoiElement& elem = GetElement(i);
		ofstr << "\t<element_" << i << ">\n";
		ofstr << "\t\t<type> " << elem.GetName() << " </type>\n";

		for(int iParam=0; iParam<elem.GetParamCount(); ++iParam)
		{
			std::string strParam = elem.GetParamName(iParam);
			double dValue = elem.GetParam(iParam);

			ofstr << "\t\t<" << strParam << "> ";
			ofstr << dValue;
			ofstr << " </" << strParam << ">\n";
		}

		ofstr << "\t</element_" << i << ">\n\n";
	}

	ofstr << "</elements>\n\n";
	ofstr << "</roi>\n";
	ofstr.close();

	return true;
}


// drawing routines

void Roi::DrawRoi(QPainter& painter, const XYRange& range)
{
	if(!m_bActive)
		return;

	for(unsigned int iElem=0; iElem<GetNumElements(); ++iElem)
	{
		RoiElement& elem = GetElement(iElem);
		elem.draw(painter, range);
	}
}

void RoiElement::draw(QPainter& painter, const XYRange& range)
{
	if(GetVertexCount()<1)
		return;

	ublas::vector<double> vertBegin = GetVertex(0);
	ublas::vector<double> vertPrev = vertBegin;
	for(unsigned int iVert=1; iVert<GetVertexCount(); ++iVert)
	{
		ublas::vector<double> vert = GetVertex(iVert);

		QPointF pt0(range.GetPixelXPos(vertPrev[0]), range.GetPixelYPos(vertPrev[1])),
				pt1(range.GetPixelXPos(vert[0]), range.GetPixelYPos(vert[1]));
		painter.drawLine(pt0, pt1);

		vertPrev = vert;
	}

	QPointF pt0_(range.GetPixelXPos(vertPrev[0]), range.GetPixelYPos(vertPrev[1])),
			pt1_(range.GetPixelXPos(vertBegin[0]), range.GetPixelYPos(vertBegin[1]));
	painter.drawLine(pt0_, pt1_);
}
