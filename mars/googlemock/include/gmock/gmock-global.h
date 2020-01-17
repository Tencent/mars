#pragma once
#include <memory>

//
// Mock classes for different argument count definitions
//

#define MOCK_GLOBAL_CHECK_INIT(Method)                                                \
    if (!gmock_globalmock_##Method##_instance)                                        \
    {                                                                                 \
        throw std::logic_error("You forgot to call EXPECT_GLOBAL_CALL for " #Method); \
    }

//
// Mock class and macroses for 0 arguments global function
//
#define MOCK_GLOBAL_FUNC0_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method () constness {  \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 0), \
        this_method_does_not_take_0_arguments); \
    GMOCK_MOCKER_(0, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(0, constness, Method).Invoke(); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method() constness { \
    GMOCK_MOCKER_(0, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(0, constness, Method).With(); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(0, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method() constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method();\
      }\

#define MOCK_GLOBAL_FUNC0(m, ...) MOCK_GLOBAL_FUNC0_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC0_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC0_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 1 arguments global function
//
#define MOCK_GLOBAL_FUNC1_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1) constness {  \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 1), \
        this_method_does_not_take_1_arguments); \
    GMOCK_MOCKER_(1, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(1, constness, Method).Invoke(gmock_a1); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1) constness { \
    GMOCK_MOCKER_(1, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(1, constness, Method).With(gmock_a1); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(1, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1);\
      }\

#define MOCK_GLOBAL_FUNC1(m, ...) MOCK_GLOBAL_FUNC1_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC1_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC1_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 2 arguments global function
//
#define MOCK_GLOBAL_FUNC2_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 2), \
        this_method_does_not_take_2_arguments); \
    GMOCK_MOCKER_(2, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(2, constness, Method).Invoke(gmock_a1, gmock_a2); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2) constness { \
    GMOCK_MOCKER_(2, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(2, constness, Method).With(gmock_a1, gmock_a2); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(2, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2);\
      }\

#define MOCK_GLOBAL_FUNC2(m, ...) MOCK_GLOBAL_FUNC2_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC2_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC2_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 3 arguments global function
//
#define MOCK_GLOBAL_FUNC3_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 3), \
        this_method_does_not_take_3_arguments); \
    GMOCK_MOCKER_(3, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(3, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3) constness { \
    GMOCK_MOCKER_(3, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(3, constness, Method).With(gmock_a1, gmock_a2, gmock_a3); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(3, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3);\
      }\

#define MOCK_GLOBAL_FUNC3(m, ...) MOCK_GLOBAL_FUNC3_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC3_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC3_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 4 arguments global function
//
#define MOCK_GLOBAL_FUNC4_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 4), \
        this_method_does_not_take_4_arguments); \
    GMOCK_MOCKER_(4, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(4, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4) constness { \
    GMOCK_MOCKER_(4, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(4, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(4, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4 ) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4);\
      }\

#define MOCK_GLOBAL_FUNC4(m, ...) MOCK_GLOBAL_FUNC4_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC4_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC4_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 5 arguments global function
//
#define MOCK_GLOBAL_FUNC5_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 5), \
        this_method_does_not_take_5_arguments); \
    GMOCK_MOCKER_(5, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(5, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5) constness { \
    GMOCK_MOCKER_(5, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(5, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(5, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5);\
      }\

#define MOCK_GLOBAL_FUNC5(m, ...) MOCK_GLOBAL_FUNC5_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC5_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC5_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 6 arguments global function
//
#define MOCK_GLOBAL_FUNC6_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 6), \
        this_method_does_not_take_6_arguments); \
    GMOCK_MOCKER_(6, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(6, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6) constness { \
    GMOCK_MOCKER_(6, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(6, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(6, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6);\
      }\

#define MOCK_GLOBAL_FUNC6(m, ...) MOCK_GLOBAL_FUNC6_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC6_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC6_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 7 arguments global function
//
#define MOCK_GLOBAL_FUNC7_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 7), \
        this_method_does_not_take_7_arguments); \
    GMOCK_MOCKER_(7, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(7, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7) constness { \
    GMOCK_MOCKER_(7, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(7, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(7, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7);\
      }\

#define MOCK_GLOBAL_FUNC7(m, ...) MOCK_GLOBAL_FUNC7_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC7_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC7_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 8 arguments global function
//
#define MOCK_GLOBAL_FUNC8_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 8), \
        this_method_does_not_take_8_arguments); \
    GMOCK_MOCKER_(8, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(8, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8) constness { \
    GMOCK_MOCKER_(8, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(8, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(8, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8);\
      }\

#define MOCK_GLOBAL_FUNC8(m, ...) MOCK_GLOBAL_FUNC8_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC8_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC8_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 9 arguments global function
//
#define MOCK_GLOBAL_FUNC9_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 9), \
        this_method_does_not_take_9_arguments); \
    GMOCK_MOCKER_(9, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(9, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9) constness { \
    GMOCK_MOCKER_(9, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(9, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(9, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9);\
      }\

#define MOCK_GLOBAL_FUNC9(m, ...) MOCK_GLOBAL_FUNC9_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC9_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC9_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 10 arguments global function
//
#define MOCK_GLOBAL_FUNC10_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 10), \
        this_method_does_not_take_10_arguments); \
    GMOCK_MOCKER_(10, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(10, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                     GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10 ) constness { \
    GMOCK_MOCKER_(10, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(10, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(10, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10);\
      }\

#define MOCK_GLOBAL_FUNC10(m, ...) MOCK_GLOBAL_FUNC10_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC10_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC10_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 11 arguments global function
//
#define MOCK_GLOBAL_FUNC11_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 11), \
        this_method_does_not_take_11_arguments); \
    GMOCK_MOCKER_(11, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(11, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                     GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10, \
                     GMOCK_MATCHER_(tn, 11, __VA_ARGS__) gmock_a11 ) constness { \
    GMOCK_MOCKER_(11, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(11, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(11, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11);\
      }\

#define MOCK_GLOBAL_FUNC11(m, ...) MOCK_GLOBAL_FUNC11_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC11_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC11_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 12 arguments global function
//
#define MOCK_GLOBAL_FUNC12_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11, \
      GMOCK_ARG_(tn, 12, __VA_ARGS__) gmock_a12) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 12), \
        this_method_does_not_take_12_arguments); \
    GMOCK_MOCKER_(12, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(12, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                     GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10, \
                     GMOCK_MATCHER_(tn, 11, __VA_ARGS__) gmock_a11, \
                     GMOCK_MATCHER_(tn, 12, __VA_ARGS__) gmock_a12 ) constness { \
    GMOCK_MOCKER_(12, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(12, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(12, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11, \
      GMOCK_ARG_(tn, 12, __VA_ARGS__) gmock_a12) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12);\
      }\

#define MOCK_GLOBAL_FUNC12(m, ...) MOCK_GLOBAL_FUNC12_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC12_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC12_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 13 arguments global function
//
#define MOCK_GLOBAL_FUNC13_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11, \
      GMOCK_ARG_(tn, 12, __VA_ARGS__) gmock_a12, \
      GMOCK_ARG_(tn, 13, __VA_ARGS__) gmock_a13) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 13), \
        this_method_does_not_take_13_arguments); \
    GMOCK_MOCKER_(13, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(13, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                     GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10, \
                     GMOCK_MATCHER_(tn, 11, __VA_ARGS__) gmock_a11, \
                     GMOCK_MATCHER_(tn, 12, __VA_ARGS__) gmock_a12, \
                     GMOCK_MATCHER_(tn, 13, __VA_ARGS__) gmock_a13 ) constness { \
    GMOCK_MOCKER_(13, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(13, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(13, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11, \
      GMOCK_ARG_(tn, 12, __VA_ARGS__) gmock_a12, \
      GMOCK_ARG_(tn, 13, __VA_ARGS__) gmock_a13) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13);\
      }\

#define MOCK_GLOBAL_FUNC13(m, ...) MOCK_GLOBAL_FUNC13_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC13_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC13_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 14 arguments global function
//
#define MOCK_GLOBAL_FUNC14_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11, \
      GMOCK_ARG_(tn, 12, __VA_ARGS__) gmock_a12, \
      GMOCK_ARG_(tn, 13, __VA_ARGS__) gmock_a13, \
      GMOCK_ARG_(tn, 14, __VA_ARGS__) gmock_a14) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 14), \
        this_method_does_not_take_14_arguments); \
    GMOCK_MOCKER_(14, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(14, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13, gmock_a14); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                     GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10, \
                     GMOCK_MATCHER_(tn, 11, __VA_ARGS__) gmock_a11, \
                     GMOCK_MATCHER_(tn, 12, __VA_ARGS__) gmock_a12, \
                     GMOCK_MATCHER_(tn, 13, __VA_ARGS__) gmock_a13, \
                     GMOCK_MATCHER_(tn, 14, __VA_ARGS__) gmock_a14 ) constness { \
    GMOCK_MOCKER_(14, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(14, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13, gmock_a14); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(14, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11, \
      GMOCK_ARG_(tn, 12, __VA_ARGS__) gmock_a12, \
      GMOCK_ARG_(tn, 13, __VA_ARGS__) gmock_a13, \
      GMOCK_ARG_(tn, 14, __VA_ARGS__) gmock_a14) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13, gmock_a14);\
      }\

#define MOCK_GLOBAL_FUNC14(m, ...) MOCK_GLOBAL_FUNC14_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC14_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC14_(, , ct, m, __VA_ARGS__)

//
// Mock class and macroses for 15 arguments global function
//
#define MOCK_GLOBAL_FUNC15_(tn, constness, ct, Method, ...) \
class gmock_globalmock_##Method { \
public:\
  gmock_globalmock_##Method(const char* tag) : m_tag(tag) {}  \
  const char* const m_tag; \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11, \
      GMOCK_ARG_(tn, 12, __VA_ARGS__) gmock_a12, \
      GMOCK_ARG_(tn, 13, __VA_ARGS__) gmock_a13, \
      GMOCK_ARG_(tn, 14, __VA_ARGS__) gmock_a14, \
      GMOCK_ARG_(tn, 15, __VA_ARGS__) gmock_a15) constness { \
    GTEST_COMPILE_ASSERT_((std::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 15), \
        this_method_does_not_take_15_arguments); \
    GMOCK_MOCKER_(15, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(15, constness, Method).Invoke(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13, gmock_a14, gmock_a15); \
  } \
  ::testing::MockSpec<__VA_ARGS__> \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                     GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10, \
                     GMOCK_MATCHER_(tn, 11, __VA_ARGS__) gmock_a11, \
                     GMOCK_MATCHER_(tn, 12, __VA_ARGS__) gmock_a12, \
                     GMOCK_MATCHER_(tn, 13, __VA_ARGS__) gmock_a13, \
                     GMOCK_MATCHER_(tn, 14, __VA_ARGS__) gmock_a14, \
                     GMOCK_MATCHER_(tn, 15, __VA_ARGS__) gmock_a15 ) constness { \
    GMOCK_MOCKER_(15, constness, Method).RegisterOwner(this); \
    return GMOCK_MOCKER_(15, constness, Method).With(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13, gmock_a14, gmock_a15); \
  } \
  mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(15, constness, \
      Method); \
   }; \
   std::unique_ptr< gmock_globalmock_##Method > gmock_globalmock_##Method##_instance;\
   GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11, \
      GMOCK_ARG_(tn, 12, __VA_ARGS__) gmock_a12, \
      GMOCK_ARG_(tn, 13, __VA_ARGS__) gmock_a13, \
      GMOCK_ARG_(tn, 14, __VA_ARGS__) gmock_a14, \
      GMOCK_ARG_(tn, 15, __VA_ARGS__) gmock_a15) constness { \
       MOCK_GLOBAL_CHECK_INIT(Method); \
       return gmock_globalmock_##Method##_instance->Method(gmock_a1, gmock_a2, gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, gmock_a10, gmock_a11, gmock_a12, gmock_a13, gmock_a14, gmock_a15);\
      }\

#define MOCK_GLOBAL_FUNC15(m, ...) MOCK_GLOBAL_FUNC15_(, , , m, __VA_ARGS__)
#define MOCK_GLOBAL_FUNC15_WITH_CALLTYPE(ct, m, ...) MOCK_GLOBAL_FUNC15_(, , ct, m, __VA_ARGS__)

//
// Code for deleting mock objects generated for global function in the end of each test.
//

template <typename T>
class GlobalMockDeleter
{
public:
    GlobalMockDeleter(std::unique_ptr<T>& ref) : m_mockReference(ref) {}

    ~GlobalMockDeleter()
    {
        m_mockReference.reset();
    }

private:
    std::unique_ptr<T>& m_mockReference;
};

#define GLOBAL_MOCK_DELETER_NAME3(x, y)     x##y
#define GLOBAL_MOCK_DELETER_NAME2(x, y)     GLOBAL_MOCK_DELETER_NAME3(x, y)
#define GLOBAL_MOCK_DELETER_NAME(x)         GLOBAL_MOCK_DELETER_NAME2(x, __COUNTER__)

#define GLOBAL_MOCK_TYPE(name)              gmock_globalmock_##name
#define GLOBAL_MOCK_INSTANCE(name)          gmock_globalmock_##name##_instance

#define GLOBAL_MOCK_CALL(name, method, callType) \
GlobalMockDeleter<GLOBAL_MOCK_TYPE(name)> GLOBAL_MOCK_DELETER_NAME(mock_deleter)(GLOBAL_MOCK_INSTANCE(name));\
if (!GLOBAL_MOCK_INSTANCE(name) || 0 != strcmp(GLOBAL_MOCK_INSTANCE(name)->m_tag, __FUNCTION__)) GLOBAL_MOCK_INSTANCE(name).reset(new GLOBAL_MOCK_TYPE(name)(__FUNCTION__));\
callType(*GLOBAL_MOCK_INSTANCE(name), method)

#define EXPECT_GLOBAL_CALL(name, method)    GLOBAL_MOCK_CALL(name, method, EXPECT_CALL)
#define ON_GLOBAL_CALL(name, method)        GLOBAL_MOCK_CALL(name, method, ON_CALL)