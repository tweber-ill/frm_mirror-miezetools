#include "../helper/linalg.h"
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


static int g_iPhase = 0;

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

				ublas::vector<double> vec = helix_vec(r, c, vecCoord);

				int iIdx = iX + iY*iCntX + iZ*iCntX*iCntY;
				datx.a[iIdx] = dScale2 * vec[0];
				daty.a[iIdx] = dScale2 * vec[1];
				datz.a[iIdx] = dScale2 * vec[2];
			}
		}
	}


	static double dParam = 0.;
	static double dParam2 = 0.;
	static double dParam3 = 0.;
	
	gr->Zoom(0.15+dParam3, 0.15+dParam3, 0.85-dParam3, 0.85-dParam3);
	
	//gr->Rotate(0, 0, 90);
	gr->Rotate(0, 45-dParam2, 45+dParam);

	//gr->Vect(datx, daty, datz, "");
	gr->Vect3(datx, daty, datz, "fx");	

	if(g_iPhase == 0)
		dParam += 0.25;
	else if(g_iPhase == 1)
		dParam2 += 0.25;
	else if(g_iPhase == 2)
		dParam3 += 0.00125;


	static int iNum = -1;
	++iNum;
	std::ostringstream ostrName;
	ostrName << "frame_";

	ostrName.width(5);
	char cFill = ostrName.fill('0');
	ostrName <<  iNum;

	ostrName.fill(cFill);
	ostrName << ".jpg";
	
	std::cout << ostrName.str() << std::endl;
	gr->WriteJPEG(ostrName.str().c_str());


	return 0;
}


// gcc -march=native -O2 -o helix helix2.cpp -lstdc++ -lm -lmgl-qt -lmgl -std=c++11
// ffmpeg -r 25 -b 1800 -i frame_%05d.jpg  -q:v 0 -r 25 -ab 320k movie.mp4
int main(int argc, char **argv)
{
	/*
	mglQT *pMGL = new mglQT(draw);
	pMGL->Run();
	delete pMGL;
	*/

	g_iPhase = 0;
	for(int iFrame=0; iFrame<180; ++iFrame)
	{
		mglGraph graph(0, 1920, 1080);
		draw(&graph);
	}
	
	g_iPhase = 1;
	for(int iFrame=0; iFrame<180; ++iFrame)
	{
		mglGraph graph(0, 1920, 1080);
		draw(&graph);
	}	

	g_iPhase = 2;
	for(int iFrame=0; iFrame<120; ++iFrame)
	{
		mglGraph graph(0, 1920, 1080);
		draw(&graph);
	}	

	return 0;
}
