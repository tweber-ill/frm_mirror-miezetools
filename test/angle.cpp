#include "../helper/linalg.h"

int main()
{
	double dAngle = 12.3;

	ublas::vector<double> v0(3);
	ublas::vector<double> v1(3);

	v0[0] = 1.;
	v0[1] = 0.;
	v0[2] = 0.;

	v1[0] = cos(dAngle/180.*M_PI);
	v1[1] = sin(dAngle/180.*M_PI);
	v1[2] = 0.;

	ublas::vector<double> vbase0 = v0;
	ublas::vector<double> vbase1(3);
	vbase1[0] = 0.;
	vbase1[1] = 1.;
	vbase1[2] = 0.;
	ublas::vector<double> vecNorm = cross_3(vbase0, vbase1);
	std::cout << vec_angle(v0, v1, &vecNorm)/M_PI*180. << std::endl;

	return 0;
}
