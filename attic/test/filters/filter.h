/*
 * Filer chain
 * @author tw
 */

#include <string.h>
#include <vector>

template<typename T=float>
class BWImage
{
	protected:
		T* m_pData;
		unsigned int m_iW, m_iH;
		bool m_bOwnsImage;

	public:
		BWImage(unsigned int iW, unsigned int iH, const T* pDat=0, bool bDeepCopy=1)
				: m_bOwnsImage(bDeepCopy), m_iW(iW), m_iH(iH)
		{
			if(pDat)
			{
				if(bDeepCopy)
				{
					m_pData = new T[m_iW*m_iH];
					memcpy(m_pData, pDat, m_iW*m_iH*sizeof(T));
				}
				else
				{
					m_pData = (T*)pDat;
				}
			}
			else
			{
				m_pData = new T[m_iW*m_iH];
				memset(m_pData, 0, m_iW*m_iH*sizeof(T));
				m_bOwnsImage = 1;
			}
		}
		
		virtual ~BWImage()
		{
			if(m_bOwnsImage)
				delete[] m_pData;
			
			m_pData = 0;
		}
		
		const T* GetRawData() const { return m_pData; }
		T* GetRawData() { return m_pData; }
		
		unsigned int GetWidth() const { return m_iW; }
		unsigned int GetHeight() const { return m_iH; }
		
		T& operator()(unsigned int iX, unsigned int iY)
		{
			return m_pData[iY*m_iW + iX];
		}
		
		const T& operator() (unsigned int iX, unsigned int iY) const 
		{
			return m_pData[iY*m_iW + iX];
		}		
};


template<typename T=float>
class FilterBase
{
	protected:
		BWImage<T>* m_pImg;
		const BWImage<T> *m_pDark, *m_pOpen;
	
	public:
		FilterBase() : m_pImg(0), m_pDark(0), m_pOpen(0) {}
		virtual ~FilterBase() {}
		
		virtual void Exec() = 0;
		
		virtual void SetImage(BWImage<T>* pImg) { m_pImg = pImg; }
		virtual void SetOpen(const BWImage<T>* pImg) { m_pOpen = pImg; }
		virtual void SetDark(const BWImage<T>* pImg) { m_pDark = pImg; }
};

template<typename T=float>
class Despeckle : public FilterBase<T>
{
	public:
		Despeckle() { }
		virtual ~Despeckle() {}
		
		virtual void Exec()
		{
			// ...
		}
};

template<typename T=float>
class Normalize : public FilterBase<T>
{
	public:
		Normalize() {}
		virtual ~Normalize() {}
		
		virtual void Exec()
		{
			// ...
		}
};



template<typename T=float>
class FilterStack
{
	protected:
		typedef std::vector<FilterBase<T>*> t_vecStack;
		t_vecStack m_vecStack;
	
	public:
		FilterStack() {}
		
		void CleanFilters()
		{
			for(unsigned int iFilter=0; iFilter<m_vecStack.size(); ++iFilter)
			{
				if(m_vecStack[iFilter])
					delete m_vecStack[iFilter];
			}
			
			m_vecStack.clear();
		}
		
		virtual ~FilterStack() { CleanFilters(); }
		
		void AddFilter(FilterBase<T> *pFilter)
		{
			m_vecStack.push_back(pFilter);
		}
		
		void Exec(BWImage<T>* pImg=0, const BWImage<T>* pDark=0, const BWImage<T>* pOpen=0)
		{
			for(unsigned int iFilter=0; iFilter<m_vecStack.size(); ++iFilter)
			{
				if(pImg) m_vecStack[iFilter]->SetImage(pImg);
				if(pDark) m_vecStack[iFilter]->SetDark(pDark);
				if(pOpen) m_vecStack[iFilter]->SetOpen(pOpen);
					
				m_vecStack[iFilter]->Exec();
			}
		}
};
