#include "../helper/linalg.h"
#include <vector>
#include <cmath>
#include <iostream>

#include <mgl2/qt.h>


ublas::vector<double> helix_vec(double r, double c, const ublas::vector<double>& vecCoord)
{	
	static std::vector<ublas::matrix<double> > matrices;
	static std::vector<ublas::matrix<double> > inv_matrices;
	
	static bool bMatricesInited = 0;
	if(!bMatricesInited)
	{
		const double dAngles[3] = {0., 120., 240.};
		
		for(double dAngle : dAngles)
		{
			dAngle = dAngle/180.*M_PI;

			ublas::matrix<double> matRotNeg = rotation_matrix_3d_x<double>(-dAngle);
			ublas::matrix<double> matRotPos = rotation_matrix_3d_x<double>(dAngle);
			
			matrices.push_back(matRotPos);
			inv_matrices.push_back(matRotNeg);
		}
		
		bMatricesInited = 1;
	}
	

	ublas::vector<double> vecRet = ublas::zero_vector<double>(3);

	for(int iMatrix=0; iMatrix<matrices.size(); ++iMatrix)
	{
		const ublas::matrix<double>& matRotNeg = inv_matrices[iMatrix];
		const ublas::matrix<double>& matRotPos = matrices[iMatrix];

		ublas::vector<double> vecCoordRot = ublas::prod(matRotNeg, vecCoord);

		ublas::vector<double> vec(3);

		vec[0] = r * cos(vecCoordRot[2]);
		vec[1] = r * sin(vecCoordRot[2]);
		vec[2] = /*c * vecCoordRot[2]*/ 0.;

		vec = ublas::prod(matRotPos, vec);

		vecRet += vec;
	}
	return vecRet;
}


int draw(mglGraph *gr)
{
	const int iCnt = 64;
	mglData datx, daty, datz;

	datx.Create(iCnt, iCnt, iCnt);
	daty.Create(iCnt, iCnt, iCnt);
	datz.Create(iCnt, iCnt, iCnt);

	double r = 1.;
	double c = 1.;
	
	const double dTScale = 4.;
	const double dXScale = 24.;
	const double dYScale = 24.;
	
	const double dScale2 = 0.5;

	for(int iZ=0; iZ<iCnt; ++iZ)
	{
		double dT = double(iZ)/double(iCnt-1) * 2.*M_PI;

		for(int iY=0; iY<iCnt; ++iY)
		{
			double dY = double(iY)/double(iCnt-1);

			for(int iX=0; iX<iCnt; ++iX)
			{
				double dX = double(iX)/double(iCnt-1);

				ublas::vector<double> vecCoord(3);
				vecCoord[0] = dXScale*dX;
				vecCoord[1] = dYScale*dY;
				vecCoord[2] = dTScale*dT;

				ublas::vector<double> vec = helix_vec(r, c, vecCoord);


				int iIdx = iX + iY*iCnt + iZ*iCnt*iCnt;
				datx.a[iIdx] = dScale2 * vec[0];
				daty.a[iIdx] = dScale2 * vec[1];
				datz.a[iIdx] = dScale2 * vec[2];
			}
		}
	}


	//gr->Plot(datx, daty, datz, "");
	//gr->Vect(datx, daty, datz, "");
	gr->Vect3(datx, daty, datz, "fx");

	//gr->SetRanges(0, 1, 0, 1, 0, 1);
	return 0;
}


int main(int argc, char **argv)
{
	mglQT *pMGL;
	pMGL = new mglQT(draw);
	pMGL->Run();
	delete pMGL;

	return 0;
}
