/*
 * FFT & DFT routines
 *
 * @author Tobias Weber <tweber@frm2.tum.de>
 * @date August 2012
 */

#ifndef __MIEZE_FOURIER__
#define __MIEZE_FOURIER__

#include "../tlibs/math/fourier.h"


//------------------------------------------------------------------------------
// algorithms
// perform a zero-order phase correction;
// for a description (albeit in the context of NMR) see e.g. here:
// http://www-keeler.ch.cam.ac.uk/lectures/Irvine/chapter4.pdf
template<typename T>
std::complex<T> phase_correction_0(const std::complex<T>& c, T dPhase)
{
	return c * std::complex<T>(cos(-dPhase), sin(-dPhase));
}

// perform a first-order phase correction:
// dPhase = dPhaseOffs + x*dPhaseSlope
template<typename T>
std::complex<T> phase_correction_1(const std::complex<T>& c,
				T dPhaseOffs, T dPhaseSlope, T x)
{
	return phase_correction_0<T>(c, dPhaseOffs + x*dPhaseSlope);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
class MFourier : public tl::Fourier<double>
{
	public:
		MFourier(unsigned int iSize);
		virtual ~MFourier();

		// shift a sine given in pDatIn by dPhase
		// we cannot phase shift a sine directly in the time domain due to
		// binning constraints; but in the frequency domain the phase is
		// a continuous variable which we can arbitrarily change and then
		// transform the data back into the time domain to get a shifted rebinning
		bool shift_sin(double dNumOsc, const double* pDatIn,
						double *pDataOut, double dPhase);

		bool phase_correction_0(const double* pDatIn, double *pDataOut,
								double dPhase);

		bool phase_correction_1(const double* pDatIn, double *pDataOut,
								double dPhaseOffs, double dPhaseSlope);

		bool get_contrast(double dNumOsc, const double* pDatIn,
						  double& dC, double& dPh);
};
//------------------------------------------------------------------------------


#endif
