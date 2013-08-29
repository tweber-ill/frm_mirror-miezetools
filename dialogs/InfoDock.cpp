/*
 * mieze-tool
 * @author tweber
 * @date 29-aug-2013
 */

#include "InfoDock.h"

InfoDock::InfoDock(QWidget* pParent)
			: QDockWidget(pParent), m_widget(this)
{
	setWidget(&m_widget);
	setWindowTitle("Infos");

	/*
	std::map<std::string, std::string> map;
	map["123"] = "abc";
	map["124"] = "abx";
	SetMiscParams(map);
	*/
}

InfoDock::~InfoDock()
{}


void InfoDock::SetMiscParams(const StringMap& mapStr)
{
	static std::string strLastSelectedKey;

	QTableWidgetItem *pCurKey = m_widget.tableMisc->item(m_widget.tableMisc->currentRow(),0);
	if(pCurKey)
	{
		strLastSelectedKey = pCurKey->text().toStdString();
		//std::cout << "last selected key: " << strLastSelectedKey << std::endl;
	}


	const StringMap::t_map& mapParams = mapStr.GetMap();
	m_widget.tableMisc->setRowCount(mapParams.size());

	unsigned int iRow = 0;
	for(const auto& pair : mapParams)
	{
		const std::string& strKey = pair.first;
		const std::string& strVal = pair.second;

		QTableWidgetItem *pItemKey = new QTableWidgetItem(strKey.c_str());
		QTableWidgetItem *pItemVal = new QTableWidgetItem(strVal.c_str());

		m_widget.tableMisc->setItem(iRow, 0, pItemKey);
		m_widget.tableMisc->setItem(iRow, 1, pItemVal);

		++iRow;
	}

	m_widget.tableMisc->selectRow(0);
	if(strLastSelectedKey != "")
	{
		for(unsigned int iRow=0; iRow<m_widget.tableMisc->rowCount(); ++iRow)
		{
			QTableWidgetItem *pItem = m_widget.tableMisc->item(iRow,0);
			if(pItem && pItem->text().toStdString() == strLastSelectedKey)
			{
				m_widget.tableMisc->selectRow(iRow);
				break;
			}
		}
	}
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
