/*
 * Sample reduction calculation to investigate the sign problem
 * @author tweber
 *
 * gcc -O2 -march=native -DNDEBUG -o sred sred.cpp -lstdc++ -std=c++11 -lm -fopenmp
 */

#include <thread>
#include <boost/units/io.hpp>

#include "../helper/math.h"
#include "../helper/mieze.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

typedef units::quantity<units::si::time> t_time;
typedef units::quantity<units::si::length> length;
typedef units::quantity<units::si::plane_angle> angle;
typedef units::quantity<units::si::frequency> freq;

void calc_red(double* pdRed, const length& w, const length& h, const length& d,
		const freq& fm, const length& lam, const angle& twotheta,
		const angle& theta_s)
{
	double dRed = std::fabs(mieze_reduction_sample_cuboid(w, h, d, fm, lam, twotheta, theta_s));
	//std::cout << dRed << std::endl;
	*pdRed = dRed;
}

int main(int argc, char** argv) {
	const t_time sec = 1. * units::si::second;
	const length meter = 1. * units::si::meter;
	const angle rad = 1. * units::si::radian;
	const freq Hz = 1. / sec;

	length lam = 10e-10 * meter;
	length L1 = 15. * meter;
	length L2 = 15. * meter;
	length Lb = 5. * meter;
	length w = 0.01 * meter;
	length h = 0.01 * meter;
	length d = 0.001 * meter;

	std::vector<t_time> vecTaus = linspace<t_time>(0.01e-9 * sec, 30e-9 * sec, 128);
	std::vector<angle> vecTT = linspace<angle>(0. * rad, (M_PI / 2. + M_PI / 4.) * rad, 128);

	std::ofstream ofstr("sred_fix.dat");
	std::ofstream ofstr_m("sred_minus.dat");
	std::ofstream ofstr_p("sred_plus.dat");

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
	ofstr_m << strHeaders;
	ofstr_p << strHeaders;

	unsigned int iSize = vecTaus.size() * vecTT.size();
	unsigned int iCur = 0;
	for (const t_time& tau : vecTaus) {
		for (const angle& twotheta : vecTT) {
			std::cout << "\rPoint " << ++iCur << " of " << iSize << ": "
					<< "tau=" << tau << ", twotheta=" << twotheta << ", ";
			std::cout.flush();

			freq fm = ::mieze_tau_fm(tau, L2 - Lb, lam);

			// fixed
			angle theta_s_fix = 0. * rad;
			// rot +
			angle theta_s_p = twotheta / 2.;
			// rot -
			angle theta_s_m = -twotheta / 2.;

			double dRed_fix=0., dRed_m=0., dRed_p=0.;

			//dRed_fix = std::fabs(mieze_reduction_sample_cuboid(w,h,d, fm, lam, twotheta, theta_s_fix));
			//dRed_p = std::fabs(mieze_reduction_sample_cuboid(w,h,d, fm, lam, twotheta, theta_s_p));
			//dRed_m = std::fabs(mieze_reduction_sample_cuboid(w,h,d, fm, lam, twotheta, theta_s_m));

			std::thread th_fix(calc_red, &dRed_fix, w, h, d, fm, lam, twotheta, theta_s_fix);
			std::thread th_p(calc_red, &dRed_p, w, h, d, fm, lam, twotheta, theta_s_p);
			std::thread th_m(calc_red, &dRed_m, w, h, d, fm, lam, twotheta, theta_s_m);

			th_fix.join();
			th_p.join();
			th_m.join();

			std::cout << "red_fix=" << dRed_fix << "        ";
			ofstr << dRed_fix << " ";
			ofstr_p << dRed_p << " ";
			ofstr_m << dRed_m << " ";

			ofstr.flush();
			ofstr_p.flush();
			ofstr_m.flush();
		}
		ofstr << "\n";
		ofstr_m << "\n";
		ofstr_p << "\n";
	}

	std::cout << std::endl;
	return 0;
}
