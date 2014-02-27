#include <cmath>
#include <boost/math/quaternion.hpp>

using namespace boost::math;


template<typename T>
T quat_angle(const quaternion<T>& q1, const quaternion<T>& q2)
{
	T dot = q1.R_component_1()*q2.R_component_1() +
		q1.R_component_2()*q2.R_component_2() +
		q1.R_component_3()*q2.R_component_3() +
		q1.R_component_4()*q2.R_component_4();

	dot /= norm(q1);
	dot /= norm(q2);

	return std::acos(dot);
}

// see: http://run.usc.edu/cs520-s12/assign2/p245-shoemake.pdf
template<typename T>
quaternion<T> slerp(const quaternion<T>& q1, const quaternion<T>& q2, double t)
{
	T angle = quat_angle(q1, q2);

	quaternion<T> q = std::sin((1-t)*angle)/std::sin(angle) * q1 +
			std::sin(t*angle)/std::sin(angle) * q2;

	return q;
}


int main()
{
	quaternion<double> q1(1.,1.,1.,1.);
	quaternion<double> q2(1.,2.,2.,2.);

	quaternion<double> q = slerp(q1, q2, 1.);
	std::cout << q << std::endl;

	return 0;
}
