#include "../helper/linalg.h"
#include <vector>
#include <cmath>
#include <iostream>

#include <mgl2/qt.h>


// http://mathworld.wolfram.com/Helix.html
ublas::vector<double> helix_vec(double r, double c, double t)
{
	ublas::vector<double> vec(3);

	vec[0] = r * cos(t);
	vec[1] = r * sin(t);
	vec[2] = c * t;

	return vec;
}

/*
ublas::vector<double> helices_vec(double r, double c, const ublas::vector<double>& vecPos)
{
	ublas::vector<double> vec = ublas::zero_vector<double>(3);

	const int iCnt = 3;
	for(int i=0; i<iCnt; ++i)
	{
		ublas::vector<double> vecT(3);
		vecT[0] = 0.;
		vecT[1] = 0.;
		vecT[2] = 1.;

		double dAngle = 2.*M_PI * double(i)/double(iCnt);

		ublas::matrix<double> mat = rotation_matrix_3d_x<double>(dAngle);
		ublas::vector<double> vecTRot = ublas::prod(mat, vecT);
		double dTProj = ublas::inner_prod(vecPos, vecTRot);

		ublas::vector<double> vecHelix = helix_vec(r, c, dTProj);

		//double dAnglePos = acos(ublas::inner_prod(vecPos, vecTRot));
		mat = rotation_matrix_3d_x<double>(dAngle);
		vecT[0] = 0.;
		vecT[1] = 0.;
		vecT[2] = dTProj;

		vecHelix += vecT;
		vecHelix = ublas::prod(mat, vecHelix);
		vecT = ublas::prod(mat, vecT);
		vecHelix -= vecT;

		vec += vecHelix;
	}

	return vec;
}
*/

int draw(mglGraph *gr)
{
	const int iCnt = 16;
	mglData datx, daty, datz;

	datx.Create(iCnt, iCnt, iCnt);
	daty.Create(iCnt, iCnt, iCnt);
	datz.Create(iCnt, iCnt, iCnt);

	double r = 1;
	double c = 0.05;
	const double dScale = 1;

	for(int iZ=0; iZ<iCnt; ++iZ)
	{
		double dT = double(iZ)/double(iCnt-1) * 2.*M_PI - M_PI;
		ublas::vector<double> vec = helix_vec(r, c, dT);

		for(int iY=0; iY<iCnt; ++iY)
		{
			double dY = double(iY)/double(iCnt-1);

			for(int iX=0; iX<iCnt; ++iX)
			{
				double dX = double(iX)/double(iCnt-1);

				int iIdx = iX + iY*iCnt + iZ*iCnt*iCnt;
				datx.a[iIdx] = dScale*vec[0];
				daty.a[iIdx] = dScale*vec[1];
				datz.a[iIdx] = dScale*vec[2];
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
