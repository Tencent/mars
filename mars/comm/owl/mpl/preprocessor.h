/*
 ============================================================================
 Name		: preprocessor.h
 Author		: peterfan
 Version	: 1.0
 Created on	: 2010-4-18
 Copyright	: Copyright (C) 1998 - 2009 TENCENT Inc. All Right Reserved
 Description:
 ============================================================================
 */

#ifndef PREPROCESSOR_H_
#define PREPROCESSOR_H_

#define PP_EMPTY()
#define PP_COMMA() ,

// PP_CAT
#define PP_CAT(x, y)	PP_CAT_I(x, y)
#define PP_CAT_I(x, y)	x##y

// PP_EXPAND
// FUCK: MSVC doesn't expand __VA_ARGS__ correctly
#define PP_EXPAND(...) __VA_ARGS__

// PP_S
// FUCK: PP_S() should expanded to "", but expanded to nothing in MSVC
#define PP_S(...)       PP_S_I(__VA_ARGS__)
#define PP_S_I(...)     PP_IF(PP_VARIADIC_SIZE(__VA_ARGS__), PP_S_II(__VA_ARGS__), "")
#define PP_S_II(...)    #__VA_ARGS__

// PP_ERROR
#define PP_ERROR(code) PP_CAT(PP_ERROR_, code)
#define PP_ERROR_0x0000 PP_ERROR(0x0000, PP_INDEX_OUT_OF_BOUNDS)
#define PP_ERROR_0x0001 PP_ERROR(0x0001, PP_PP_WHILE_OVERFLOW)
#define PP_ERROR_0x0002 PP_ERROR(0x0002, PP_FOR_OVERFLOW)
#define PP_ERROR_0x0003 PP_ERROR(0x0003, PP_REPEAT_OVERFLOW)
#define PP_ERROR_0x0004 PP_ERROR(0x0006, PP_ARITHMETIC_OVERFLOW)
#define PP_ERROR_0x0005 PP_ERROR(0x0007, PP_DIVISION_BY_ZERO)


// PP_BOOL
#define PP_BOOL(x)		PP_BOOL_I(x)
#define PP_BOOL_I(x)	PP_BOOL_##x

#define PP_BOOL_0 0
#define PP_BOOL_1 1
#define PP_BOOL_2 1
#define PP_BOOL_3 1
#define PP_BOOL_4 1
#define PP_BOOL_5 1
#define PP_BOOL_6 1
#define PP_BOOL_7 1
#define PP_BOOL_8 1
#define PP_BOOL_9 1
#define PP_BOOL_10 1
#define PP_BOOL_11 1
#define PP_BOOL_12 1
#define PP_BOOL_13 1
#define PP_BOOL_14 1
#define PP_BOOL_15 1
#define PP_BOOL_16 1
#define PP_BOOL_17 1
#define PP_BOOL_18 1
#define PP_BOOL_19 1
#define PP_BOOL_20 1


// PP_BITNOT
#define PP_BITNOT(x)		PP_BITNOT_I(x)
#define PP_BITNOT_I(x)		PP_NOT_##x

#define PP_NOT_0 1
#define PP_NOT_1 0


// PP_BITAND
#define PP_BITAND(x, y)		PP_BITAND_I(x, y)
#define PP_BITAND_I(x, y)	PP_BITAND_##x##y

#define PP_BITAND_00 0
#define PP_BITAND_01 0
#define PP_BITAND_10 0
#define PP_BITAND_11 1


// PP_BITOR
#define PP_BITOR(x, y)		PP_BITOR_I(x, y)
#define PP_BITOR_I(x, y)	PP_BITOR_##x##y

#define PP_BITOR_00 0
#define PP_BITOR_01 1
#define PP_BITOR_10 1
#define PP_BITOR_11 1


// PP_BITXOR
#define PP_BITXOR(x, y)		PP_BITXOR_I(x, y)
#define PP_BITXOR_I(x, y)	PP_BITXOR_##x##y

#define PP_BITXOR_00 0
#define PP_BITXOR_01 1
#define PP_BITXOR_10 1
#define PP_BITXOR_11 0


// PP_NOT
#define PP_NOT(x)			PP_BITNOT(PP_BOOL(x))


// PP_AND
#define PP_AND(p, q)		PP_BITAND(PP_BOOL(p), PP_BOOL(q))


// PP_OR
#define PP_OR(p, q)			PP_BITOR(PP_BOOL(p), PP_BOOL(q))


// PP_XOR
#define PP_XOR(p, q)		PP_BITXOR(PP_BOOL(p), PP_BOOL(q))


// PP_IF
#define PP_IF(c, t, f)		PP_IF_I(PP_BOOL(c))(t, f)
#define PP_IF_I(b)			PP_IF_II(b)
#define PP_IF_II(b)			PP_IF_##b

#define PP_IF_0(t, f) f
#define PP_IF_1(t, f) t


// PP_COMMA_IF
#define PP_COMMA_IF(c)      PP_IF(c, PP_COMMA, PP_EMPTY)()


// PP_CALL
#define PP_CALL(f, arg)     f arg

// PP_VARIADIC_SIZE
#ifdef _WIN32
#define PP_VARIADIC_SIZE(...)   PP_CALL(PP_VARIADIC_SIZE_, (, __VA_ARGS__))
#else
#define PP_VARIADIC_SIZE(...)   PP_CALL(PP_VARIADIC_SIZE_, (, ##__VA_ARGS__))
#endif
//#define PP_VARIADIC_SIZE(...)   PP_VARIADIC_SIZE_I(_, ##__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define PP_VARIADIC_SIZE_(...)  PP_EXPAND(PP_VARIADIC_SIZE_I(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define PP_VARIADIC_SIZE_I(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N

// deprecated, please use PP_VARIADIC_SIZE instead
#define PP_NUM_PARAMS PP_VARIADIC_SIZE

// PP_ENUM
#define PP_ENUM(count, param) PP_REPEAT(count, PP_ENUM_M, param)
#define PP_ENUM_M(n, text) PP_COMMA_IF(n) text


// PP_ENUM_PARAMS
#define PP_ENUM_PARAMS(count, param) PP_REPEAT(count, PP_ENUM_PARAMS_M, param)
#define PP_ENUM_PARAMS_M(n, text) PP_COMMA_IF(n) text##n


// PP_ENUM_BINARY_PARAMS
#define PP_ENUM_BINARY_PARAMS(count, a, b) PP_BINARY_REPEAT(count, PP_ENUM_BINARY_PARAMS_M, a, b)
#define PP_ENUM_BINARY_PARAMS_M(n, a, b) PP_COMMA_IF(n) a##n b##n


// PP_ENUM_TRAILING
#define PP_ENUM_TRAILING(count, param) PP_REPEAT(count, PP_ENUM_TRAILING_M, param)
#define PP_ENUM_TRAILING_M(n, text) ,text


// PP_ENUM_TRAILING_PARAMS
#define PP_ENUM_TRAILING_PARAMS(count, param) PP_REPEAT(count, PP_ENUM_TRAILING_PARAMS_M, param)
#define PP_ENUM_TRAILING_PARAMS_M(n, text) ,text##n


// PP_ENUM_TRAILING_BINARY_PARAMS
#define PP_ENUM_TRAILING_BINARY_PARAMS(count, a, b) PP_BINARY_REPEAT(count, PP_ENUM_TRAILING_BINARY_PARAMS_M, a, b)
#define PP_ENUM_TRAILING_BINARY_PARAMS_M(n, a, b) , a##n b##n


// PP_REPEAT
#define PP_REPEAT(count, macro, data)		PP_REPEAT_I(count, macro, data)
#define PP_REPEAT_I(count, macro, data)		PP_REPEAT_##count(macro, data)

#define PP_REPEAT_0(macro, data)
#define PP_REPEAT_1(macro, data)	PP_REPEAT_0(macro, data)macro(0, data)
#define PP_REPEAT_2(macro, data)	PP_REPEAT_1(macro, data)macro(1, data)
#define PP_REPEAT_3(macro, data)	PP_REPEAT_2(macro, data)macro(2, data)
#define PP_REPEAT_4(macro, data)	PP_REPEAT_3(macro, data)macro(3, data)
#define PP_REPEAT_5(macro, data)	PP_REPEAT_4(macro, data)macro(4, data)
#define PP_REPEAT_6(macro, data)	PP_REPEAT_5(macro, data)macro(5, data)
#define PP_REPEAT_7(macro, data)	PP_REPEAT_6(macro, data)macro(6, data)
#define PP_REPEAT_8(macro, data)	PP_REPEAT_7(macro, data)macro(7, data)
#define PP_REPEAT_9(macro, data)	PP_REPEAT_8(macro, data)macro(8, data)
#define PP_REPEAT_10(macro, data)	PP_REPEAT_9(macro, data)macro(9, data)
#define PP_REPEAT_11(macro, data)	PP_REPEAT_10(macro, data)macro(10, data)
#define PP_REPEAT_12(macro, data)	PP_REPEAT_11(macro, data)macro(11, data)
#define PP_REPEAT_13(macro, data)	PP_REPEAT_12(macro, data)macro(12, data)
#define PP_REPEAT_14(macro, data)	PP_REPEAT_13(macro, data)macro(13, data)
#define PP_REPEAT_15(macro, data)	PP_REPEAT_14(macro, data)macro(14, data)
#define PP_REPEAT_16(macro, data)	PP_REPEAT_15(macro, data)macro(15, data)
#define PP_REPEAT_17(macro, data)	PP_REPEAT_16(macro, data)macro(16, data)
#define PP_REPEAT_18(macro, data)	PP_REPEAT_17(macro, data)macro(17, data)
#define PP_REPEAT_19(macro, data)	PP_REPEAT_18(macro, data)macro(18, data)
#define PP_REPEAT_20(macro, data)	PP_REPEAT_19(macro, data)macro(19, data)


// PP_BINARY_REPEAT
#define PP_BINARY_REPEAT(count, macro, data1, data2) PP_BINARY_REPEAT_I(count, macro, data1, data2)
//#define PP_BINARY_REPEAT_I(count, macro, data1, data2) PP_BINARY_REPEAT_##count(macro, data1, data2)
#define PP_BINARY_REPEAT_I(count, macro, data1, data2) PP_CAT(PP_BINARY_REPEAT_, count)(macro, data1, data2)

#define PP_BINARY_REPEAT_0(macro, data1, data2)
#define PP_BINARY_REPEAT_1(macro, data1, data2) PP_BINARY_REPEAT_0(macro, data1, data2)macro(0, data1, data2)
#define PP_BINARY_REPEAT_2(macro, data1, data2) PP_BINARY_REPEAT_1(macro, data1, data2)macro(1, data1, data2)
#define PP_BINARY_REPEAT_3(macro, data1, data2) PP_BINARY_REPEAT_2(macro, data1, data2)macro(2, data1, data2)
#define PP_BINARY_REPEAT_4(macro, data1, data2) PP_BINARY_REPEAT_3(macro, data1, data2)macro(3, data1, data2)
#define PP_BINARY_REPEAT_5(macro, data1, data2) PP_BINARY_REPEAT_4(macro, data1, data2)macro(4, data1, data2)
#define PP_BINARY_REPEAT_6(macro, data1, data2) PP_BINARY_REPEAT_5(macro, data1, data2)macro(5, data1, data2)
#define PP_BINARY_REPEAT_7(macro, data1, data2) PP_BINARY_REPEAT_6(macro, data1, data2)macro(6, data1, data2)
#define PP_BINARY_REPEAT_8(macro, data1, data2) PP_BINARY_REPEAT_7(macro, data1, data2)macro(7, data1, data2)
#define PP_BINARY_REPEAT_9(macro, data1, data2) PP_BINARY_REPEAT_8(macro, data1, data2)macro(8, data1, data2)
#define PP_BINARY_REPEAT_10(macro, data1, data2) PP_BINARY_REPEAT_9(macro, data1, data2)macro(9, data1, data2)
#define PP_BINARY_REPEAT_11(macro, data1, data2) PP_BINARY_REPEAT_10(macro, data1, data2)macro(10, data1, data2)
#define PP_BINARY_REPEAT_12(macro, data1, data2) PP_BINARY_REPEAT_11(macro, data1, data2)macro(11, data1, data2)
#define PP_BINARY_REPEAT_13(macro, data1, data2) PP_BINARY_REPEAT_12(macro, data1, data2)macro(12, data1, data2)
#define PP_BINARY_REPEAT_14(macro, data1, data2) PP_BINARY_REPEAT_13(macro, data1, data2)macro(13, data1, data2)
#define PP_BINARY_REPEAT_15(macro, data1, data2) PP_BINARY_REPEAT_14(macro, data1, data2)macro(14, data1, data2)
#define PP_BINARY_REPEAT_16(macro, data1, data2) PP_BINARY_REPEAT_15(macro, data1, data2)macro(15, data1, data2)
#define PP_BINARY_REPEAT_17(macro, data1, data2) PP_BINARY_REPEAT_16(macro, data1, data2)macro(16, data1, data2)
#define PP_BINARY_REPEAT_18(macro, data1, data2) PP_BINARY_REPEAT_17(macro, data1, data2)macro(17, data1, data2)
#define PP_BINARY_REPEAT_19(macro, data1, data2) PP_BINARY_REPEAT_18(macro, data1, data2)macro(18, data1, data2)
#define PP_BINARY_REPEAT_20(macro, data1, data2) PP_BINARY_REPEAT_19(macro, data1, data2)macro(19, data1, data2)


// PP_INC
#define PP_INC(x)		PP_INC_I(x)
#define PP_INC_I(x)		PP_INC_##x

#define PP_INC_0 1
#define PP_INC_1 2
#define PP_INC_2 3
#define PP_INC_3 4
#define PP_INC_4 5
#define PP_INC_5 6
#define PP_INC_6 7
#define PP_INC_7 8
#define PP_INC_8 9
#define PP_INC_9 10
#define PP_INC_10 11
#define PP_INC_11 12
#define PP_INC_12 13
#define PP_INC_13 14
#define PP_INC_14 15
#define PP_INC_15 16
#define PP_INC_16 17
#define PP_INC_17 18
#define PP_INC_18 19
#define PP_INC_19 20
#define PP_INC_20 PP_ERROR(0x0006)


// PP_DEC
#define PP_DEC(x) PP_DEC_I(x)
#define PP_DEC_I(x) PP_DEC_##x

#define PP_DEC_0 PP_ERROR(0x0006)
#define PP_DEC_1 0
#define PP_DEC_2 1
#define PP_DEC_3 2
#define PP_DEC_4 3
#define PP_DEC_5 4
#define PP_DEC_6 5
#define PP_DEC_7 6
#define PP_DEC_8 7
#define PP_DEC_9 8
#define PP_DEC_10 9
#define PP_DEC_11 10
#define PP_DEC_12 11
#define PP_DEC_13 12
#define PP_DEC_14 13
#define PP_DEC_15 14
#define PP_DEC_16 15
#define PP_DEC_17 16
#define PP_DEC_18 17
#define PP_DEC_19 18
#define PP_DEC_20 19

#if 1
// PP_WHILE
#define PP_WHILE(p, o, s)		PP_WHILE_0(p, o, s)

#define PP_WHILE__(p, o, s)		s
#define PP_WHILE_0(p, o, s)		PP_WHILE_N(1, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_1(p, o, s)		PP_WHILE_N(2, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_2(p, o, s)		PP_WHILE_N(3, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_3(p, o, s)		PP_WHILE_N(4, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_4(p, o, s)		PP_WHILE_N(5, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_5(p, o, s)		PP_WHILE_N(6, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_6(p, o, s)		PP_WHILE_N(7, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_7(p, o, s)		PP_WHILE_N(8, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_8(p, o, s)		PP_WHILE_N(9, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_9(p, o, s)		PP_WHILE_N(10, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_10(p, o, s)	PP_WHILE_N(11, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_11(p, o, s)	PP_WHILE_N(12, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_12(p, o, s)	PP_WHILE_N(13, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_13(p, o, s)	PP_WHILE_N(14, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_14(p, o, s)	PP_WHILE_N(15, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_15(p, o, s)	PP_WHILE_N(16, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_16(p, o, s)	PP_WHILE_N(17, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_17(p, o, s)	PP_WHILE_N(18, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_18(p, o, s)	PP_WHILE_N(19, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_19(p, o, s)	PP_WHILE_N(20, p, s)(p, o, PP_WHILE_S(p, o, s))
#define PP_WHILE_20(p, o, s)	PP_ERROR(0x0001)

#define PP_WHILE_N(n, p, s)		PP_CAT(PP_WHILE_, PP_IF(p(s), n, _))
#define PP_WHILE_S(p, o, s)		PP_IF(p(s), o(s), s)
#endif


// PP_TUPLE
#define PP_TUPLE(...)			(__VA_ARGS__)

// PP_TUPLE_SIZE
#define PP_TUPLE_SIZE(tuple)	PP_VARIADIC_SIZE tuple

// PP_TUPLE_EAT
#define PP_TUPLE_EAT(...)

// PP_TUPLE_REM
#define PP_TUPLE_REM(tuple)		PP_TUPLE_REM_I tuple
#define PP_TUPLE_REM_I(...)		__VA_ARGS__


// PP_TUPLE_AT
#if 0
#define PP_TUPLE_AT(i, tuple)	PP_TUPLE_AT_II(i, PP_TUPLE_REM_I tuple)
#define PP_TUPLE_AT_II(i, ...)	PP_TUPLE_AT_II_##i(, ##__VA_ARGS__, , , , , , , , , , , , , , , ,)
#else
#if 0
#define PP_TUPLE_AT(i, tuple)	PP_TUPLE_AT_I(i, tuple)
#define PP_TUPLE_AT_I(i, tuple)	PP_TUPLE_AT_I_##i tuple
#define PP_TUPLE_AT_I_0(...)	PP_TUPLE_AT_II_0(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_1(...)	PP_TUPLE_AT_II_1(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_2(...)	PP_TUPLE_AT_II_2(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_3(...)	PP_TUPLE_AT_II_3(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_4(...)	PP_TUPLE_AT_II_4(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_5(...)	PP_TUPLE_AT_II_5(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_6(...)	PP_TUPLE_AT_II_6(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_7(...)	PP_TUPLE_AT_II_7(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_8(...)	PP_TUPLE_AT_II_8(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_9(...)	PP_TUPLE_AT_II_9(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_10(...)	PP_TUPLE_AT_II_10(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_11(...)	PP_TUPLE_AT_II_11(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_12(...)	PP_TUPLE_AT_II_12(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_13(...)	PP_TUPLE_AT_II_13(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_14(...)	PP_TUPLE_AT_II_14(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_15(...)	PP_TUPLE_AT_II_15(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_16(...)	PP_TUPLE_AT_II_16(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_17(...)	PP_TUPLE_AT_II_17(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_18(...)	PP_TUPLE_AT_II_18(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)
#define PP_TUPLE_AT_I_19(...)	PP_TUPLE_AT_II_19(, ##__VA_ARGS__, , , , , , , , , , , , , , , , , , , ,)

#define PP_TUPLE_AT_II_0(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _0
#define PP_TUPLE_AT_II_1(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _1
#define PP_TUPLE_AT_II_2(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _2
#define PP_TUPLE_AT_II_3(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _3
#define PP_TUPLE_AT_II_4(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _4
#define PP_TUPLE_AT_II_5(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _5
#define PP_TUPLE_AT_II_6(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _6
#define PP_TUPLE_AT_II_7(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _7
#define PP_TUPLE_AT_II_8(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _8
#define PP_TUPLE_AT_II_9(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _9
#define PP_TUPLE_AT_II_10(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _10
#define PP_TUPLE_AT_II_11(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _11
#define PP_TUPLE_AT_II_12(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _12
#define PP_TUPLE_AT_II_13(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _13
#define PP_TUPLE_AT_II_14(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _14
#define PP_TUPLE_AT_II_15(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _15
#define PP_TUPLE_AT_II_16(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _16
#define PP_TUPLE_AT_II_17(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _17
#define PP_TUPLE_AT_II_18(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _18
#define PP_TUPLE_AT_II_19(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _19
#else
#define PP_TUPLE_AT(i, tuple)       PP_VARIDIC_AT(i, PP_TUPLE_REM(tuple))

#define PP_VARIDIC_AT(i, ...)       PP_VARIDIC_AT_I(i, __VA_ARGS__)
#define PP_VARIDIC_AT_I(i, ...)     PP_EXPAND(PP_CAT(PP_VARIDIC_AT_I_, i)(__VA_ARGS__,))

#define PP_VARIDIC_AT_I_0(_0, ...) _0
#define PP_VARIDIC_AT_I_1(_0, _1, ...) _1
#define PP_VARIDIC_AT_I_2(_0, _1, _2, ...) _2
#define PP_VARIDIC_AT_I_3(_0, _1, _2, _3, ...) _3
#define PP_VARIDIC_AT_I_4(_0, _1, _2, _3, _4, ...) _4
#define PP_VARIDIC_AT_I_5(_0, _1, _2, _3, _4, _5, ...) _5
#define PP_VARIDIC_AT_I_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define PP_VARIDIC_AT_I_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
#define PP_VARIDIC_AT_I_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define PP_VARIDIC_AT_I_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define PP_VARIDIC_AT_I_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10
#define PP_VARIDIC_AT_I_11(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) _11
#define PP_VARIDIC_AT_I_12(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...) _12
#define PP_VARIDIC_AT_I_13(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...) _13
#define PP_VARIDIC_AT_I_14(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...) _14
#define PP_VARIDIC_AT_I_15(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define PP_VARIDIC_AT_I_16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...) _16
#define PP_VARIDIC_AT_I_17(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, ...) _17
#define PP_VARIDIC_AT_I_18(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, ...) _18
#define PP_VARIDIC_AT_I_19(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _19
#endif
#endif

// PP_TUPLE_REPEAT
#define PP_TUPLE_REPEAT(macro, tuple)				PP_TUPLE_REPEAT_I(macro, PP_TUPLE_SIZE(tuple), tuple)
#define PP_TUPLE_REPEAT_I(macro, count, tuple)		PP_REPEAT(count, macro, tuple)

// PP_TUPLE_FOREACH
#if 0
#define PP_TUPLE_FOREACH(macro, tuple)              PP_TUPLE_FOREACH_I(macro, tuple)
#define PP_TUPLE_FOREACH_I(macro, tuple)            PP_REPEAT(PP_TUPLE_SIZE(tuple), PP_TUPLE_FOREACH_M, (macro, tuple))
#define PP_TUPLE_FOREACH_M(i, tuple)                PP_TUPLE_FOREACH_M_I(i, PP_TUPLE_REM(tuple))
#define PP_TUPLE_FOREACH_M_I(i, ...)                PP_TUPLE_FOREACH_M_II(i, __VA_ARGS__)
#define PP_TUPLE_FOREACH_M_II(i, macro, tuple)      macro(i, PP_TUPLE_AT(i, tuple))
#else
#define PP_TUPLE_FOREACH(macro, tuple)              PP_TUPLE_FOREACH_I(macro, tuple)
#define PP_TUPLE_FOREACH_I(macro, tuple)            PP_BINARY_REPEAT(PP_TUPLE_SIZE(tuple), PP_TUPLE_FOREACH_M, macro, tuple)
#define PP_TUPLE_FOREACH_M(i, macro, tuple)         macro(i, PP_TUPLE_AT(i, tuple))
#endif

// PP_FOREACH
#define PP_FOREACH(macro, ...)                      PP_TUPLE_FOREACH(macro, PP_TUPLE(__VA_ARGS__))


// PP_ADD
#define PP_ADD(x, y)			PP_TUPLE_AT(0, PP_WHILE(PP_ADD_P, PP_ADD_O, (x, y)))
#define PP_ADD_P(xy)			PP_TUPLE_AT(1, xy)
#define PP_ADD_O(xy)			PP_ADD_O_I xy
#define PP_ADD_O_I(x, y)		(PP_INC(x), PP_DEC(y))

#define PP_SUB_I(x, y)			PP_WHILE(PP_SUB_I_P, PP_SUB_I_O, (x, y))
#define PP_SUB_I_P(xy)			PP_SUB_I_P_I xy
#define PP_SUB_I_P_I(x, y)		PP_AND(x, y)
#define PP_SUB_I_O(xy)			PP_SUB_I_O_I xy
#define PP_SUB_I_O_I(x, y)		(PP_DEC(x), PP_DEC(y))

// PP_SUB
#define PP_SUB(x, y)			PP_TUPLE_AT(0, PP_SUB_I(x, y))

// PP_EQUAL
#define PP_EQUAL(x, y)			PP_AND(\
									PP_NOT(PP_TUPLE_AT(0, PP_SUB_I(x, y))),\
									PP_NOT(PP_TUPLE_AT(1, PP_SUB_I(x, y)))\
								)

// PP_NOT_EQUAL
#define PP_NOT_EQUAL(x, y)		PP_NOT(PP_EQUAL(x, y))

// PP_LESS
#define PP_LESS(x, y)			PP_AND(\
									PP_NOT(PP_TUPLE_AT(0, PP_SUB_I(x, y))),\
									PP_TUPLE_AT(1, PP_SUB_I(x, y))\
								)

// PP_GREATER
#define PP_GREATER(x, y)		PP_BOOL(PP_SUB(x, y))

// PP_LESS_EQUAL
#define PP_LESS_EQUAL(x, y)		PP_NOT(PP_GREATER(x, y))

// PP_GREATER_EQUAL
#define PP_GREATER_EQUAL(x, y)	PP_NOT(PP_LESS(x, y))

// PP_MIN
#define PP_MIN(x, y)			PP_IF(PP_GREATER(x, y), y, x)

// PP_MAX
#define PP_MAX(x, y)			PP_IF(PP_GREATER(x, y), x, y)


//PP_RANGE
#define PP_RANGE(begin, end)	PP_RANGE_I(begin, end)
#define PP_RANGE_I(begin, end)	PP_REPEAT(PP_SUB(end, begin), PP_RANGE_M, begin)
#define PP_RANGE_M(i, n)		PP_COMMA_IF(i) PP_ADD(i, n)

//PP_LINE_VAR
#define PP_LINE_VAR(x)			PP_CAT(x, __LINE__)

#endif /* PREPROCESSOR_H_ */
