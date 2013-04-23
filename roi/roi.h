/*
 * mieze-tool
 * roi stuff
 * @author tweber
 * @date 2011, 23-apr-2013
 */

#ifndef __TOF_ROI__
#define __TOF_ROI__

#include <vector>
#include <string>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

#define CIRCLE_VERTICES 256


struct BoundingRect
{
	ublas::vector<double> bottomleft;
	ublas::vector<double> topright;

	void SetInvalidBounds();
	void AddVertex(const ublas::vector<double>& vertex);

	BoundingRect() : bottomleft(2), topright(2)
	{
		bottomleft[0] = bottomleft[1] = topright[0] = topright[1] = 0.;
	}
};


/**
 * \brief base class for roi elements (rectangle, circle, ...)
 */
class RoiElement
{
	protected:
		BoundingRect m_boundingrect;
		virtual void CalculateBoundingRect();

		RoiElement();

	public:
		virtual ~RoiElement() {}

		virtual RoiElement& operator=(const RoiElement& elem);
		virtual RoiElement* copy() const = 0;


		/// get name of element
		virtual std::string GetName() const = 0;


		/// is point (dX, dY) inside roi element?
		virtual bool IsInside(double dX, double dY) const = 0;

		/// what fraction (0.0 .. 1.0) of pixel (iX, iY) is inside roi element?
		virtual double HowMuchInside(int iX, int iY) const;


		//----------------------------------------------------------------------
		/// vertices of element (interpolated for circles)
		virtual int GetVertexCount() const = 0;
		virtual ublas::vector<double> GetVertex(int i) const = 0;
		//----------------------------------------------------------------------


		//----------------------------------------------------------------------
		// parameters
		/// how many parameters does the element have?
		virtual int GetParamCount() const = 0;

		/// get name of a parameter
		virtual std::string GetParamName(int iParam) const = 0;

		/// get value of a parameter
		virtual double GetParam(int iParam) const = 0;

		/// set value of a parameter
		virtual void SetParam(int iParam, double dVal) = 0;
		//----------------------------------------------------------------------

		/// bounding rect for the element's current parameters
		virtual const BoundingRect& GetBoundingRect() const;

		/// is point (iX, iY) inside elementzy
		virtual bool IsInBoundingRect(double dX, double dY) const;
};


class RoiRect : public RoiElement
{
	protected:
		ublas::vector<double> m_bottomleft, m_topright;
		double m_dAngle;

	public:
		RoiRect(double dX1, double dY1, double dX2, double dY2, double dAngle=0.);
		RoiRect(const ublas::vector<double>& bottomleft,
				const ublas::vector<double>& topright, double dAngle=0.);
		RoiRect();
		RoiRect(const RoiRect& rect);

		virtual bool IsInside(double dX, double dY) const;

		virtual std::string GetName() const;

		virtual int GetParamCount() const;
		virtual std::string GetParamName(int iParam) const;
		virtual double GetParam(int iParam) const;
		virtual void SetParam(int iParam, double dVal);

		virtual int GetVertexCount() const;
		virtual ublas::vector<double> GetVertex(int i) const;

		virtual RoiRect& operator=(const RoiRect& elem);
		virtual RoiElement* copy() const;
};


class RoiCircle : public RoiElement
{
	protected:
		ublas::vector<double> m_vecCenter;
		double m_dRadius;

		virtual void CalculateBoundingRect();

	public:
		RoiCircle(const ublas::vector<double>& vecCenter, double dRadius);
		RoiCircle();
		RoiCircle(const RoiCircle& elem);

		virtual bool IsInside(double dX, double dY) const;

		virtual std::string GetName() const;

		virtual int GetParamCount() const;
		virtual std::string GetParamName(int iParam) const;
		virtual double GetParam(int iParam) const;
		virtual void SetParam(int iParam, double dVal);

		virtual int GetVertexCount() const;
		virtual ublas::vector<double> GetVertex(int i) const;

		virtual RoiCircle& operator=(const RoiCircle& elem);
		virtual RoiElement* copy() const;
};


class RoiEllipse : public RoiElement
{
	protected:
		ublas::vector<double> m_vecCenter;
		double m_dRadiusX, m_dRadiusY;

		virtual void CalculateBoundingRect();

	public:
		RoiEllipse(const ublas::vector<double>& vecCenter,
					double dRadiusX, double dRadiusY);
		RoiEllipse();
		RoiEllipse(const RoiEllipse& elem);

		virtual bool IsInside(double dX, double dY) const;

		virtual std::string GetName() const;

		virtual int GetParamCount() const;
		virtual std::string GetParamName(int iParam) const;
		virtual double GetParam(int iParam) const;
		virtual void SetParam(int iParam, double dVal);

		virtual int GetVertexCount() const;
		virtual ublas::vector<double> GetVertex(int i) const;

		virtual RoiEllipse& operator=(const RoiEllipse& elem);
		virtual RoiElement* copy() const;
};


class RoiCircleRing : public RoiElement
{
	protected:
		ublas::vector<double> m_vecCenter;
		double m_dInnerRadius, m_dOuterRadius;

		virtual void CalculateBoundingRect();

	public:
		RoiCircleRing(const ublas::vector<double>& vecCenter,
					  double dInnerRadius, double dOuterRadius);
		RoiCircleRing();
		RoiCircleRing(const RoiCircleRing& elem);

		virtual bool IsInside(double dX, double dY) const;

		virtual std::string GetName() const;

		virtual int GetParamCount() const;
		virtual std::string GetParamName(int iParam) const;
		virtual double GetParam(int iParam) const;
		virtual void SetParam(int iParam, double dVal);

		virtual int GetVertexCount() const;
		virtual ublas::vector<double> GetVertex(int i) const;

		virtual RoiCircleRing& operator=(const RoiCircleRing& elem);
		virtual RoiElement* copy() const;
};


class RoiCircleSegment : public RoiCircleRing
{
	protected:
		double m_dBeginAngle, m_dEndAngle;

		virtual void CalculateBoundingRect();

	public:
		RoiCircleSegment(const ublas::vector<double>& vecCenter,
						double dInnerRadius, double dOuterRadius,
						double dBeginAngle, double dEndAngle);
		RoiCircleSegment();
		RoiCircleSegment(const RoiCircleSegment& elem);

		virtual bool IsInside(double dX, double dY) const;

		virtual std::string GetName() const;

		virtual int GetParamCount() const;
		virtual std::string GetParamName(int iParam) const;
		virtual double GetParam(int iParam) const;
		virtual void SetParam(int iParam, double dVal);

		virtual int GetVertexCount() const;
		virtual ublas::vector<double> GetVertex(int i) const;

		virtual RoiCircleSegment& operator=(const RoiCircleSegment& elem);
		virtual RoiElement* copy() const;
};



class RoiPolygon : public RoiElement
{
	protected:
		std::vector<ublas::vector<double> > m_vertices;

	public:
		RoiPolygon();
		RoiPolygon(const RoiPolygon& elem);

		virtual bool IsInside(double dX, double dY) const;

		virtual std::string GetName() const;

		virtual int GetParamCount() const;
		virtual std::string GetParamName(int iParam) const;
		virtual double GetParam(int iParam) const;
		virtual void SetParam(int iParam, double dVal);

		virtual int GetVertexCount() const;
		virtual ublas::vector<double> GetVertex(int i) const;
		void AddVertex(const ublas::vector<double>& vertex);

		virtual RoiPolygon& operator=(const RoiPolygon& elem);
		virtual RoiElement* copy() const;
};


//------------------------------------------------------------------------------


class Roi
{
	protected:
		std::vector<RoiElement*> m_vecRoi;

	public:
		Roi();
		Roi(const Roi& roi);
		Roi& operator=(const Roi& roi);

		virtual ~Roi();

		/// add element, return position of element
		int add(RoiElement* elem);
		void clear();

		/// is point (dX, dY) inside roi?
		bool IsInside(double dX, double dY) const;

		/// what fraction (0.0 .. 1.0) of pixel (iX, iY) is inside roi?
		double HowMuchInside(int iX, int iY) const;

		RoiElement& GetElement(int iElement);
		const RoiElement& GetElement(int iElement) const;
		void DeleteElement(int iElement);
		int GetNumElements() const;

		/// get total bounding rectangle of all elements
		BoundingRect GetBoundingRect() const;

		bool Load(const char* pcFile);
		bool Save(const char* pcFile);
};

#endif
