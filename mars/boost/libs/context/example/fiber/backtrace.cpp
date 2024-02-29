
//          Copyright Oliver Kowalke 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define UNW_LOCAL_ONLY

#include <cstdlib>
#include <iostream>

#include <libunwind.h>

#include <boost/context/fiber.hpp>

namespace ctx = mars_boost::context;

void backtrace() {
	unw_cursor_t cursor;
	unw_context_t context;
	unw_getcontext( & context);
	unw_init_local( & cursor, & context);
	while ( 0 < unw_step( & cursor) ) {
		unw_word_t offset, pc;
		unw_get_reg( & cursor, UNW_REG_IP, & pc);
		if ( 0 == pc) {
			break;
		}
		std::cout << "0x" << pc << ":";

		char sym[256];
		if ( 0 == unw_get_proc_name( & cursor, sym, sizeof( sym), & offset) ) {
			std::cout << " (" << sym << "+0x" << offset << ")" << std::endl;
		} else {
			std::cout << " -- error: unable to obtain symbol name for this frame" << std::endl;
		}
	}
}

void bar() {
	backtrace();
}

void foo() {
	bar();
}

ctx::fiber f1( ctx::fiber && c) {
    foo();
    return std::move( c);
}

int main() {
    ctx::fiber{ f1 }.resume();
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}
