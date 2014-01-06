#include "../helper/math.h"
#include "../helper/linalg.h"

int main()
{
	ublas::vector<double> line_0(3), line_dir(3);
	ublas::vector<double> plane_0(3), plane_dir0(3), plane_dir1(3);


	line_0[0] = 10.;
	line_0[1] = 0.;
	line_0[2] = 0.;

	line_dir[0] = -1.;
	line_dir[1] = 1.;
	line_dir[2] = 0.5;


	plane_0[0] = 0.;
	plane_0[1] = 0.;
	plane_0[2] = 0.;

	plane_dir0[0] = 0.;
	plane_dir0[1] = 1.;
	plane_dir0[2] = 0.;

	plane_dir1[0] = 0.;
	plane_dir1[1] = 0.;
	plane_dir1[2] = 1.;


	Plane<double> plane(plane_0, plane_dir0, plane_dir1);
	Line<double> line(line_0, line_dir);

	double d;
	line.intersect(plane, d);
	std::cout << line(d) << std::endl;

	return 0;
}
