//
// Created by  kylinnhuang on 2020/8/19.
// Copyright (c) 2020 peterfan. All rights reserved.
//

#ifndef NGOWL_SCOPE_H
#define NGOWL_SCOPE_H

#include "misc.h"

namespace owl {
    struct AlwaysCall {
        template <typename T>
        constexpr std::true_type operator()(T&&) const noexcept {
            return {};
        }
    };

    struct CallOnError {
#if __cplusplus >= 201703L
        int ExceptionCount = std::uncaught_exceptions();
#endif

        template <typename T>
        bool operator()(T&&) const noexcept {
#if __cplusplus < 201703L
            return std::uncaught_exception();
#else
            return std::uncaught_exceptions() > ExceptionCount;
#endif
        }
    };

    template <typename Callable, typename ShouldCallCleanerPredicate = AlwaysCall>
    class Scope
        : CompressedPair<RemoveCVRef<Callable>, RemoveCVRef<ShouldCallCleanerPredicate>> {
        using Storage = CompressedPair<RemoveCVRef<Callable>, RemoveCVRef<ShouldCallCleanerPredicate>>;

    public:
        template <typename CallableValue, typename ShouldCallCleanerPredicateValue>
        Scope(CallableValue&& callable, ShouldCallCleanerPredicateValue&& shouldCallCleaner)
            : Storage(std::forward<CallableValue>(callable), std::forward<ShouldCallCleanerPredicateValue>(shouldCallCleaner)), m_ShouldCall{ true } {
        }

        Scope(Scope&& other) noexcept
            : Storage{ static_cast<Storage&&>(std::move(other)) }, m_ShouldCall{ std::exchange(other.m_ShouldCall, false) } {
        }

        ~Scope() {
            if (m_ShouldCall && Storage::Second()(Storage::First())) {
                Storage::First()();
            }
        }

        void SetShouldCall(bool value) noexcept {
            m_ShouldCall = value;
        }

        void ExplicitCall() {
            if (!m_ShouldCall || !Storage::Second()(Storage::First())) {
                return;
            }
            Storage::First()();
            m_ShouldCall = false;
        }

    private:
        bool m_ShouldCall;
    };

    namespace detail {
        template <typename ShouldCallCleanerPredicate>
        struct ScopeBuilder {
            template <typename ShouldCallCleanerPredicateValue>
            ScopeBuilder(ShouldCallCleanerPredicateValue&& shouldCallCleaner)
                    : ShouldCallCleaner{ std::forward<ShouldCallCleanerPredicateValue>(shouldCallCleaner) } {
            }

            ScopeBuilder(ScopeBuilder const&) = delete;
            ScopeBuilder& operator=(ScopeBuilder const&) = delete;

            RemoveCVRef<ShouldCallCleanerPredicate> ShouldCallCleaner;

            template <typename Callable>
            Scope<Callable, ShouldCallCleanerPredicate> operator*(Callable&& callable) const {
                return { std::forward<Callable>(callable), std::move(ShouldCallCleaner) };
            }
        };
    }

#define OWL_SCOPE_EXIT_WITH_ID(id) auto id = ::owl::detail::ScopeBuilder<::owl::AlwaysCall>{ ::owl::AlwaysCall{} } * [&] () mutable
#define OWL_SCOPE_FAIL_WITH_ID(id) auto id = ::owl::detail::ScopeBuilder<::owl::CallOnError>{ ::owl::CallOnError{} } * [&] () mutable

/**
 * @brief 在退出当前范围时执行操作
 * @code
 * OWL_SCOPE_EXIT {
 *     // 执行清理操作
 * };
 * @endcode
 */
#define OWL_SCOPE_EXIT OWL_SCOPE_EXIT_WITH_ID(OWL_MAGIC_ID)

/**
 * @brief 在因异常发生退出当前范围时执行操作
 * @code
 * OWL_SCOPE_FAIL {
 *     // 执行回滚等操作
 * };
 * @endcode
 */
#define OWL_SCOPE_FAIL OWL_SCOPE_FAIL_WITH_ID(OWL_MAGIC_ID)
}

#endif //NGOWL_SCOPE_H
