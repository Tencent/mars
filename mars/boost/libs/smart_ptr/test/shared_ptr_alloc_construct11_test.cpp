/*
Copyright 2017 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/core/lightweight_test.hpp>
#include <boost/make_shared.hpp>

#if !defined(BOOST_NO_CXX11_ALLOCATOR)
struct counters {
    unsigned allocate;
    unsigned construct;
};

template<class T = void>
class creator {
public:
    typedef T value_type;

    creator(counters* state)
        : state_(state) { }

    template<class U>
    creator(const creator<U>& other)
        : state_(other.state()) { }

    T* allocate(std::size_t size) {
        void* ptr = ::operator new(sizeof(T) * size);
        ++state_->allocate;
        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, std::size_t) {
        ::operator delete(ptr);
        --state_->allocate;
    }

    template<class... Args>
    void construct(T* ptr, Args&&... args) {
        ::new(static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
        ++state_->construct;
    }

    void destroy(T* ptr) {
        ptr->~T();
        --state_->construct;
    }

    counters* state() const {
        return state_;
    }

private:
    counters* state_;
};

template<class T, class U>
inline bool
operator==(const creator<T>& lhs, const creator<U>& rhs)
{
    return lhs.state() == rhs.state();
}

template<class T, class U>
inline bool
operator!=(const creator<T>& lhs, const creator<U>& rhs)
{
    return !(lhs == rhs);
}

struct deleter {
    template<class U>
    void operator()(U ptr) const {
        delete ptr;
    }
};

int main()
{
    {
        counters state = { };
        mars_boost::shared_ptr<int> pointer(new int(), deleter(),
            creator<int>(&state));
        BOOST_TEST(state.allocate == 1);
        BOOST_TEST(state.construct == 0);
        pointer.reset();
        BOOST_TEST(state.allocate == 0);
    }
    {
        counters state = { };
        mars_boost::shared_ptr<int> pointer =
            mars_boost::allocate_shared<int>(creator<int>(&state));
        BOOST_TEST(state.allocate == 1);
        BOOST_TEST(state.construct == 1);
        pointer.reset();
        BOOST_TEST(state.allocate == 0);
        BOOST_TEST(state.construct == 0);
    }
    {
        counters state = { };
        mars_boost::shared_ptr<int[]> pointer =
            mars_boost::allocate_shared<int[]>(creator<>(&state), 5);
        BOOST_TEST(state.allocate == 1);
        BOOST_TEST(state.construct == 5);
        pointer.reset();
        BOOST_TEST(state.allocate == 0);
        BOOST_TEST(state.construct == 0);
    }
    {
        counters state = { };
        mars_boost::shared_ptr<int[5]> pointer =
            mars_boost::allocate_shared<int[5]>(creator<>(&state));
        BOOST_TEST(state.allocate == 1);
        BOOST_TEST(state.construct == 5);
        pointer.reset();
        BOOST_TEST(state.allocate == 0);
        BOOST_TEST(state.construct == 0);
    }
    return mars_boost::report_errors();
}
#else
int main()
{
    return 0;
}
#endif
