/**
 * Helper functions for mcstas
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 4-nov-14
 */

#ifndef __NEUTRONS_H__
#define __NEUTRONS_H__

#include <sys/time.h>


#define GAMMA_N 1.83247185e+08           /* 1/(Ts), value from: boost/units/systems/si/codata/neutron_constants.hpp */


double neutron_v(double dLam)
{
	return HBAR*2.*M_PI / (dLam*1e-10 * MNEUTRON);
}

double neutron_k(double dLam)
{
	return 2.*M_PI / dLam;
}

double larmor_B(double dv, double dl, double dPhase)
{
	/* omega = -gamma*B
	 * omega*t = -gamma*B*t
	 * phi = - gamma * B * l/v
	 * B = -phi*v / (gamma*l)
	 * phi = -pi  =>  B = pi*v / (gamma*l)
	 */

	return -dPhase*dv / (GAMMA_N*dl);
}


void rf_flipper(double x, double y, double z, double t, double dOm, double dB_rf, double dB0,
		double *pBx, double *pBy, double *pBz)
{
	*pBx = dB_rf*cos(dOm*t);
	*pBy = dB0;
	*pBz = dB_rf*sin(dOm*t);
}

void rf_flipper_rot(double x, double y, double z, double t, double dOm, double dB_rf, double dB0,
		double *pBx, double *pBy, double *pBz)
{
	*pBx = dB_rf*cos(-dOm*t);
	*pBy = dB_rf*sin(-dOm*t);
	*pBz = dB0;
}

void rf_rot_xy(double x, double y, double z, double t, double dOm, double dB_rf,
		double *pBx, double *pBy, double *pBz)
{
	*pBx = dB_rf*cos(-dOm*t);
	*pBy = dB_rf*sin(-dOm*t);
	*pBz = 0.;
}

double fuzzy_border(double x, double amp/*=1.*/, double const_size/*=1.5*/, double x0/*=0.*/, double slope_scale/*=1.*/)
{
	x -= x0;
	double(*fkt)(double) = tanh;

	if(x<-const_size/2.)
		return fkt(x*slope_scale + 3. + const_size/2.) * amp*0.5 + amp*0.5;
	else if(x>=-const_size/2. && x<=const_size/2.)
		return amp;
	else
		return fkt(-x*slope_scale + 3. + const_size/2.) * amp*0.5 + amp*0.5;
}

double triangle_border(double x, double z, double xw, double zl)
{
	if(x < -0.5*xw || x > 0.5*xw)
		return 0.;

	double dXPos = x + 0.5*xw;
	double dZLen = dXPos/xw * zl;

	if(z < -0.5*dZLen || z > 0.5*dZLen)
		return 0.;

	return 1.;
}

double grad_field(double x, double xw)
{
	if(x < -0.5*xw || x > 0.5*xw)
		return 0.;

	double dXPos = x + 0.5*xw;
	return dXPos / xw;
}


void random_seed()
{
	struct timeval tv;
	gettimeofday(&tv, 0);

	mcseed = tv.tv_sec ^ tv.tv_usec;
#ifdef USE_MPI
	mcseed += mpi_node_rank;
#endif

	srandom(mcseed);
	printf("Seed: %li\n", mcseed);
}

#endif
