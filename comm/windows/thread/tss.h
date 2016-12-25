
#ifndef TSS_H_
#define TSS_H_


#if UWP  
struct TssValuePack
{
	tss_dtor_t m_cleanup;//free or delete or heepfree...
	void * m_pVal;
};

class Tss {
  public:
    explicit Tss(tss_dtor_t cleanup) {
        m_dwTlsIndex = FlsAlloc((Tss::on_thread_exitOrTssFreeed));
		m_cleanup = cleanup;
    }

    ~Tss() {
        FlsFree(m_dwTlsIndex);
    }

	void* get() const {
		TssValuePack * pValuePack = (TssValuePack *)FlsGetValue(m_dwTlsIndex); 
		if (NULL != pValuePack)
		{
			return pValuePack->m_pVal;
		}
        return NULL;
    }

    void set(void* value
		) {
		TssValuePack * pValuePack = (TssValuePack *)FlsGetValue(m_dwTlsIndex);
		if (NULL != pValuePack)
		{
			pValuePack->m_pVal = value;
			return;
		}

		pValuePack = (TssValuePack *)malloc(sizeof(TssValuePack));
		pValuePack->m_cleanup = m_cleanup;
		pValuePack->m_pVal = value;
        FlsSetValue(m_dwTlsIndex, pValuePack);
    }

  private:
    Tss(const Tss&);
    Tss& operator =(const Tss&);

  private:
    int  m_dwTlsIndex ;
	tss_dtor_t m_cleanup;
	static void NTAPI  on_thread_exitOrTssFreeed(
		IN PVOID lpFlsData
		)
	{
		TssValuePack * pValuePack = (TssValuePack *)lpFlsData;
		if (NULL != pValuePack)
		{
			if (NULL != pValuePack->m_pVal && NULL != pValuePack->m_cleanup)
			{
				pValuePack->m_cleanup(pValuePack->m_pVal);
			}
			else
			{
			}
			free(pValuePack);
		}
	}
};

#elif WIN32
#include "TssGC.h"

class Tss {
  public:
    explicit Tss(tss_dtor_t cleanup) {
        m_dwTlsIndex = TlsAlloc();
        m_cleanup = cleanup;
    }

    ~Tss() {
        TlsFree(m_dwTlsIndex);
    }

    void* get() const {
        return TlsGetValue(m_dwTlsIndex);
    }

    void set(void* value) {
        TlsSetValue(m_dwTlsIndex, value);

        TssGC::add(this, m_cleanup, value);
    }

  private:
    Tss(const Tss&);
    Tss& operator =(const Tss&);

  private:
    int  m_dwTlsIndex ;
    tss_dtor_t m_cleanup;
};

#else  // tss implement by vc11 tss lib

#include <thr/threads.h>
#include "assert/__assert.h"

class Tss {
  public:
    explicit Tss(tss_dtor_t cleanup) {
        int ret = tss_create(&m_key, cleanup);
        ASSERT2(thrd_success == ret, "%d", ret);
    }

    ~Tss() {
        tss_delete(m_key);
    }

    void* get() const {
        return tss_get(m_key);
    }

    void set(void* value) {
        tss_set(m_key, value);
    }

  private:
    Tss(const Tss&);
    Tss& operator =(const Tss&);

  private:
    tss_t m_key;
};

#endif  // end of tss

#endif /* TSS_H_ */
