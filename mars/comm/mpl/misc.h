//
// Created by  kylinnhuang on 2020/8/19.
// Copyright (c) 2020 peterfan. All rights reserved.
//

#ifndef NGOWL_MISC_H
#define NGOWL_MISC_H

#include <type_traits>
#include <cstdint>
#include <limits>
#include <tuple>
#include <array>

#include "preprocessor.h"
#include "typelist.h"

namespace owl {
    template <typename T>
    struct TypeIdentity {
        using type = T;
    };

    template <typename T>
    struct ArgumentSize;

    template <template <typename...> class Template, typename... T>
    struct ArgumentSize<Template<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {
    };

    template <typename From, template <typename...> class ToTemplate>
    struct TransferArgumentTo;

    template <template <typename...> class FromTemplate, typename... Args, template <typename...> class ToTemplate>
    struct TransferArgumentTo<FromTemplate<Args...>, ToTemplate> {
        using type = ToTemplate<Args...>;
    };

    template <typename From, template <typename...> class ToTemplate>
    using TransferArgumentToType = typename TransferArgumentTo<From, ToTemplate>::type;

    template <typename Tuple>
    using ConvertTupleToTypeList = TransferArgumentToType<Tuple, make_typelist>;

    template <typename Tuple>
    struct IsEmptyTuple : std::false_type {
    };

    template <template <typename...> class TupleTemplate>
    struct IsEmptyTuple<TupleTemplate<>> : std::true_type {
    };

    namespace detail {
        template <typename TypeList, typename Tuple>
        struct ConvertTypeListToTupleImpl;

        template <typename Tuple>
        struct ConvertTypeListToTupleImpl<null_t, Tuple> {
            using type = Tuple;
        };

        template <typename TypeListHead, typename TypeListTail, template <typename...> class TupleTemplate, typename... TupleTypes>
        struct ConvertTypeListToTupleImpl<typelist<TypeListHead, TypeListTail>, TupleTemplate<TupleTypes...>> {
            using type = typename ConvertTypeListToTupleImpl<TypeListTail, TupleTemplate<TupleTypes..., TypeListHead>>::type;
        };
    }

    template <typename TypeList, template <typename...> class TupleTemplate = std::tuple>
    using ConvertTypeListToTuple = detail::ConvertTypeListToTupleImpl<TypeList, TupleTemplate<>>;

    template <template <typename...> class Template, typename... T>
    struct BindFront {
        template <typename... TRest>
        using Result = Template<T..., TRest...>;
    };

    template <template <typename...> class Template, typename... T>
    struct BindBack {
        template <typename... TRest>
        using Result = Template<TRest..., T...>;
    };

    template <template <typename...> class Template>
    struct MakeSingleParamTemplate {
        template <typename T>
        using Result = Template<T>;
    };

    namespace detail {
        template <template <typename> class Predicate, typename Tuple, std::size_t Current, std::size_t Fallback>
        struct FindFirstOfImpl : std::conditional_t<Predicate<std::tuple_element_t<Current, Tuple>>::value, std::integral_constant<std::size_t, Current>,
                std::conditional_t<Current == std::tuple_size<Tuple>::value - 1, std::integral_constant<std::size_t, Fallback>, FindFirstOfImpl<Predicate, Tuple, Current + 1, Fallback>>> {
        };
    }

    template <template <typename> class Predicate, typename Tuple, std::size_t Fallback = std::size_t(-1)>
    struct FindFirstOf : detail::FindFirstOfImpl<Predicate, Tuple, 0, Fallback> {
    };

    template <template <typename> class Predicate, typename... T>
    struct FindFirstOfSequence : FindFirstOf<Predicate, std::tuple<T...>> {
        using type = std::tuple_element_t<FindFirstOf<Predicate, std::tuple<T...>>::value, std::tuple<T...>>;
    };

    template <typename T, typename Tuple, std::size_t Fallback = std::size_t(-1)>
    struct FindTypeFrom : FindFirstOf<
// 根据 https://clang.llvm.org/cxx_status.html#p0522 在任何语言标准下 clang 在不使用特殊标志开启此功能时都无法使用
#if __cplusplus >= 201703L && !defined(__clang__)
        BindFront<std::is_same, T>::template Result,
#else
        MakeSingleParamTemplate<BindFront<std::is_same, T>::template Result>::template Result,
#endif
        Tuple, Fallback> {
    };

    namespace detail {
        template <std::uintmax_t Number>
        struct FindSmallestUnsignedTypeToHoldPredicateBuilder {
            template <typename T>
            struct Result : std::integral_constant<bool, Number <= std::numeric_limits<T>::max()> {
            };
        };
    }

    template <std::uintmax_t Number>
    struct FindSmallestUnsignedTypeToHold
        : FindFirstOfSequence<detail::FindSmallestUnsignedTypeToHoldPredicateBuilder<Number>::template Result, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t, std::uintmax_t> {
    };

    template <std::uintmax_t Number>
    using FindSmallestUnsignedTypeToHoldType = typename FindSmallestUnsignedTypeToHold<Number>::type;

    template <typename T>
    using RemoveCVRef = std::remove_cv_t<std::remove_reference_t<T>>;

    namespace detail {
        template <typename T, std::size_t>
        struct TaggedHolder {
            T Value;
        };
    }

    template <typename T1, typename T2>
    class CompressedPair : detail::TaggedHolder<T1, 0>, detail::TaggedHolder<T2, 1> {
    public:
        template <typename DummyT1 = T1, typename DummyT2 = T2, std::enable_if_t<std::is_default_constructible<DummyT1>::value && std::is_default_constructible<DummyT2>::value, int> = 0>
        constexpr CompressedPair()
            : detail::TaggedHolder<T1, 0>{}, detail::TaggedHolder<T2, 1>{} {
        }

        template <typename T1Value, typename T2Value>
        constexpr CompressedPair(T1Value&& t1Value, T2Value&& t2Value)
            : detail::TaggedHolder<T1, 0>{ std::forward<T1Value>(t1Value) }, detail::TaggedHolder<T2, 1>{ std::forward<T2Value>(t2Value) } {
        }

    private:
        template <typename... T1Args, std::size_t... T1Indexes, typename... T2Args, std::size_t... T2Indexes>
        constexpr CompressedPair(std::piecewise_construct_t, std::tuple<T1Args...> t1Args, std::index_sequence<T1Indexes...>, std::tuple<T2Args...> t2Args, std::index_sequence<T2Indexes...>)
            : detail::TaggedHolder<T1, 0>{ static_cast<T1Args&&>(std::get<T1Indexes>(t1Args))... }, detail::TaggedHolder<T2, 1>{ static_cast<T2Args&&>(std::get<T2Indexes>(t2Args))...  } {
        }

    public:
        template <typename... T1Args, typename... T2Args>
        constexpr CompressedPair(std::piecewise_construct_t, std::tuple<T1Args...> t1Args, std::tuple<T2Args...> t2Args)
            : CompressedPair(std::piecewise_construct, std::move(t1Args), std::make_index_sequence<sizeof...(T1Args)>{}, std::move(t2Args), std::make_index_sequence<sizeof...(T2Args)>{}) {
        }

        constexpr T1& First() & noexcept {
            return detail::TaggedHolder<T1, 0>::Value;
        }

        constexpr const T1& First() const & noexcept {
            return detail::TaggedHolder<T1, 0>::Value;
        }

        constexpr T1&& First() && noexcept {
            return std::move(First());
        }

        constexpr const T1&& First() const && noexcept {
            return std::move(First());
        }

        constexpr T2& Second() & noexcept {
            return detail::TaggedHolder<T2, 1>::Value;
        }

        constexpr const T2& Second() const & noexcept {
            return detail::TaggedHolder<T2, 1>::Value;
        }

        constexpr T2&& Second() && noexcept {
            return std::move(Second());
        }

        constexpr const T2&& Second() const && noexcept {
            return std::move(Second());
        }
    };

    template <typename... T>
    struct Conjunction : std::true_type {
    };

    template <typename T>
    struct Conjunction<T> : T {
    };

    template <typename T, typename... TRest>
    struct Conjunction<T, TRest...> : std::conditional_t<bool(T::value), Conjunction<TRest...>, T> {
    };

    template <typename... T>
    struct Disjunction : std::false_type {
    };

    template <typename T>
    struct Disjunction<T> : T {
    };

    template <typename T, typename... TRest>
    struct Disjunction<T, TRest...> : std::conditional_t<bool(T::value), T, Disjunction<TRest...>> {
    };

    template <typename T>
    struct Negation : std::integral_constant<bool, !bool(T::value)> {
    };

    template <template <typename> class Predicate, typename... T>
    struct IsAll : Conjunction<Predicate<T>...> {
    };

    template <template <typename> class Predicate, typename... T>
    struct IsAny : Disjunction<Predicate<T>...> {
    };

    template <typename Callable, typename T>
    constexpr decltype(auto) Aggregate(Callable&&, T&& head) {
        return head;
    }

    template <typename Callable, typename T, typename TCur, typename... TRest>
    constexpr decltype(auto) Aggregate(Callable&& callable, T&& head, TCur&& cur, TRest&&... rest) {
        return Aggregate(std::forward<Callable>(callable), std::forward<Callable>(std::forward<T>(head), std::forward<TCur>(cur)), std::forward<TRest>(rest)...);
    }

    template <typename T, std::size_t Index>
    struct Split;

    namespace detail {
        template <typename THead, typename TTail, std::size_t Index>
        struct SplitImpl;

        template <typename THead, typename TTail>
        struct SplitImpl<THead, TTail, 0> {
            using Head = THead;
            using Tail = TTail;
        };

        template <template <typename...> class Template, typename... THead, typename TCur, typename... TTail, std::size_t Index>
        struct SplitImpl<Template<THead...>, Template<TCur, TTail...>, Index>
            : SplitImpl<Template<THead..., TCur>, Template<TTail...>, Index - 1> {
        };
    }

    template <template <typename...> class Template, typename... T, std::size_t Index>
    struct Split<Template<T...>, Index> : detail::SplitImpl<Template<>, Template<T...>, Index> {
    };

    template <typename T, std::size_t Size>
    struct Head {
        using type = typename Split<T, Size>::Head;
    };

    template <typename T, std::size_t Size>
    struct Tail {
        using type = typename Split<T, ArgumentSize<T>::value - Size>::Tail;
    };
}

#define OWL_MAGIC_ID_HEAD OWL_MAGIC_DO_NOT_USE_

#ifdef __COUNTER__
#define OWL_MAGIC_ID_TAIL __COUNTER__
#else
#define OWL_MAGIC_ID_TAIL __LINE__
#endif

#define OWL_MAGIC_ID _MPL_PP_CAT(OWL_MAGIC_ID_HEAD, OWL_MAGIC_ID_TAIL)

#endif //NGOWL_MISC_H
