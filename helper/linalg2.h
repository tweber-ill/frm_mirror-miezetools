/*
 * advanced linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#ifndef __MIEZE_LINALG2__
#define __MIEZE_LINALG2__


#include "math.h"
#include "linalg.h"


template<typename T=double>
bool qr(const ublas::matrix<T>& M, ublas::matrix<T>& Q, ublas::matrix<T>& R)
{
	std::cerr << "Error: No specialisation of \"eigenvec\" available for this type." << std::endl;
	return false;
}

template<>
bool qr<double>(const ublas::matrix<double>& M,
		ublas::matrix<double>& Q,
		ublas::matrix<double>& R);


template<typename T=double>
bool eigenvec(const ublas::matrix<T>& mat, std::vector<ublas::vector<T> >& evecs, std::vector<T>& evals)
{
	std::cerr << "Error: No specialisation of \"eigenvec\" available for this type." << std::endl;
	return false;
}

template<typename T=double>
bool eigenvec_sym(const ublas::matrix<T>& mat, std::vector<ublas::vector<T> >& evecs, std::vector<T>& evals)
{
	std::cerr << "Error: No specialisation of \"eigenvec_sym\" available for this type." << std::endl;
	return false;
}

template<>
bool eigenvec<double>(const ublas::matrix<double>& mat,
			std::vector<ublas::vector<double> >& evecs,
			std::vector<double>& evals);
template<>
bool eigenvec_sym<double>(const ublas::matrix<double>& mat,
			std::vector<ublas::vector<double> >& evecs,
			std::vector<double>& evals);

#endif
