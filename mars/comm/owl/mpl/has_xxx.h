/*
 * has_xxx.h
 *
 *  Created on: 2008-8-29
 *      Author: peterfan
 */

#ifndef _MPL_HAS_XXX_H_
#define _MPL_HAS_XXX_H_

#include <type_traits>

#include "meta_function.h"

namespace detail {

struct yes_type { char x; };
struct no_type { short x; };

} // namespace detail


#define HAS_MEMBER_VARIABLE_DEFINE(trait_name, member_name, member_type) \
    __HAS_XXX_DEFINE(trait_name, member_name, member_type, )

#define HAS_MEMBER_FUNCTION_DEFINE(trait_name, member_name, return_type, param_list) \
    __HAS_XXX_DEFINE(trait_name, member_name, return_type, param_list)

#define __HAS_XXX_DEFINE(trait_name, name, type, params) \
    __HAS_XXX_DEFINE_I(trait_name, name, type, params)

#define __HAS_XXX_DEFINE_I(__trait_name, __name, __type, __params) \
	template <typename T, typename Enable = void> \
	class __trait_name { \
	private: \
		template <__type (T::*) __params> \
		struct tester; \
		template <typename U> \
		static detail::yes_type test(tester<&U::__name>*); \
		template <typename U> \
		static detail::no_type test(...); \
	public: \
		static const bool value = (sizeof(test<T>(0)) == sizeof(detail::yes_type)); \
		typedef std::integral_constant<bool, value> type; \
	}; \
    template <typename T> \
    class __trait_name<T, typename std::enable_if<!std::is_class<T>::value>::type> { \
    public: \
        static const bool value = false; \
        typedef std::integral_constant<bool, value> type; \
    }; \


#endif /* _MPL_HAS_XXX_H_ */
