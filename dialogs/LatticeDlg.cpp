/*
 * Lattice Dialog
 * @author tweber
 * @date 09-oct-2013
 */

#include "LatticeDlg.h"
#include "../helper/linalg.h"
#include "../helper/math.h"
#include "../helper/string.h"

LatticeDlg::LatticeDlg(QWidget* pParent) : QDialog(pParent)
{
	setupUi(this);

	m_vecEditReal.push_back(editVec1);
	m_vecEditReal.push_back(editVec2);
	m_vecEditReal.push_back(editVec3);
	m_vecEditReal.push_back(editVec4);

	m_vecEditRecip.push_back(editRVec1);
	m_vecEditRecip.push_back(editRVec2);
	m_vecEditRecip.push_back(editRVec3);
	m_vecEditRecip.push_back(editRVec4);

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
						QLabel* pLabIn=0, QLabel* pLabOut=0, QLabel* pLabelStatus=0)
{
	std::vector<double> vecVals0;
	get_tokens<double>(vecEditIn[0]->text().toStdString(), " ,;\t", vecVals0);
	unsigned int iDim = vecVals0.size();
	const unsigned int iMaxDim = vecEditIn.size();
	if(iDim > iMaxDim) iDim = iMaxDim;

	for(unsigned int iVec=0; iVec<iDim; ++iVec)
	{
		vecEditIn[iVec]->setEnabled(true);
		vecEditOut[iVec]->setEnabled(true);
	}
	for(unsigned int iVec=iDim; iVec<iMaxDim; ++iVec)
	{
		vecEditIn[iVec]->setEnabled(false);
		vecEditOut[iVec]->setEnabled(false);
	}

	if(pLabelStatus)
	{
		std::ostringstream ostrDim;
		ostrDim << iDim << "D Lattice";
		pLabelStatus->setText(ostrDim.str().c_str());
	}

	ublas::matrix<double> matReal(iDim, iDim);
	for(unsigned int iVec=0; iVec<iDim; ++iVec)
	{
		std::string strVec = vecEditIn[iVec]->text().toStdString();
		std::vector<double> vecVals;
		get_tokens<double>(strVec, " ,;\t", vecVals);
		while(vecVals.size() < iDim) vecVals.push_back(0.);

		for(unsigned int iComp=0; iComp<iDim; ++iComp)
			matReal(iComp, iVec) = vecVals[iComp];
	}

	if(pLabIn)
	{
		double dVol = get_volume(matReal);
		pLabIn->setText(QString::number(dVol));
	}



	ublas::matrix<double> matInv;
	inverse<double>(ublas::trans(matReal), matInv);

	ublas::matrix<double> matRecip = 2.*M_PI*matInv;

	if(matRecip.size1()!=iDim || matRecip.size2()!=iDim)
	{
		for(QLineEdit* pEdit : vecEditOut)
			pEdit->setText("invalid");
		if(pLabOut) pLabOut->setText("invalid");
		return;
	}

	if(pLabOut)
	{
		double dVol = get_volume(matRecip);
		pLabOut->setText(QString::number(dVol));
	}


	for(unsigned int iVec=0; iVec<iDim; ++iVec)
	{
		std::ostringstream ostrVec;

		for(unsigned int iComp=0; iComp<iDim; ++iComp)
		{
			ostrVec << matRecip(iComp, iVec);
			if(iComp<iDim-1) ostrVec << ", ";
		}

		vecEditOut[iVec]->setText(ostrVec.str().c_str());
	}
}

void LatticeDlg::CalcRecip()
{
	CalcRealRecip(m_vecEditReal, m_vecEditRecip, labelVol, labelRVol, labelDim);
}

void LatticeDlg::CalcReal()
{
	CalcRealRecip(m_vecEditRecip, m_vecEditReal, labelRVol, labelVol, labelDim);
}


#include "LatticeDlg.moc"
