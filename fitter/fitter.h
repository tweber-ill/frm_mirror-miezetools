/*
 * fitter base class
 *
 * Author: Tobias Weber
 * Date: April 2012
 *
 * general fitter structure (i.e. function => chi^2 calculation => calling
 * minuit) originally based on the examples in the Minuit user's guide:
 * http://seal.cern.ch/documents/minuit/mnusersguide.pdf
 */

#ifndef __FITTER__H__
#define __FITTER__H__

#include <iostream>
#include <string>
#include <vector>
#include <boost/numeric/ublas/vector.hpp>


// function interface
class FunctionModel
{
	public:
		virtual bool SetParams(const std::vector<double>& vecParams) = 0;
		virtual double operator()(double x) const = 0;

		virtual FunctionModel* copy() const = 0;
		virtual std::string print(bool bFillInSyms=true) const = 0;

		virtual ~FunctionModel();

		virtual const char* GetModelName() const = 0;
		virtual std::vector<std::string> GetParamNames() const = 0;
		virtual std::vector<double> GetParamValues() const = 0;
		virtual std::vector<double> GetParamErrors() const = 0;
};

// interface for n dimensional function
class FunctionModel_nd
{
	protected:

	public:
		virtual unsigned int GetDim() const = 0;

		virtual bool SetParams(const std::vector<double>& vecParams) = 0;
		virtual double operator()(const double* px) const = 0;

		virtual FunctionModel_nd* copy() const = 0;
		virtual std::string print(bool bFillInSyms=true) const = 0;

		virtual ~FunctionModel_nd();
};

std::ostream& operator<<(std::ostream& ostr, const FunctionModel& fkt);
std::ostream& operator<<(std::ostream& ostr, const FunctionModel_nd& fkt);


//----------------------------------------------------------------------
#endif	// __FITTER__H__
