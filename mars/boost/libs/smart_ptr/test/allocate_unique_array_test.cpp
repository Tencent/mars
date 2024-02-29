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

    type()
        : value_(0.0) {
        ++instances;
    }

    ~type() {
        --instances;
    }

    void set(long double value) {
        value_ = value;
    }

    long double get() const {
        return value_;
    }

private:
    type(const type&);
    type& operator=(const type&);

    long double value_;
};

unsigned type::instances = 0;

int main()
{
    {
        std::unique_ptr<int[],
            mars_boost::alloc_deleter<int[], creator<int> > > result =
            mars_boost::allocate_unique<int[]>(creator<int>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result[0] == 0);
        BOOST_TEST(result[1] == 0);
        BOOST_TEST(result[2] == 0);
    }
    {
        std::unique_ptr<int[],
            mars_boost::alloc_deleter<int[3], creator<int> > > result =
            mars_boost::allocate_unique<int[3]>(creator<int>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result[0] == 0);
        BOOST_TEST(result[1] == 0);
        BOOST_TEST(result[2] == 0);
    }
    {
        std::unique_ptr<int[][2],
            mars_boost::alloc_deleter<int[][2], creator<> > > result =
            mars_boost::allocate_unique<int[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 0);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 0);
    }
    {
        std::unique_ptr<int[][2],
            mars_boost::alloc_deleter<int[2][2], creator<> > > result =
            mars_boost::allocate_unique<int[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 0);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 0);
    }
    {
        std::unique_ptr<const int[],
            mars_boost::alloc_deleter<const int[], creator<> > > result =
            mars_boost::allocate_unique<const int[]>(creator<>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result[0] == 0);
        BOOST_TEST(result[1] == 0);
        BOOST_TEST(result[2] == 0);
    }
    {
        std::unique_ptr<const int[],
            mars_boost::alloc_deleter<const int[3], creator<> > > result =
            mars_boost::allocate_unique<const int[3]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result[0] == 0);
        BOOST_TEST(result[1] == 0);
        BOOST_TEST(result[2] == 0);
    }
    {
        std::unique_ptr<const int[][2],
            mars_boost::alloc_deleter<const int[][2], creator<> > > result =
            mars_boost::allocate_unique<const int[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 0);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 0);
    }
    {
        std::unique_ptr<const int[][2],
            mars_boost::alloc_deleter<const int[2][2], creator<> > > result =
            mars_boost::allocate_unique<const int[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result[0][0] == 0);
        BOOST_TEST(result[0][1] == 0);
        BOOST_TEST(result[1][0] == 0);
        BOOST_TEST(result[1][1] == 0);
    }
    {
        std::unique_ptr<type[],
            mars_boost::alloc_deleter<type[], creator<type> > > result =
            mars_boost::allocate_unique<type[]>(creator<type>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        std::unique_ptr<type[],
            mars_boost::alloc_deleter<type[3], creator<type> > > result =
            mars_boost::allocate_unique<type[3]>(creator<type>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        std::unique_ptr<type[][2],
            mars_boost::alloc_deleter<type[][2], creator<> > > result =
            mars_boost::allocate_unique<type[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        std::unique_ptr<type[][2],
            mars_boost::alloc_deleter<type[2][2], creator<> > > result =
            mars_boost::allocate_unique<type[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        std::unique_ptr<const type[],
            mars_boost::alloc_deleter<const type[], creator<> > > result =
            mars_boost::allocate_unique<const type[]>(creator<>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        std::unique_ptr<const type[],
            mars_boost::alloc_deleter<const type[3], creator<> > > result =
            mars_boost::allocate_unique<const type[3]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 3);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        std::unique_ptr<const type[][2],
            mars_boost::alloc_deleter<const type[][2], creator<> > > result =
            mars_boost::allocate_unique<const type[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 4);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        std::unique_ptr<const type[][2],
            mars_boost::alloc_deleter<const type[2][2], creator<> > > result =
            mars_boost::allocate_unique<const type[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
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
