/*
 * mieze-tool
 * @author tweber
 * @date 25-apr-2013
 * @license GPLv3
 */

#include "ComboDlg.h"

static void SetValues(QComboBox* pCombo, const std::vector<std::string>& vec)
{
	for(const std::string& str : vec)
		pCombo->addItem(QString(str.c_str()));
}

static void SelectValue(QComboBox* pCombo, const std::string& str)
{
	QString qstr = str.c_str();
	for(int iIdx=0; iIdx<pCombo->count(); ++iIdx)
	{
		if(pCombo->itemText(iIdx) == qstr)
		{
			pCombo->setCurrentIndex(iIdx);
			break;
		}
	}
}

void ComboDlg::SetCurFile(const char* pcStr) { labelFile->setText(pcStr); }
void ComboDlg::SetTitle(const char* pcStr) { setWindowTitle(pcStr); }

void ComboDlg::SetLabel(const char* pcStr) { this->labelValues->setText(pcStr); }
void ComboDlg::SetLabelY(const char* pcStr) { this->labelValuesY->setText(pcStr); }

int ComboDlg::GetSelectedValue() const { return comboValues->currentIndex(); }
int ComboDlg::GetSelectedValueY() const { return comboValuesY->currentIndex(); }

void ComboDlg::SetValues(const std::vector<std::string>& vec) { ::SetValues(comboValues, vec); }
void ComboDlg::SetValuesY(const std::vector<std::string>& vec) { ::SetValues(comboValuesY, vec); }

void ComboDlg::SelectValue(const std::string& str) { ::SelectValue(comboValues, str); }
void ComboDlg::SelectValueY(const std::string& str) { ::SelectValue(comboValuesY, str); }

#include "ComboDlg.moc"
