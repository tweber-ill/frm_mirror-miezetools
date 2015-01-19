#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdio>

// dummy defs
int mcseed = 0;
#define MNEUTRON 0
#define HBAR 0
void srandom(int) {}
#include "neutrons.h"

int main()
{
	for(double dX=-1.; dX<1.; dX+=0.1)
	{
		for(double dZ=-1.; dZ<1.; dZ+=0.1)
		{
			//double dI = triangle_border(-dX, dZ, 2., 2.);
			double dI = grad_field(-dX, 2.);
			std::cout << std::setw(4) << std::setprecision(4) << dI << " ";
		}
		std::cout << "\n";
	}
	std::cout.flush();

	return 0;
}
