/*
 * random numbers
 * @author tweber
 * @date 16-aug-2013
 */

#ifndef __M_RAND_H__
#define __M_RAND_H__

#include <random>
#include <vector>
#include <future>


extern std::mt19937/*_64*/ g_randeng;


extern void init_rand();
extern void init_rand_seed(unsigned int uiSeed);


extern unsigned int simple_rand(unsigned int iMax);


template<typename INT>
INT rand_int(int iMin, int iMax)
{
	std::uniform_int_distribution<INT> dist(iMin, iMax);
	return dist(g_randeng);
}

template<typename REAL>
REAL rand_real(REAL dMin, REAL dMax)
{
	std::uniform_real_distribution<REAL> dist(dMin, dMax);
	return dist(g_randeng);
}

template<typename REAL>
REAL rand01()
{
	return rand_real<REAL>(0., 1.);
}

template<typename REAL>
REAL rand_norm(REAL dMu, REAL dSigma)
{
	std::normal_distribution<REAL> dist(dMu, dSigma);
	return dist(g_randeng);
}

template<typename INT, typename REAL=double>
INT rand_poisson(REAL dMu)
{
	std::poisson_distribution<INT> dist(dMu);
	return dist(g_randeng);
}


template<class REAL>
std::vector<REAL> rand_norm_nd(const std::vector<REAL>& vecMu, const std::vector<REAL>& vecSigma)
{
	std::vector<REAL> vecRet;

	if(vecMu.size() != vecSigma.size())
		return vecRet;

	unsigned int iDim = vecMu.size();
	vecRet.reserve(iDim);


	std::vector<std::future<REAL>> vecFut;
	vecFut.reserve(iDim);

	for(unsigned int i=0; i<iDim; ++i)
	{
		std::future<REAL> fut =std::async(std::launch::deferred | std::launch::async,
						rand_norm<REAL>, vecMu[i], vecSigma[i]);
		vecFut.push_back(std::move(fut));
	}

	for(unsigned int i=0; i<iDim; ++i)
		vecRet.push_back(vecFut[i].get());

	return vecRet;
}

#endif
