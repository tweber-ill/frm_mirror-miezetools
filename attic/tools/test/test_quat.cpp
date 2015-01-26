#include "../helper/linalg.h"
#include "../helper/math.h"

int main()
{
	ublas::matrix<double> rot(3,3);

	double dAngle = 1.2345;
	double dC = cos(dAngle);
	double dS = sin(dAngle);

	rot(0,0)=dC; rot(0,1)=-dS; rot(0,2)=0.;
	rot(1,0)=dS; rot(1,1)=dC; rot(1,2)=0.;
	rot(2,0)=0.; rot(2,1)=0.; rot(2,2)=1.;

	std::vector<double> angles = rotation_angle(rot);
	std::cout << angles[0] << ", " << angles[1] << ", " << angles[2] << std::endl;

	return 0;
}
