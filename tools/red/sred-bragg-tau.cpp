/*
 * Sample reduction calculation with extinction (for HOPG analyser contrast reduction)
 * @author tweber
 *
 * gcc -O2 -march=native -DNDEBUG -o sred-bragg-tau sred-bragg-tau.cpp -lstdc++ -std=c++11 -lm -fopenmp
 */

#include <thread>
#include <boost/units/io.hpp>

#include "../../helper/math.h"
#include "../../helper/mieze.hpp"

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
	const int NUM_THICK = 128;
	const int NUM_ITER = 64;

	const t_time sec = 1. * units::si::second;
	const t_time ns = 1e-9 * units::si::second;
	const t_time ps = 1e-12 * units::si::second;
	const length meter = 1. * units::si::meter;
	const length cm = meter / 100.;
	const length mm = meter / 1000.;
	const angle rad = 1. * units::si::radian;
	const freq Hz = 1. / sec;

	length lam = 4e-10 * meter;
	length L1 = 0.9 * meter;
	length L2 = 1.6 * meter;
	length Lb = 0.6 * meter;
	length w = 0.005 * meter;
	length h = 0.005 * meter;
	length d_ana = 3.355 * angstrom;

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

	std::vector<length> vecThick = linspace<length>(0.01 * mm, 10. * mm, NUM_THICK);

	std::ofstream ofstr("sred_bragg.dat");

	std::ostringstream ostrHeaders;
	ostrHeaders << "# type: array_1d\n";
	ostrHeaders << "# subtype: tobisown\n";
	ostrHeaders << "# xlabel: Thickness [mm]\n";
	ostrHeaders << "# ylabel: Reduction\n";
	ostrHeaders << "# xylog: 0 0\n";
	ostrHeaders << "# ylimits: 0 1\n";
	std::string strHeaders = ostrHeaders.str();
	ofstr << strHeaders;


	t_time tau = 25. * ps;
	for (const length& d : vecThick)
	{
		freq fm = ::mieze_tau_fm(tau, L2 - Lb, lam);
		double dRed = std::fabs(mieze_reduction_sample_cuboid_bragg(w,h,d, mu, fm, lam, d_ana, NUM_ITER));

		ofstr << double(d/mm) << " " << dRed << "\n";
		ofstr.flush();
	}

	return 0;
}
