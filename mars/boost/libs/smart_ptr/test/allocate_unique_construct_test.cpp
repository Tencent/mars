/*
Copyright 2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/config.hpp>
#if !defined(BOOST_NO_CXX11_SMART_PTR) && !defined(BOOST_NO_CXX11_ALLOCATOR)
#include <boost/smart_ptr/allocate_unique.hpp>
#include <boost/core/lightweight_test.hpp>

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
        std::unique_ptr<type,
            mars_boost::alloc_deleter<type, creator<type> > > result =
            mars_boost::allocate_unique<type>(creator<type>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 1);
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        std::unique_ptr<const type,
            mars_boost::alloc_deleter<const type, creator<> > > result =
            mars_boost::allocate_unique<const type>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(type::instances == 1);
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
