/*
 * Formula Stuff
 * @author: Tobias Weber
 * @date: May 2012, 16-jul-2013
 */

#include "formulas.h"
#include <map>

static std::map<std::string, double> g_mapPrefixes;

void init_formulas()
{
	g_mapPrefixes.clear();

	// prefixes
	g_mapPrefixes[""] = 1.;
	g_mapPrefixes["k"] = 1e3;
	g_mapPrefixes["M"] = 1e6;
	g_mapPrefixes["G"] = 1e9;
	g_mapPrefixes["T"] = 1e12;
	g_mapPrefixes["P"] = 1e15;
	g_mapPrefixes["E"] = 1e18;
	g_mapPrefixes["Z"] = 1e21;
	g_mapPrefixes["Y"] = 1e24;

	g_mapPrefixes["da"] = 1e1;	// deka
	g_mapPrefixes["h"] = 1e2;	// hecto


	g_mapPrefixes["m"] = 1e-3;
	g_mapPrefixes["u"] = 1e-6;
	g_mapPrefixes["n"] = 1e-9;
	g_mapPrefixes["p"] = 1e-12;
	g_mapPrefixes["f"] = 1e-15;
	g_mapPrefixes["a"] = 1e-18;
	g_mapPrefixes["z"] = 1e-21;
	g_mapPrefixes["y"] = 1e-24;

	g_mapPrefixes["d"] = 1e-1;	// deci
	g_mapPrefixes["c"] = 1e-2;	// centi
}


static bool ends_with(const std::string& str, const std::string& strEnd, double& dPow)
{
	std::size_t iLenStr = str.length();
	std::size_t iLenStrEnd = strEnd.length();

	if(iLenStrEnd > iLenStr)
		return false;

	std::string strPrefix = str.substr(0, iLenStr-iLenStrEnd);
	std::string strUnit = str.substr(iLenStr-iLenStrEnd);

	//std::cout << "prefix: " << strPrefix << ", unit: " << strUnit << std::endl;

	// str does not end with strEnd
	if(strUnit != strEnd)
		return false;

	std::map<std::string, double>::const_iterator iter =
												g_mapPrefixes.find(strPrefix);
	if(iter==g_mapPrefixes.end())
	{
		std::cerr << "Error: Unknown prefix \"" << strPrefix << "\"." << std::endl;
		dPow = 1.;
	}
	else
		dPow = (*iter).second;

	return str.substr(iLenStr-iLenStrEnd) == strEnd;
}

bool get_val(const std::string& str, double& dVal, std::string& strUnit)
{
	std::istringstream istr(str);
	istr >> dVal;
	istr >> strUnit;

	return strUnit != "";
}

units::quantity<units::si::frequency>
get_freq(const std::string& strVar)
{
	units::quantity<units::si::frequency> freq;

	double dVal = 0.;
	std::string strUnit;
	get_val(strVar, dVal, strUnit);

	double dPow = 1.;
	if(ends_with(strUnit,"Hz", dPow))
	{
		freq = dPow * dVal * units::si::hertz;
	}
	else
	{
		std::cerr << "Error: Unknown frequency unit: \"" << strUnit << "\"."
					<< std::endl;
		return 0. * units::si::hertz;
	}

	return freq;
}

units::quantity<units::si::energy>
get_energy(const std::string& strVar)
{
	units::quantity<units::si::energy> energy;

	double dVal = 0.;
	std::string strUnit;
	get_val(strVar, dVal, strUnit);

	double dPow = 1.;
	if(ends_with(strUnit,"J", dPow))
	{
		energy = dPow * dVal * units::si::joules;
	}
	else if(ends_with(strUnit,"eV", dPow))
	{
		energy = dPow * dVal * one_eV;
	}
	else
	{
		std::cerr << "Error: Unknown energy unit: \"" << strUnit << "\"."
					<< std::endl;
		return 0. * units::si::joules;
	}

	return energy;
}

units::quantity<units::si::length>
get_length(const std::string& strVar)
{
	units::quantity<units::si::length> length;

	double dVal = 0.;
	std::string strUnit;
	get_val(strVar, dVal, strUnit);

	double dPow = 1.;
	if(ends_with(strUnit, "m", dPow))
	{
		length = dPow * dVal * units::si::meter;
	}
	else if(ends_with(strUnit, "A", dPow))
	{
		length = dPow * dVal * 1e-10 * units::si::meter;
	}
	else
	{
		std::cerr << "Error: Unknown length unit: \"" << strUnit << "\"."
					<< std::endl;
		return 0. * units::si::meter;
	}

	return length;
}

units::quantity<units::si::wavenumber>
get_wavenumber(const std::string& strVar)
{
	units::quantity<units::si::wavenumber> k;

	double dVal = 0.;
	std::string strUnit;
	get_val(strVar, dVal, strUnit);

	double dPow = 1.;
	if(ends_with(strUnit, "m^-1", dPow))
	{
		k = dPow * dVal * units::si::reciprocal_meter;
	}
	else if(ends_with(strUnit, "A^-1", dPow))
	{
		k = dPow * dVal * 1e10 * units::si::reciprocal_meter;
	}
	else
	{
		std::cerr << "Error: Unknown wavenumber unit: \"" << strUnit << "\"."
					<< std::endl;
		return 0. * units::si::reciprocal_meter;
	}

	return k;
}

units::quantity<units::si::time>
get_time(const std::string& strVar)
{
	units::quantity<units::si::time> t;

	double dVal = 0.;
	std::string strUnit;
	get_val(strVar, dVal, strUnit);

	double dPow = 1.;
	if(ends_with(strUnit, "s", dPow))
	{
		t = dPow * dVal * units::si::second;
	}
	else
	{
		std::cerr << "Error: Unknown length unit: \"" << strUnit << "\"."
					<< std::endl;
		return 0. * units::si::second;
	}

	return t;
}

units::quantity<units::si::temperature>
get_temperature(const std::string& strVar)
{
	units::quantity<units::si::temperature> T;

	double dVal = 0.;
	std::string strUnit;
	get_val(strVar, dVal, strUnit);

	double dPow = 1.;
	if(ends_with(strUnit, "K", dPow))
	{
		T = dPow * dVal * units::si::kelvin;
	}
	/*
	else if(ends_with(strUnit, "°C", dPow))
	{
		T = dPow * dVal * units::celsius::degrees;
	}
	else if(ends_with(strUnit, "C", dPow))
	{
		T = dPow * dVal * units::celsius::degrees;
	}*/
	else
	{
		std::cerr << "Error: Unknown temperature unit: \"" << strUnit << "\"."
					<< std::endl;
		return 0. * units::si::kelvin;
	}

	return T;
}

units::quantity<units::si::plane_angle>
get_angle(const std::string& strVar)
{
	units::quantity<units::si::plane_angle> angle;

	double dVal = 0.;
	std::string strUnit;
	get_val(strVar, dVal, strUnit);

	double dPow = 1.;
	if(ends_with(strUnit, "deg", dPow) || ends_with(strUnit, "°", dPow))
	{
		angle = (dPow * dVal/180.*M_PI) * units::si::radian;
	}
	else if(ends_with(strUnit, "rad", dPow) || ends_with(strUnit, "", dPow))
	{
		angle = dPow * dVal * units::si::radian;
	}
	else
	{
		std::cerr << "Error: Unknown angle unit: \"" << strUnit << "\"."
					<< std::endl;
		return 0. * units::si::radian;
	}

	return angle;
}

double get_scalar(const std::string& strVar)
{
	double d;

	double dVal = 0.;
	std::string strUnit;
	get_val(strVar, dVal, strUnit);

	double dPow = 1.;
	if(ends_with(strUnit, "", dPow))
	{
		d = dPow * dVal;
	}
	else
	{
		std::cerr << "Error: Unknown scalar unit: \"" << strUnit << "\"."
					<< std::endl;
		return 0.;
	}

	return d;
}
