//
// Created by Changpeng Pan on 2023/10/10.
//

#ifndef MMNET_ANY_H
#define MMNET_ANY_H

#include <iostream>
#include <memory>
#include <stdexcept>
#include <typeinfo>

namespace mars_overlay {

class any {
 public:
    any() : data(nullptr) {
    }

    template <typename T>
    any(const T& value) : data(new ValueHolder<T>(value)) {
    }

    ~any() {
        delete data;
    }

    template <typename T>
    T& cast() {
        ValueType* holder = dynamic_cast<ValueHolder<T>*>(data);
        if (holder == nullptr) {
            //throw std::bad_cast();
        }
        return static_cast<ValueHolder<T>*>(holder)->value;
    }

    bool empty() const {
        return data == nullptr;
    }

 private:
    class ValueType {
     public:
        virtual ~ValueType() {
        }
        virtual const std::type_info& type() const = 0;
    };

    template <typename T>
    class ValueHolder : public ValueType {
     public:
        ValueHolder(const T& value) : value(value) {
        }
        const std::type_info& type() const override {
            return typeid(T);
        }
        T value;
    };

    ValueType* data;
};

}  // namespace mars_overlay

#endif  // MMNET_ANY_H
