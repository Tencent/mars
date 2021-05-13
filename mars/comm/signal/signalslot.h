/*
 * signalslot.h
 *
 *  Created on: 2018年4月9日
 *      Author: peterfan
 */

#ifndef _SIGNALSLOT_SIGNALSLOT_H_
#define _SIGNALSLOT_SIGNALSLOT_H_

#include <cassert>
#include <type_traits>
#include <functional>
#include <list>
#include <algorithm>

#include "function_base.h"
#include "signal_base.h"
#include "function.h"
#include "mpl/preprocessor.h"
#include "mpl/meta_function.h"
#include "mpl/scope.h"

namespace owl {

class signal_conn_t {
public:
    signal_conn_t(void* ptr):ptr_(ptr) {}
    void* ptr() const { return ptr_; }
    
private:
    void* ptr_ = nullptr;
};

struct stdfn_slot_list {
    stdfn_slot_list():signal_destroyed(false) {}
    ~stdfn_slot_list() {}
    
    std::list<void*> list;
    bool signal_destroyed;
};


template <typename Signature> class xsignal;

template <typename R, typename... Args>
class xsignal<R (Args...)>: public signal_base, public trackable {
	typedef function<R (Args...)> function_type;
	typedef xsignal<R (Args...)> xsignal_type;
public:
    xsignal() {
        stdfn_slot_list_ = new stdfn_slot_list();
    }
	virtual ~xsignal() {
        if (is_blocked_) {
            stdfn_slot_list_->signal_destroyed = true;
        }
        else {
            clear_stdfn_slot_list(stdfn_slot_list_);
        }
    }

	template <typename T, typename U>
	void connect(T* object, R (U::*mfn)(Args...)) {
		assert(object && mfn);
		connect_impl(object, mfn, std::is_base_of<trackable, T>());
	}

	template <typename T, typename U>
	void connect(const T* object, R (U::*mfn)(Args...) const) {
		assert(object && mfn);
		connect_impl(object, mfn, std::is_base_of<trackable, T>());
	}

	void connect(R (*fn)(Args...)) {
		assert(fn);
		function_type f(fn);
		connect_slot(f.holder(), 0);
	}

	void connect(xsignal_type& sig) {
		connect_impl(&sig, &xsignal_type::operator(), std::is_base_of<trackable, xsignal_type>());
	}

private:
	typedef std::function<R (Args...)> stdfunction_type;

	stdfn_slot_list* stdfn_slot_list_;
    
	static void clear_stdfn_slot_list(stdfn_slot_list* stdfn_list) {
		for (auto fn: stdfn_list->list) {
			auto stdfn = static_cast<stdfunction_type*>(fn);
			delete stdfn;
		}
        delete stdfn_list;
	}
    
public:
	template <typename F, typename std::enable_if<!std::is_function<F>::value>::type* = nullptr>
	signal_conn_t connect(const F& fn) {
		stdfunction_type* stdfn = new stdfunction_type(fn);
		stdfn_slot_list_->list.push_back(stdfn);
		connect_impl((const stdfunction_type*)stdfn, &stdfunction_type::operator(), std::false_type());
        return stdfn;
	}

    template <typename F, typename std::enable_if<!std::is_function<F>::value>::type* = nullptr>
    signal_conn_t operator+=(const F& fn) {
        return connect(fn);
    }

    void disconnect(signal_conn_t conn) {
        auto& list = stdfn_slot_list_->list;
        auto it = std::find(list.begin(), list.end(), conn.ptr());
        if (it != list.end()) {
            list.erase(it);
        }
        disconnect(conn.ptr());
    }
    
	template <typename T, typename U>
	void disconnect(T* object, R (U::*mfn)(Args...)) {
		assert(object && mfn);
		function_type f(object, mfn);
		disconnect_slot(f.holder());
	}

	template <typename T, typename U>
	void disconnect(const T* object, R (U::*mfn)(Args...) const) {
		assert(object && mfn);
		function_type f(object, mfn);
		disconnect_slot(f.holder());
	}

	void disconnect(R (*fn)(Args...)) {
		assert(fn);
		function_type f(fn);
		disconnect_slot(f.holder());
	}

	void disconnect(const void* object) {
		assert(object);
		signal_base::disconnect(object);
	}

	void disconnect(xsignal_type& sig) {
		function_type f(&sig, &xsignal_type::operator());
		disconnect_slot(f.holder());
	}

	void operator()(Args... args) {
		if (is_blocked())
			return;

		block();

        stdfn_slot_list* stdfn_list = stdfn_slot_list_;
		slot_list* list = slot_list_;
        
        OWL_SCOPE_EXIT {
            clear_slot_list(list);
            if (stdfn_list->signal_destroyed) {
                clear_stdfn_slot_list(stdfn_list);
            } else {
                unblock();
            }
        };
        
		iterator_type it = slot_list_->list.iterator();
		while (it.has_next()) {
			slot_function& slot = it.next();
			if (!slot.deleted_) {
				function_type::invoke(slot.holder_, std::forward<Args>(args)...);
			}
		}
	}

private:
	template <typename T, typename U>
	void connect_impl(T* object, R (U::*mfn)(Args...), std::true_type) {
		function_type f(object, mfn);
		connect_slot(f.holder(), static_cast<trackable*>(object));
		object->append_tracker(this);
		object->set_this(object);
	}

	template <typename T, typename U>
	void connect_impl(T* object, R (U::*mfn)(Args...), std::false_type) {
		function_type f(object, mfn);
		connect_slot(f.holder(), 0);
	}

	template <typename T, typename U>
	void connect_impl(const T* object, R (U::*mfn)(Args...) const, std::true_type) {
		function_type f(object, mfn);
		connect_slot(f.holder(), static_cast<const trackable*>(object));
		object->append_tracker(this);
		object->set_this(object);
	}

	template <typename T, typename U>
	void connect_impl(const T* object, R (U::*mfn)(Args...) const, std::false_type) {
		function_type f(object, mfn);
		connect_slot(f.holder(), 0);
	}
};

} //namespace owl

#endif /* _SIGNALSLOT_SIGNALSLOT_H_ */
