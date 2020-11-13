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
    struct in_place : std::integral_constant<std::size_t, I> {
    };

    namespace detail {
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
     * @todo 实现多分派
     * @param callable 访问者
     * @param variant 变体
     */
    template <typename Callable, typename Variant>
    void visit(Callable&& callable, Variant&& variant) {
        if (!variant.is_valid()) {
            throw invalid_variant{};
        }
        detail::DispatchTableGenerator<Callable, Variant>::DispatchTable[variant.index()](std::forward<Callable>(callable), std::forward<Variant>(variant));
    }

    // TODO: 修改为 union 实现以允许作为 LiteralType
    template <typename... T>
    class simple_union_variant_holder {
    public:
        using Types = std::tuple<T...>;
        static constexpr std::size_t TypeCount = sizeof...(T);
        static constexpr std::size_t NPos = TypeCount;

        template <std::size_t I>
        using GetType = std::tuple_element_t<I, Types>;

        template <typename TFind>
        static constexpr std::size_t get_index_from_types() noexcept {
            return FindTypeFrom<TFind, Types>::value;
        }

        template <std::size_t I, typename... Args>
        constexpr simple_union_variant_holder(in_place<I>, Args&&... args) : m_CurrentIndex{ I } {
            new (static_cast<void*>(m_Storage)) GetType<I>(std::forward<Args>(args)...);
        }

        simple_union_variant_holder(const simple_union_variant_holder& other) : m_CurrentIndex{ other.m_CurrentIndex } {
            visit([this](const auto &value) {
                visit([&](auto &place) {
                    new(static_cast<void *>(&place)) RemoveCVRef<decltype(place)>(value);
                }, *this);
            }, other);
        }

        simple_union_variant_holder(simple_union_variant_holder&& other) noexcept : m_CurrentIndex{ other.m_CurrentIndex } {
            visit([this](auto &&value) {
                visit([&](auto &place) {
                    new(static_cast<void *>(&place)) RemoveCVRef<decltype(place)>(std::move(value));
                }, *this);
            }, std::move(other));
        }

    protected:
        ~simple_union_variant_holder() {
            DestroyCurrentAlternative();
        }

    public:
        simple_union_variant_holder& operator=(const simple_union_variant_holder& other) {
            if (m_CurrentIndex != other.m_CurrentIndex) {
                this->~simple_union_variant_holder();
                new (static_cast<void*>(this)) simple_union_variant_holder(other);
            } else {
                visit([this](const auto &value) {
                    visit([&](auto &place) {
                        place = value;
                    }, *this);
                }, other);
            }
            return *this;
        }

        simple_union_variant_holder& operator=(simple_union_variant_holder&& other) noexcept {
            if (m_CurrentIndex != other.m_CurrentIndex) {
                this->~simple_union_variant_holder();
                new (static_cast<void*>(this)) simple_union_variant_holder(std::move(other));
            } else {
                visit([this](auto &&value) {
                    visit([&](auto &place) {
                        place = std::move(value);
                    }, *this);
                }, std::move(other));
            }
            return *this;
        }

        constexpr std::size_t index() const noexcept {
            return static_cast<std::size_t>(m_CurrentIndex);
        }

        constexpr bool is_valid() const noexcept {
            return m_CurrentIndex != NPos;
        }

        template <std::size_t I>
        const GetType<I>& unsafe_get_alternative() const & noexcept {
            assert(m_CurrentIndex != NPos && I == m_CurrentIndex);
            return *reinterpret_cast<const GetType<I>*>(m_Storage);
        }

        template <std::size_t I>
        GetType<I>& unsafe_get_alternative() & noexcept {
            return const_cast<GetType<I>&>(static_cast<const simple_union_variant_holder *>(this)->unsafe_get_alternative<I>());
        }

        template <std::size_t I>
        const GetType<I>&& unsafe_get_alternative() const && noexcept {
            return std::move(unsafe_get_alternative<I>());
        }

        template <std::size_t I>
        GetType<I>&& unsafe_get_alternative() && noexcept {
            return std::move(unsafe_get_alternative<I>());
        }

        template <std::size_t I>
        const GetType<I>& get_alternative() const & {
            if (m_CurrentIndex == NPos) {
                throw invalid_variant{};
            }
            if (I != m_CurrentIndex) {
                throw bad_variant_access{};
            }
            return *reinterpret_cast<const GetType<I>*>(m_Storage);
        }

        template <std::size_t I>
        GetType<I>& get_alternative() & {
            return const_cast<GetType<I>&>(static_cast<const simple_union_variant_holder*>(this)->get_alternative<I>());
        }

        template <std::size_t I>
        const GetType<I>&& get_alternative() const && {
            return std::move(get_alternative<I>());
        }

        template <std::size_t I>
        GetType<I>&& get_alternative() && {
            return std::move(get_alternative<I>());
        }

        template <std::size_t I, typename... Args>
        void emplace(Args&&... args) {
            DestroyCurrentAlternative();

            OWL_SCOPE_FAIL {
                m_CurrentIndex = NPos;
            };
            new (static_cast<void*>(m_Storage)) GetType<I>(std::forward<Args>(args)...);
            m_CurrentIndex = I;
        }

    private:
        FindSmallestUnsignedTypeToHoldType<sizeof...(T)> m_CurrentIndex;
        alignas(T...) unsigned char m_Storage[std::max<std::size_t>({ std::size_t{ 1 }, sizeof(T)... })];

        void DestroyCurrentAlternative() noexcept {
            if (is_valid()) {
                visit([](auto &value) {
                    using Type = RemoveCVRef<decltype(value)>;
                    value.~Type();
                }, *this);
            }
        }
    };

    template <typename... T>
    class always_valid_variant_holder {
    public:
        using Types = std::tuple<T...>;
        static constexpr std::size_t TypeCount = sizeof...(T);

        template <std::size_t I>
        using GetType = std::tuple_element_t<I, Types>;

        template <typename TFind>
        static constexpr std::size_t get_index_from_types() noexcept {
            return FindTypeFrom<TFind, Types>::value;
        }

        template <std::size_t I, typename... Args>
        constexpr always_valid_variant_holder(in_place<I>, Args&&... args) : m_CurrentIndex{ I }, m_UsingRightStorage{ false } {
            new (static_cast<void*>(m_Storage[0])) GetType<I>(std::forward<Args>(args)...);
        }

        always_valid_variant_holder(const always_valid_variant_holder& other) : m_CurrentIndex{ other.m_CurrentIndex }, m_UsingRightStorage{ false } {
            visit([this](const auto &value) {
                visit([&](auto &place) {
                    new(static_cast<void *>(&place)) RemoveCVRef<decltype(place)>(value);
                }, *this);
            }, other);
        }

        always_valid_variant_holder(always_valid_variant_holder&& other) noexcept : m_CurrentIndex{ other.m_CurrentIndex }, m_UsingRightStorage{ false } {
            visit([this](auto &&value) {
                visit([&](auto &place) {
                    new(static_cast<void *>(&place)) RemoveCVRef<decltype(place)>(std::move(value));
                }, *this);
            }, std::move(other));
        }

    protected:
        ~always_valid_variant_holder() {
            DestroyCurrentAlternative();
        }

    public:
        always_valid_variant_holder& operator=(const always_valid_variant_holder& other) {
            if (m_CurrentIndex != other.m_CurrentIndex) {
                this->~always_valid_variant_holder();
                new (static_cast<void*>(this)) always_valid_variant_holder(other);
            } else {
                visit([this](const auto &value) {
                    visit([&](auto &place) {
                        place = value;
                    }, *this);
                }, other);
            }
            return *this;
        }

        always_valid_variant_holder& operator=(always_valid_variant_holder&& other) noexcept {
            if (m_CurrentIndex != other.m_CurrentIndex) {
                this->~always_valid_variant_holder();
                new (static_cast<void*>(this)) always_valid_variant_holder(std::move(other));
            } else {
                visit([this](auto &&value) {
                    visit([&](auto &place) {
                        place = std::move(value);
                    }, *this);
                }, std::move(other));
            }
            return *this;
        }

        constexpr std::size_t index() const noexcept {
            return static_cast<std::size_t>(m_CurrentIndex);
        }

        constexpr bool is_valid() const noexcept {
            return true;
        }

        template <std::size_t I>
        const GetType<I>& unsafe_get_alternative() const & noexcept {
            assert(I == m_CurrentIndex);
            return *reinterpret_cast<const GetType<I>*>(m_Storage[m_UsingRightStorage]);
        }

        template <std::size_t I>
        GetType<I>& unsafe_get_alternative() & noexcept {
            return const_cast<GetType<I>&>(static_cast<const always_valid_variant_holder *>(this)->unsafe_get_alternative<I>());
        }

        template <std::size_t I>
        const GetType<I>&& unsafe_get_alternative() const && noexcept {
            return std::move(unsafe_get_alternative<I>());
        }

        template <std::size_t I>
        GetType<I>&& unsafe_get_alternative() && noexcept {
            return std::move(unsafe_get_alternative<I>());
        }

        template <std::size_t I>
        const GetType<I>& get_alternative() const & {
            if (I != m_CurrentIndex) {
                throw bad_variant_access{};
            }
            return *reinterpret_cast<const GetType<I>*>(m_Storage[m_UsingRightStorage]);
        }

        template <std::size_t I>
        GetType<I>& get_alternative() & {
            return const_cast<GetType<I>&>(static_cast<const always_valid_variant_holder*>(this)->get_alternative<I>());
        }

        template <std::size_t I>
        const GetType<I>&& get_alternative() const && {
            return std::move(get_alternative<I>());
        }

        template <std::size_t I>
        GetType<I>&& get_alternative() && {
            return std::move(get_alternative<I>());
        }

        template <std::size_t I, typename... Args>
        void emplace(Args&&... args) {
            new (static_cast<void*>(m_Storage[!m_UsingRightStorage])) GetType<I>(std::forward<Args>(args)...);
            DestroyCurrentAlternative();
            m_CurrentIndex = I;
            m_UsingRightStorage = !m_UsingRightStorage;
        }

    private:
        FindSmallestUnsignedTypeToHoldType<sizeof...(T)> m_CurrentIndex;
        bool m_UsingRightStorage;
        alignas(T...) unsigned char m_Storage[2][std::max<std::size_t>({ std::size_t{ 1 }, sizeof(T)... })];

        void DestroyCurrentAlternative() noexcept {
            visit([](auto &value) {
                using Type = RemoveCVRef<decltype(value)>;
                value.~Type();
            }, *this);
        }
    };

    template <typename VariantHolder>
    class basic_variant : public VariantHolder {
    public:
        using VariantHolder::VariantHolder;

        template <typename Test = typename VariantHolder::template GetType<0>, std::enable_if_t<std::is_default_constructible<Test>::value, int> = 0>
        basic_variant() : VariantHolder{ in_place<0>{} } {
        }

        template <typename T, std::enable_if_t<!std::is_same<RemoveCVRef<T>, basic_variant>::value, int> = 0>
        basic_variant(T&& value) : VariantHolder{ in_place<VariantHolder::template get_index_from_types<RemoveCVRef<T>>()>{}, std::forward<T>(value) } {
        }
    };

    /**
     * @brief 表示多种类型的变体
     */
    template <typename... T>
    using variant = basic_variant<simple_union_variant_holder<T...>>;

    template <typename... T>
    using always_valid_variant = basic_variant<always_valid_variant_holder<T...>>;

    struct monostate {
    };

    class bad_optional_access : public std::runtime_error {
    public:
        bad_optional_access() : std::runtime_error{ "optional does not have a value" } {
        }

        using std::runtime_error::runtime_error;
    };

    /**
     * @brief 表示可空类型
     */
    template <typename T>
    class optional {
    public:
        optional() = default;

        template <typename U, std::enable_if_t<!std::is_same<RemoveCVRef<U>, optional>::value, int> = 0>
        optional(U&& value) : m_Storage{ in_place<1>{}, std::forward<U>(value) } {
        }

        template <typename... Args>
        void emplace(Args&&... args) {
            OWL_SCOPE_FAIL {
                m_Storage.template emplace<0>();
            };
            m_Storage.template emplace<1>(std::forward<Args>(args)...);
        }

        bool has_value() const noexcept {
            return m_Storage.index() == 1;
        }

        const T* try_get_value() const noexcept {
            if (!has_value()) {
                return nullptr;
            }
            return &m_Storage.template unsafe_get_alternative<1>();
        }

        T* try_get_value() noexcept {
            return const_cast<T*>(static_cast<const optional*>(this)->try_get_value());
        }

        const T& value() const & {
            if (const auto value = try_get_value()) {
                return *value;
            }
            throw bad_optional_access{};
        }

        T& value() & {
            return const_cast<T&>(static_cast<const optional*>(this)->value());
        }

        const T&& value() const && {
            return std::move(value());
        }

        T&& value() && {
            return std::move(value());
        }

        template <typename U>
        T value_or(U&& defaultValue) const {
            if (const auto value = try_get_value()) {
                return *value;
            }
            return static_cast<T>(std::forward<U>(defaultValue));
        }

        template <typename ValueProducer>
        T value_or_produce(ValueProducer&& valueProducer) const {
            if (const auto value = try_get_value()) {
                return *value;
            }
            return static_cast<T>(std::forward<ValueProducer>(valueProducer)());
        }

        template <typename ReceiverCallable>
        void invoke_if_has_value(ReceiverCallable&& receiver) & {
            if (const auto value = try_get_value()) {
                std::forward<ReceiverCallable>(receiver)(*value);
            }
        }

        template <typename ReceiverCallable>
        void invoke_if_has_value(ReceiverCallable&& receiver) const & {
            if (const auto value = try_get_value()) {
                std::forward<ReceiverCallable>(receiver)(*value);
            }
        }

        template <typename ReceiverCallable>
        void invoke_if_has_value(ReceiverCallable&& receiver) && {
            if (const auto value = try_get_value()) {
                std::forward<ReceiverCallable>(receiver)(std::move(*value));
            }
        }

        template <typename ReceiverCallable>
        void invoke_if_has_value(ReceiverCallable&& receiver) const && {
            if (const auto value = try_get_value()) {
                std::forward<ReceiverCallable>(receiver)(std::move(*value));
            }
        }

        // 模仿指针操作

        explicit operator bool() const noexcept {
            return has_value();
        }

        T& operator*() & noexcept {
            return *try_get_value();
        }

        const T& operator*() const & noexcept {
            return *try_get_value();
        }

        T&& operator*() && noexcept {
            return std::move(*try_get_value());
        }

        const T&& operator*() const && noexcept {
            return std::move(*try_get_value());
        }

        T* operator->() noexcept {
            return try_get_value();
        }

        const T* operator->() const noexcept {
            return try_get_value();
        }

    private:
        variant<monostate, T> m_Storage;
    };
}

#endif //NGOWL_VARIANT_H
