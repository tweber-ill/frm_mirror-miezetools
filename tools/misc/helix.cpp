#include "../../helper/linalg.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>

#include <mgl2/qt.h>


ublas::vector<double> helix_vec(double r, double c, const ublas::vector<double>& vecCoord, double dAngleScale=1.)
{	
	static std::vector<ublas::matrix<double> > matrices;
	static std::vector<ublas::matrix<double> > inv_matrices;

	static bool bMatricesInited = 0;
	static double dLastAngleScale = 1.;

	if(dAngleScale != dLastAngleScale)
	{
		bMatricesInited = 0;
		dLastAngleScale = dAngleScale;
	}
	
	if(!bMatricesInited)
	{
		matrices.clear();
		inv_matrices.clear();

		const double dAngles[3] = {0., dAngleScale*120., dAngleScale*240.};
		
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
	const int iCntX = 8;
	const int iCntY = 96;
	const int iCntZ = 96;
	
	mglData datx, daty, datz;

	datx.Create(iCntX, iCntY, iCntZ);
	daty.Create(iCntX, iCntY, iCntZ);
	datz.Create(iCntX, iCntY, iCntZ);

	double r = 1.;
	double c = 1.;
	
	const double dTScale = 4.;
	const double dXScale = 1.;
	const double dYScale = 16.;
	
	const double dScale2 = 0.15;

	static double dAngleScale = 0.;
	std::cout << "angle scale: " << dAngleScale << std::endl;


	for(int iZ=0; iZ<iCntZ; ++iZ)
	{
		double dT = double(iZ)/double(iCntZ-1) * 2.*M_PI;

		for(int iY=0; iY<iCntY; ++iY)
		{
			double dY = double(iY)/double(iCntY-1);

			for(int iX=0; iX<iCntX; ++iX)
			{
				double dX = double(iX)/double(iCntX-1);

				ublas::vector<double> vecCoord(3);
				vecCoord[0] = dXScale*dX;
				vecCoord[1] = dYScale*dY;
				vecCoord[2] = dTScale*dT;

				ublas::vector<double> vec = helix_vec(r, c, vecCoord, dAngleScale);

				int iIdx = iX + iY*iCntX + iZ*iCntX*iCntY;
				datx.a[iIdx] = dScale2 * vec[0];
				daty.a[iIdx] = dScale2 * vec[1];
				datz.a[iIdx] = dScale2 * vec[2];
			}
		}
	}


	std::ostringstream ostrTitle;
	ostrTitle << "Helix angles: ";
	ostrTitle.width(3);
	ostrTitle << int(120.*dAngleScale);
	ostrTitle << " deg, ";
	ostrTitle.width(3);
	ostrTitle << int(240.*dAngleScale);
	ostrTitle << " deg";
	//gr->Title(ostrTitle.str().c_str());
	gr->Puts(0.5, 0.78, ostrTitle.str().c_str());



	gr->Zoom(0.15,0.15,0.85,0.85);

	//gr->Rotate(0, 0, 90);
	gr->Rotate(0, 45, -55);

	//gr->Vect(datx, daty, datz, "");
	gr->Vect3(datx, daty, datz, "fx");




	static int iNum = -1;
	++iNum;
	std::ostringstream ostrName;
	ostrName << "frame_";

	ostrName.width(5);
	char cFill = ostrName.fill('0');
	ostrName <<  iNum;

	ostrName.fill(cFill);
	ostrName << ".jpg";
	gr->WriteJPEG(ostrName.str().c_str());


	dAngleScale += 0.005;

	return 0;
}


// gcc -march=native -O2 -o helix helix.cpp -lstdc++ -lm -lmgl-qt -lmgl -std=c++11
// ffmpeg -r 10 -b 1800 -i frame_%05d.jpg  -q:v 0 -r 25 -ab 320k movie.mp4
int main(int argc, char **argv)
{
	/*
	mglQT *pMGL = new mglQT(draw);
	pMGL->Run();
	delete pMGL;
	*/

	for(int iFrame=0; iFrame<=200; ++iFrame)
	{
		mglGraph graph(0, 1920, 1080);
		draw(&graph);
	}

	return 0;
}
