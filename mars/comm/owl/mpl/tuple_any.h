//
//  tuple_any.h
//  ngowl
//
//  Created by peterfan on 2019/3/27.
//  Copyright © 2019年 peterfan. All rights reserved.
//

#ifndef tuple_any_h
#define tuple_any_h

#include <cassert>
#include <cstdio>
#include <cstring>

#include <tuple>
#include <typeindex>
#include <typeinfo>

#include "owl/mpl/meta_function.h"

namespace owl {

namespace detail {
    
    class tuple_placeholder {
    public:
        virtual ~tuple_placeholder() {}
        virtual tuple_placeholder* clone() const = 0;
        virtual const std::type_info& type() const = 0;
        
        virtual size_t tuple_size() const = 0;
        virtual std::type_index tuple_type(size_t i) const = 0;
        virtual void* tuple_element(size_t i) const = 0;
    };
    

    template <typename T>
    struct ensure_tuple {
        using type = std::tuple<T>;
    };
    
    template <typename... Args>
    struct ensure_tuple<std::tuple<Args...>> {
        using type = std::tuple<Args...>;
    };
    
    template <typename T>
    using ensure_tuple_t = typename ensure_tuple<typename std::decay<T>::type>::type;
    
    
    template <typename T>
    struct tuple_types_t;
    
    template <>
    struct tuple_types_t<std::tuple<>> {
        std::type_index type_at(size_t i) const {
            return typeid(void);
        }
    };
    
    template <typename Tuple, size_t N>
    struct type_array {
        std::type_index types[N];
        
        template <size_t... I>
        type_array(index_seq<I...>)
        :types { typeid(typename std::tuple_element<I, Tuple>::type)... } {}
    };
    
    template <typename... Args>
    struct tuple_types_t<std::tuple<Args...>> {
    private:
        using Tuple = std::tuple<Args...>;
        enum { kTupleSize = std::tuple_size<Tuple>::value };
        
        type_array<Tuple, kTupleSize> array_;
    public:
        tuple_types_t():array_(make_index_seq<kTupleSize>()) {}
        
        std::type_index type_at(size_t i) const {
            return array_.types[i];
        }
    };
    
    
    template <typename T>
    struct tuple_elements_t;
    
    template <>
    struct tuple_elements_t<std::tuple<>> {
        using Tuple = std::tuple<>;

        explicit tuple_elements_t(Tuple& t) {}
        
        void* element_at(size_t i) const {
            return nullptr;
        }
    };
    
    template <typename Tuple, size_t N>
    struct element_array {
        void* elements[N];
        
        template <size_t... I>
        element_array(Tuple& t, index_seq<I...>)
        :elements { reinterpret_cast<void*>(&std::get<I>(t))... } {}
    };
    
    template <typename... Args>
    struct tuple_elements_t<std::tuple<Args...>> {
    private:
        using Tuple = std::tuple<Args...>;
        enum { kTupleSize = std::tuple_size<Tuple>::value };

        element_array<Tuple, kTupleSize> array_;
        
    public:
        explicit tuple_elements_t(Tuple& t):array_(t, make_index_seq<kTupleSize>()) {}
        
        void* element_at(size_t i) const {
            return array_.elements[i];
        }
    };
    
    template <typename T>
    class tuple_holder;
    
    template <typename T>
    class tuple_holder: public tuple_placeholder {
        using Tuple = ensure_tuple_t<T>;
    public:
        tuple_holder(const Tuple& value)
        :held(value), types_(), elements_(held) {}
        
        tuple_holder(Tuple&& value)
        :held(std::move(value)), types_(), elements_(held) {}
        
        virtual tuple_placeholder* clone() const {
            return new tuple_holder(held);
        }
        
        virtual const std::type_info& type() const {
            return typeid(held);
        }
        
        virtual size_t tuple_size() const {
            return std::tuple_size<Tuple>::value;
        }
        
        virtual std::type_index tuple_type(size_t i) const {
            return types_.type_at(i);
        }
        
        virtual void* tuple_element(size_t i) const {
            return elements_.element_at(i);
        }
        
    public:
        Tuple held;
        tuple_types_t<Tuple> types_;
        tuple_elements_t<Tuple> elements_;
    };

} // namespace detail


class tuple_any {
public:
    tuple_any()
    :content(nullptr) {}
    
    template <typename ValueType, typename std::enable_if<!std::is_same<tuple_any, typename std::decay<ValueType>::type>::value>::type* = nullptr>
    tuple_any(ValueType&& value)
    :content(new detail::tuple_holder<ValueType>(std::forward<ValueType>(value))) {}
    
    tuple_any(const tuple_any& other)
    :content(other.content ? other.content->clone() : nullptr) {}
    
    tuple_any(tuple_any&& other)
    :content(other.content) {
        other.content = nullptr;
    }
    
    ~tuple_any() {
        reset();
    }
    
    tuple_any& swap(tuple_any& rhs) {
        detail::tuple_placeholder* temp = content;
        content = rhs.content;
        rhs.content = temp;
        return *this;
    }
    
    template <typename ValueType, typename std::enable_if<!std::is_same<tuple_any, typename std::decay<ValueType>::type>::value>::type* = nullptr>
    tuple_any& operator=(ValueType&& rhs) {
        tuple_any(std::forward<ValueType>(rhs)).swap(*this);
        return *this;
    }
    
    tuple_any& operator=(const tuple_any& rhs) {
        tuple_any(rhs).swap(*this);
        return *this;
    }
    
    tuple_any& operator=(tuple_any&& rhs) {
        tuple_any(std::move(rhs)).swap(*this);
        return *this;
    }
    
    void reset() {
        if (content) {
            delete content;
            content = nullptr;
        }
    }
    
    bool has_value() const {
        return content != nullptr;
    }
    
    const std::type_info& type() const {
        return content ? content->type() : typeid(void);
    }
    
    size_t tuple_size() const {
        return content ? content->tuple_size() : 0;
    }
    std::type_index tuple_type(size_t i) const {
        return content ? content->tuple_type(i) : typeid(void);
    }
    void* tuple_element(size_t i) const {
        return content ? content->tuple_element(i) : nullptr;
    }
    
private:
    template <typename ValueType>
    friend ValueType* tuple_any_cast(tuple_any*);
    
private:
    detail::tuple_placeholder* content;
};


template <typename ValueType>
inline ValueType* tuple_any_cast(tuple_any* operand) {
    if (operand) {
        // compare two type_info by name
        // @see https://github.com/android/ndk/issues/926
        if (strcmp(operand->type().name(), typeid(ValueType).name()) != 0) {
            printf("tuple_any_cast from %s to %s\n", operand->type().name(), typeid(ValueType).name());
            return nullptr;
        }
    }
    return operand ? &static_cast<detail::tuple_holder<ValueType>*>(operand->content)->held : nullptr;
}

template <typename ValueType>
inline const ValueType* tuple_any_cast(const tuple_any* operand) {
    return tuple_any_cast<ValueType>(const_cast<tuple_any*>(operand));
}

template <typename ValueType>
inline ValueType& tuple_any_cast(tuple_any& operand) {
    ValueType* result = tuple_any_cast<ValueType>(&operand);
    if (!result) {
        //TODO
        throw std::bad_cast();
    }
    return *result;
}

template <typename ValueType>
inline const ValueType& tuple_any_cast(const tuple_any& operand) {
    return tuple_any_cast<ValueType>(const_cast<tuple_any&>(operand));
}

} // namespace owl

#endif /* tuple_any_h */
