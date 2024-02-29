/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if (!defined(BOOST_LIBSTDCXX_VERSION) || \
    BOOST_LIBSTDCXX_VERSION >= 48000) && \
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

int main()
{
    {
        std::unique_ptr<int[][2],
            mars_boost::alloc_deleter<int[][2], creator<int> > > result =
            mars_boost::allocate_unique<int[][2]>(creator<int>(), 2, {0, 1});
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 1);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 1);
    }
    {
        std::unique_ptr<int[][2],
            mars_boost::alloc_deleter<int[2][2], creator<int> > > result =
            mars_boost::allocate_unique<int[2][2]>(creator<int>(), {0, 1});
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 1);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 1);
    }
    {
        std::unique_ptr<const int[][2],
            mars_boost::alloc_deleter<const int[][2], creator<> > > result =
            mars_boost::allocate_unique<const int[][2]>(creator<>(), 2, {0, 1});
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 1);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 1);
    }
    {
        std::unique_ptr<const int[][2],
            mars_boost::alloc_deleter<const int[2][2], creator<> > > result =
            mars_boost::allocate_unique<const int[2][2]>(creator<>(), {0, 1});
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 1);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 1);
    }
    return mars_boost::report_errors();
}
#else
int main()
{
    return 0;
}
#endif
