/**
 * mieze-tool
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 29-aug-2013
 * @license GPLv3
 */

#include "InfoDock.h"


static void set_params(const StringMap& mapStr, QTableWidget* pTable, std::string& strLastSelectedKey)
{
	QTableWidgetItem *pCurKey = pTable->item(pTable->currentRow(),0);
	if(pCurKey)
	{
		strLastSelectedKey = pCurKey->text().toStdString();
		//std::cout << "last selected key: " << strLastSelectedKey << std::endl;
	}


	const StringMap::t_map& mapParams = mapStr.GetMap();
	pTable->setRowCount(mapParams.size());

	unsigned int iRow = 0;
	for(const auto& pair : mapParams)
	{
		const std::string& strKey = pair.first;
		const std::string& strVal = pair.second;

		QTableWidgetItem *pItemKey = new QTableWidgetItem(strKey.c_str());
		QTableWidgetItem *pItemVal = new QTableWidgetItem(strVal.c_str());

		pTable->setItem(iRow, 0, pItemKey);
		pTable->setItem(iRow, 1, pItemVal);

		++iRow;
	}

	pTable->selectRow(0);
	if(strLastSelectedKey != "")
	{
		for(unsigned int iRow=0; iRow<pTable->rowCount(); ++iRow)
		{
			QTableWidgetItem *pItem = pTable->item(iRow,0);
			if(pItem && pItem->text().toStdString() == strLastSelectedKey)
			{
				pTable->selectRow(iRow);
				break;
			}
		}
	}
}


InfoDock::InfoDock(QWidget* pParent)
			: QDockWidget(pParent), m_widget(this)
{
	setWidget(&m_widget);
	setWindowTitle("Info Panel");

	/*
	std::map<std::string, std::string> map;
	map["123"] = "abc";
	map["124"] = "abx";
	SetMiscParams(map);
	*/
}

InfoDock::~InfoDock()
{}

void InfoDock::SetParamsDyn(const StringMap& mapStr)
{
	static std::string strLastSel;
	set_params(mapStr, m_widget.tablePlot, strLastSel);
}

void InfoDock::SetParamsStat(const StringMap& mapStr)
{
	static std::string strLastSel;
	set_params(mapStr, m_widget.tableMisc, strLastSel);
}

InfoWidget::InfoWidget(QWidget* pParent)
			: QWidget(pParent)
{
	setupUi(this);


	QTableWidget* pTables[] = {tablePlot, tableMisc};

	for(QTableWidget* pTable : pTables)
	{
		pTable->setColumnCount(2);
		pTable->setColumnWidth(0, 75);
		pTable->setColumnWidth(1, 150);
		pTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Key"));
		pTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));

		pTable->verticalHeader()->setVisible(false);
		pTable->verticalHeader()->setDefaultSectionSize(pTable->verticalHeader()->minimumSectionSize()+2);
	}
}

InfoWidget::~InfoWidget()
{
}


#include "InfoDock.moc"
