/*
 * random numbers
 * @author tweber
 * @date 16-aug-2013
 */

#include "rand.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

void init_rand()
{
	//unsigned int uiSeed = time(NULL);

    struct timeval timev;
    gettimeofday(&timev, 0);
    unsigned int uiSeed = timev.tv_usec;

	srand(uiSeed);
}


unsigned int simple_rand(unsigned int iMax)
{
	return rand() % iMax;
}
