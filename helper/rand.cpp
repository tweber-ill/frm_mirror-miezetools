/*
 * random numbers
 * @author tweber
 * @date 16-aug-2013
 */

#include "rand.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


std::mt19937/*_64*/ g_randeng;

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
	g_randeng = std::mt19937/*_64*/(uiSeed);
}

unsigned int simple_rand(unsigned int iMax)
{
	return rand() % iMax;
}
