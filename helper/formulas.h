/**
 * neutron formulas
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date: May 2012, 11-jul-2013
 * @license GPLv3
 */

#ifndef __NEUTRON_FORMULAS__
#define __NEUTRON_FORMULAS__

#include "tlibs/phys/neutrons.h"

namespace units = boost::units;
namespace co = boost::units::si::constants::codata;

extern void init_formulas();
extern bool get_val(const std::string& str, double& dVal, std::string& strUnit);
extern units::quantity<units::si::frequency> get_freq(const std::string& strVar);
extern units::quantity<units::si::energy> get_energy(const std::string& strVar);
extern units::quantity<units::si::length> get_length(const std::string& strUnit);
extern units::quantity<units::si::wavenumber> get_wavenumber(const std::string& strVar);
extern units::quantity<units::si::time> get_time(const std::string& strVar);
extern units::quantity<units::si::plane_angle> get_angle(const std::string& strVar);
extern units::quantity<units::si::temperature> get_temperature(const std::string& strVar);
extern double get_scalar(const std::string& strVar);

#endif
