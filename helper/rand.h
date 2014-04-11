/*
 * random numbers
 * @author tweber
 * @date 16-aug-2013
 */

#ifndef __M_RAND_H__
#define __M_RAND_H__

extern void init_rand();
extern void init_rand_seed(unsigned int uiSeed);

extern unsigned int simple_rand(unsigned int iMax);

extern double rand_int(int iMin, int iMax);
extern double rand_double(double dMin, double dMax);
extern double rand01();

extern double rand_norm(double dMu, double dSigma);
extern int rand_poisson(double dMu);

#endif
