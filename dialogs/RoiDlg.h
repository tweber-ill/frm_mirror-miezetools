/*
 * Roi Dialog
 * @author tweber
 * @date 2011, 24-04-2013
 */

#ifndef __ROI_DLG__
#define __ROI_DLG__

#include <QtGui/QDialog>
#include "../ui/ui_roi.h"

#include "../roi/roi.h"


class RoiDlg : public QDialog, Ui::RoiDlg
{Q_OBJECT

protected:
	Roi *m_pRoi;
	int m_iCurrentItem;

	void NewElement(RoiElement* pNewElem);
	void Deinit();

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

	void ButtonBoxClicked(QAbstractButton*);

signals:
	void NewRoiAvailable(const Roi* pROI);
};

#endif
