/*
 * abstract function model classes
 * @author tweber
 * @date 2013
 */

#ifndef __FUNC_MOD_H__
#define __FUNC_MOD_H__

// parametric function
template<class T = double> class FunctionModel_param_gen
{
	public:
		virtual ~FunctionModel_param_gen() {}

		// t = 0..1
		virtual boost::numeric::ublas::vector<T> operator()(T t) const = 0;
		virtual const char* GetModelName() const = 0;
};

typedef class FunctionModel_param_gen<double> FunctionModel_param;

#endif
