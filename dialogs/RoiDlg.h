/*
 * Roi Dialog
 * @author tweber
 * @date 2011, 24-04-2013
 * @license GPLv3
 */

#ifndef __ROI_DLG__
#define __ROI_DLG__

#include <QtGui/QDialog>
#include "../ui/ui_roi.h"

#include "../roi/roi.h"


class RoiDlg : public QDialog, Ui::RoiDlg
{Q_OBJECT

protected:
	Roi m_roi, m_roi_last;
	int m_iCurrentItem;

	void NewElement(RoiElement* pNewElem);

public:
	RoiDlg(QWidget* pParent);
	virtual ~RoiDlg();

	void SetRoi(const Roi* pRoi);
	const Roi* GetRoi(void) const;
	void ClearList();

public slots:
	void ItemSelected();
	void ValueChanged(QTableWidgetItem *pItem);

	void NewCircle();
	void NewEllipse();
	void NewCircleRing();
	void NewCircleSeg();
	void NewRect();

	void DeleteItem();
	void CopyItem();

	void SetRoiActive(bool);
	void ButtonBoxClicked(QAbstractButton*);

	void LoadRoi();
	void SaveRoi();

signals:
	void SetRoiForActive();
	void SetRoiForAll();
	void WantActiveRoi();

protected:
	virtual void showEvent(QShowEvent *event);
};

#endif
