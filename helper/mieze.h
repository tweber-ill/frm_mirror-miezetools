/**
 * more specialised mieze stuff than in tlibs
 * @author tweber
 * @date 2012, jan-2015
 * @license GPLv3
 */

#ifndef __CATTUS_MIEZE_H__
#define __CATTUS_MIEZE_H__


template<typename T=double>
T get_mieze_freq(const T* px, unsigned int iLen, T dNumOsc=2.)
{
        if(iLen==0)
                return -1.;
       	T dTLen = (px[iLen-1]-px[0])/T(iLen-1)*T(iLen);
       	return dNumOsc * 2.*M_PI/dTLen;
}


#endif
