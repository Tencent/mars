// Copyright (C) 2001-2003
// William E. Kempf
//
// Copyright Frank Mori Hess 2009
//
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This is a simplified/modified version of libs/thread/test/test_mutex.cpp
// added to test mars_boost::signals2::mutex.
// For more information, see http://www.boost.org

// Note boost/test/minimal.hpp can cause windows.h to get included, which
// can screw up our checks of _WIN32_WINNT if it is included
// after boost/signals2/mutex.hpp.  Frank Hess 2009-03-07.
// We now use boost/test/included/unit_test.hpp, not sure if above still
// applies, but might as well leave the include where it is.
#define BOOST_TEST_MODULE mutex_test
#include <boost/test/included/unit_test.hpp>

#include <boost/bind/bind.hpp>
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/condition.hpp>

using namespace mars_boost::placeholders;

class execution_monitor
{
public:
    execution_monitor(int secs)
        : done(false), m_secs(secs) { }
    void start()
    {
        mars_boost::mutex::scoped_lock lock(mutex); 
        done = false;
    }
    void finish()
    {
        mars_boost::mutex::scoped_lock lock(mutex);
        done = true;
        cond.notify_one();
    }
    bool wait()
    {
        mars_boost::posix_time::time_duration timeout = mars_boost::posix_time::seconds(m_secs);
        mars_boost::mutex::scoped_lock lock(mutex);
        while (!done) {
            if (!cond.timed_wait(lock, timeout))
                break;
        }
        return done;
    }

private:
    mars_boost::mutex mutex;
    mars_boost::condition cond;
    bool done;
    int m_secs;
};

template <typename F>
class indirect_adapter
{
public:
    indirect_adapter(F func, execution_monitor& monitor)
        : m_func(func), m_monitor(monitor) { }
    void operator()() const
    {
        try
        {
            mars_boost::thread thrd(m_func);
            thrd.join();
        }
        catch (...)
        {
            m_monitor.finish();
            throw;
        }
        m_monitor.finish();
    }

private:
    F m_func;
    execution_monitor& m_monitor;
    void operator=(indirect_adapter&);
};

template <typename F>
void timed_test(F func, int secs)
{
    execution_monitor monitor(secs);
    indirect_adapter<F> ifunc(func, monitor);
    monitor.start();
    mars_boost::thread thrd(ifunc);
    BOOST_REQUIRE(monitor.wait()); // Timed test didn't complete in time, possible deadlock
}

template <typename M>
struct test_lock
{
    typedef M mutex_type;
    typedef typename mars_boost::unique_lock<M> lock_type;

    void operator()()
    {
        mutex_type mutex;
        mars_boost::condition condition;

        // Test the lock's constructors.
        {
            lock_type lock(mutex, mars_boost::defer_lock);
            BOOST_CHECK(!lock);
        }
        lock_type lock(mutex);
        BOOST_CHECK(lock ? true : false);

        // Construct a fast time out.
        mars_boost::posix_time::time_duration timeout = mars_boost::posix_time::milliseconds(100);

        // Test the lock and the mutex with condition variables.
        // No one is going to notify this condition variable.  We expect to
        // time out.
        BOOST_CHECK(!condition.timed_wait(lock, timeout));
        BOOST_CHECK(lock ? true : false);

        // Test the lock and unlock methods.
        lock.unlock();
        BOOST_CHECK(!lock);
        lock.lock();
        BOOST_CHECK(lock ? true : false);
    }
};

template <typename M>
struct test_trylock
{
    typedef M mutex_type;
    typedef typename mars_boost::unique_lock<M> lock_type;

    void operator()()
    {
        mutex_type mutex;
        mars_boost::condition condition;

        // Test the lock's constructors.
        {
            lock_type lock(mutex, mars_boost::try_to_lock);
            BOOST_CHECK(lock ? true : false);
        }
        {
            lock_type lock(mutex, mars_boost::defer_lock);
            BOOST_CHECK(!lock);
        }
        lock_type lock(mutex, mars_boost::try_to_lock);
        BOOST_CHECK(lock ? true : false);

        // Construct a fast time out.
        mars_boost::posix_time::time_duration timeout = mars_boost::posix_time::milliseconds(100);

        // Test the lock and the mutex with condition variables.
        // No one is going to notify this condition variable.  We expect to
        // time out.
        BOOST_CHECK(!condition.timed_wait(lock, timeout));
        BOOST_CHECK(lock ? true : false);

        // Test the lock, unlock and trylock methods.
        lock.unlock();
        BOOST_CHECK(!lock);
        lock.lock();
        BOOST_CHECK(lock ? true : false);
        lock.unlock();
        BOOST_CHECK(!lock);
        BOOST_CHECK(lock.try_lock());
        BOOST_CHECK(lock ? true : false);
    }
};

template<typename Mutex>
struct test_lock_exclusion
{
    typedef mars_boost::unique_lock<Mutex> Lock;

    Mutex m;
    mars_boost::mutex done_mutex;
    bool done;
    bool locked;
    mars_boost::condition_variable done_cond;

    test_lock_exclusion():
        done(false),locked(false)
    {}

    void locking_thread()
    {
        Lock lock(m);

        mars_boost::lock_guard<mars_boost::mutex> lk(done_mutex);
        locked=lock.owns_lock();
        done=true;
        done_cond.notify_one();
    }

    bool is_done() const
    {
        return done;
    }

    typedef test_lock_exclusion<Mutex> this_type;

    void do_test(void (this_type::*test_func)())
    {
        Lock lock(m);

	{
            mars_boost::lock_guard<mars_boost::mutex> lk(done_mutex);
            locked=false;
	}

        done=false;

        mars_boost::thread t(test_func,this);

        try
        {
            {
                mars_boost::mutex::scoped_lock lk(done_mutex);
                BOOST_CHECK(!done_cond.timed_wait(lk, mars_boost::posix_time::seconds(1),
                                                 mars_boost::bind(&this_type::is_done,this)));
            }
            lock.unlock();
            {
                mars_boost::mutex::scoped_lock lk(done_mutex);
                BOOST_CHECK(done_cond.timed_wait(lk, mars_boost::posix_time::seconds(1),
                                                 mars_boost::bind(&this_type::is_done,this)));
            }
            t.join();
            BOOST_CHECK(locked);
        }
        catch(...)
        {
            lock.unlock();
            t.join();
            throw;
        }
    }


    void operator()()
    {
        do_test(&this_type::locking_thread);
    }
};


void do_test_mutex()
{
    test_lock<mars_boost::signals2::mutex>()();
// try_lock not supported on old versions of windows
#if !defined(BOOST_HAS_WINTHREADS) || (defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400))
    test_trylock<mars_boost::signals2::mutex>()();
#endif
    test_lock_exclusion<mars_boost::signals2::mutex>()();
}

void test_mutex()
{
    timed_test(&do_test_mutex, 3);
}

void do_test_dummy_mutex()
{
    test_lock<mars_boost::signals2::dummy_mutex>()();
    test_trylock<mars_boost::signals2::dummy_mutex>()();
}

void test_dummy_mutex()
{
    timed_test(&do_test_dummy_mutex, 2);
}

BOOST_AUTO_TEST_CASE(test_main)
{
    test_mutex();
    test_dummy_mutex();
}
