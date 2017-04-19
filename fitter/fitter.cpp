/**
 * fitter base class
 *
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date April 2012
 * @license GPLv3
 */

#include <cmath>
#include <limits>
#include <algorithm>
#include <sstream>

#include "../tlibs/helper/misc.h"
#include "fitter.h"


std::ostream& operator<<(std::ostream& ostr, const FunctionModel& fkt)
{
	ostr << fkt.print();
	return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const FunctionModel_nd& fkt)
{
	ostr << fkt.print();
	return ostr;
}

FunctionModel::~FunctionModel() {}
FunctionModel_nd::~FunctionModel_nd() {}

//----------------------------------------------------------------------
