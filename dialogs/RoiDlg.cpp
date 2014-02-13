/*
 * Roi Dialog
 * @author tweber
 * @date 2011, 24-04-2013
 */

#include "RoiDlg.h"
#include "../main/settings.h"
#include "../helper/string.h"

#include <QtGui/QMenu>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

RoiDlg::RoiDlg(QWidget *pParent)
			: QDialog(pParent), m_iCurrentItem(0)
{
	setupUi(this);
	checkEnabled->setChecked(m_roi.IsRoiActive());

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
	btnAdd->setMenu(pMenu);


	connect(actionNewRect, SIGNAL(triggered()), this, SLOT(NewRect()));
	connect(actionNewCircle, SIGNAL(triggered()), this, SLOT(NewCircle()));
	connect(actionNewEllipse, SIGNAL(triggered()), this, SLOT(NewEllipse()));
	connect(actionNewCircleRing, SIGNAL(triggered()), this, SLOT(NewCircleRing()));
	connect(actionNewCircleSeg, SIGNAL(triggered()), this, SLOT(NewCircleSeg()));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ButtonBoxClicked(QAbstractButton*)));

	connect(btnGetActive, SIGNAL(clicked()), this, SIGNAL(WantActiveRoi()));
	connect(btnSetActive, SIGNAL(clicked()), this, SIGNAL(SetRoiForActive()));
	connect(btnSetAll, SIGNAL(clicked()), this, SIGNAL(SetRoiForAll()));

	connect(btnLoad, SIGNAL(clicked()), this, SLOT(LoadRoi()));
	connect(btnSave, SIGNAL(clicked()), this, SLOT(SaveRoi()));

	QObject::connect(checkEnabled, SIGNAL(toggled(bool)), this, SLOT(SetRoiActive(bool)));
}

RoiDlg::~RoiDlg() {}

// an item (e.g. "circle", "rectangle", ... has been selected)
void RoiDlg::ItemSelected()
{
	m_iCurrentItem = listRois->currentRow();

	if(m_iCurrentItem<0 || m_iCurrentItem>=int(m_roi.GetNumElements()))
		return;

	RoiElement& elem = m_roi.GetElement(m_iCurrentItem);

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
	// only edit if this flag is set
	if(pItem->data(Qt::UserRole+1).value<int>() != 1)
		return;

	if(m_iCurrentItem<0 || m_iCurrentItem>=int(m_roi.GetNumElements()))
		return;
	RoiElement& elem = m_roi.GetElement(m_iCurrentItem);

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
	if(m_iCurrentItem<0 || m_iCurrentItem>=int(m_roi.GetNumElements()))
		return;

	RoiElement& elem = m_roi.GetElement(m_iCurrentItem);
	NewElement(elem.copy());
}

void RoiDlg::NewElement(RoiElement* pNewElem)
{
	int iPos = m_roi.add(pNewElem);
	new QListWidgetItem(m_roi.GetElement(iPos).GetName().c_str(), listRois);

	listRois->setCurrentRow(iPos);
}

void RoiDlg::NewCircle() { NewElement(new RoiCircle); }
void RoiDlg::NewEllipse() { NewElement(new RoiEllipse); }
void RoiDlg::NewCircleRing() { NewElement(new RoiCircleRing); }
void RoiDlg::NewCircleSeg() { NewElement(new RoiCircleSegment); }
void RoiDlg::NewRect() { NewElement(new RoiRect); }

void RoiDlg::DeleteItem()
{
	if(m_iCurrentItem<0 || m_iCurrentItem>=int(m_roi.GetNumElements()))
		return;

	int iCurItem = m_iCurrentItem;

	QListWidgetItem* pItem = listRois->item(iCurItem);
	if(pItem)
	{
		tableParams->setRowCount(0);

		delete pItem;
		m_roi.DeleteElement(iCurItem);

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
	m_roi = *pRoi;
	checkEnabled->setChecked(m_roi.IsRoiActive());

	// add all roi elements to list
	for(unsigned int i=0; i<m_roi.GetNumElements(); ++i)
		new QListWidgetItem(m_roi.GetElement(i).GetName().c_str(), listRois);

	if(m_roi.GetNumElements() > 0)
		listRois->setCurrentRow(0);
	else
		tableParams->setRowCount(0);
}

const Roi* RoiDlg::GetRoi(void) const { return &m_roi; }

void RoiDlg::ButtonBoxClicked(QAbstractButton* pBtn)
{
	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::ApplyRole ||
		buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		m_roi_last = m_roi;
	}
	else if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::RejectRole)
	{
		SetRoi(&m_roi_last);
		reject();
	}

	if(buttonBox->buttonRole(pBtn) == QDialogButtonBox::AcceptRole)
	{
		accept();
	}
}

void RoiDlg::SetRoiActive(bool bActive)
{
	m_roi.SetRoiActive(bActive);
}

void RoiDlg::LoadRoi()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir_roi", ".").toString();

	QString strFile = QFileDialog::getOpenFileName(this, "Open ROI file...", strLastDir,
					"ROI files (*.roi *.ROI);;All files (*.*)"/*,
					0, QFileDialog::DontUseNativeDialog*/);
	if(strFile.length() == 0)
		return;


	bool bDirSet=false;
	std::string strFile1 = strFile.toStdString();

	if(!bDirSet)
	{
		pGlobals->setValue("main/lastdir_roi", QString(::get_dir(strFile1).c_str()));
		bDirSet = true;
	}

	Roi roi;
	if(!roi.Load(strFile1.c_str()))
	{
		QMessageBox::critical(this, "Error", "Could not load ROI.");
		return;
	}

	SetRoi(&roi);
}

void RoiDlg::SaveRoi()
{
	QSettings *pGlobals = Settings::GetGlobals();
	QString strLastDir = pGlobals->value("main/lastdir_roi", ".").toString();

	QString strFile = QFileDialog::getSaveFileName(this, "Save ROI file...", strLastDir,
					"ROI files (*.roi *.ROI);;All files (*.*)"/*,
					0, QFileDialog::DontUseNativeDialog*/);
	if(strFile.length() == 0)
		return;


	bool bDirSet=false;
	std::string strFile1 = strFile.toStdString();

	if(!bDirSet)
	{
		pGlobals->setValue("main/lastdir_roi", QString(::get_dir(strFile1).c_str()));
		bDirSet = true;
	}

	const Roi* pRoi = GetRoi();
	if(!pRoi->Save(strFile1.c_str()))
		QMessageBox::critical(this, "Error", "Could not save ROI.");
}

void RoiDlg::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);
}

#include "RoiDlg.moc"
