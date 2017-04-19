/**
 * Sample reduction calculation with extinction (for HOPG analyser contrast reduction)
 * @author Tobias Weber <tobias.weber@tum.de>
 * @license GPLv3
 *
 * gcc -O2 -march=native -DNDEBUG -o sred-ext sred-ext.cpp -lstdc++ -std=c++11 -lm -fopenmp
 */


#include <thread>
#include <boost/units/io.hpp>

#include "../../helper/math.h"
#include "../../helper/mieze.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

typedef units::quantity<units::si::time> t_time;
typedef units::quantity<units::si::length> length;
typedef units::quantity<units::si::plane_angle> angle;
typedef units::quantity<units::si::frequency> freq;

int main(int argc, char** argv)
{
	const t_time sec = 1. * units::si::second;
	const length meter = 1. * units::si::meter;
	const length cm = meter / 100.;
	const angle rad = 1. * units::si::radian;
	const freq Hz = 1. / sec;

	length lam = 10e-10 * meter;
	length L1 = 15. * meter;
	length L2 = 15. * meter;
	length Lb = 5. * meter;
	length w = 0.01 * meter;
	length h = 0.01 * meter;
	length d = 0.005 * meter;

	length a = 2.464e-10 * meter;		// McStas graphite file: C_graphite.lau
	length c = 6.711e-10 * meter;

	auto sig_ext = (5.551e-24 + 0.0035e-24) *cm*cm;	// Furrer Appendix B: sigmas for C
	auto vol_uc = std::sqrt(3.)/2. * a*a*c;	// Furrer Appendix E: hexagonal unit cell volume

	//double dNumUnitCells = w*h*d / vol_uc;
	//std::cout << "Number of unit cells in sample: " << dNumUnitCells << std::endl;

	double dAtomsPerUnitCell = 4.;
	double dNumUnitCells = 1.*meter*meter*meter / vol_uc;
	std::cout << "Number of unit cells per m^3: " << dNumUnitCells << std::endl;

	auto Sig_ext = sig_ext * dAtomsPerUnitCell/vol_uc;
	auto mu = Sig_ext;
	std::cout << "Macroscopic extinction cross-section: " << mu << std::endl;
	std::cout << "Mean free path (length for 1/e intensity): " /*ca. 1 cm*/ << (1./mu) << std::endl;
	std::cout << "\n";

	std::vector<t_time> vecTaus = linspace<t_time>(0.01e-9 * sec, 10e-9 * sec, 8);
	std::vector<angle> vecTT = linspace<angle>(0.01 * rad, M_PI / 4. * rad, 8);

	std::ofstream ofstr("sred_ext.dat");

	std::ostringstream ostrHeaders;
	ostrHeaders << "# type: array_2d\n";
	ostrHeaders << "# subtype: tobisown\n";
	ostrHeaders << "# mieze-content: contrast\n";
	ostrHeaders << "# xlabel: 2theta [deg]\n";
	ostrHeaders << "# ylabel: tau [ns]\n";
	ostrHeaders << "# zlabel: reduction\n";
	ostrHeaders << "# xylog: 0 0\n";
	ostrHeaders << "# xylimits: " << double(*vecTT.begin()/rad)/M_PI*180. << " " << double(*vecTT.rbegin()/rad)/M_PI*180.
								<< " " << double(*vecTaus.begin()/sec) * 1e9 << " "
								<< double(*vecTaus.rbegin()/sec) * 1e9 << " 0 1\n";
	std::string strHeaders = ostrHeaders.str();
	ofstr << strHeaders;

	unsigned int iSize = vecTaus.size() * vecTT.size();
	unsigned int iCur = 0;
	for (const t_time& tau : vecTaus)
	{
		for (const angle& twotheta : vecTT)
		{
			std::cout << "\rPoint " << ++iCur << " of " << iSize << ": "
					<< "tau=" << tau << ", twotheta=" << twotheta << ", ";
			std::cout.flush();

			freq fm = ::mieze_tau_fm(tau, L2 - Lb, lam);
			double dRed = std::fabs(mieze_reduction_sample_cuboid_extinction(w,h,d, mu, fm, lam, twotheta, 32));

			std::cout << "C = " << dRed << "        ";
			ofstr << dRed << " ";

			ofstr.flush();
		}
		ofstr << "\n";
	}

	std::cout << std::endl;
	return 0;
}
