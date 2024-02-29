/*
Copyright 2012-2015 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/align/is_aligned.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/type_traits/alignment_of.hpp>

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
        mars_boost::shared_ptr<int[]> result =
            mars_boost::allocate_shared_noinit<int[]>(creator<int>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
    }
    {
        mars_boost::shared_ptr<int[3]> result =
            mars_boost::allocate_shared_noinit<int[3]>(creator<int>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
    }
    {
        mars_boost::shared_ptr<int[][2]> result =
            mars_boost::allocate_shared_noinit<int[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
    }
    {
        mars_boost::shared_ptr<int[2][2]> result =
            mars_boost::allocate_shared_noinit<int[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
    }
    {
        mars_boost::shared_ptr<const int[]> result =
            mars_boost::allocate_shared_noinit<const int[]>(creator<>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
    }
    {
        mars_boost::shared_ptr<const int[3]> result =
            mars_boost::allocate_shared_noinit<const int[3]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
    }
    {
        mars_boost::shared_ptr<const int[][2]> result =
            mars_boost::allocate_shared_noinit<const int[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
    }
    {
        mars_boost::shared_ptr<const int[2][2]> result =
            mars_boost::allocate_shared_noinit<const int[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<int>::value));
    }
    {
        mars_boost::shared_ptr<type[]> result =
            mars_boost::allocate_shared_noinit<type[]>(creator<type>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 3);
        mars_boost::weak_ptr<type[]> other = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<type[3]> result =
            mars_boost::allocate_shared_noinit<type[3]>(creator<type>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 3);
        mars_boost::weak_ptr<type[3]> other = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<type[][2]> result =
            mars_boost::allocate_shared_noinit<type[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 4);
        mars_boost::weak_ptr<type[][2]> other = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<type[2][2]> result =
            mars_boost::allocate_shared_noinit<type[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 4);
        mars_boost::weak_ptr<type[2][2]> other = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<const type[]> result =
            mars_boost::allocate_shared_noinit<const type[]>(creator<>(), 3);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 3);
        mars_boost::weak_ptr<const type[]> other = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<const type[3]> result =
            mars_boost::allocate_shared_noinit<const type[3]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 3);
        mars_boost::weak_ptr<const type[3]> other = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<const type[][2]> result =
            mars_boost::allocate_shared_noinit<const
                type[][2]>(creator<>(), 2);
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 4);
        mars_boost::weak_ptr<const type[][2]> other = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    {
        mars_boost::shared_ptr<const type[2][2]> result =
            mars_boost::allocate_shared_noinit<const type[2][2]>(creator<>());
        BOOST_TEST(result.get() != 0);
        BOOST_TEST(result.use_count() == 1);
        BOOST_TEST(mars_boost::alignment::is_aligned(result.get(),
            mars_boost::alignment_of<type>::value));
        BOOST_TEST(type::instances == 4);
        mars_boost::weak_ptr<const type[2][2]> other = result;
        result.reset();
        BOOST_TEST(type::instances == 0);
    }
    return mars_boost::report_errors();
}
