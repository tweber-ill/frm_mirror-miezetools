/*
 * nthread test
 * @author tweber
 */

#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

template<typename T>
std::ostream& operator<<(std::ostream& ostr, const std::vector<T>& vec)
{
        for(const T& t : vec)
                ostr << t << ", ";
        return ostr;
}

void th_func(const std::vector<double>& vec, int i)
{
        std::cout << "vec: " << vec
                          << "i: " << i << std::endl;
}


template<typename T, typename FUNC, typename... ARGS>
void nthread(const std::size_t N, const std::vector<T>& vec, FUNC&& func, ARGS&&...
args)
{
        const std::size_t iVecSize = vec.size();
        if(N>iVecSize)
        {
                *const_cast<std::size_t*>(&N) = iVecSize;
                std::cerr << "Warning: More threads requested than necessary, "
                                  << "reducing to array size (" << N << ")."
                                  << std::endl;
        }

        std::vector<T> *pvecs = new std::vector<T>[N];

        std::size_t iCurTh = 0;
        for(const T& t : vec)
        {
                pvecs[iCurTh].push_back(t);

                ++iCurTh;
                if(iCurTh == N)
                        iCurTh = 0;
        }


        std::vector<std::thread*> vecThreads;
        vecThreads.reserve(N);

        for(iCurTh=0; iCurTh<N; ++iCurTh)
        {
                std::thread *pth = new std::thread(func, pvecs[iCurTh], args...);
                vecThreads.push_back(pth);
        }

        for(iCurTh=0; iCurTh<N; ++iCurTh)
        {
                vecThreads[iCurTh]->join();
                delete vecThreads[iCurTh];
                vecThreads[iCurTh] = 0;
        }

        delete[] pvecs;
}

int main()
{
        std::vector<double> vec = {1.,2.,3.,4.};
        nthread(2, vec, th_func, 123);
}