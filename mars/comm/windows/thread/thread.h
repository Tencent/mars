// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#ifndef THREAD_H_
#define THREAD_H_

#include <errno.h>
#include <stdlib.h>
#include <thr/threads.h>

#include "assert/__assert.h"
#include "condition.h"
#include "thread/runnable.h"


#ifndef _WINRT_DLL   // for wp8 
//#define USED_BOOST_THREAD_LIB   // else used vc11 std thread 
#endif


#ifdef USED_BOOST_THREAD_LIB
#include <boost/thread/thread.hpp>
typedef boost::thread*  thread_handler;
#else
#include <thr/threads.h>
typedef thrd_t*  thread_handler;
#endif

#define thrd_success 0

typedef unsigned int thread_tid;

typedef void* (*THREAD_START_PROC)(void*  arg);

class ThreadUtil {
  public:
    static void yield() {
#ifdef  USED_BOOST_THREAD_LIB
        boost::this_thread::yield();
#else
        thrd_yield();
#endif
    }

    static void sleep(unsigned int _sec) {
        struct xtime xt = {0, 0};
        xtime_get(&xt, TIME_UTC);
        xt.sec += _sec;
        thrd_sleep(&xt);
    }

    static void usleep(unsigned int _usec) {
        struct xtime xt = {0, 0};
        xtime_get(&xt, TIME_UTC);
        xt.nsec += _usec;
        thrd_sleep(&xt);
    }

    static thread_tid currentthreadid() {
#ifdef  USED_BOOST_THREAD_LIB
        return boost::detail::win32::GetCurrentThreadId();
#else
        return thrd_current()._Id;
#endif
    }

    static bool isruning(thread_tid /*_id*/);
    //{
    //    ASSERT(false);
    //    return false;
    //}

    static int createThread(thread_handler& pth, THREAD_START_PROC proc, void* args) {
#ifdef  USED_BOOST_THREAD_LIB

        if (pth) {
            pth->detach();
            delete pth;
            pth = NULL;
        }

        pth = new boost::thread(proc, args);
        return thrd_success;
#else

        if (pth != NULL) {
            delete pth;
        }

        pth = new thrd_t();
        return thrd_create(pth, (thrd_start_t)proc, (void*)args);
#endif
    }

    static void join(thread_handler& pth) {
        if (pth == NULL)
            return ;

#ifdef  USED_BOOST_THREAD_LIB
        pth->join();
#else
        thrd_join(*pth, 0);
#endif
    }

	static void join (thread_tid _tid) {
	
#ifdef USED_BOOST_THREAD_LIB
	#error "todo"
#else
	HANDLE handler = OpenThread(THREAD_ALL_ACCESS, FALSE, _tid);
	if (NULL == handler) {
		ASSERT(false);
		return;
	}
	thrd_t thrd;
	thrd._Hnd = &handler;
	thrd._Id = _tid;
	thread_handler th = &thrd;
	join(th);
	CloseHandle(handler);
#endif
	}

    static void detach(thread_handler& pth) {
        if (pth == NULL)
            return ;

#ifdef  USED_BOOST_THREAD_LIB
        pth->detach();
#else
        thrd_detach(*pth);
#endif
    }

    static thread_tid getThreadId(thread_handler& pth) {
        if (pth == NULL)
            return 0;

#ifdef  USED_BOOST_THREAD_LIB
        return pth->get_thread_info()->id;
#else
        return  pth->_Id;
#endif
    }
};

class Thread {
  private:
    class RunnableReference {
      public:
        RunnableReference(Runnable* _target)
            : target(_target), count(0), isjoined(false), isended(true) ,
              aftertime(UINT_MAX), periodictime(UINT_MAX), iscanceldelaystart(false)
            , isinthread(false), killsig(0) {
            // tid._Hnd = 0;
            // tid._Id = 0;
            m_th = NULL;
            // ASSERT(target);
        }

        ~RunnableReference() {
            delete target;
            ASSERT(0 == count);
            ASSERT(isended);

            if (m_th != NULL) {
                delete m_th;
                m_th = NULL;
            }
        }

        void AddRef() { count++;}
        void RemoveRef(ScopedSpinLock& _lock) {
            ASSERT(0 < count);
            ASSERT(_lock.islocked());

            bool willdel = false;
            count--;

            if (0 == count) willdel = true;

            _lock.unlock();

            if (willdel) delete this;
        }

      private:
        RunnableReference(const RunnableReference&);
        RunnableReference& operator=(const RunnableReference&);

      public:
        Runnable* target;
        int count;
        // thrd_t tid;
        thread_handler m_th;
        bool isjoined;
        bool isended;
        unsigned int aftertime;
        unsigned int periodictime;
        bool iscanceldelaystart;
        Condition condtime;
        SpinLock splock;
        bool isinthread; 
        int killsig;
    };

  public:
    template<class T>
    explicit Thread(const T& op, const char* _thread_name = NULL, bool _outside_join = false)
        : m_runableref(NULL), outside_join_(_outside_join) {
        m_runableref = new RunnableReference(detail::transform(op));
        ScopedSpinLock lock(m_runableref->splock);
        m_runableref->AddRef();
    }

    Thread(const char* _thread_name = NULL, bool _outside_join = false)
        : m_runableref(NULL), outside_join_(_outside_join) {
        m_runableref = new RunnableReference(NULL);
        ScopedSpinLock lock(m_runableref->splock);
        m_runableref->AddRef();
    }

    virtual ~Thread() {
        ScopedSpinLock lock(m_runableref->splock);
        m_runableref->RemoveRef(lock);
    }

    int start(bool* _newone = NULL) {
        ScopedSpinLock lock(m_runableref->splock);

        if (_newone) *_newone = false;

        if (isruning())return 0;

        m_runableref->isended = false;
		m_runableref->isjoined = outside_join_;
        m_runableref->AddRef();

        // int ret = thrd_create(&m_runableref->tid, (thrd_start_t)&start_routine, (void*)m_runableref);
        int ret = ThreadUtil::createThread(m_runableref->m_th, &start_routine, (void*)m_runableref);
        ASSERT(thrd_success == ret);

        if (_newone) *_newone = true;

        if (thrd_success != ret) {
            m_runableref->isended = true;
            m_runableref->RemoveRef(lock);
        }

        return ret;
    }

    template <typename T>
    int start(const T& op, bool* _newone = NULL) {
        ScopedSpinLock lock(m_runableref->splock);

        if (_newone) *_newone = false;

        if (isruning())return 0;

        delete m_runableref->target;
        m_runableref->target = detail::transform(op);

        m_runableref->isended = false;
		m_runableref->isjoined = outside_join_;
        m_runableref->AddRef();

        // int ret = thrd_create(&m_runableref->tid, (thrd_start_t)&start_routine, (void*)m_runableref);
        int ret = ThreadUtil::createThread(m_runableref->m_th, &start_routine, (void*)m_runableref);
        ASSERT(thrd_success == ret);

        if (_newone) *_newone = true;

        if (thrd_success != ret) {
            m_runableref->isended = true;
            m_runableref->RemoveRef(lock);
        }

        return ret;
    }

    int start_after(unsigned int after) {
        ScopedSpinLock lock(m_runableref->splock);

        if (isruning())return 0;

        m_runableref->condtime.cancelAnyWayNotify();
        m_runableref->iscanceldelaystart = false;
        m_runableref->isended = false;
		m_runableref->isjoined = outside_join_;
        m_runableref->aftertime = after;
        m_runableref->AddRef();

        // int ret = thrd_create(&m_runableref->tid, (thrd_start_t)&start_routine_after, (void*)m_runableref);
        int ret = ThreadUtil::createThread(m_runableref->m_th, &start_routine_after, (void*)m_runableref);
        ASSERT(thrd_success == ret);

        if (thrd_success != ret) {
            m_runableref->isended = true;
            m_runableref->aftertime = UINT_MAX;
            m_runableref->RemoveRef(lock);
        }

        return ret;
    }

    void cancel_after() {
        ScopedSpinLock lock(m_runableref->splock);

        if (!isruning()) return;

        m_runableref->iscanceldelaystart = true;
        m_runableref->condtime.notifyAll(true);
    }

    int start_periodic(unsigned int after, unsigned int periodic) { // ms
        ScopedSpinLock lock(m_runableref->splock);

        if (isruning()) return 0;

        m_runableref->condtime.cancelAnyWayNotify();
        m_runableref->iscanceldelaystart = false;
        m_runableref->isended = false;
		m_runableref->isjoined = outside_join_;
        m_runableref->aftertime = after;
        m_runableref->periodictime = periodic;
        m_runableref->AddRef();

        // int ret = thrd_create(&m_runableref->tid, (thrd_start_t)&start_routine_periodic, (void*)m_runableref);
        int ret = ThreadUtil::createThread(m_runableref->m_th, &start_routine_periodic, (void*)m_runableref);
        ASSERT(thrd_success == ret);

        if (thrd_success != ret) {
            m_runableref->isended = true;
            m_runableref->aftertime = UINT_MAX;
            m_runableref->periodictime = UINT_MAX;
            m_runableref->RemoveRef(lock);
        }

        return ret;
    }

    void cancel_periodic() {
        ScopedSpinLock lock(m_runableref->splock);

        if (!isruning()) return;

        m_runableref->iscanceldelaystart = true;
        m_runableref->condtime.notifyAll(true);
    }

    void join() const {
        ScopedSpinLock lock(m_runableref->splock);
        ASSERT(!m_runableref->isjoined);

        if (isruning()) {
            m_runableref->isjoined = true;
            lock.unlock();
            ThreadUtil::join(m_runableref->m_th);
            // thrd_join(m_runableref->tid, 0);
        }
    }

    int kill(int /*sig*/) const;
    //  {
    // ASSERT(false);
    //  }

    thread_tid tid() const {
        return ThreadUtil::getThreadId(m_runableref->m_th);
        // return m_runableref->tid._Id;
    }

    bool isruning() const {
        return !m_runableref->isended;
    }

  private:
    static void init(void* arg) {
        volatile RunnableReference* runableref = static_cast<RunnableReference*>(arg);
        ScopedSpinLock lock((const_cast<RunnableReference*>(runableref))->splock);
        ASSERT(runableref != 0);
        ASSERT(runableref->target != 0);
        ASSERT(!runableref->isinthread);

        runableref->isinthread = true;

        if (!(0 < runableref->killsig && runableref->killsig <= 32))
            return;

        lock.unlock();
    }

    static void cleanup(void* arg) {
        volatile RunnableReference* runableref = static_cast<RunnableReference*>(arg);
        ScopedSpinLock lock((const_cast<RunnableReference*>(runableref))->splock);

        ASSERT(runableref != 0);
        ASSERT(runableref->target != 0);
        ASSERT(runableref->isinthread);

        runableref->isinthread = false;
        runableref->killsig = 0;
        runableref->isended = true;

        if (!runableref->isjoined) {
            // thrd_detach(const_cast<RunnableReference*>(runableref)->tid);
            ThreadUtil::detach(const_cast<RunnableReference*>(runableref)->m_th);
        }

        runableref->isjoined = false;
        (const_cast<RunnableReference*>(runableref))->RemoveRef(lock);
    }

    static void* start_routine(void* arg) {
        init(arg);
        volatile RunnableReference* runableref = static_cast<RunnableReference*>(arg);
        runableref->target->run();
        cleanup(arg);
        return 0;
    }

    static void* start_routine_after(void* arg) {
        init(arg);
        volatile RunnableReference* runableref = static_cast<RunnableReference*>(arg);

        if (!runableref->iscanceldelaystart) {
            (const_cast<RunnableReference*>(runableref))->condtime.wait(runableref->aftertime);

            if (!runableref->iscanceldelaystart)
                runableref->target->run();
        }

        cleanup(arg);
        return 0;
    }

    static void* start_routine_periodic(void* arg) {
        init(arg);
        volatile RunnableReference* runableref = static_cast<RunnableReference*>(arg);

        if (!runableref->iscanceldelaystart) {
            (const_cast<RunnableReference*>(runableref))->condtime.wait(runableref->aftertime);

            while (!runableref->iscanceldelaystart) {
                runableref->target->run();

                if (!runableref->iscanceldelaystart)
                    (const_cast<RunnableReference*>(runableref))->condtime.wait(runableref->periodictime);
            }
        }

        cleanup(arg);
        return 0;
    }
  private:
    Thread(const Thread&);
    Thread& operator=(const Thread&);
  private:
    RunnableReference*  m_runableref;
	bool outside_join_;
};


// inline bool operator==(const thread_t& lhs, const thread_t& rhs)
//{
//    return lhs== rhs;
//}


#endif /* THREAD_H_ */
