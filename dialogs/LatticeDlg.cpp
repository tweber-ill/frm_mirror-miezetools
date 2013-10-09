/*
 * Lattice Dialog
 * @author tweber
 * @date 09-oct-2013
 */

#include "LatticeDlg.h"
#include "../helper/linalg.h"
#include "../helper/string.h"

LatticeDlg::LatticeDlg(QWidget* pParent) : QDialog(pParent)
{
	setupUi(this);

	m_vecEditReal.push_back(editVec1);
	m_vecEditReal.push_back(editVec2);
	m_vecEditReal.push_back(editVec3);

	m_vecEditRecip.push_back(editRVec1);
	m_vecEditRecip.push_back(editRVec2);
	m_vecEditRecip.push_back(editRVec3);

	for(QLineEdit* pEdit : m_vecEditReal)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)),
						this, SLOT(CalcRecip()));

	for(QLineEdit* pEdit : m_vecEditRecip)
		QObject::connect(pEdit, SIGNAL(textEdited(const QString&)),
						this, SLOT(CalcReal()));

	CalcRecip();
}

LatticeDlg::~LatticeDlg()
{}


static void CalcRealRecip(const std::vector<QLineEdit*>& vecEditIn,
						const std::vector<QLineEdit*>& vecEditOut,
						QLabel* pLabIn=0, QLabel* pLabOut=0)
{
	ublas::matrix<double> matReal(3,3);
	for(unsigned int iVec=0; iVec<3; ++iVec)
	{
		std::string strVec = vecEditIn[iVec]->text().toStdString();
		std::vector<double> vecVals;
		get_tokens<double>(strVec, " ,;\t", vecVals);
		while(vecVals.size() < 3) vecVals.push_back(0.);

		for(unsigned int iComp=0; iComp<3; ++iComp)
			matReal(iComp, iVec) = vecVals[iComp];
	}

	if(pLabIn)
	{
		double dVol = get_volume_3(matReal);
		pLabIn->setText(QString::number(dVol));
	}



	ublas::matrix<double> matInv;
	inverse<double>(ublas::trans(matReal), matInv);

	ublas::matrix<double> matRecip = 2.*M_PI*matInv;

	if(matRecip.size1()!=3 || matRecip.size2()!=3)
	{
		for(QLineEdit* pEdit : vecEditOut)
			pEdit->setText("invalid");
		if(pLabOut) pLabOut->setText("invalid");
		return;
	}

	if(pLabOut)
	{
		double dVol = get_volume_3(matRecip);
		pLabOut->setText(QString::number(dVol));
	}


	for(unsigned int iVec=0; iVec<3; ++iVec)
	{
		std::ostringstream ostrVec;

		for(unsigned int iComp=0; iComp<3; ++iComp)
		{
			ostrVec << matRecip(iComp, iVec);
			if(iComp<2) ostrVec << ", ";
		}

		vecEditOut[iVec]->setText(ostrVec.str().c_str());
	}
}

void LatticeDlg::CalcRecip()
{
	CalcRealRecip(m_vecEditReal, m_vecEditRecip, labelVol, labelRVol);
}

void LatticeDlg::CalcReal()
{
	CalcRealRecip(m_vecEditRecip, m_vecEditReal, labelRVol, labelVol);
}


#include "LatticeDlg.moc"
