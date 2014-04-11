/*
 * random numbers
 * @author tweber
 * @date 16-aug-2013
 */

#include "rand.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <random>


//static std::default_random_engine g_eng;
static std::mt19937/*_64*/ g_eng;

void init_rand()
{
	// seed for C random functions
	//unsigned int uiSeed = time(NULL);
    struct timeval timev;
    gettimeofday(&timev, 0);
	unsigned int uiSeed = timev.tv_usec;


	init_rand_seed(uiSeed);
}

void init_rand_seed(unsigned int uiSeed)
{
	srand(uiSeed);
	g_eng = std::mt19937/*_64*/(uiSeed);
}

unsigned int simple_rand(unsigned int iMax)
{
	return rand() % iMax;
}


double rand_int(int iMin, int iMax)
{
	std::uniform_int_distribution<int> dist(iMin, iMax);
	return dist(g_eng);
}

double rand_double(double dMin, double dMax)
{
	std::uniform_real_distribution<double> dist(dMin, dMax);
	return dist(g_eng);
}

double rand01()
{
	return rand_double(0., 1.);
}


double rand_norm(double dMu, double dSigma)
{
	std::normal_distribution<double> dist(dMu, dSigma);
	return dist(g_eng);
}

int rand_poisson(double dMu)
{
	std::poisson_distribution<int> dist(dMu);
	return dist(g_eng);
}
