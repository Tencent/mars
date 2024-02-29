/*
Copyright 2017 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && \
    !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && \
    !defined(BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX)
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

template<class T = void>
struct creator {
    typedef T value_type;

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
        mars_boost::local_shared_ptr<int[][2]> result =
            mars_boost::allocate_local_shared<int[][2]>(creator<int>(), 2, {0, 1});
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 1);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 1);
    }
    {
        mars_boost::local_shared_ptr<int[2][2]> result =
            mars_boost::allocate_local_shared<int[2][2]>(creator<int>(), {0, 1});
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 1);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 1);
    }
    {
        mars_boost::local_shared_ptr<const int[][2]> result =
            mars_boost::allocate_local_shared<const int[][2]>(creator<>(), 2, {0, 1});
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 1);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 1);
    }
    {
        mars_boost::local_shared_ptr<const int[2][2]> result =
            mars_boost::allocate_local_shared<const int[2][2]>(creator<>(), {0, 1});
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
