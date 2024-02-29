/*
Copyright 2017 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && \
    !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && \
    !defined(BOOST_NO_CXX11_ALLOCATOR)
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

struct allow { };

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

    template<class U>
    void construct(U* ptr) {
        ::new(static_cast<void*>(ptr)) U(allow());
    }

    template<class U>
    void destroy(U* ptr) {
        ptr->~U();
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

    explicit type(allow) {
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
    {
        mars_boost::local_shared_ptr<type[]> result =
            mars_boost::allocate_local_shared<type[]>(creator<type>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.local_use_count() == 1);
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::local_shared_ptr<type[3]> result =
            mars_boost::allocate_local_shared<type[3]>(creator<type>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.local_use_count() == 1);
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::local_shared_ptr<type[][2]> result =
            mars_boost::allocate_local_shared<type[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.local_use_count() == 1);
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::local_shared_ptr<type[2][2]> result =
            mars_boost::allocate_local_shared<type[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.local_use_count() == 1);
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::local_shared_ptr<const type[]> result =
            mars_boost::allocate_local_shared<const type[]>(creator<>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.local_use_count() == 1);
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::local_shared_ptr<const type[3]> result =
            mars_boost::allocate_local_shared<const type[3]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.local_use_count() == 1);
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::local_shared_ptr<const type[][2]> result =
            mars_boost::allocate_local_shared<const type[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.local_use_count() == 1);
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::local_shared_ptr<const type[2][2]> result =
            mars_boost::allocate_local_shared<const type[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.local_use_count() == 1);
        BOOST_TEST(type::instances == 4);
        result.reset();
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
