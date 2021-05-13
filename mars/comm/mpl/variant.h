//
// Created by  kylinnhuang on 2020/8/19.
// Copyright (c) 2020 peterfan. All rights reserved.
//

#ifndef NGOWL_VARIANT_H
#define NGOWL_VARIANT_H

#include <cstdint>
#include <type_traits>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <cassert>

#include "misc.h"
#include "scope.h"

#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && __GNUC__ <= 7 || __cplusplus <= 201402L
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47226
#define BUGFIX_VARIANT_DISPATCH_TABLE 1
#endif

namespace owl {
    template <std::size_t I>
    struct in_place_index_t : std::integral_constant<std::size_t, I> {
    };

    namespace detail {
        template <bool IsTriviallyDestructible, std::size_t Index, typename... T>
        union variant_union_storage {
        };

        template <std::size_t IndexValue, typename T, typename... TRest>
        union variant_union_storage<true, IndexValue, T, TRest...> {
            static constexpr std::size_t Index = IndexValue;

            constexpr variant_union_storage() : Dummy{} {
            }

            template <typename... Args>
            explicit constexpr variant_union_storage(in_place_index_t<IndexValue>, Args&&... args) : CurrentValue(std::forward<Args>(args)...) {
            }

            template <std::size_t I, typename... Args>
            explicit constexpr variant_union_storage(in_place_index_t<I>, Args&&... args) : RestValues(in_place_index_t<I>{}, std::forward<Args>(args)...) {
            }

            ~variant_union_storage() = default;

            char Dummy;
            T CurrentValue;
            variant_union_storage<true, IndexValue + 1, TRest...> RestValues;

            constexpr T& GetCurrent() & noexcept {
                return CurrentValue;
            }

            constexpr T&& GetCurrent() && noexcept {
                return std::move(CurrentValue);
            }

            constexpr const T& GetCurrent() const & noexcept {
                return CurrentValue;
            }

            constexpr const T&& GetCurrent() const && noexcept {
                return std::move(CurrentValue);
            }
        };

        template <std::size_t IndexValue, typename T, typename... TRest>
        union variant_union_storage<false, IndexValue, T, TRest...> {
            static constexpr std::size_t Index = IndexValue;

            constexpr variant_union_storage() : Dummy{} {
            }

            template <typename... Args>
            explicit constexpr variant_union_storage(in_place_index_t<IndexValue>, Args&&... args) : CurrentValue(std::forward<Args>(args)...) {
            }

            template <std::size_t I, typename... Args>
            explicit constexpr variant_union_storage(in_place_index_t<I>, Args&&... args) : RestValues(in_place_index_t<I>{}, std::forward<Args>(args)...) {
            }

            /**
             * 用户定义析构，否则会因为存在非平凡析构的成员导致析构函数被删除
             */
            ~variant_union_storage() {
            }

            char Dummy;
            T CurrentValue;
            variant_union_storage<false, IndexValue + 1, TRest...> RestValues;

            constexpr T& GetCurrent() & noexcept {
                return CurrentValue;
            }

            constexpr T&& GetCurrent() && noexcept {
                return std::move(CurrentValue);
            }

            constexpr const T& GetCurrent() const & noexcept {
                return CurrentValue;
            }

            constexpr const T&& GetCurrent() const && noexcept {
                return std::move(CurrentValue);
            }
        };

        template <std::size_t Index, typename Storage>
        constexpr decltype(auto) RawGet(Storage&& storage) noexcept;

        template <std::size_t Index, typename Storage>
        constexpr decltype(auto) RawGetImpl(Storage&& storage, std::true_type) noexcept {
            return std::forward<Storage>(storage).GetCurrent();
        }

        template <std::size_t Index, typename Storage>
        constexpr decltype(auto) RawGetImpl(Storage&& storage, std::false_type) noexcept {
            return RawGet<Index>(std::forward<Storage>(storage).RestValues);
        }

        template <std::size_t Index, typename Storage>
        constexpr decltype(auto) RawGet(Storage&& storage) noexcept {
            return RawGetImpl<Index>(std::forward<Storage>(storage), std::integral_constant<bool, RemoveCVRef<Storage>::Index == Index>{});
        }

        template <typename... T>
        using is_all_trivially_destructible = IsAll<std::is_trivially_destructible, T...>;

        template <typename... T>
        using is_all_trivially_copy_constructible = IsAll<std::is_trivially_copy_constructible, T...>;

        template <typename... T>
        using is_all_trivially_copy_assignable = IsAll<std::is_trivially_copy_assignable, T...>;

        template <typename... T>
        using is_all_trivially_move_constructible = IsAll<std::is_trivially_move_constructible, T...>;

        template <typename... T>
        using is_all_trivially_move_assignable = IsAll<std::is_trivially_move_assignable, T...>;

        template <typename... T>
        using variant_union_storage_base = variant_union_storage<is_all_trivially_destructible<T...>::value, 0, T...>;

#if BUGFIX_VARIANT_DISPATCH_TABLE
        template <std::size_t I, typename Callable, typename Variant>
        constexpr void ApplyAlternativeCallable(Callable&& callable, Variant&& variant) {
            std::forward<Callable>(callable)(std::forward<Variant>(variant).template unsafe_get_alternative<I>());
        }
#endif

        template <typename Callable, typename Variant>
        struct DispatchTableGenerator {
            template <std::size_t... I>
            static constexpr std::array<void(*)(Callable&&, Variant&&), RemoveCVRef<Variant>::TypeCount> GenerateDispatchTableImpl(std::index_sequence<I...>) noexcept {
#if BUGFIX_VARIANT_DISPATCH_TABLE
                return { &ApplyAlternativeCallable<I, Callable&&, Variant&&>... };
#else
                return { +[](Callable&& c, Variant&& v) {
                    static_cast<Callable&&>(c)(static_cast<Variant&&>(v).template unsafe_get_alternative<I>());
                }... };
#endif
            }

#if __cplusplus >= 201703L
            static constexpr std::array<void(*)(Callable&&, Variant&&), RemoveCVRef<Variant>::TypeCount> DispatchTable = GenerateDispatchTableImpl(std::make_index_sequence<RemoveCVRef<Variant>::TypeCount>{});
#else
            static const std::array<void(*)(Callable&&, Variant&&), RemoveCVRef<Variant>::TypeCount> DispatchTable;
#endif
        };

#if __cplusplus < 201703L
        template <typename Callable, typename Variant>
        const std::array<void(*)(Callable&&, Variant&&), RemoveCVRef<Variant>::TypeCount> DispatchTableGenerator<Callable, Variant>::DispatchTable = GenerateDispatchTableImpl(std::make_index_sequence<RemoveCVRef<Variant>::TypeCount>{});
#endif
    }

    class invalid_variant : public std::runtime_error {
    public:
        invalid_variant() : std::runtime_error{ "variant is invalid" } {
        }
        using std::runtime_error::runtime_error;
    };

    class bad_variant_access : public std::runtime_error {
    public:
        bad_variant_access() : std::runtime_error{ "variant does not hold the specified alternative" } {
        }
        using std::runtime_error::runtime_error;
    };

    /**
     * @brief 访问变体当前拥有的实际类型的值
     * @param callable 访问者
     * @param variant 变体
     */
    template <typename Callable, typename Variant>
    constexpr void visit(Callable&& callable, Variant&& variant) {
        if (!variant.is_valid()) {
            throw invalid_variant{};
        }
        detail::DispatchTableGenerator<Callable, Variant>::DispatchTable[variant.index()](std::forward<Callable>(callable), std::forward<Variant>(variant));
    }

    /**
     * @brief 访问多个变体当前拥有的实际类型的值
     * @todo 使用分派矩阵实现
     * @param callable 访问者
     * @param variant 变体
     */
    template <typename Callable, typename Variant, typename... RestVariants>
    void visit(Callable&& callable, Variant&& variant, RestVariants&&... restVariants) {
        visit([&](auto&& value) {
            visit([&](auto&&... args) {
                std::forward<Callable>(callable)(static_cast<decltype(value)&&>(value), static_cast<decltype(args)&&>(args)...);
            }, std::forward<RestVariants>(restVariants)...);
        }, std::forward<Variant>(variant));
    }

    namespace detail {
        template <typename... T>
        struct simple_union_variant_base {
            using Types = std::tuple<T...>;
            using IndexType = FindSmallestUnsignedTypeToHoldType<sizeof...(T)>;

            template <std::size_t I>
            using GetType = std::tuple_element_t<I, Types>;

            template <typename TFind>
            static constexpr std::size_t get_index_from_type() noexcept {
                return FindTypeFrom<TFind, Types, NPos>::value;
            }

            template <typename TFind>
            static constexpr bool has_type() noexcept {
                return get_index_from_type<TFind>() == NPos;
            }

            static constexpr std::size_t TypeCount = sizeof...(T);
            static constexpr std::size_t NPos = TypeCount;

            variant_union_storage_base<T...> Storage;
            IndexType Index;

            template <std::size_t I, typename... Args>
            constexpr explicit simple_union_variant_base(in_place_index_t<I> inPlaceArg, Args&&... args) : Storage(inPlaceArg, std::forward<Args>(args)...), Index(static_cast<IndexType>(I)) {
            }

            constexpr simple_union_variant_base() noexcept : Storage(), Index() {
            }

            constexpr std::size_t index() const noexcept {
                return static_cast<std::size_t>(Index);
            }

            constexpr bool is_valid() const noexcept {
                return index() != NPos;
            }

            template <std::size_t I>
            constexpr decltype(auto) unsafe_get_alternative() & noexcept {
                assert(is_valid() && index() == I);
                return RawGet<I>(Storage);
            }

            template <std::size_t I>
            constexpr decltype(auto) unsafe_get_alternative() const & noexcept {
                assert(is_valid() && index() == I);
                return RawGet<I>(Storage);
            }

            template <std::size_t I>
            constexpr decltype(auto) unsafe_get_alternative() && noexcept {
                assert(is_valid() && index() == I);
                return RawGet<I>(std::move(Storage));
            }

            template <std::size_t I>
            constexpr decltype(auto) unsafe_get_alternative() const && noexcept {
                assert(is_valid() && index() == I);
                return RawGet<I>(std::move(Storage));
            }

            template <std::size_t I>
            constexpr decltype(auto) get_alternative() & {
                if (Index == NPos) {
                    throw invalid_variant{};
                }
                if (I != Index) {
                    throw bad_variant_access{};
                }
                return unsafe_get_alternative<I>();
            }

            template <std::size_t I>
            constexpr decltype(auto) get_alternative() const & {
                if (Index == NPos) {
                    throw invalid_variant{};
                }
                if (I != Index) {
                    throw bad_variant_access{};
                }
                return unsafe_get_alternative<I>();
            }

            template <std::size_t I>
            constexpr decltype(auto) get_alternative() && {
                return std::move(get_alternative<I>());
            }

            template <std::size_t I>
            constexpr decltype(auto) get_alternative() const && {
                return std::move(get_alternative<I>());
            }

            constexpr void destroy_current_alternative() {
                if (is_valid()) {
                    visit([](auto &value) {
                        using Type = RemoveCVRef<decltype(value)>;
                        value.~Type();
                    }, *this);
                }
            }

            template <std::size_t I, typename... Args>
            constexpr void emplace(Args&&... args) {
                OWL_SCOPE_FAIL {
                    Index = NPos;
                };
                destroy_current_alternative();
                Index = I;
                auto& place = unsafe_get_alternative<I>();
                using Type = RemoveCVRef<decltype(place)>;
                new (static_cast<void*>(std::addressof(place))) Type(std::forward<Args>(args)...);
            }
        };

        template <typename... T>
        struct always_valid_variant_base {
            using Types = std::tuple<T...>;
            using IndexType = FindSmallestUnsignedTypeToHoldType<sizeof...(T)>;

            template <std::size_t I>
            using GetType = std::tuple_element_t<I, Types>;

            template <typename TFind>
            static constexpr std::size_t get_index_from_type() noexcept {
                return FindTypeFrom<TFind, Types, NPos>::value;
            }

            template <typename TFind>
            static constexpr bool has_type() noexcept {
                return get_index_from_type<TFind>() == NPos;
            }

            static constexpr std::size_t TypeCount = sizeof...(T);
            static constexpr std::size_t NPos = TypeCount;

            variant_union_storage_base<T...> FirstStorage, SecondStorage;
            IndexType Index;
            bool UsingSecondStorage;

            template <std::size_t I, typename... Args>
            constexpr explicit always_valid_variant_base(in_place_index_t<I>, Args&&... args) : FirstStorage{ in_place_index_t<I>{}, std::forward<Args>(args)...}, SecondStorage{}, Index{ static_cast<IndexType>(I) }, UsingSecondStorage{ false } {
            }

            constexpr always_valid_variant_base() noexcept : FirstStorage{}, SecondStorage{}, Index{}, UsingSecondStorage{ false } {
            }

            constexpr std::size_t index() const noexcept {
                return static_cast<std::size_t>(Index);
            }

            constexpr bool is_valid() const noexcept {
                return true;
            }

            template <std::size_t I>
            constexpr decltype(auto) unsafe_get_alternative() & noexcept {
                return RawGet<I>(UsingSecondStorage ? SecondStorage : FirstStorage);
            }

            template <std::size_t I>
            constexpr decltype(auto) unsafe_get_alternative() const & noexcept {
                return RawGet<I>(UsingSecondStorage ? SecondStorage : FirstStorage);
            }

            template <std::size_t I>
            constexpr decltype(auto) unsafe_get_alternative() && noexcept {
                return RawGet<I>(std::move(UsingSecondStorage ? SecondStorage : FirstStorage));
            }

            template <std::size_t I>
            constexpr decltype(auto) unsafe_get_alternative() const && noexcept {
                return RawGet<I>(std::move(UsingSecondStorage ? SecondStorage : FirstStorage));
            }

            template <std::size_t I>
            constexpr decltype(auto) get_alternative() & {
                if (I != Index) {
                    throw bad_variant_access{};
                }
                return unsafe_get_alternative<I>();
            }

            template <std::size_t I>
            constexpr decltype(auto) get_alternative() const & {
                if (I != Index) {
                    throw bad_variant_access{};
                }
                return unsafe_get_alternative<I>();
            }

            template <std::size_t I>
            constexpr decltype(auto) get_alternative() && {
                return std::move(get_alternative<I>());
            }

            template <std::size_t I>
            constexpr decltype(auto) get_alternative() const && {
                return std::move(get_alternative<I>());
            }

            void destroy_current_alternative() {
                visit([](auto &value) {
                    using Type = RemoveCVRef<decltype(value)>;
                    value.~Type();
                }, *this);
            }

            void destroy_specific_alternative(IndexType index, bool secondStorage) {
                const auto oldIndex = std::exchange(Index, index);
                const auto oldUsingSecondStorage = std::exchange(UsingSecondStorage, secondStorage);
                OWL_SCOPE_EXIT {
                    Index = oldIndex;
                    UsingSecondStorage = oldUsingSecondStorage;
                };
                destroy_current_alternative();
            }

            template <std::size_t I, typename... Args>
            void emplace(Args&&... args) {
                const auto oldIndex = std::exchange(Index, I);
                const auto oldUsingSecondStorage = std::exchange(UsingSecondStorage, !UsingSecondStorage);
                OWL_SCOPE_FAIL {
                    Index = oldIndex;
                    UsingSecondStorage = oldUsingSecondStorage;
                };
                auto& place = unsafe_get_alternative<I>();
                using Type = RemoveCVRef<decltype(place)>;
                new (static_cast<void*>(std::addressof(place))) Type(std::forward<Args>(args)...);
                destroy_specific_alternative(oldIndex, oldUsingSecondStorage);
            }
        };

        template <bool IsTriviallyDestructible, typename VariantBase>
        struct variant_destructor : VariantBase {
            using VariantBase::VariantBase;

            ~variant_destructor() {
                this->destroy_current_alternative();
            }
        };

        template <typename VariantBase>
        struct variant_destructor<true, VariantBase> : VariantBase {
            using VariantBase::VariantBase;

            ~variant_destructor() = default;
        };

        template <bool IsTriviallyCopyConstructible, typename VariantBase>
        struct variant_copy_constructor : VariantBase {
            using VariantBase::VariantBase;

            variant_copy_constructor(variant_copy_constructor const& other) : VariantBase() {
                visit([this](const auto& value) {
                    using Type = RemoveCVRef<decltype(value)>;
                    constexpr auto index = VariantBase::template get_index_from_type<Type>();
                    this->Index = index;
                    auto& place = this->template unsafe_get_alternative<index>();
                    new(static_cast<void *>(std::addressof(place))) Type(value);
                }, other);
            }
        };

        template <typename VariantBase>
        struct variant_copy_constructor<true, VariantBase> : VariantBase {
            using VariantBase::VariantBase;

            constexpr variant_copy_constructor(variant_copy_constructor const&) = default;
        };

        template <bool IsTriviallyCopyAssignable, typename VariantBase>
        struct variant_copy_assignment : VariantBase {
            using VariantBase::VariantBase;

            variant_copy_assignment& operator=(const variant_copy_assignment& other) {
                if (this == &other) {
                    return *this;
                }

                if (this->Index != other.Index) {
                    this->~variant_copy_assignment();
                    new (static_cast<void*>(this)) variant_copy_assignment(other);
                } else {
                    visit([this](const auto &value) {
                        using Type = RemoveCVRef<decltype(value)>;
                        constexpr auto index = VariantBase::template get_index_from_type<Type>();
                        auto& place = this->template unsafe_get_alternative<index>();
                        place = value;
                    }, other);
                }
                return *this;
            }
        };

        template <typename VariantBase>
        struct variant_copy_assignment<true, VariantBase> : VariantBase {
            using VariantBase::VariantBase;

            constexpr variant_copy_assignment& operator=(variant_copy_assignment const&) = default;
        };

        template <bool IsTriviallyMoveConstructible, typename VariantBase>
        struct variant_move_constructor : VariantBase {
            using VariantBase::VariantBase;

            variant_move_constructor(variant_move_constructor const& other) : VariantBase() {
                visit([this](const auto& value) {
                    using Type = RemoveCVRef<decltype(value)>;
                    constexpr auto index = VariantBase::template get_index_from_type<Type>();
                    this->Index = index;
                    auto& place = this->template unsafe_get_alternative<index>();
                    new(static_cast<void *>(std::addressof(place))) Type(value);
                }, other);
            }
        };

        template <typename VariantBase>
        struct variant_move_constructor<true, VariantBase> : VariantBase {
            using VariantBase::VariantBase;

            constexpr variant_move_constructor(variant_move_constructor const&) = default;
        };

        template <bool IsTriviallyMoveAssignable, typename VariantBase>
        struct variant_move_assignment : VariantBase {
            using VariantBase::VariantBase;

            variant_move_assignment& operator=(const variant_move_assignment& other) {
                if (this == &other) {
                    return *this;
                }

                if (this->Index != other.Index) {
                    this->~variant_move_assignment();
                    new (static_cast<void*>(this)) variant_move_assignment(other);
                } else {
                    visit([this](const auto &value) {
                        using Type = RemoveCVRef<decltype(value)>;
                        constexpr auto index = VariantBase::template get_index_from_type<Type>();
                        auto& place = this->template unsafe_get_alternative<index>();
                        place = value;
                    }, other);
                }
                return *this;
            }
        };

        template <typename VariantBase>
        struct variant_move_assignment<true, VariantBase> : VariantBase {
            using VariantBase::VariantBase;

            constexpr variant_move_assignment& operator=(variant_move_assignment const&) = default;
        };

        template <typename VariantHolder>
        class variant_convertible_constructor_assignment : public VariantHolder {
        public:
            using VariantHolder::VariantHolder;

            template <typename Test = typename VariantHolder::template GetType<0>, std::enable_if_t<std::is_default_constructible<Test>::value, int> = 0>
            constexpr variant_convertible_constructor_assignment() : VariantHolder{ in_place_index_t<0>{} } {
            }

            template <typename T, std::enable_if_t<!std::is_base_of<variant_convertible_constructor_assignment, RemoveCVRef<T>>::value, int> = 0>
            constexpr variant_convertible_constructor_assignment(T&& value) : VariantHolder{ in_place_index_t<VariantHolder::template get_index_from_type<RemoveCVRef<T>>()>{}, std::forward<T>(value) } {
            }

            using VariantHolder::operator=;

            template <typename T, std::enable_if_t<VariantHolder::template has_type<T>() && !std::is_base_of<variant_convertible_constructor_assignment, T>::value, int> = 0>
            variant_convertible_constructor_assignment& operator=(T&& value) {
                using Type = RemoveCVRef<T>;
                constexpr auto objIndex = VariantHolder::template get_index_from_type<Type>();
                if (objIndex == this->index()) {
                    this->template unsafe_get_alternative<objIndex>() = std::forward<T>(value);
                } else {
                    this->template emplace<objIndex>(std::forward<T>(value));
                }
                return *this;
            }
        };

        // 已知继承构造函数可能在 clang 11 及以下导致无法 constexpr，如 https://godbolt.org/z/ro1dT7
        // 这个问题在 commit 7337f296194483e0959ff980049e2835e226f396 被解决，预计将随 clang 12 发布
        template <template <typename...> class VariantHolderBase, typename... T>
        using basic_variant = variant_convertible_constructor_assignment<
            variant_copy_constructor<
                is_all_trivially_copy_constructible<T...>::value,
                variant_copy_assignment<
                    is_all_trivially_copy_assignable<T...>::value,
                    variant_move_constructor<
                        is_all_trivially_move_constructible<T...>::value,
                        variant_move_assignment<
                            is_all_trivially_move_assignable<T...>::value,
                            variant_destructor<
                                is_all_trivially_destructible<T...>::value,
                                VariantHolderBase<T...>
                            >
                        >
                    >
                >
            >
        >;
    }

    /**
     * @brief 表示多种类型的变体
     */
    template <typename... T>
    using variant = detail::basic_variant<detail::simple_union_variant_base, T...>;

    template <typename... T>
    using always_valid_variant = detail::basic_variant<detail::always_valid_variant_base, T...>;

    struct monostate {
    };

    template <typename T, typename Variant>
    constexpr bool holds_alternative(const Variant& variant) noexcept {
        return variant.index() == owl::RemoveCVRef<Variant>::template get_index_from_type<T>();
    }

    template <typename T, typename Variant>
    constexpr T* get_if(Variant&& variant) noexcept {
        constexpr auto expectedIndex = owl::RemoveCVRef<Variant>::template get_index_from_type<T>();
        if (variant.index() == expectedIndex) {
            return std::addressof(variant.template unsafe_get_alternative<expectedIndex>());
        }
        return nullptr;
    }

    class bad_optional_access : public std::runtime_error {
    public:
        bad_optional_access() : std::runtime_error{ "optional does not have a value" } {
        }

        using std::runtime_error::runtime_error;
    };

    struct nullopt_t {
        constexpr nullopt_t() = default;
    };

    constexpr nullopt_t nullopt{};

    /**
     * @brief 表示可空类型
     */
    template <typename T>
    class optional {
    public:
        constexpr optional() = default;

        constexpr optional(nullopt_t) {
        }

        template <typename U, std::enable_if_t<!std::is_same<RemoveCVRef<U>, optional>::value, int> = 0>
        constexpr optional(U&& value) : m_Storage{ in_place_index_t<1>{}, std::forward<U>(value) } {
        }

        template <typename U, std::enable_if_t<std::is_constructible<T, U&&>::value && !std::is_same<U, optional>::value, int> = 0>
        optional& operator=(U&& value) {
            if (has_value()) {
                unsafe_get_value() = std::forward<U>(value);
            } else {
                emplace(std::forward<U>(value));
            }
            return *this;
        }

        template <typename... Args>
        void emplace(Args&&... args) {
            OWL_SCOPE_FAIL {
                m_Storage.template emplace<0>();
            };
            m_Storage.template emplace<1>(std::forward<Args>(args)...);
        }

        void reset() {
            m_Storage.template emplace<0>();
        }

        constexpr bool has_value() const noexcept {
            return m_Storage.index() == 1;
        }

        constexpr const T* try_get_value() const noexcept {
            if (!has_value()) {
                return nullptr;
            }
            return std::addressof(m_Storage.template unsafe_get_alternative<1>());
        }

        constexpr T* try_get_value() noexcept {
            return const_cast<T*>(static_cast<const optional*>(this)->try_get_value());
        }

        constexpr const T& value() const & {
            if (const auto value = try_get_value()) {
                return *value;
            }
            throw bad_optional_access{};
        }

        constexpr T& value() & {
            return const_cast<T&>(static_cast<const optional*>(this)->value());
        }

        constexpr const T&& value() const && {
            return std::move(value());
        }

        constexpr T&& value() && {
            return std::move(value());
        }

        constexpr const T& unsafe_get_value() const & {
            return m_Storage.template unsafe_get_alternative<1>();
        }

        constexpr T& unsafe_get_value() & {
            return const_cast<T&>(static_cast<const optional*>(this)->unsafe_get_value());
        }

        constexpr const T&& unsafe_get_value() const && {
            return std::move(unsafe_get_value());
        }

        constexpr T&& unsafe_get_value() && {
            return std::move(unsafe_get_value());
        }

        template <typename U>
        constexpr T value_or(U&& defaultValue) const {
            if (const auto value = try_get_value()) {
                return *value;
            }
            return static_cast<T>(std::forward<U>(defaultValue));
        }

        template <typename ValueProducer>
        constexpr T value_or_produce(ValueProducer&& valueProducer) const {
            if (const auto value = try_get_value()) {
                return *value;
            }
            return static_cast<T>(std::forward<ValueProducer>(valueProducer)());
        }

        template <typename ReceiverCallable>
        constexpr void invoke_if_has_value(ReceiverCallable&& receiver) & {
            if (const auto value = try_get_value()) {
                std::forward<ReceiverCallable>(receiver)(*value);
            }
        }

        template <typename ReceiverCallable>
        constexpr void invoke_if_has_value(ReceiverCallable&& receiver) const & {
            if (const auto value = try_get_value()) {
                std::forward<ReceiverCallable>(receiver)(*value);
            }
        }

        template <typename ReceiverCallable>
        constexpr void invoke_if_has_value(ReceiverCallable&& receiver) && {
            if (const auto value = try_get_value()) {
                std::forward<ReceiverCallable>(receiver)(std::move(*value));
            }
        }

        template <typename ReceiverCallable>
        constexpr void invoke_if_has_value(ReceiverCallable&& receiver) const && {
            if (const auto value = try_get_value()) {
                std::forward<ReceiverCallable>(receiver)(std::move(*value));
            }
        }

        // 模仿指针操作

        constexpr explicit operator bool() const noexcept {
            return has_value();
        }

        constexpr T& operator*() & noexcept {
            return unsafe_get_value();
        }

        constexpr const T& operator*() const & noexcept {
            return unsafe_get_value();
        }

        constexpr T&& operator*() && noexcept {
            return unsafe_get_value();
        }

        constexpr const T&& operator*() const && noexcept {
            return unsafe_get_value();
        }

        constexpr T* operator->() noexcept {
            return std::addressof(unsafe_get_value());
        }

        constexpr const T* operator->() const noexcept {
            return std::addressof(unsafe_get_value());
        }

    private:
        variant<monostate, T> m_Storage;
    };
}

#endif //NGOWL_VARIANT_H
