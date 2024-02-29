/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if (!defined(BOOST_LIBSTDCXX_VERSION) || \
    BOOST_LIBSTDCXX_VERSION >= 46000) && \
    !defined(BOOST_NO_CXX11_SMART_PTR) && \
    !defined(BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX)
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

struct type {
    int x;
    int y;
};

int main()
{
    {
        std::unique_ptr<type,
            mars_boost::alloc_deleter<type, creator<type> > > result =
            mars_boost::allocate_unique<type>(creator<type>(), { 1, 2 });
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result->x == 1);
        BOOST_TEST(result->y == 2);
    }
    {
        std::unique_ptr<const type,
            mars_boost::alloc_deleter<const type, creator<> > > result =
            mars_boost::allocate_unique<const type>(creator<>(), { 1, 2 });
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result->x == 1);
        BOOST_TEST(result->y == 2);
    }
    return mars_boost::report_errors();
}
#else
int main()
{
    return 0;
}
#endif
