/*
 * global symbols
 * @author tweber
 * @date 2013-2014
 */

#include "globals.h"
#include "../runtime/calls_basic.h"
#include "../runtime/calls_plot.h"
#include "../runtime/calls_math.h"
#include "../runtime/calls_fit.h"
#include "../runtime/calls_file.h"
#include "../runtime/calls_thread.h"
#include "../helper/neutrons.hpp"
#include <boost/units/systems/si/codata/electron_constants.hpp>

int yydebug = 0;

const t_char* g_pcVersion = T_STR"Hermelin script interpreter, version 0.7";

static inline void init_funcs()
{
	init_ext_basic_calls();
	init_ext_thread_calls();
	init_ext_file_calls();
	init_ext_math_calls();
	init_ext_fit_calls();
	init_ext_plot_calls();
}


static inline void init_constants(SymbolTable *pSymTab)
{
	pSymTab->InsertSymbol(T_STR"pi", new SymbolReal(M_PI));

	// hbar in eVs
	pSymTab->InsertSymbol(T_STR"hbar_eVs", new SymbolReal(co::hbar / one_eV / units::si::second));
	// hbar in Js
	pSymTab->InsertSymbol(T_STR"hbar", new SymbolReal(co::hbar / units::si::joule / units::si::second));
	// neutron mass
	pSymTab->InsertSymbol(T_STR"m_n", new SymbolReal(co::m_n / units::si::kilogram));
	// atomic mass unit
	pSymTab->InsertSymbol(T_STR"m_u", new SymbolReal(co::m_u / units::si::kilogram));
	// electron mass
	pSymTab->InsertSymbol(T_STR"m_e", new SymbolReal(co::m_e / units::si::kilogram));
	// Boltzmann const
	pSymTab->InsertSymbol(T_STR"k_B", new SymbolReal(co::k_B * units::si::kelvin/units::si::joules));
	// Boltzmann const in eV/K
	pSymTab->InsertSymbol(T_STR"k_B_eVperK", new SymbolReal(co::k_B * units::si::kelvin/one_eV));
	// Avogadro const
	pSymTab->InsertSymbol(T_STR"N_A", new SymbolReal(co::N_A * units::si::moles));
	// speed of light
	pSymTab->InsertSymbol(T_STR"c_0", new SymbolReal(co::c / units::si::meters*units::si::seconds));
	// electron charge
	pSymTab->InsertSymbol(T_STR"q_e", new SymbolReal(co::e / units::si::coulomb));
}

extern void init_global_syms(SymbolTable *pSymTab)
{
	init_funcs();
	init_constants(pSymTab);
}