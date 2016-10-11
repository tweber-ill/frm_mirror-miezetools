#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>

#include <mgl2/qt.h>


typedef double t_real;
typedef ublas::vector<t_real> t_vec;
typedef ublas::matrix<t_real> t_mat;

t_mat rotx(t_real angle)
{
    t_real s, c;
	s = std::sin(angle);
	c = std::cos(angle);

	t_mat mat(3,3);
	mat(0,0) = 1;	mat(0,1) = 0;	mat(0,2) = 0;
	mat(1,0) = 0;	mat(1,1) = c;	mat(1,2) = -s;
	mat(2,0) = 0;	mat(2,1) = s;	mat(2,2) = c;

	return mat;
}

t_vec helix_vec(t_real r, t_real c, const t_vec& vecCoord)
{
	std::vector<t_mat> matrices;
	std::vector<t_mat> inv_matrices;

	const t_real dAngles[3] = {0., 120., 240.};

	for(int i=0; i<sizeof(dAngles)/sizeof(dAngles[0]); ++i)
	{
		t_real dAngle = dAngles[i];
		dAngle = dAngle/180.*M_PI;

		t_mat matRotNeg = rotx(-dAngle);
		t_mat matRotPos = rotx(dAngle);

		matrices.push_back(matRotPos);
		inv_matrices.push_back(matRotNeg);
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
	const int iCntX = 12;
	const int iCntY = 64;
	const int iCntZ = 64;

	mglData datx, daty, datz;

	datx.Create(iCntX, iCntY, iCntZ);
	daty.Create(iCntX, iCntY, iCntZ);
	datz.Create(iCntX, iCntY, iCntZ);

	t_real r = 1.;
	t_real c = 1.;

	const t_real dTScale = 4.;
	const t_real dXScale = 1.;
	const t_real dYScale = 32.;

	const t_real dScale2 = 0.3;


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

				t_vec vec = helix_vec(r, c, vecCoord);

				int iIdx = iX + iY*iCntX + iZ*iCntX*iCntY;
				datx.a[iIdx] = dScale2 * vec[0];
				daty.a[iIdx] = dScale2 * vec[1];
				datz.a[iIdx] = dScale2 * vec[2];
			}
		}
	}

	gr->Zoom(0.45,0.45, 0.65,0.65);
	gr->Rotate(0, 45, -55);
	//gr->Rotate(0, 0, 90);
	gr->Vect3(datx, daty, datz, "fx");	// x slice

	gr->WritePNG("helix.png");

	return 0;
}


// gcc -march=native -O2 -o helix3 helix3.cpp -lstdc++ -lm -lmgl-qt -lmgl
int main(int argc, char **argv)
{
	mglGraph graph(0, 800, 800);
	draw(&graph);

	return 0;
}
