/*
 * FFT & DFT routines
 *
 * @author Tobias Weber <tweber@frm2.tum.de>
 * @date August 2012
 * @license GPLv3
 */

#include "mfourier.h"

MFourier::MFourier(unsigned int iSize) : tl::Fourier<double>(iSize)
{}

MFourier::~MFourier()
{}

bool MFourier::shift_sin(double dNumOsc, const double* pDatIn,
				double *pDataOut, double dPhase)
{
	unsigned int iSize = m_iSize;
	const int iNumOsc = int(dNumOsc);
	dNumOsc = double(iNumOsc);			// consider only full oscillations

	//double dShiftSamples = dPhase/(2.*M_PI) * dSize;

	std::unique_ptr<double[]> dMem(new double[3*iSize]);
	double *pdMem = dMem.get();
	double *pZero = pdMem;
	double *pDatFFT_real = pdMem + iSize;
	double *pDatFFT_imag = pdMem + 2*iSize;

	memset(pZero, 0, sizeof(double)*iSize);

	fft(pDatIn, pZero, pDatFFT_real, pDatFFT_imag);

	// filter out everything not concerning the sine with iNumOsc oscillations
	for(unsigned int i=0; i<iSize; ++i)
	{
		if(int(i)==iNumOsc || i==0)
			continue;

		pDatFFT_real[i] = 0.;
		pDatFFT_imag[i] = 0.;
	}

	// amp & phase
	std::complex<double> c(pDatFFT_real[iNumOsc], pDatFFT_imag[iNumOsc]);

	// since the signal is real we can take the first half of the fft data
	// and multiply by two.
	c *= 2.;
	c = ::phase_correction_0<double>(c, dPhase);

	pDatFFT_real[iNumOsc] = c.real();
	pDatFFT_imag[iNumOsc] = c.imag();

	// offset
	pDatFFT_imag[0] = 0.;


	ifft(pDatFFT_real, pDatFFT_imag, pDataOut, pZero);

	// normalization
	for(unsigned int i=0; i<iSize; ++i)
		pDataOut[i] /= double(iSize);

	//save_dat("in.dat", pDatIn, iSize);
	//save_dat("out.dat", pDataOut, iSize);
	return true;
}

bool MFourier::phase_correction_0(const double* pDatIn, double *pDataOut,
				double dPhase)
{
	unsigned int iSize = m_iSize;

	std::unique_ptr<double[]> dMem(new double[3*iSize]);
	double *pdMem = dMem.get();
	double *pZero = pdMem;
	double *pDatFFT_real = pdMem + iSize;
	double *pDatFFT_imag = pdMem + 2*iSize;

	memset(pZero, 0, sizeof(double)*iSize);

	fft(pDatIn, pZero, pDatFFT_real, pDatFFT_imag);

	for(unsigned int i=1; i<iSize; ++i)
	{
		std::complex<double> c(pDatFFT_real[i], pDatFFT_imag[i]);
		if(i<iSize/2)
		{
			c *= 2.;
			c = ::phase_correction_0<double>(c, dPhase*double(i));
		}
		else
		{
			// not needed in real input data
			c = std::complex<double>(0., 0.);
		}

		pDatFFT_real[i] = c.real();
		pDatFFT_imag[i] = c.imag();
	}

	ifft(pDatFFT_real, pDatFFT_imag, pDataOut, pZero);

	// normalization
	for(unsigned int i=0; i<iSize; ++i)
		pDataOut[i] /= double(iSize);

	//save_dat("in.dat", pDatIn, iSize);
	//save_dat("out.dat", pDataOut, iSize);
	return true;
}

bool MFourier::phase_correction_1(const double* pDatIn,
				double *pDataOut, double dPhaseOffs, double dPhaseSlope)
{
	unsigned int iSize = m_iSize;

	std::unique_ptr<double[]> dMem(new double[3*iSize]);
	double *pdMem = dMem.get();
	double *pZero = pdMem;
	double *pDatFFT_real = pdMem + iSize;
	double *pDatFFT_imag = pdMem + 2*iSize;

	memset(pZero, 0, sizeof(double)*iSize);

	fft(pDatIn, pZero, pDatFFT_real, pDatFFT_imag);

	for(unsigned int i=1; i<iSize; ++i)
	{
		std::complex<double> c(pDatFFT_real[i], pDatFFT_imag[i]);
		if(i<iSize/2)
		{
			double dX = double(i)/double(iSize);

			c *= 2.;
			c = ::phase_correction_1<double>(c,
							dPhaseOffs*double(i), dPhaseSlope*double(i),
							dX);
		}
		else
		{
			// not needed in real input data
			c = std::complex<double>(0., 0.);
		}

		pDatFFT_real[i] = c.real();
		pDatFFT_imag[i] = c.imag();
	}

	ifft(pDatFFT_real, pDatFFT_imag, pDataOut, pZero);

	// normalization
	for(unsigned int i=0; i<iSize; ++i)
		pDataOut[i] /= double(iSize);

	//save_dat("in.dat", pDatIn, iSize);
	//save_dat("out.dat", pDataOut, iSize);
	return true;
}


bool MFourier::get_contrast(double dNumOsc, const double* pDatIn,
				   double& dC, double& dPh)
{
	unsigned int iSize = m_iSize;
	const int iNumOsc = int(dNumOsc);
	dNumOsc = double(iNumOsc);			// consider only full oscillations

	std::unique_ptr<double[]> dMem(new double[3*iSize]);
	double *pdMem = dMem.get();
	double *pZero = pdMem;
	double *pDatFFT_real = pdMem + iSize;
	double *pDatFFT_imag = pdMem + 2*iSize;

	memset(pZero, 0, sizeof(double)*iSize);

	fft(pDatIn, pZero, pDatFFT_real, pDatFFT_imag);

	double dReal = 2.*pDatFFT_real[iNumOsc]/double(m_iSize);
	double dImag = 2.*pDatFFT_imag[iNumOsc]/double(m_iSize);

	double dAmp = sqrt(dReal*dReal + dImag*dImag);
	double dOffs = pDatFFT_real[0] / double(m_iSize);

	//std::cout << "amp = " << dAmp << std::endl;
	//std::cout << "offs = " << dOffs << std::endl;

	dC = dAmp/dOffs;
	dPh = atan2(dImag, dReal) + M_PI/2.;

	// half a bin
	dPh -= 2.*M_PI*dNumOsc*0.5/double(m_iSize);

	// half of first bin
	//dPh += - 0.5/dNumOsc * 2.*M_PI;

	if(dPh<0.)
		dPh += 2.*M_PI;

	return true;
}
