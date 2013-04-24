/*
 * Roi Dialog
 * @author tweber
 * @date 2011, 24-04-2013
 */

#include "RoiDlg.h"
#include <QtGui/QMenu>

RoiDlg::RoiDlg(QWidget *pParent)
			: QDialog(pParent), m_pRoi(0), m_iCurrentItem(0)
{
	setupUi(this);

	connect(listRois, SIGNAL(itemSelectionChanged()), this, SLOT(ItemSelected()));
	connect(tableParams, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(ValueChanged(QTableWidgetItem *)));
	connect(btnDelete, SIGNAL(clicked()), this, SLOT(DeleteItem()));
	connect(btnCopy, SIGNAL(clicked()), this, SLOT(CopyItem()));


	QAction *actionNewRect = new QAction("Rectangle", this);
	QAction *actionNewCircle = new QAction("Circle", this);
	QAction *actionNewEllipse = new QAction("Ellipse", this);
	QAction *actionNewCircleRing = new QAction("Circle Ring", this);
	QAction *actionNewCircleSeg = new QAction("Circle Segment", this);

	QMenu *pMenu = new QMenu(this);
	pMenu->addAction(actionNewRect);
	pMenu->addAction(actionNewCircle);
	pMenu->addAction(actionNewEllipse);
	pMenu->addAction(actionNewCircleRing);
	pMenu->addAction(actionNewCircleSeg);

	connect(actionNewRect, SIGNAL(triggered()), this, SLOT(NewRect()));
	connect(actionNewCircle, SIGNAL(triggered()), this, SLOT(NewCircle()));
	connect(actionNewEllipse, SIGNAL(triggered()), this, SLOT(NewEllipse()));
	connect(actionNewCircleRing, SIGNAL(triggered()), this, SLOT(NewCircleRing()));
	connect(actionNewCircleSeg, SIGNAL(triggered()), this, SLOT(NewCircleSeg()));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	btnAdd->setMenu(pMenu);
}

RoiDlg::~RoiDlg()
{
	Deinit();
}

// an item (e.g. "circle", "rectangle", ... has been selected)
void RoiDlg::ItemSelected()
{
	if(!m_pRoi) return;

	m_iCurrentItem = listRois->currentRow();

	if(m_iCurrentItem<0 || m_iCurrentItem >= m_pRoi->GetNumElements())
		return;

	RoiElement& elem = m_pRoi->GetElement(m_iCurrentItem);

	tableParams->setRowCount(elem.GetParamCount());
	tableParams->setColumnCount(2);

	for(int iParam=0; iParam<elem.GetParamCount(); ++iParam)
	{
		std::string strParamName = elem.GetParamName(iParam);
		double dParamValue = elem.GetParam(iParam);

		std::ostringstream ostrValue;
		ostrValue << dParamValue;

		QTableWidgetItem *pItemName =
								new QTableWidgetItem(strParamName.c_str());
		pItemName->setFlags(pItemName->flags() & ~Qt::ItemIsEditable);
		tableParams->setItem(iParam, 0, pItemName);

		QTableWidgetItem *pItemValue =
								new QTableWidgetItem(ostrValue.str().c_str());

		pItemValue->setData(Qt::UserRole, iParam);
		pItemValue->setData(Qt::UserRole+1, 1);		// flag 'editable'

		//pItemValue->setData(Qt::UserRole, QVariant::fromValue(pvElem));
		tableParams->setItem(iParam, 1, pItemValue);
	}
}

// a property of the selected item has changed
void RoiDlg::ValueChanged(QTableWidgetItem* pItem)
{
	if(!m_pRoi) return;

	// only edit if this flag is set
	if(pItem->data(Qt::UserRole+1).value<int>() != 1)
		return;

	if(m_iCurrentItem<0 || m_iCurrentItem >= m_pRoi->GetNumElements())
		return;
	RoiElement& elem = m_pRoi->GetElement(m_iCurrentItem);

	QVariant var = pItem->data(Qt::UserRole);
	int iParam = var.value<int>();

	bool bOk = true;
	double dVal = pItem->text().toDouble(&bOk);
	if(!bOk)
	{	// reset to original value
		QString strOldVal;
		strOldVal.setNum(elem.GetParam(iParam));
		pItem->setText(strOldVal);
	}
	else
	{	// accept new value
		elem.SetParam(iParam,dVal);
	}
}

void RoiDlg::CopyItem()
{
	if(!m_pRoi) return;

	if(m_iCurrentItem<0 || m_iCurrentItem >= m_pRoi->GetNumElements())
		return;

	RoiElement& elem = m_pRoi->GetElement(m_iCurrentItem);
	NewElement(elem.copy());
}

void RoiDlg::NewElement(RoiElement* pNewElem)
{
	if(!m_pRoi)
		m_pRoi = new Roi;

	int iPos = m_pRoi->add(pNewElem);
	new QListWidgetItem(m_pRoi->GetElement(iPos).GetName().c_str(), listRois);

	listRois->setCurrentRow(iPos);
}

void RoiDlg::NewCircle() { NewElement(new RoiCircle); }
void RoiDlg::NewEllipse() { NewElement(new RoiEllipse); }
void RoiDlg::NewCircleRing() { NewElement(new RoiCircleRing); }
void RoiDlg::NewCircleSeg() { NewElement(new RoiCircleSegment); }
void RoiDlg::NewRect() { NewElement(new RoiRect); }

void RoiDlg::DeleteItem()
{
	if(!m_pRoi) return;

	if(m_iCurrentItem<0 || m_iCurrentItem >= m_pRoi->GetNumElements())
		return;

	int iCurItem = m_iCurrentItem;

	QListWidgetItem* pItem = listRois->item(iCurItem);
	if(pItem)
	{
		tableParams->setRowCount(0);

		delete pItem;
		m_pRoi->DeleteElement(iCurItem);

		m_iCurrentItem = listRois->currentRow();
	}
}

void RoiDlg::ClearList()
{
	listRois->clear();
}

void RoiDlg::SetRoi(const Roi* pRoi)
{
	ClearList();

	if(m_pRoi)
		delete m_pRoi;

	m_pRoi = new Roi(*pRoi);

	// add all roi elements to list
	for(int i=0; i<m_pRoi->GetNumElements(); ++i)
		new QListWidgetItem(m_pRoi->GetElement(i).GetName().c_str(), listRois);

	if(m_pRoi->GetNumElements() > 0)
		listRois->setCurrentRow(0);
}

const Roi* RoiDlg::GetRoi(void) const
{
	return m_pRoi;
}

void RoiDlg::Deinit()
{
	if(m_pRoi)
		delete m_pRoi;
}

void RoiDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
		buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		if(m_pRoi)
			emit NewRoiAvailable(this->m_pRoi);
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		accept();
	}
}


#include "RoiDlg.moc"
