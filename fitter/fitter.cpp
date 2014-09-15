/*
 * fitter base class
 *
 * Author: Tobias Weber
 * Date: April 2012
 */

#include <cmath>
#include <limits>
#include <algorithm>
#include <sstream>

#include "../helper/misc.h"
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
