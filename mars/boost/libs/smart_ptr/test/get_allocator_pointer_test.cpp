/*
Copyright 2021 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if (!defined(BOOST_LIBSTDCXX_VERSION) || \
    BOOST_LIBSTDCXX_VERSION >= 46000) && \
    !defined(BOOST_NO_CXX11_SMART_PTR)
#include <boost/smart_ptr/allocate_unique.hpp>
#include <boost/core/lightweight_test.hpp>

template<class T>
class point {
public:
    point()
        : state_()
        , ptr_() { }

    point(int count, T* value)
        : state_(count)
        , ptr_(value) { }

    operator bool() const {
        return static_cast<bool>(ptr_);
    }

    T* operator->() const {
        return ptr_;
    }

    int state() const {
        return state_;
    }

private:
    int state_;
    T* ptr_;
};

template<class T = void>
class creator {
public:
    typedef T value_type;
    typedef point<T> pointer;
    typedef std::ptrdiff_t difference_type;

    creator()
        : state_() { }

    explicit creator(int value)
        : state_(value) { }

    template<class U>
    creator(const creator<U>& other)
        : state_(other.state()) { }

    pointer allocate(std::size_t size) {
        return pointer(state_,
            static_cast<T*>(::operator new(sizeof(T) * size)));
    }

    void deallocate(pointer ptr, std::size_t) {
        ::operator delete(ptr.operator->());
    }

    int state() const {
        return state_;
    }

private:
    int state_;
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
        std::unique_ptr<int,
            mars_boost::alloc_deleter<int, creator<> > > result =
            mars_boost::allocate_unique<int>(creator<>(1));
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 1);
    }
    {
        std::unique_ptr<const int,
            mars_boost::alloc_deleter<const int, creator<> > > result =
            mars_boost::allocate_unique<const int>(creator<>(2));
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 2);
    }
    {
        std::unique_ptr<int[],
            mars_boost::alloc_deleter<int[], creator<> > > result =
            mars_boost::allocate_unique<int[]>(creator<>(3), 3);
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 3);
    }
    {
        std::unique_ptr<int[],
            mars_boost::alloc_deleter<int[3], creator<> > > result =
            mars_boost::allocate_unique<int[3]>(creator<>(4));
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 4);
    }
    {
        std::unique_ptr<int[][2],
            mars_boost::alloc_deleter<int[][2], creator<> > > result =
            mars_boost::allocate_unique<int[][2]>(creator<>(5), 2);
        point<int[2]> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 5);
    }
    {
        std::unique_ptr<int[][2],
            mars_boost::alloc_deleter<int[2][2], creator<> > > result =
            mars_boost::allocate_unique<int[2][2]>(creator<>(6));
        point<int[2]> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 6);
    }
    {
        std::unique_ptr<const int[],
            mars_boost::alloc_deleter<const int[], creator<> > > result =
            mars_boost::allocate_unique<const int[]>(creator<>(7), 3);
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 7);
    }
    {
        std::unique_ptr<const int[],
            mars_boost::alloc_deleter<const int[3], creator<> > > result =
            mars_boost::allocate_unique<const int[3]>(creator<>(8));
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 8);
    }
    {
        std::unique_ptr<const int[][2],
            mars_boost::alloc_deleter<const int[][2], creator<> > > result =
            mars_boost::allocate_unique<const int[][2]>(creator<>(9), 2);
        point<int[2]> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 9);
    }
    {
        std::unique_ptr<const int[][2],
            mars_boost::alloc_deleter<const int[2][2], creator<> > > result =
            mars_boost::allocate_unique<const int[2][2]>(creator<>(10));
        point<int[2]> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 10);
    }
    {
        std::unique_ptr<int,
            mars_boost::alloc_deleter<int,
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<int>(creator<>(11));
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 11);
    }
    {
        std::unique_ptr<const int,
            mars_boost::alloc_deleter<const int,
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<const int>(creator<>(12));
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 12);
    }
    {
        std::unique_ptr<int[],
            mars_boost::alloc_deleter<int[],
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<int[]>(creator<>(13), 3);
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 13);
    }
    {
        std::unique_ptr<int[],
            mars_boost::alloc_deleter<int[3],
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<int[3]>(creator<>(14));
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 14);
    }
    {
        std::unique_ptr<int[][2],
            mars_boost::alloc_deleter<int[][2],
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<int[][2]>(creator<>(15), 2);
        point<int[2]> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 15);
    }
    {
        std::unique_ptr<int[][2],
            mars_boost::alloc_deleter<int[2][2],
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<int[2][2]>(creator<>(16));
        point<int[2]> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 16);
    }
    {
        std::unique_ptr<const int[],
            mars_boost::alloc_deleter<const int[],
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<const int[]>(creator<>(17), 3);
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 17);
    }
    {
        std::unique_ptr<const int[],
            mars_boost::alloc_deleter<const int[3],
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<const int[3]>(creator<>(18));
        point<int> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 18);
    }
    {
        std::unique_ptr<const int[][2],
            mars_boost::alloc_deleter<const int[][2],
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<const int[][2]>(creator<>(19), 2);
        point<int[2]> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 19);
    }
    {
        std::unique_ptr<const int[][2],
            mars_boost::alloc_deleter<const int[2][2],
            mars_boost::noinit_adaptor<creator<> > > > result =
            mars_boost::allocate_unique_noinit<const int[2][2]>(creator<>(20));
        point<int[2]> ptr = mars_boost::get_allocator_pointer(result);
        BOOST_TEST_EQ(ptr.state(), 20);
    }
    return mars_boost::report_errors();
}
#else
int main()
{
    return 0;
}
#endif
