/*
 * Helper functions for mcstas
 * @author tweber
 * @date 4-nov-14
 */

#ifndef __NEUTRONS_H__
#define __NEUTRONS_H__


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

#endif
