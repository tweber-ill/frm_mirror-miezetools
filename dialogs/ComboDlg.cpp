/*
 * mieze-tool
 * @author tweber
 * @date 25-apr-2013
 */

#include "ComboDlg.h"

void ComboDlg::SetLabel(const char* pcStr) { this->labelValues->setText(pcStr); }
void ComboDlg::SetTitle(const char* pcStr) { setWindowTitle(pcStr); }
int ComboDlg::GetSelectedValue() const { return comboValues->currentIndex(); }

void ComboDlg::SetValues(const std::vector<std::string>& vec)
{
		for(const std::string& str : vec)
			this->comboValues->addItem(QString(str.c_str()));
}

#include "ComboDlg.moc"
