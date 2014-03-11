/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#ifndef __TAZ_SCATT_TRIAG_H__
#define __TAZ_SCATT_TRIAG_H__

#include "helper/linalg.h"
#include "helper/lattice.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QWheelEvent>

#include "tasoptions.h"

class ScatteringTriangle;
class ScatteringTriangleNode : public QGraphicsItem
{
	protected:
		ScatteringTriangle *m_pParentItem;

	protected:
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		void mousePressEvent(QGraphicsSceneMouseEvent *pEvt);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *pEvt);
		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

	public:
		ScatteringTriangleNode(ScatteringTriangle* pSupItem);
};

class RecipPeak : public QGraphicsItem
{
	protected:
		QColor m_color = Qt::red;
		QRectF boundingRect() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	protected:
		QString m_strLabel;

	public:
		RecipPeak();

		void SetLabel(const QString& str) { m_strLabel = str; }
		void SetColor(const QColor& col) { m_color = col; }
};

class ScatteringTriangleScene;
class ScatteringTriangle : public QGraphicsItem
{
	private:
		bool m_bReady = 0;

	protected:
		ScatteringTriangleScene &m_scene;

		ScatteringTriangleNode *m_pNodeKiQ = 0;
		ScatteringTriangleNode *m_pNodeKiKf = 0;
		ScatteringTriangleNode *m_pNodeKfQ = 0;

		double m_dScaleFactor = 75.;	// pixels per A^-1 for zoom == 1.
		double m_dZoom = 1.;
		double m_dPlaneDistTolerance = 0.01;
		double m_dMaxPeaks = 5.;

		std::vector<RecipPeak*> m_vecPeaks;
		void ClearPeaks();

		double m_dAngleRot = 0.;

	protected:
		QRectF boundingRect() const;

	public:
		ScatteringTriangle(ScatteringTriangleScene& scene);
		virtual ~ScatteringTriangle();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
		void nodeMoved(const ScatteringTriangleNode* pNode=0);

		bool IsReady() const { return m_bReady; }
		double GetTheta(bool bPosSense) const;
		double GetTwoTheta(bool bPosSense) const;
		double GetMonoTwoTheta(double dMonoD, bool bPosSense) const;
		double GetAnaTwoTheta(double dAnaD, bool bPosSense) const;

		void SetTwoTheta(double dTT);
		void SetAnaTwoTheta(double dTT, double dAnaD);
		void SetMonoTwoTheta(double dTT, double dMonoD);

	public:
		void CalcPeaks(const Lattice& lattice,
						const Lattice& recip, const Lattice& recip_unrot,
						const Plane<double>& plane);

		void SetPlaneDistTolerance(double dTol) { m_dPlaneDistTolerance = dTol; }
		void SetMaxPeaks(double dMax) { m_dMaxPeaks = dMax; }
		void SetZoom(double dZoom);
};


class ScatteringTriangleScene : public QGraphicsScene
{	Q_OBJECT
	protected:
		ScatteringTriangle *m_pTri;
		double m_dMonoD = 3.355;
		double m_dAnaD = 3.355;

		bool m_bSamplePosSense = 1;
		bool m_bAnaPosSense = 0;
		bool m_bMonoPosSense = 0;

		bool m_bDontEmitChange = 0;

	public:
		ScatteringTriangleScene();
		virtual ~ScatteringTriangleScene();

		void emitUpdate();
		void SetDs(double dMonoD, double dAnaD);

		void SetSampleSense(bool bPos);
		void SetMonoSense(bool bPos);
		void SetAnaSense(bool bPos);

		ScatteringTriangle* GetTriangle() { return m_pTri; }

	public slots:
		void tasChanged(const TriangleOptions& opts);
		void scaleChanged(double dTotalScale);
	signals:
		void triangleChanged(const TriangleOptions& opts);
};


class ScatteringTriangleView : public QGraphicsView
{
	Q_OBJECT
	protected:
		double m_dTotalScale = 1.;
		virtual void wheelEvent(QWheelEvent* pEvt);

	public:
		ScatteringTriangleView(QWidget* pParent = 0);
		virtual ~ScatteringTriangleView();

	signals:
		void scaleChanged(double dTotalScale);
};

#endif
