/*
Copyright 2012-2015 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_shared.hpp>

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
        mars_boost::allocate_shared<type[]>(creator<type>(), 6);
        BOOST_ERROR("allocate_shared did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_shared<type[][2]>(creator<type>(), 3);
        BOOST_ERROR("allocate_shared did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_shared<type[6]>(creator<>());
        BOOST_ERROR("allocate_shared did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_shared<type[3][2]>(creator<>());
        BOOST_ERROR("allocate_shared did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_shared_noinit<type[]>(creator<>(), 6);
        BOOST_ERROR("allocate_shared_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_shared_noinit<type[][2]>(creator<>(), 3);
        BOOST_ERROR("allocate_shared_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_shared_noinit<type[6]>(creator<>());
        BOOST_ERROR("allocate_shared_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    try {
        mars_boost::allocate_shared_noinit<type[3][2]>(creator<>());
        BOOST_ERROR("allocate_shared_noinit did not throw");
    } catch (...) {
        BOOST_TEST(type::instances == 0);
    }
    return mars_boost::report_errors();
}
