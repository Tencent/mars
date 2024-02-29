/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if (!defined(BOOST_LIBSTDCXX_VERSION) || \
    BOOST_LIBSTDCXX_VERSION >= 48000) && \
    !defined(BOOST_NO_CXX11_SMART_PTR)
#include <boost/smart_ptr/allocate_unique.hpp>
#include <boost/core/lightweight_test.hpp>

template<class T = void>
struct creator {
    typedef T value_type;
    typedef T* pointer;

    template<class U>
    struct rebind {
        typedef creator<U> other;
    };

    creator() { }

    template<class U>
    creator(const creator<U>&) { }

    T* allocate(std::size_t size) {
        return static_cast<T*>(::operator new(sizeof(T) * size));
    }

    void deallocate(T* ptr, std::size_t) {
        ::operator delete(ptr);
    }
};

template<class T, class U>
inline bool
operator==(const creator<T>&, const creator<U>&)
{
    return true;
}

template<class T, class U>
inline bool
operator!=(const creator<T>&, const creator<U>&)
{
    return false;
}

class type {
public:
    static unsigned instances;

    type() {
        if (instances == 5) {
            throw true;
        }
        ++instances;
    }

    ~type() {
        --instances;
    }

private:
    type(const type&);
    type& operator=(const type&);
};

unsigned type::instances = 0;

int main()
{
    try {
        mars_boost::allocate_unique<type[]>(creator<type>(), 6);
        BOOST_ERROR("allocate_unique did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_unique<type[][2]>(creator<type>(), 3);
        BOOST_ERROR("allocate_unique did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_unique<type[6]>(creator<>());
        BOOST_ERROR("allocate_unique did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_unique<type[3][2]>(creator<>());
        BOOST_ERROR("allocate_unique did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_unique_noinit<type[]>(creator<>(), 6);
        BOOST_ERROR("allocate_unique_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_unique_noinit<type[][2]>(creator<>(), 3);
        BOOST_ERROR("allocate_unique_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_unique_noinit<type[6]>(creator<>());
        BOOST_ERROR("allocate_unique_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_unique_noinit<type[3][2]>(creator<>());
        BOOST_ERROR("allocate_unique_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    return mars_boost::report_errors();
}
#else
int main()
{
    return 0;
}
#endif
