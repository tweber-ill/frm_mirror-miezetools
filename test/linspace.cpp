#include "../helper/math.h"

int main()
{
	std::vector<double> vec0 = linspace<double>(1.,2.,10);
	std::vector<double> vec1 = logspace<double>(1.,2.,10);

	for(double d: vec0)
		std::cout << d << ", ";
	std::cout << std::endl;

	for(double d: vec1)
		std::cout << d << ", ";
	std::cout << std::endl;

	return 0;
}
