//  boost/libs/filesystem/test/config_info.cpp  ----------------------------------------//

//  Copyright Beman Dawes 2017

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/filesystem

//--------------------------------------------------------------------------------------//

#include <boost/filesystem.hpp>
#include <iostream>
#include "macro_value.hpp"

using std::cout;
using std::endl;

int main()
{
    cout << "Verify macro reporting works correctly\n";
    cout << "  NOSUCHMACRO: " << BOOST_MACRO_VALUE(NOSUCHMACRO) << endl;
#define SUCHAMACRO
    cout << "  SUCHAMACRO: " << BOOST_MACRO_VALUE(SUCHAMACRO) << endl;
    cout << "  BOOST_VERSION: " << BOOST_MACRO_VALUE(BOOST_VERSION) << endl;

    cout << "Report macro values that may be useful in debugging various test programs\n";
    cout << "  BOOST_VERSION: " << BOOST_MACRO_VALUE(BOOST_VERSION) << endl;
    cout << "  BOOST_FILESYSTEM_VERSION: " << BOOST_MACRO_VALUE(BOOST_FILESYSTEM_VERSION) << endl;
    cout << "  BOOST_FILESYSTEM_DEPRECATED: " << BOOST_MACRO_VALUE(BOOST_FILESYSTEM_DEPRECATED) << endl;
    cout << "  BOOST_FILESYSTEM_SOURCE: " << BOOST_MACRO_VALUE(BOOST_FILESYSTEM_SOURCE) << endl;
    cout << "  BOOST_FILESYSTEM_DYN_LINK: " << BOOST_MACRO_VALUE(BOOST_FILESYSTEM_DYN_LINK) << endl;
    cout << "  BOOST_FILESYSTEM_STATIC_LINK: " << BOOST_MACRO_VALUE(BOOST_FILESYSTEM_STATIC_LINK) << endl;
    cout << "  BOOST_ALL_NO_LIB: " << BOOST_MACRO_VALUE(BOOST_ALL_NO_LIB) << endl;
    cout << "  BOOST_FILESYSTEM_NO_LIB: " << BOOST_MACRO_VALUE(BOOST_FILESYSTEM_NO_LIB) << endl;
    cout << "  BOOST_LIB_NAME: " << BOOST_MACRO_VALUE(BOOST_LIB_NAME) << endl;
    cout << "  BOOST_POSIX_API: " << BOOST_MACRO_VALUE(BOOST_POSIX_API) << endl;
    cout << "  BOOST_WINDOWS_API: " << BOOST_MACRO_VALUE(BOOST_WINDOWS_API) << endl;
    cout << "  _MSC_VER: " << BOOST_MACRO_VALUE(_MSC_VER) << endl;
    cout << "  __MINGW32__: " << BOOST_MACRO_VALUE(__MINGW32__) << endl;
    //cout << "  : " << BOOST_MACRO_VALUE() << endl;
    //cout << "  : " << BOOST_MACRO_VALUE() << endl;
    //cout << "  : " << BOOST_MACRO_VALUE() << endl;
    //cout << "  : " << BOOST_MACRO_VALUE() << endl;
    //cout << "  : " << BOOST_MACRO_VALUE() << endl;
    //cout << "  : " << BOOST_MACRO_VALUE() << endl;
    //cout << "  : " << BOOST_MACRO_VALUE() << endl;

    return 0;
}
