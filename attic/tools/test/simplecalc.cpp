#include "../helper/simplecalc.hpp"
#include <vector>

int main()
{
	auto angle = 180._deg + 3.14_rad;
	std::cout << angle << std::endl;

	auto lam = 2._A * sin(35._deg);
	std::cout << lam << std::endl;


	vec v({1.,2.,3.});
	std::cout << inner_prod(v,v) << std::endl;

	mat M(3, 3);
	M(0,0) = 1.; M(0,1) = 2.; M(0,2) = 2.;
	M(1,0) = 2.; M(1,1) = 1.; M(1,2) = 2.;
	M(2,0) = 3.; M(2,1) = 0.; M(2,2) = 2.;
	std::cout << prod(M,v) << std::endl;

	return 0;
}

