/*
 * function.h
 *
 *  Created on: 2018年4月8日
 *      Author: peterfan
 */

#ifndef _SIGNALSLOT_FUNCTION_H_
#define _SIGNALSLOT_FUNCTION_H_

#include <functional>
#include <cassert>

#include "function_base.h"

namespace owl {

template <typename T>
class function;

template <typename R, typename... Args>
class function<R (Args...)> {
	typedef R (*invoke_stub_function)(void*, const any_fn_pointer&, Args&&...);
public:
	function() {}
	~function() {}

	template <typename T, typename U>
	function(T* object, R (U::*mfn)(Args...)) {
		static_assert((std::is_base_of<U, T>::value), "type T must be derived from U");
		typedef R (T::*F)(Args...);
		holder_.object = object;
		holder_.stub = reinterpret_cast<generic_fn_pointer>(&mfn_stub<T, F>::invoke);
		holder_.any.u.mfn = unsafe_cast<generic_mfn_pointer>(mfn);
	}

	template <typename T, typename U>
	function(const T* object, R (U::*mfn)(Args...) const) {
		static_assert((std::is_base_of<U, T>::value), "type T must be derived from U");
		typedef R (T::*F)(Args...) const;
		holder_.object = const_cast<T*>(object);
		holder_.stub = reinterpret_cast<generic_fn_pointer>(&mfn_stub<const T, F>::invoke);
		holder_.any.u.mfn = unsafe_cast<generic_mfn_pointer>(mfn);
	}

	function(R (*fn)(Args...)) {
		typedef R (*F)(Args...);
		holder_.object = nullptr;
		holder_.stub = reinterpret_cast<generic_fn_pointer>(&fn_stub<F>::invoke);
		holder_.any.u.fn = reinterpret_cast<generic_fn_pointer>(fn);
	}

    R operator()(Args... args) const {
        invoke_stub_function stub = reinterpret_cast<invoke_stub_function>(holder_.stub);
        return (*stub)(holder_.object, holder_.any, std::forward<Args>(args)...);
    }
    
    template <typename... Args2>
    static R invoke(function_holder& holder, Args2&&... args) {
        invoke_stub_function stub = reinterpret_cast<invoke_stub_function>(holder.stub);
        return (*stub)(holder.object, holder.any, std::forward<Args2>(args)...);
    }

	operator bool() const {
		return holder_.stub != 0;
	}

	bool operator!() const {
		return !(operator bool());
	}

	bool operator==(const function& rhs) const {
		return holder_ == rhs.holder_;
	}

	bool operator!=(const function& rhs) const {
		return !(*this == rhs);
	}

	const void* object() const {
		return holder_.object;
	}

	const function_holder& holder() const {
		return holder_;
	}

private:
	template <typename T, typename F>
	struct mfn_stub {
		static R invoke(void* obj_ptr, const any_fn_pointer& any, Args&&... args) {
			T* object = static_cast<T*>(obj_ptr);
			F mfn = unsafe_cast<F>(any.u.mfn);
			return (object->*mfn)(std::forward<Args>(args)...);
		}
	};

	template <typename F>
	struct fn_stub {
		static R invoke(void*, const any_fn_pointer& any, Args&&... args) {
			F fn = reinterpret_cast<F>(any.u.fn);
			return (*fn)(std::forward<Args>(args)...);
		}
	};

private:
	function_holder holder_;
};


template <typename T, typename U, typename R, typename... Args>
inline function<R (Args...)> make_function(T* object, R (U::*mfn)(Args...)) {
	assert(object && mfn);
	return function<R (Args...)>(object, mfn);
}

template <typename T, typename U, typename R, typename... Args>
inline function<R (Args...)> make_function(const T* object, R (U::*mfn)(Args...) const) {
	assert(object && mfn);
	return function<R (Args...)>(object, mfn);
}

template <typename R, typename... Args>
inline function<R (Args...)> make_function(R (*fn)(Args...)) {
	assert(fn);
	return function<R (Args...)>(fn);
}

} //namespace owl

#endif /* _SIGNALSLOT_FUNCTION_H_ */
