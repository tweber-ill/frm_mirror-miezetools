/*
 * Helper functions for mcstas
 * @author tweber
 * @date 4-nov-14
 */

#ifndef __NEUTRONS_H__
#define __NEUTRONS_H__

#include <time.h>


#define GAMMA_N 1.83247185e+08           /* 1/(Ts), value from: boost/units/systems/si/codata/neutron_constants.hpp */


double neutron_v(double dLam)
{
	return HBAR*2.*M_PI / (dLam*1e-10 * MNEUTRON);
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

void random_seed()
{
	struct timeval tv;
	gettimeofday(&tv, 0);

	mcseed = tv.tv_usec;
#ifdef USE_MPI
	mcseed += mpi_node_rank;
#endif

	srandom(mcseed);
	printf("Seed: %li\n", mcseed);
}

#endif
