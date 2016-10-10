#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>

#include <mgl2/qt.h>

#include "../../tlibs/math/linalg.h"
using namespace tl;

using t_real = double;
using t_vec = ublas::vector<double>;
using t_mat = ublas::matrix<double>;


t_vec helix_vec(t_real r, t_real c, const t_vec& vecCoord, t_real dAngleScale=1.)
{
	static std::vector<t_mat > matrices;
	static std::vector<t_mat > inv_matrices;

	static bool bMatricesInited = 0;
	static t_real dLastAngleScale = 1.;

	if(dAngleScale != dLastAngleScale)
	{
		bMatricesInited = 0;
		dLastAngleScale = dAngleScale;
	}

	if(!bMatricesInited)
	{
		matrices.clear();
		inv_matrices.clear();

		const t_real dAngles[3] = {0., dAngleScale*120., dAngleScale*240.};

		for(t_real dAngle : dAngles)
		{
			dAngle = dAngle/180.*M_PI;

			t_mat matRotNeg = rotation_matrix_3d_x<t_mat>(-dAngle);
			t_mat matRotPos = rotation_matrix_3d_x<t_mat>(dAngle);

			matrices.push_back(matRotPos);
			inv_matrices.push_back(matRotNeg);
		}

		bMatricesInited = 1;
	}


	t_vec vecRet = ublas::zero_vector<t_real>(3);

	for(int iMatrix=0; iMatrix<matrices.size(); ++iMatrix)
	{
		const t_mat& matRotNeg = inv_matrices[iMatrix];
		const t_mat& matRotPos = matrices[iMatrix];

		t_vec vecCoordRot = ublas::prod(matRotNeg, vecCoord);

		t_vec vec(3);

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

	t_real r = 1.;
	t_real c = 1.;

	const t_real dTScale = 4.;
	const t_real dXScale = 1.;
	const t_real dYScale = 16.;

	const t_real dScale2 = 0.15;

	static t_real dAngleScale = 0.;
	std::cout << "angle scale: " << dAngleScale << std::endl;


	for(int iZ=0; iZ<iCntZ; ++iZ)
	{
		t_real dT = t_real(iZ)/t_real(iCntZ-1) * 2.*M_PI;

		for(int iY=0; iY<iCntY; ++iY)
		{
			t_real dY = t_real(iY)/t_real(iCntY-1);

			for(int iX=0; iX<iCntX; ++iX)
			{
				t_real dX = t_real(iX)/t_real(iCntX-1);

				t_vec vecCoord(3);
				vecCoord[0] = dXScale*dX;
				vecCoord[1] = dYScale*dY;
				vecCoord[2] = dTScale*dT;

				t_vec vec = helix_vec(r, c, vecCoord, dAngleScale);

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
