/**
 * gcc -std=c++11 -o helix3_gpl helix3_gpl.cpp -lstdc++ -lm
 * @author tw
 * @license GPLv3
 */

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>


using t_real = double;
using t_vec = ublas::vector<t_real>;
using t_mat = ublas::matrix<t_real>;

t_mat rot(t_real angle)
{
	t_real s = std::sin(angle);
	t_real c = std::cos(angle);

	t_mat mat(3,3);
	mat(0,0) = c;	mat(0,1) = -s;	mat(0,2) = 0;
	mat(1,0) = s;	mat(1,1) = c;	mat(1,2) = 0;
	mat(2,0) = 0;	mat(2,1) = 0;	mat(2,2) = 1;
	return mat;
}

t_vec helix_vec(const t_vec& vecCoord)
{
	std::vector<t_mat> matrices;
	std::vector<t_mat> inv_matrices;

	for(t_real dAngle : {0., 120., 240.})
	{
		dAngle = dAngle/180.*M_PI;

		t_mat matRotNeg = rot(-dAngle);
		t_mat matRotPos = rot(dAngle);

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
		vec[2] = cos(vecCoordRot[0]);
		vec[1] = sin(vecCoordRot[0]);
		vec[0] = 0;

		vec = ublas::prod(matRotPos, vec);
		vecRet += vec;
	}

	vecRet /= ublas::norm_2(vecRet);
	return vecRet;
}


void calc()
{
	const int iCntX = 32, iCntY = 32;
	const t_real dXScale = 2., dYScale = 2.;
	const t_real dDirScale = 0.3;

	std::ofstream ofstr("/tmp/helix.dat");

	for(int iX=0; iX<iCntX; ++iX)
	{
		t_real dX = -M_PI + t_real(iX)/t_real(iCntX-1) * 2.*M_PI;

		for(int iY=0; iY<iCntY; ++iY)
		{
			t_real dY = -M_PI + t_real(iY)/t_real(iCntY-1) * 2.*M_PI;

			t_vec vecCoord(3);
			vecCoord[0] = dXScale*dX;
			vecCoord[1] = dYScale*dY;
			vecCoord[2] = 0.;

			t_vec vec = helix_vec(vecCoord)*dDirScale;

			ofstr << vecCoord[0] << " " << vecCoord[1] << " " << vecCoord[2] << "\t";
			ofstr << vec[0] << " " << vec[1] << " " << vec[2] << "\n";
			//ofstr << int(t_real(iX)/t_real(iCntX-1) * 255) << "\n";
		}
	}

	ofstr.flush();
}


void plot()
{
	const char* pcPlot = R"RAW(
set xrange [-2*pi : 2*pi]
set yrange [-2*pi : 2*pi]
set zrange [-1 : 1]

set xyplane 0
unset key

set xlabel "x (a.u.)"
set ylabel "y (a.u.)"
set zlabel "z (a.u.)"

farb(r,g,b) = ((int(r)&0xff)<<0x10000) | ((int(g)&0xff)<<0x100) | (int(b)&0xff)
splot "/tmp/helix.dat" using 1:2:3:4:5:6:(farb(0,0,($6+0.5)*255)) with vectors lc rgb variable lw 1.5 filled head
	)RAW";

	std::ofstream ofstr("/tmp/helix.gpl");
	ofstr << pcPlot;

	ofstr.flush();
	ofstr.close();

	std::system("gnuplot -p \"/tmp/helix.gpl\"");
}


int main(int argc, char **argv)
{
	calc();
	plot();

	return 0;
}

