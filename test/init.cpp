#include "../helper/linalg.h"

int main()
{
	ublas::vector<double> vec = make_vec<double>({1.4, 5, 1.11});
	std::cout << vec << std::endl;

	ublas::matrix<double> mat = make_mat<double>({{1., 2., 3.}, {4., 5., 6.}});
	std::cout << mat << std::endl;
}
