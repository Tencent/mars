/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

#ifndef TSSGC_H_
#define TSSGC_H_

#include <windows.h>
#include <thr/threads.h>
#include <list>
#include  <map>
#include  <set>


struct TssResList {
    tss_dtor_t         pfnFree;
    std::map<DWORD, void*>  m_list;

    void add(void* res) {
        m_list[::GetCurrentThreadId()] = res;
    }

    void clearAll() {
        if (m_list.size() <= 0)
            return;

        for (auto itor = m_list.begin() ; itor != m_list.end() ; itor++) {
            void* res = (*itor).second;

            if (res != NULL) {
                pfnFree(res);
            }
        }

        m_list.clear();
    }
};

class TssGC {
  public :
    static  void add(void* tss, tss_dtor_t pfn , void* res);

    static void uninit();
    static void init();
  private :
    static std::map<void*, TssResList>  mTssList;
    static CRITICAL_SECTION    mLock;
    static bool    mLockInitFlag;
    static DWORD   mLastCheckTime;

    static void lock();
    static void unlock();
    static void checkThread();
    static std::set<DWORD> getThreadIdAll(void);
};  // end of  TssGC



#endif


