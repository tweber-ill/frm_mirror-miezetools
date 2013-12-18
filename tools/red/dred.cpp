/*
 * C reduction due to detector thickness
 * @author tweber
 *
 * gcc -O2 -march=native -DNDEBUG -o dred dred.cpp ../../helper/fourier.cpp -lstdc++ -std=c++11 -lm -lfftw3
 */

#include <thread>
#include <boost/units/io.hpp>

#include "../../helper/math.h"
#include "../../helper/mieze.hpp"
#include "../../helper/fourier.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

typedef units::quantity<units::si::time> t_time;
typedef units::quantity<units::si::length> length;
typedef units::quantity<units::si::plane_angle> angle;
typedef units::quantity<units::si::frequency> freq;

double mieze_reduction_det_d2(const length& d, const freq& fm, const length& lam)
{
	const double SUBDIV = 100.;

	auto vel = lam2p(lam)/co::m_n;
	freq omM = 2.*M_PI*fm;
	const length dl = d/SUBDIV;

	const int NUM = 32;
	double dSig[NUM];
	for(int i=0; i<NUM; ++i) dSig[i] = 0.;

	for(length len=-d/2.; len<d/2.; len+=dl)
	{
		double dPhase = omM*len/vel;
		for(int i=0; i<NUM; ++i)
			dSig[i] += std::sin(2.*M_PI/double(NUM)*double(i) + dPhase) + 1.;
	}

	for(int i=0; i<NUM; ++i)
		dSig[i] /= SUBDIV;

/*	std::ofstream ofstrDebug("dred_debug.dat");
	for(int i=0; i<NUM; ++i)
		ofstrDebug << i << " " << dSig[i] << "\n";
	ofstrDebug.flush();
	exit(0);*/

	double dC=0., dPh=0.;
	Fourier fourier(NUM);
	fourier.get_contrast(1., dSig, dC, dPh);

	return dC;
}

int main(int argc, char** argv)
{
	const t_time sec = 1. * units::si::second;
	const t_time ps = 1e-12 * units::si::second;
	const t_time ns = 1e-9 * units::si::second;
	const length meter = 1. * units::si::meter;
	const length cm = meter / 100.;
	const angle rad = 1. * units::si::radian;
	const freq Hz = 1. / sec;

	length lam = 15e-10 * meter;
	length L1 = 16. * meter;
	length L2 = 16. * meter;
	length Lb = 3. * meter;
	length d = 2e-6 * meter;

	std::vector<t_time> vecTaus = linspace<t_time>(0.01e-9 * sec, 10e-6 * sec, 128);

	std::ofstream ofstr("dred.dat");
	std::ofstream ofstr2("dred2.dat");

	std::ostringstream ostrHeaders;
	ostrHeaders << "# type: array_1d\n";
	ostrHeaders << "# subtype: tobisown\n";
	ostrHeaders << "# mieze-content: contrast\n";
	ostrHeaders << "# xlabel: tau [ns]\n";
	ostrHeaders << "# ylabel: Contrast\n";
	ostrHeaders << "# xylog: 0 0\n";
	ostrHeaders << "# xylimits: " << "0 1 "
		<< double(*vecTaus.begin()/sec) * 1e9 << " "
		<< double(*vecTaus.rbegin()/sec) * 1e9 
		<< " 0 1\n";
	std::string strHeaders = ostrHeaders.str();
	ofstr << strHeaders;
	ofstr2 << strHeaders;

	unsigned int iSize = vecTaus.size();
	unsigned int iCur = 0;
	for (const t_time& tau : vecTaus)
	{
		std::cout << "\rPoint " << ++iCur << " of " << iSize << ": "
				<< "tau=" << tau << ", ";
		std::cout.flush();

		freq fm = ::mieze_tau_fm(tau, L2 - Lb, lam);
		double dRed = std::fabs(mieze_reduction_det_d(d, fm, lam));
		double dRed2 = std::fabs(mieze_reduction_det_d2(d, fm, lam));

		std::cout << "C = " << dRed;
		std::cout << ", C2 = " << dRed2 << "                ";

		ofstr << double(tau/ns) << " " << dRed << " ";
		ofstr2 << double(tau/ns) << " " << dRed2 << " ";

		ofstr << "\n";
		ofstr2 << "\n";
	}

	std::cout << std::endl;
	return 0;
}
