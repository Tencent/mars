//  Boost Filesystem path_times.cpp  ---------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

#include <boost/config/warning_disable.hpp>

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED
#endif

#include <boost/timer/timer.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/cstdint.hpp>

#include <boost/detail/lightweight_main.hpp>

namespace fs = mars_boost::filesystem;
using namespace mars_boost::timer;

#include <fstream>
#include <iostream>

using std::cout;
using std::endl;

namespace {
mars_boost::int64_t max_cycles;

template< class STD_STRING >
nanosecond_type time_ctor(const STD_STRING& s)
{
    mars_boost::timer::auto_cpu_timer tmr;
    mars_boost::int64_t count = 0;
    do
    {
        fs::path p(s);
        ++count;
    } while (count < max_cycles);

    mars_boost::timer::cpu_times elapsed = tmr.elapsed();
    return elapsed.user + elapsed.system;
}

nanosecond_type time_loop()
{
    mars_boost::timer::auto_cpu_timer tmr;
    mars_boost::int64_t count = 0;
    do
    {
        ++count;
    } while (count < max_cycles);

    mars_boost::timer::cpu_times elapsed = tmr.elapsed();
    return elapsed.user + elapsed.system;
}
} // unnamed namespace

//--------------------------------------------------------------------------------------//
//                                      main                                            //
//--------------------------------------------------------------------------------------//

int cpp_main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cout << "Usage: path_times <cycles-in-millions>\n";
        return 1;
    }

    max_cycles = std::atoi(argv[1]) * 1000000LL;
    cout << "testing " << std::atoi(argv[1]) << " million cycles" << endl;

    cout << "time_loop" << endl;
    nanosecond_type x = time_loop();

    cout << "time_ctor with string" << endl;
    nanosecond_type s = time_ctor(std::string("/foo/bar/baz"));

    cout << "time_ctor with wstring" << endl;
    nanosecond_type w = time_ctor(std::wstring(L"/foo/bar/baz"));

    if (s > w)
        cout << "narrow/wide CPU-time ratio = " << long double(s) / w << endl;
    else
        cout << "wide/narrow CPU-time ratio = " << long double(w) / s << endl;

    cout << "returning from main()" << endl;
    return 0;
}
