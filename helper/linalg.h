/*
 * linalg helpers
 *
 * @author: tweber
 * @date: 30-apr-2013
 */

#ifndef __MIEZE_LINALG__
#define __MIEZE_LINALG__

#include "flags.h"
#include "exception.h"
#include <cmath>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/exception.hpp>
#include <boost/math/quaternion.hpp>
namespace ublas = boost::numeric::ublas;
namespace math = boost::math;

#include <initializer_list>


//#include "math.h"
template<typename T> bool float_equal(T t1, T t2, T eps = std::numeric_limits<T>::epsilon());
template<typename T> T sign(T t);
template<typename INT> bool is_even(INT i);
template<typename INT> bool is_odd(INT i);

template<class matrix_type, typename T>
T determinant(const matrix_type& mat);


// create a ublas vector
template<typename T=double>
ublas::vector<T> ublas_vec(const std::initializer_list<T>& lst)
{
	ublas::vector<T> vec(lst.size());
	typename std::initializer_list<T>::const_iterator iter = lst.begin();
	for(std::size_t i=0; i<lst.size(); ++i, ++iter)
		vec[i] = *iter;

	return vec;
}

// create a ublas matrix
template<typename T=double>
ublas::matrix<T> ublas_mat(const std::initializer_list<std::initializer_list<T> >& lst)
{
	std::size_t I = lst.size();
	std::size_t J = lst.begin()->size();

	ublas::matrix<T> mat(I, J);
	typename std::initializer_list<std::initializer_list<T> >::const_iterator iter = lst.begin();

	for(std::size_t i=0; i<I; ++i, ++iter)
	{
		typename std::initializer_list<T>::const_iterator iterinner = iter->begin();
		for(std::size_t j=0; j<J; ++j, ++iterinner)
		{
			mat(i,j) = *iterinner;
		}
	}

	return mat;
}


template<class vec_type>
bool vec_equal(const vec_type& vec0, const vec_type& vec1,
		typename vec_type::value_type eps = std::numeric_limits<typename vec_type::value_type>::epsilon())
{
	typedef typename vec_type::value_type T;

	if(vec0.size() != vec1.size())
		return false;

	for(unsigned int i=0; i<vec0.size(); ++i)
		if(!float_equal<T>(vec0[i], vec1[i], eps))
			return false;

	return true;
}


template<class vec_type>
typename vec_type::value_type vec_len(const vec_type& vec)
{
	typename vec_type::value_type t = typename vec_type::value_type();

	for(unsigned int i=0; i<vec.size(); ++i)
		t += vec[i]*vec[i];

	t = std::sqrt(t);
	return t;
}


/*
 * remove an element from a vector
 */
template<class vector_type>
vector_type remove_elem(const vector_type& vec, unsigned int iIdx)
{
        vector_type vecret(vec.size()-1);

        for(unsigned int i=0, j=0; i<vec.size() && j<vecret.size();)
        {
                vecret[j] = vec[i];

                if(i!=iIdx) ++j;
                ++i;
        }

        return vecret;
}

template<class matrix_type>
matrix_type submatrix(const matrix_type& mat, unsigned int iRow, unsigned int iCol)
{
        matrix_type matret(mat.size1()-1, mat.size2()-1);

        for(unsigned int i=0, i0=0; i<mat.size1() && i0<matret.size1();)
        {
                for(unsigned int j=0, j0=0; j<mat.size2() && j0<matret.size2();)
                {
                        matret(i0,j0) = mat(i,j);

                        if(j!=iCol) ++j0;
                        ++j;
                }

                if(i!=iRow) ++i0;
                ++i;
        }

        return matret;
}

template<class matrix_type>
matrix_type remove_column(const matrix_type& mat, unsigned int iCol)
{
	matrix_type matret(mat.size1(), mat.size2()-1);
	for(unsigned int i=0; i<mat.size1(); ++i)
	{
		for(unsigned int j=0, j0=0; j<mat.size2() && j0<matret.size2(); ++j)
		{
			matret(i,j0) = mat(i,j);
                        if(j!=iCol) ++j0;
		}
	}
	return matret;
}

template<class matrix_type>
void submatrix_copy(matrix_type& mat, const matrix_type& sub, unsigned int iRowBegin, unsigned int iColBegin)
{
	for(unsigned int i=0; i<sub.size1(); ++i)
		for(unsigned int j=0; j<sub.size2(); ++j)
			mat(iRowBegin+i, iColBegin+j) = sub(i,j);
}

template<class matrix_type>
matrix_type remove_elems(const matrix_type& mat, unsigned int iIdx)
{
	return submatrix(mat, iIdx, iIdx);
}


template<class vector_type=ublas::vector<double>, class matrix_type=ublas::matrix<double> >
vector_type get_column(const matrix_type& mat, unsigned int iCol)
{
        vector_type vecret(mat.size1());

        for(unsigned int i=0; i<mat.size1(); ++i)
        	vecret[i] = mat(i, iCol);

        return vecret;
}


template<class matrix_type=ublas::matrix<double>, typename T=double>
matrix_type rotation_matrix_2d(T angle)
{
	T s, c;

	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	matrix_type mat(2,2);

	mat(0,0) = c; mat(0,1) = -s;
	mat(1,0) = s; mat(1,1) = c;

	return mat;
}

template<class matrix_type=ublas::matrix<double>, typename T=double>
matrix_type rotation_matrix_3d_x(T angle)
{
	matrix_type mat(3,3);

	T s, c;
	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	mat(0,0)=1; mat(0,1)=0; mat(0,2)=0;
	mat(1,0)=0; mat(1,1)=c; mat(1,2)=-s;
	mat(2,0)=0; mat(2,1)=s; mat(2,2)=c;

	return mat;
}

template<class matrix_type=ublas::matrix<double>, typename T=double>
matrix_type rotation_matrix_3d_y(T angle)
{
	matrix_type mat(3,3);

	T s, c;
	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	mat(0,0)=c; mat(0,1)=0; mat(0,2)=s;
	mat(1,0)=0; mat(1,1)=1; mat(1,2)=0;
	mat(2,0)=-s; mat(2,1)=0; mat(2,2)=c;

	return mat;
}

template<class matrix_type=ublas::matrix<double>, typename T=double>
matrix_type rotation_matrix_3d_z(T angle)
{
	matrix_type mat(3,3);

	T s, c;
	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	mat(0,0)=c; mat(0,1)=-s; mat(0,2)=0;
	mat(1,0)=s; mat(1,1)=c; mat(1,2)=0;
	mat(2,0)=0; mat(2,1)=0; mat(2,2)=1;

	return mat;
}

template<typename T=double>
ublas::matrix<T> skew(const ublas::vector<T>& vec)
{
	ublas::matrix<T> mat = ublas::zero_matrix<T>(vec.size(), vec.size());

	if(vec.size() == 3)
	{
		mat(0,1) = -vec[2];
		mat(0,2) = vec[1];
		mat(1,2) = -vec[0];

		mat(1,0) = -mat(0,1);
		mat(2,0) = -mat(0,2);
		mat(2,1) = -mat(1,2);
	}
	else
		throw Err("Skew only defined for three dimensions.");

	return mat;
}

template<typename T=double>
ublas::matrix<T> unit_matrix(unsigned int N)
{
	ublas::matrix<T> mat = ublas::zero_matrix<T>(N,N);
	for(unsigned int i=0; i<N; ++i)
		mat(i,i) = 1;
	return mat;
}


// Euler-Rodrigues formula
template<typename T=double>
ublas::matrix<T> rotation_matrix(const ublas::vector<T>& vec, T angle)
{
	T s, c;
	if(angle==0.)
	{
		s = T(0);
		c = T(1);
	}
	else
	{
		s = std::sin(angle);
		c = std::cos(angle);
	}

	return (T(1) - c) * ublas::outer_prod(vec,vec) +
				c * unit_matrix(vec.size()) +
				s * skew(vec);
}


template<class matrix_type=ublas::matrix<double>, typename T=double>
T trace(const matrix_type& mat)
{
	if(mat.size1() != mat.size2())
		return T(0);

	T tr = T(0.);
	for(unsigned int i=0; i<mat.size1(); ++i)
		tr += mat(i,i);
	return tr;
}




template<class matrix_type=ublas::matrix<double>, typename T=double>
bool isnan(const matrix_type& mat)
{
	for(unsigned int i=0; i<mat.size1(); ++i)
		for(unsigned int j=0; j<mat.size2(); ++j)
			if(std::isnan(mat(i,j)))
				return true;
	return false;
}

template<class matrix_type=ublas::matrix<double>, typename T=double>
bool isinf(const matrix_type& mat)
{
	for(unsigned int i=0; i<mat.size1(); ++i)
		for(unsigned int j=0; j<mat.size2(); ++j)
			if(std::isinf(mat(i,j)))
				return true;
	return false;
}


template<typename T=double>
bool qr(const ublas::matrix<T>& M, ublas::matrix<T>& Q, ublas::matrix<T>& R)
{
	std::cerr << "Error: No specialisation of \"eigenvec\" available for this type." << std::endl;
	return false;
}

#ifdef USE_LAPACK
	template<>
	bool qr<double>(const ublas::matrix<double>& M,
					ublas::matrix<double>& Q,
					ublas::matrix<double>& R);
#endif


// code for inverse based on boost/libs/numeric/ublas/test/test_lu.cpp
template<typename T=double>
bool inverse(const ublas::matrix<T>& mat, ublas::matrix<T>& inv)
{
	const typename ublas::matrix<T>::size_type N = mat.size1();
	if(N != mat.size2())
		return false;
	//if(isnan(mat) || isinf(mat))
	//	return false;

	try
	{
		ublas::matrix<T> lu = mat;
		ublas::permutation_matrix<> perm(N);

		if(ublas::lu_factorize(lu, perm) != 0)
			return false;

		inv = ublas::identity_matrix<T>(N);
		ublas::lu_substitute(lu, perm, inv);
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Error: Matrix inversion failed with exception: " << ex.what() << "." << "\n";
		std::cerr << "Matrix to be inverted was: " << mat << "." << std::endl;
		//std::cerr << "with determinant " << determinant(mat) << "." << std::endl;
		return false;
	}

	return true;
}


template<typename T>
bool solve_linear_approx(const ublas::matrix<T>& M, const ublas::vector<T>& v,
						ublas::vector<T>& x);

// solve Mx = v for x
template<typename T=double>
bool solve_linear(const ublas::matrix<T>& M, const ublas::vector<T>& v,
						ublas::vector<T>& x)
{
	if(M.size1() == M.size2())		// determined, TODO: check rank
	{
		try
		{
			const unsigned int N = M.size1();

			ublas::matrix<T> lu = M;
			ublas::permutation_matrix<> perm(N);

			typename ublas::matrix<T>::size_type sing = ublas::lu_factorize(lu, perm);
			if(sing != 0)
				return false;

			x = v;
			ublas::lu_substitute(lu, perm, x);
		}
		catch(const std::exception& ex)
		{
			std::cerr << "Error: Linear equation solver failed with exception: "
						<< ex.what() << "." << "\n";
			return false;
		}
	}
	else if(M.size1() < M.size2())	// underdetermined
	{
		ublas::matrix<T> Q, R;
		if(!qr(M, Q, R))
			return false;
		typedef typename ublas::vector<T>::size_type t_int;

		// M x = v
		// QR x = v
		// R x = Q^T v

		ublas::vector<T> vnew = ublas::prod(ublas::trans(Q), v);

		/*std::cout << "M = " << M << std::endl;
		std::cout << "Q = " << Q << std::endl;
		std::cout << "R = " << R << std::endl;
		std::cout << "v' = " << vnew << std::endl;*/

		x = ublas::zero_vector<T>(M.size2());
		ublas::vector<T> xnew(R.size1());
		bool bOk = 0;

		/*
		// pick one of the solutions
		// TODO: resort columns so that Rupper doesn't get singular
		ublas::matrix<T> Rupper = ublas::subrange(R, 0, R.size1(),
						R.size2()-R.size1(), R.size2());
		//std::cout << "Rupper = " << Rupper << std::endl;

		bOk = solve_linear(Rupper, vnew, xnew);

		for(t_int i=0; i<xnew.size(); ++i)
			x[x.size()-xnew.size()+i] = xnew[i];
		*/

		// find non-singular right-upper submatrix
		std::vector<t_int> vecDelCols;
		unsigned int iNumToDel = R.size2()-R.size1();
		if(iNumToDel != 1)
		{
			std::cerr << "Error: Still unimplemented." << std::endl;
			return false;
		}

		bool bFoundNonSingular = 0;
		ublas::matrix<T> Rsub;
		for(int iCol=int(R.size2()-1); iCol>=0; --iCol)
		{
			Rsub = remove_column(R, (unsigned int)iCol);
			//std::cout << "Rsub" << Rsub << std::endl;
			//std::cout << "det: " << determinant(Rsub) << std::endl;

			T det = determinant<ublas::matrix<T>, T>(Rsub);
			if(!float_equal(det, 0.))
			{
				bFoundNonSingular = 1;
				vecDelCols.push_back(iCol);
				break;
			}
		}

		if(!bFoundNonSingular)
		{
			std::cerr << "Error: No non-singluar submatrix found in linear equation solver." 
				<< std::endl;
			return false;
		}

		bOk = solve_linear(Rsub, vnew, xnew);
		//std::cout << "Rsub = " << Rsub << std::endl;
		//std::cout << "v' = " << vnew << std::endl;
		//std::cout << "x' = " << xnew << std::endl;

		for(t_int i=0, i0=0; i<xnew.size() && i0<x.size(); ++i, ++i0)
		{
			while(std::find(vecDelCols.begin(), vecDelCols.end(), i0) != vecDelCols.end())
				++i0;
			x[i0] = xnew[i];
		}

		return bOk;
	}
	else if(M.size1() > M.size2())	// overdetermined
		return solve_linear_approx<T>(M,v,x);
	else
		return false;

	return true;
}

// solve M^T M x = M^T v for x
template<typename T=double>
bool solve_linear_approx(const ublas::matrix<T>& M, const ublas::vector<T>& v,
						ublas::vector<T>& x)
{
	if(M.size1() <= M.size2())
	{
		//std::cerr << "Error: Matrix has to be overdetermined." << std::endl;
		return false;
	}

	ublas::matrix<T> Q, R;
	if(!qr(M, Q, R))
		return false;

	// M^T M x = M^T v
	// R^T Q^T Q R x = R^T Q^T v
	// R^T R x = R^T Q^T v

	const ublas::matrix<T> RT = ublas::trans(R);
	const ublas::matrix<T> QT = ublas::trans(Q);
	const ublas::matrix<T> RTR = ublas::prod(RT, R);
	const ublas::matrix<T> RTQT = ublas::prod(RT, QT);

	const ublas::vector<T> vnew = ublas::prod(RTQT, v);
	return solve_linear<T>(RTR, vnew, x);
}


template<class matrix_type=ublas::matrix<double>, typename T=double>
bool is_diag_matrix(const matrix_type& mat)
{
	for(unsigned int i=0; i<mat.size1(); ++i)
		for(unsigned int j=0; j<mat.size2(); ++j)
		{
			if(i==j) continue;

			if(!float_equal(mat(i,j), T(0.)))
				return false;
		}

	return true;
}


// vectors form rows of matrix
template<class matrix_type=ublas::matrix<double>, class vec_type=ublas::vector<double>, typename T=double>
ublas::matrix<T> row_matrix(const std::vector<vec_type>& vecs)
{
	if(vecs.size() == 0)
		return matrix_type(0,0);

	matrix_type mat(vecs.size(), vecs[0].size());
	for(unsigned int j=0; j<vecs.size(); ++j)
		for(unsigned int i=0; i<vecs[0].size(); ++i)
			mat(j,i) = vecs[j][i];

	return mat;
}

// vectors form columns of matrix
template<class matrix_type=ublas::matrix<double>, class vec_type=ublas::vector<double>, typename T=double>
ublas::matrix<T> column_matrix(const std::vector<vec_type>& vecs)
{
	if(vecs.size() == 0)
		return matrix_type(0,0);

	matrix_type mat(vecs[0].size(), vecs.size());
	for(unsigned int j=0; j<vecs.size(); ++j)
		for(unsigned int i=0; i<vecs[0].size(); ++i)
			mat(i,j) = vecs[j][i];

	return mat;
}

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


#ifdef USE_LAPACK

	template<>
	bool eigenvec<double>(const ublas::matrix<double>& mat,
							std::vector<ublas::vector<double> >& evecs,
							std::vector<double>& evals);
	template<>
	bool eigenvec_sym<double>(const ublas::matrix<double>& mat,
							std::vector<ublas::vector<double> >& evecs,
							std::vector<double>& evals);

#endif



// algo from:
// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q55
template<typename T=double>
math::quaternion<T> rot3_to_quat(const ublas::matrix<T>& rot)
{
	T tr = trace(rot) + 1.;
	T x,y,z,w;

	if(tr > std::numeric_limits<T>::epsilon())
	{
		T s = std::sqrt(tr) * 2.;
		x = (rot(2,1) - rot(1,2)) / s;
		y = (rot(0,2) - rot(2,0)) / s;
		z = (rot(1,0) - rot(0,1)) / s;
		w = s/4.;
	}
	else
	{
		if (rot(0,0) > rot(1,1) && rot(0,0) > rot(2,2))
		{
			T s = std::sqrt(1. + rot(0,0) - rot(1,1) - rot(2,2)) * 2.;
			x = s/4.;
			y = (rot(1,0) + rot(0,1)) / s;
			z = (rot(0,2) + rot(2,0)) / s;
			w = (rot(2,1) - rot(1,2)) / s;
		}
		else if(rot(1,1) > rot(2,2))
		{
			T s = std::sqrt(1. + rot(1,1) - rot(0,0) - rot(2,2)) * 2.;
			x = (rot(1,0) + rot(0,1)) / s;
			y = s/4.;
			z = (rot(2,1) + rot(1,2)) / s;
			w = (rot(0,2) - rot(2,0)) / s;
		}
		else
		{
			T s = std::sqrt(1. + rot(2,2) - rot(0,0) - rot(1,1)) * 2.;
			x = (rot(0,2) + rot(2,0)) / s;
			y = (rot(2,1) + rot(1,2)) / s;
			z = s/4.;
			w = (rot(1,0) - rot(0,1)) / s;
		}
	}

	T n = std::sqrt(w*w + x*x + y*y + z*z);
	return math::quaternion<T>(w,x,y,z)/n;
}

// algo from:
// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
// http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche52.html
template<typename T=double>
ublas::matrix<T> quat_to_rot3(const math::quaternion<T>& quat)
{
	ublas::matrix<T> mat(3,3);
	T w = quat.R_component_1();
	T x = quat.R_component_2();
	T y = quat.R_component_3();
	T z = quat.R_component_4();

	mat(0,0) = 1. - 2.*(y*y + z*z);
	mat(1,1) = 1. - 2.*(x*x + z*z);
	mat(2,2) = 1. - 2.*(x*x + y*y);

	mat(0,1) = 2.*(x*y - z*w);
	mat(1,0) = 2.*(x*y + z*w);
	mat(0,2) = 2.*(x*z + y*w);
	mat(2,0) = 2.*(x*z - y*w);
	mat(1,2) = 2.*(y*z - x*w);
	mat(2,1) = 2.*(y*z + x*w);
	//mat = ublas::trans(mat);

	return mat;
}


template<typename T=double>
std::vector<T> quat_to_euler(const math::quaternion<T>& quat)
{
	T q[] = {quat.R_component_1(), quat.R_component_2(), quat.R_component_3(), quat.R_component_4()};

	// formulas from:
	// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	T phi = std::atan2(2.*(q[0]*q[1] + q[2]*q[3]), 1.-2.*(q[1]*q[1] + q[2]*q[2]));
	T theta = std::asin(2.*(q[0]*q[2] - q[3]*q[1]));
	T psi = std::atan2(2.*(q[0]*q[3] + q[1]*q[2]), 1.-2.*(q[2]*q[2] + q[3]*q[3]));

	std::vector<T> vec = { phi, theta, psi };
	return vec;
}

template<typename T=double>
std::vector<T> rotation_angle(const ublas::matrix<T>& rot)
{
	std::vector<T> vecResult;

	if(rot.size1()!=rot.size2())
		return vecResult;
	if(rot.size1()<2)
		return vecResult;

	if(rot.size2()==2)
	{
		T angle = atan2(rot(1,0), rot(0,0));
		vecResult.push_back(angle);
	}
	else if(rot.size2()==3)
	{
		math::quaternion<T> quat = rot3_to_quat(rot);
		vecResult = quat_to_euler(quat);
	}

	return vecResult;
}


template<typename vector_type = ublas::vector<double> >
vector_type cross_3(const vector_type& vec0, const vector_type& vec1)
{
	vector_type vec;
	vec.resize(3);

	vec[0] = vec0[1]*vec1[2] - vec1[1]*vec0[2];
	vec[1] = vec0[2]*vec1[0] - vec1[2]*vec0[0];
	vec[2] = vec0[0]*vec1[1] - vec1[0]*vec0[1];

	return vec;
}


template<class matrix_type=ublas::matrix<double>, typename T=double>
T determinant(const matrix_type& mat)
{
	if(mat.size1() != mat.size2())
		return T(0);

	if(mat.size1()==1)
	{
		return mat(0,0);
	}
	else if(mat.size1()==2)
	{
		return mat(0,0)*mat(1,1) - mat(1,0)*mat(0,1);
	}
	/*
	else if(mat.size1()==3)
	{
		ublas::vector<T> vec0 = get_column(mat, 0);
		ublas::vector<T> vec1 = get_column(mat, 1);
		ublas::vector<T> vec2 = get_column(mat, 2);

		ublas::vector<T> vecCross = cross_3<ublas::vector<T> >(vec1, vec2);
		return ublas::inner_prod(vec0, vecCross);
	}*/

	const unsigned int i = 0;
	T val = T(0);

	for(unsigned int j=0; j<mat.size2(); ++j)
	{
		T dSign = 1.;
		if(is_odd<unsigned int>(i+j))
			dSign = -1.;
		val += dSign * mat(i,j) * determinant<matrix_type,T>(submatrix(mat, i, j));
	}

	return val;
}

template<class matrix_type=ublas::matrix<double>, typename T=double>
T get_volume(const matrix_type& mat)
{
	return determinant<matrix_type, T>(mat);
}



// calculate fractional coordinate basis vectors from angles
// see: http://www.bmsc.washington.edu/CrystaLinks/man/pdb/part_75.html
// for the reciprocal lattice this is equal to the B matrix from Acta Cryst. (1967), 22, 457
template<class t_vec, class T>
bool fractional_basis_from_angles(T a, T b, T c,
			T alpha, T beta, T gamma,
			t_vec& veca, t_vec& vecb, t_vec& vecc)
{
	const T dSG = std::sin(gamma);
	const T dCG = std::cos(gamma);
	const T dCA = std::cos(alpha);
	const T dCB = std::cos(beta);

	const T dCA2 = dCA*dCA;
	const T dCB2 = dCB*dCB;
	const T dCG2 = dCG*dCG;

	const T dVol =a*b*c*std::sqrt(1.- dCA2 - dCB2 - dCG2 + 2.*dCA*dCB*dCG);
	//std::cout << "vol = " <<  dVol << std::endl;

	if(std::isinf(dVol) || std::isnan(dVol))
		return false;

	veca[0] = a;
	veca[1] = 0.;
	veca[2] = 0.;

	vecb[0] = b*dCG;
	vecb[1] = b*dSG;
	vecb[2] = 0.;

	vecc[0] = c*dCB;
	vecc[1] = c*(dCA - dCB*dCG) / dSG;
	vecc[2] = dVol / (a*b*dSG);

	return true;
}


// signed angle wrt basis
template<typename vec_type>
typename vec_type::value_type vec_angle(const vec_type& vec)
{
	if(vec.size() == 2)
		return std::atan2(vec[1], vec[0]);

	throw Err("vec_angle not yet implemented for size != 2.");
}

template<typename vec_type, typename real_type=double>
void set_eps_0(vec_type& vec)
{
	for(real_type& d : vec)
		if(std::fabs(d) < std::numeric_limits<real_type>::epsilon())
			d = real_type(0);
}

// signed angle between two vectors
template<typename vec_type, typename real_type=double>
typename vec_type::value_type vec_angle(const vec_type& vec0,
										const vec_type& vec1,
										const vec_type* pvec_norm=0)
{
	if(vec0.size() != vec1.size())
		throw Err("In vec_angle: Vector sizes do not match.");

	if(vec0.size() == 2)
	{
		return vec_angle<vec_type>(vec0) - vec_angle<vec_type>(vec1);
	}
	if(vec0.size() == 3)
	{
		real_type dNorm0 = ublas::norm_2(vec0);
		real_type dNorm1 = ublas::norm_2(vec1);

		real_type dC = ublas::inner_prod(vec0, vec1);
		vec_type veccross = cross_3<vec_type>(vec0, vec1);
		real_type dS = ublas::norm_2(veccross);

		real_type dAngle = std::atan2(dS, dC);

		// get signed angle
		if(pvec_norm)
		{
			if(ublas::inner_prod(veccross, *pvec_norm) < real_type(0))
				dAngle = -dAngle;
		}

		return dAngle;
	}

	throw Err("vec_angle only implemented for size == 2 and size == 3.");
}

// unsigned angle between two vectors
template<class T, typename REAL>
REAL vec_angle_unsigned(const T& q1, const T& q2)
{
	if(q1.size() != q2.size())
		return REAL();

	REAL dot = REAL();
	REAL len1 = REAL();
	REAL len2 = REAL();
	for(unsigned int i=0; i<q1.size(); ++i)
	{
		dot += q1[i]*q2[i];

		len1 += q1[i]*q1[i];
		len2 += q2[i]*q2[i];
	}

	len1 = std::sqrt(len1);
	len2 = std::sqrt(len2);

	dot /= len1;
	dot /= len2;

	return std::acos(dot);
}

template<>
double vec_angle_unsigned(const math::quaternion<double>& q1,
				const math::quaternion<double>& q2);

// see: http://run.usc.edu/cs520-s12/assign2/p245-shoemake.pdf
template<class T, typename REAL=double>
T slerp(const T& q1, const T& q2, REAL t)
{
	REAL angle = vec_angle_unsigned<T, REAL>(q1, q2);

	T q = std::sin((1.-t)*angle)/std::sin(angle) * q1 +
			std::sin(t*angle)/std::sin(angle) * q2;

	return q;
}

#endif
