//
//  copy_wrapper.h
//  comm
//
//  Created by Ray on 13-10-17.
//  Copyright (c) 2013 zhoushaotao. All rights reserved.
//

#ifndef COMM_COPY_WRAPPER_H_
#define COMM_COPY_WRAPPER_H_

template <class S>
class copy_wrapper_helper {
  public:
    static void copy_constructor(S& _lhs, S& _rhs)
    { _lhs = _rhs; }

    static void copy_constructor(S& _lhs, const S& _rhs)
    { _lhs = _rhs; }

    static void destructor(S& _delobj) {}
};

template <class S, class D>
class copy_wrapper_helper_ref {
  public:
    static const D& ref(const S& _ref) { return _ref; }
    static D& ref(S& _ref) { return _ref; }
};

template<class S, class D = S>
class copy_wrapper {
  public:
    typedef S SRC;
    typedef D DST;

    copy_wrapper(S& t)
    { copy_wrapper_helper<S>::copy_constructor(m_t, t); }

    copy_wrapper(const S& t)
    { copy_wrapper_helper<S>::copy_constructor(m_t, t); }

    ~copy_wrapper()
    { copy_wrapper_helper<S>::destructor(m_t);}

    copy_wrapper& operator=(copy_wrapper& _ref) {
        copy_wrapper_helper<S>::copy_constructor(m_t, _ref.m_t);
        return *this;
    }

    copy_wrapper& operator=(const copy_wrapper& _ref) {
        copy_wrapper_helper<S>::copy_constructor(m_t, _ref.m_t);
        return *this;
    }

    copy_wrapper(copy_wrapper& _ref)
    { copy_wrapper_helper<S>::copy_constructor(m_t, _ref.m_t); }

    copy_wrapper(const copy_wrapper& _ref)
    { copy_wrapper_helper<S>::copy_constructor(m_t, _ref.m_t); }

    operator D& () { return copy_wrapper_helper_ref<S, D>::ref(m_t); }
    operator const D& () const { return copy_wrapper_helper_ref<S, D>::ref(m_t); }

    D* operator->() { return &copy_wrapper_helper_ref<S, D>::ref(m_t); }
    const D* operator->() const { return &copy_wrapper_helper_ref<S, D>::ref(m_t); }

    D& get() { return copy_wrapper_helper_ref<S, D>::ref(m_t); }
    const D& get() const { return copy_wrapper_helper_ref<S, D>::ref(m_t); }

    D* get_pointer() { return &copy_wrapper_helper_ref<S, D>::ref(m_t); }
    const D* get_pointer() const { return &copy_wrapper_helper_ref<S, D>::ref(m_t); }

  private:
    S m_t;
};


#endif	// COMM_COPY_WRAPPER_H_
