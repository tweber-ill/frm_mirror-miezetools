#include <cmath>
#include <boost/math/quaternion.hpp>
#include "../helper/math.h"
#include "../helper/linalg.h"

using namespace boost::math;

int main()
{
	/*
	quaternion<double> q1(1.,1.,1.,1.);
	quaternion<double> q2(1.,2.,2.,2.);

	quaternion<double> q = slerp(q1, q2, 1.);
	std::cout << q << std::endl;
	*/

	ublas::vector<double> v1(3);
	v1[0] = 0.; v1[1] = 1.; v1[2] = 0.;
	ublas::vector<double> v2(3);
	v2[0] = 0.; v2[1] = 0.; v2[2] = 1.;

	for(double d=0.; d<=1.; d+=0.1)
	{
		ublas::vector<double> v0 = lerp(v1, v2, d);
		ublas::vector<double> v = slerp(v1, v2, d);
		std::cout << "lerp: " << v0 << ", slerp: " << v << std::endl;
	}

	return 0;
}
