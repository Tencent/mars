/*
 * any.h
 *
 *  Created on: 2008-10-9
 *      Author: peterfan
 */

#ifndef _MPL_ANY_H_
#define _MPL_ANY_H_

#include <cassert>
#include <cstdio>
#include <cstring>

#include <typeinfo>

#include <string>
#include <unordered_map>

namespace owl {

namespace detail {

	class placeholder {
	public:
		virtual ~placeholder() {}
		virtual placeholder* clone() const = 0;
        virtual const std::type_info& type() const = 0;
	};

	template <typename ValueType>
	class holder: public placeholder {
	public:
		holder(const ValueType& value)
        :held(value) {}
        
        holder(ValueType&& value)
        :held(std::move(value)) {}

		virtual placeholder* clone() const {
			return new holder(held);
		}
        
        virtual const std::type_info& type() const {
            return typeid(held);
        }

	public:
		ValueType held;
	};

} // namespace detail


class any {
public:
	any()
	:content(nullptr) {}

    template <typename ValueType, typename std::enable_if<!std::is_same<any, typename std::decay<ValueType>::type>::value>::type* = nullptr>
	any(ValueType&& value)
    :content(new detail::holder<typename std::decay<ValueType>::type>(std::forward<ValueType>(value))) {}

	any(const any& other)
	:content(other.content ? other.content->clone() : nullptr) {}
    
    any(any&& other)
    :content(other.content) {
        other.content = nullptr;
    }

	~any() {
        reset();
	}

	any& swap(any& rhs) {
		detail::placeholder* temp = content;
		content = rhs.content;
		rhs.content = temp;
		return *this;
	}

    template <typename ValueType, typename std::enable_if<!std::is_same<any, typename std::decay<ValueType>::type>::value>::type* = nullptr>
	any& operator=(ValueType&& rhs) {
        any(std::forward<ValueType>(rhs)).swap(*this);
		return *this;
	}

	any& operator=(const any& rhs) {
		any(rhs).swap(*this);
		return *this;
	}
    
    any& operator=(any&& rhs) {
        any(std::move(rhs)).swap(*this);
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

private:
	template <typename ValueType>
	friend ValueType* any_cast(any*);

private:
	detail::placeholder* content;
};


template <typename ValueType>
inline ValueType* any_cast(any* operand) {
    if (operand) {
        // compare two type_info by name
        // @see https://github.com/android/ndk/issues/926
        if (strcmp(operand->type().name(), typeid(ValueType).name()) != 0) {
            printf("any_cast from %s to %s\n", operand->type().name(), typeid(ValueType).name());
            return nullptr;
        }
    }
	return operand ? &static_cast<detail::holder<ValueType>*>(operand->content)->held : nullptr;
}

template <typename ValueType>
inline const ValueType* any_cast(const any* operand) {
	return any_cast<ValueType>(const_cast<any*>(operand));
}

template <typename ValueType>
inline ValueType& any_cast(any& operand) {
	ValueType* result = any_cast<ValueType>(&operand);
	if (!result) {
		//TODO
	}
	return *result;
}
inline std::unordered_map<unsigned, std::string>& any_cast(any& operand) {
	std::unordered_map<unsigned, std::string>* result = any_cast<std::unordered_map<unsigned, std::string> >(&operand);
	if (!result) {
		//TODO
	}
	return *result;
}

template <typename ValueType>
inline const ValueType& any_cast(const any& operand) {
	return any_cast<ValueType>(const_cast<any&>(operand));
}

} // namespace owl

#endif /* _MPL_ANY_H_ */
