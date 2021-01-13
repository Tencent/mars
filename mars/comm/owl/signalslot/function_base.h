/*
 ============================================================================
 Name		: function_base.h
 Author		: peterfan
 Version	: 1.0
 Created on	: 2010-4-28
 Copyright	: Copyright (C) 1998 - 2009 TENCENT Inc. All Right Reserved
 Description:
 ============================================================================
 */

#ifndef _SIGNALSLOT_FUNCTION_BASE_H_
#define _SIGNALSLOT_FUNCTION_BASE_H_

#include <cstring>

namespace owl {

class alignment_dummy_base1 {};
class alignment_dummy_base2 {};

// single inheritance.
class alignment_dummy_s: alignment_dummy_base1 {};

// multiple inheritance.
class alignment_dummy_m: alignment_dummy_base1, alignment_dummy_base2 {};

// virtual inheritance.
class alignment_dummy_v: virtual alignment_dummy_base1 {};

// unknown (incomplete).
class alignment_dummy_u;


// member function pointer of single inheritance class.
typedef void (alignment_dummy_s::*mfn_pointer_s)();

// member function pointer of multiple inheritance class.
typedef void (alignment_dummy_m::*mfn_pointer_m)();

// member function pointer of virtual inheritance class.
typedef void (alignment_dummy_v::*mfn_pointer_v)();

// member function pointer of unknown (incomplete) class.
typedef void (alignment_dummy_u::*mfn_pointer_u)();

// generic free function pointer.
typedef void (*generic_fn_pointer)();

// generic memeber function pointer
typedef void (alignment_dummy_m::*generic_mfn_pointer)();


struct any_fn_pointer {
	any_fn_pointer() { memset(&u, 0, sizeof(u)); }
	~any_fn_pointer() {}

	bool operator==(const any_fn_pointer& rhs) const {
		return memcmp(&u, &rhs.u, sizeof(u)) == 0;
	}

	bool operator!=(const any_fn_pointer& rhs) const {
		return !(*this == rhs);
	}

	union dummy_union {
		mfn_pointer_s __dummy_mfns;
		mfn_pointer_m __dummy_mfnm;
		mfn_pointer_v __dummy_mfnv;
		mfn_pointer_u __dummy_mfnu;
	};

	union {
		generic_fn_pointer fn;
		generic_mfn_pointer mfn;
		char __dummy[sizeof(dummy_union)];
	} u;
};


struct function_holder {
	function_holder():object(0), stub(0), any(), group('0') {}
	~function_holder() {}

	bool operator==(const function_holder& rhs) const {
		return object == rhs.object && stub == rhs.stub && any == rhs.any && group == rhs.group;
	}

	bool operator!=(const function_holder& rhs) const {
		return !(*this == rhs);
	}

	void* object;
	generic_fn_pointer stub;
	any_fn_pointer any;
	char group;
};


#ifdef _MSC_VER

template <typename A, typename B>
A unsafe_cast(B b) {
    union converter {
        A a;
        B b;
    };

    converter c;
    c.b = b;
    return c.a;
}

#else

#define unsafe_cast reinterpret_cast

#endif

} //namespace owl

#endif /* _SIGNALSLOT_FUNCTION_BASE_H_ */
