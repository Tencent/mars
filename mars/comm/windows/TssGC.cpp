/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

#include "TssGC.h"
#include "TlHelp32.h"

std::map<void*, TssResList> TssGC::mTssList;
CRITICAL_SECTION            TssGC::mLock;

bool    TssGC::mLockInitFlag = false;
DWORD   TssGC::mLastCheckTime;

void TssGC::lock() {
    if (!mLockInitFlag) {
        mLockInitFlag = true;
        ::InitializeCriticalSection(&mLock);
    }

    ::EnterCriticalSection(&mLock);
}

void TssGC::unlock() {
    ::LeaveCriticalSection(&mLock);
}

// res = null
void TssGC::add(void* tss, tss_dtor_t pfn , void* res) {
    if (!tss || !pfn)
        return;

    lock();

    int thread_id = ::GetCurrentThreadId();

    mTssList[tss].pfnFree = pfn;
    mTssList[tss].m_list[thread_id] = res;

    checkThread();
    unlock();
}

#define   TIME_MIN_30    ((30 * 60 * 1000))

void TssGC::checkThread() {
    if (mLastCheckTime == 0) {
        mLastCheckTime = ::GetTickCount();
        return;
    }

    DWORD time_span = ::GetTickCount() - mLastCheckTime;

    if (time_span  <  TIME_MIN_30)
        return;

    mLastCheckTime = ::GetTickCount();

    auto threadIds = getThreadIdAll();

    //�������� tss
    for (auto itor = mTssList.begin() ; itor != mTssList.end() ; itor++) {
        TssResList* tss = &((*itor).second);

        // ±éÀútss res
        for (auto tss_item = tss->m_list.begin();  tss_item != tss->m_list.end();) {
            DWORD id = (*tss_item).first;
            void*  res = (*tss_item).second;

            if (threadIds.find(id) == threadIds.end()) {  //É¾³ý res
                tss->pfnFree(res);
                tss->m_list.erase(tss_item++);
            } else {
                tss_item++;
            }
        }
    }
}


std::set<DWORD> TssGC::getThreadIdAll(void) {
    std::set<DWORD> idList;
    DWORD processId = ::GetCurrentProcessId();
    HANDLE hmeSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processId);

    if (hmeSnapshot == NULL)
        return idList;

    THREADENTRY32 te = { sizeof(te) };
    BOOL fOK1 = Thread32First(hmeSnapshot, &te);

    for (; fOK1; fOK1 = Thread32Next(hmeSnapshot, &te)) {
        if (te.th32OwnerProcessID != processId)
            continue;

        idList.insert(te.th32ThreadID);
    }

    return idList;
}


void  TssGC::uninit() {
    lock();

    //±éÀúËùÓÐ tss
    for (auto itor = mTssList.begin() ; itor != mTssList.end() ; itor++) {
        TssResList* tss = &((*itor).second);

        // ±éÀútss res
        for (auto tss_item = tss->m_list.begin();  tss_item != tss->m_list.end();) {
            DWORD id = (*tss_item).first;
            void*  res = (*tss_item).second;
            tss->pfnFree(res);
            tss->m_list.erase(tss_item++);
        }
    }

    mTssList.clear();
    unlock();
}

void TssGC::init() {
}