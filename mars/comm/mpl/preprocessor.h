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

#ifndef _MPL_PREPROCESSOR_H_
#define _MPL_PREPROCESSOR_H_

#define _MPL_PP_EMPTY()
#define _MPL_PP_COMMA() ,

// PP_CAT
#define _MPL_PP_CAT(x, y)	_MPL_PP_CAT_I(x, y)
#define _MPL_PP_CAT_I(x, y)	x##y

// PP_EXPAND
// FUCK: MSVC doesn't expand __VA_ARGS__ correctly
#define _MPL_PP_EXPAND(...) __VA_ARGS__

// PP_S
// FUCK: PP_S() should expanded to "", but expanded to nothing in MSVC
#define _MPL_PP_S(...)       _MPL_PP_S_I(__VA_ARGS__)
#define _MPL_PP_S_I(...)     _MPL_PP_IF(_MPL_PP_VARIADIC_SIZE(__VA_ARGS__), _MPL_PP_S_II(__VA_ARGS__), "")
#define _MPL_PP_S_II(...)    #__VA_ARGS__

// PP_ERROR
#define _MPL_PP_ERROR(code)  _MPL_PP_CAT(PP_ERROR_, code)
#define _MPL_PP_ERROR_0x0000 _MPL_PP_ERROR(0x0000, PP_INDEX_OUT_OF_BOUNDS)
#define _MPL_PP_ERROR_0x0001 _MPL_PP_ERROR(0x0001, PP_PP_WHILE_OVERFLOW)
#define _MPL_PP_ERROR_0x0002 _MPL_PP_ERROR(0x0002, PP_FOR_OVERFLOW)
#define _MPL_PP_ERROR_0x0003 _MPL_PP_ERROR(0x0003, PP_REPEAT_OVERFLOW)
#define _MPL_PP_ERROR_0x0004 _MPL_PP_ERROR(0x0006, PP_ARITHMETIC_OVERFLOW)
#define _MPL_PP_ERROR_0x0005 _MPL_PP_ERROR(0x0007, PP_DIVISION_BY_ZERO)


// PP_BOOL
#define _MPL_PP_BOOL(x)		_MPL_PP_BOOL_I(x)
#define _MPL_PP_BOOL_I(x)	_MPL_PP_BOOL_##x

#define _MPL_PP_BOOL_0 0
#define _MPL_PP_BOOL_1 1
#define _MPL_PP_BOOL_2 1
#define _MPL_PP_BOOL_3 1
#define _MPL_PP_BOOL_4 1
#define _MPL_PP_BOOL_5 1
#define _MPL_PP_BOOL_6 1
#define _MPL_PP_BOOL_7 1
#define _MPL_PP_BOOL_8 1
#define _MPL_PP_BOOL_9 1
#define _MPL_PP_BOOL_10 1
#define _MPL_PP_BOOL_11 1
#define _MPL_PP_BOOL_12 1
#define _MPL_PP_BOOL_13 1
#define _MPL_PP_BOOL_14 1
#define _MPL_PP_BOOL_15 1
#define _MPL_PP_BOOL_16 1
#define _MPL_PP_BOOL_17 1
#define _MPL_PP_BOOL_18 1
#define _MPL_PP_BOOL_19 1
#define _MPL_PP_BOOL_20 1


// PP_BITNOT
#define _MPL__MPL_PP_BITNOT(x)		_MPL_PP_BITNOT_I(x)
#define _MPL_PP_BITNOT_I(x)		_MPL_PP_NOT_##x

#define _MPL_PP_NOT_0 1
#define _MPL_PP_NOT_1 0


// PP_BITAND
#define _MPL_PP_BITAND(x, y)		_MPL_PP_BITAND_I(x, y)
#define _MPL_PP_BITAND_I(x, y)	_MPL_PP_BITAND_##x##y

#define _MPL_PP_BITAND_00 0
#define _MPL_PP_BITAND_01 0
#define _MPL_PP_BITAND_10 0
#define _MPL_PP_BITAND_11 1


// PP_BITOR
#define _MPL_PP_BITOR(x, y)		_MPL_PP_BITOR_I(x, y)
#define _MPL_PP_BITOR_I(x, y)	_MPL_PP_BITOR_##x##y

#define _MPL_PP_BITOR_00 0
#define _MPL_PP_BITOR_01 1
#define _MPL_PP_BITOR_10 1
#define _MPL_PP_BITOR_11 1


// PP_BITXOR
#define _MPL_PP_BITXOR(x, y)		_MPL_PP_BITXOR_I(x, y)
#define _MPL_PP_BITXOR_I(x, y)	_MPL_PP_BITXOR_##x##y

#define _MPL_PP_BITXOR_00 0
#define _MPL_PP_BITXOR_01 1
#define _MPL_PP_BITXOR_10 1
#define _MPL_PP_BITXOR_11 0


// PP_NOT
#define _MPL_PP_NOT(x)			_MPL_PP_BITNOT(PP_BOOL(x))


// PP_AND
#define _MPL_PP_AND(p, q)		_MPL_PP_BITAND(PP_BOOL(p), PP_BOOL(q))


// PP_OR
#define _MPL_PP_OR(p, q)			_MPL_PP_BITOR(PP_BOOL(p), PP_BOOL(q))


// PP_XOR
#define _MPL_PP_XOR(p, q)		_MPL_PP_BITXOR(PP_BOOL(p), PP_BOOL(q))


// PP_IF
#define _MPL_PP_IF(c, t, f)		_MPL_PP_IF_I(PP_BOOL(c))(t, f)
#define _MPL_PP_IF_I(b)			_MPL_PP_IF_II(b)
#define _MPL_PP_IF_II(b)		_MPL_PP_IF_##b

#define _MPL_PP_IF_0(t, f) f
#define _MPL_PP_IF_1(t, f) t


// PP_COMMA_IF
#define _MPL_PP_COMMA_IF(c)      _MPL_PP_IF(c, PP_COMMA, PP_EMPTY)()


// PP_CALL
#define _MPL_PP_CALL(f, arg)     f arg

// PP_VARIADIC_SIZE
#ifdef _WIN32
#define PP_VARIADIC_SIZE(...)   PP_CALL(PP_VARIADIC_SIZE_, (, __VA_ARGS__))
#else
#define _MPL_PP_VARIADIC_SIZE(...)   _MPL_PP_CALL(PP_VARIADIC_SIZE_, (, ##__VA_ARGS__))
#endif
//#define PP_VARIADIC_SIZE(...)   PP_VARIADIC_SIZE_I(_, ##__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define _MPL_PP_VARIADIC_SIZE_(...)  _MPL_PP_EXPAND(PP_VARIADIC_SIZE_I(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define _MPL_PP_VARIADIC_SIZE_I(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N

// deprecated, please use PP_VARIADIC_SIZE instead
#define _MPL_PP_NUM_PARAMS PP_VARIADIC_SIZE

// PP_ENUM
#define _MPL_PP_ENUM(count, param) _MPL_PP_REPEAT(count, PP_ENUM_M, param)
#define _MPL_PP_ENUM_M(n, text) _MPL_PP_COMMA_IF(n) text


// PP_ENUM_PARAMS
#define _MPL_PP_ENUM_PARAMS(count, param) _MPL_PP_REPEAT(count, PP_ENUM_PARAMS_M, param)
#define _MPL_PP_ENUM_PARAMS_M(n, text) _MPL_PP_COMMA_IF(n) text##n


// PP_ENUM_BINARY_PARAMS
#define _MPL_PP_ENUM_BINARY_PARAMS(count, a, b) _MPL_PP_BINARY_REPEAT(count, PP_ENUM_BINARY_PARAMS_M, a, b)
#define _MPL_PP_ENUM_BINARY_PARAMS_M(n, a, b) _MPL_PP_COMMA_IF(n) a##n b##n


// PP_ENUM_TRAILING
#define _MPL_PP_ENUM_TRAILING(count, param) _MPL_PP_REPEAT(count, PP_ENUM_TRAILING_M, param)
#define _MPL_PP_ENUM_TRAILING_M(n, text) ,text


// PP_ENUM_TRAILING_PARAMS
#define _MPL_PP_ENUM_TRAILING_PARAMS(count, param) _MPL_PP_REPEAT(count, PP_ENUM_TRAILING_PARAMS_M, param)
#define _MPL_PP_ENUM_TRAILING_PARAMS_M(n, text) ,text##n


// PP_ENUM_TRAILING_BINARY_PARAMS
#define _MPL_PP_ENUM_TRAILING_BINARY_PARAMS(count, a, b) _MPL_PP_BINARY_REPEAT(count, PP_ENUM_TRAILING_BINARY_PARAMS_M, a, b)
#define _MPL_PP_ENUM_TRAILING_BINARY_PARAMS_M(n, a, b) , a##n b##n


// PP_REPEAT
#define _MPL_PP_REPEAT(count, macro, data)		_MPL_PP_REPEAT_I(count, macro, data)
#define _MPL_PP_REPEAT_I(count, macro, data)		_MPL_PP_REPEAT_##count(macro, data)

#define _MPL_PP_REPEAT_0(macro, data)
#define _MPL_PP_REPEAT_1(macro, data)	_MPL_PP_REPEAT_0(macro, data)macro(0, data)
#define _MPL_PP_REPEAT_2(macro, data)	_MPL_PP_REPEAT_1(macro, data)macro(1, data)
#define _MPL_PP_REPEAT_3(macro, data)	_MPL_PP_REPEAT_2(macro, data)macro(2, data)
#define _MPL_PP_REPEAT_4(macro, data)	_MPL_PP_REPEAT_3(macro, data)macro(3, data)
#define _MPL_PP_REPEAT_5(macro, data)	_MPL_PP_REPEAT_4(macro, data)macro(4, data)
#define _MPL_PP_REPEAT_6(macro, data)	_MPL_PP_REPEAT_5(macro, data)macro(5, data)
#define _MPL_PP_REPEAT_7(macro, data)	_MPL_PP_REPEAT_6(macro, data)macro(6, data)
#define _MPL_PP_REPEAT_8(macro, data)	_MPL_PP_REPEAT_7(macro, data)macro(7, data)
#define _MPL_PP_REPEAT_9(macro, data)	_MPL_PP_REPEAT_8(macro, data)macro(8, data)
#define _MPL_PP_REPEAT_10(macro, data)	_MPL_PP_REPEAT_9(macro, data)macro(9, data)
#define _MPL_PP_REPEAT_11(macro, data)	_MPL_PP_REPEAT_10(macro, data)macro(10, data)
#define _MPL_PP_REPEAT_12(macro, data)	_MPL_PP_REPEAT_11(macro, data)macro(11, data)
#define _MPL_PP_REPEAT_13(macro, data)	_MPL_PP_REPEAT_12(macro, data)macro(12, data)
#define _MPL_PP_REPEAT_14(macro, data)	_MPL_PP_REPEAT_13(macro, data)macro(13, data)
#define _MPL_PP_REPEAT_15(macro, data)	_MPL_PP_REPEAT_14(macro, data)macro(14, data)
#define _MPL_PP_REPEAT_16(macro, data)	_MPL_PP_REPEAT_15(macro, data)macro(15, data)
#define _MPL_PP_REPEAT_17(macro, data)	_MPL_PP_REPEAT_16(macro, data)macro(16, data)
#define _MPL_PP_REPEAT_18(macro, data)	_MPL_PP_REPEAT_17(macro, data)macro(17, data)
#define _MPL_PP_REPEAT_19(macro, data)	_MPL_PP_REPEAT_18(macro, data)macro(18, data)
#define _MPL_PP_REPEAT_20(macro, data)	_MPL_PP_REPEAT_19(macro, data)macro(19, data)


// PP_BINARY_REPEAT
#define _MPL_PP_BINARY_REPEAT(count, macro, data1, data2) _MPL_PP_BINARY_REPEAT_I(count, macro, data1, data2)
//#define PP_BINARY_REPEAT_I(count, macro, data1, data2) PP_BINARY_REPEAT_##count(macro, data1, data2)
#define _MPL_PP_BINARY_REPEAT_I(count, macro, data1, data2) _MPL_PP_CAT(PP_BINARY_REPEAT_, count)(macro, data1, data2)

#define _MPL_PP_BINARY_REPEAT_0(macro, data1, data2)
#define _MPL_PP_BINARY_REPEAT_1(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_0(macro, data1, data2)macro(0, data1, data2)
#define _MPL_PP_BINARY_REPEAT_2(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_1(macro, data1, data2)macro(1, data1, data2)
#define _MPL_PP_BINARY_REPEAT_3(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_2(macro, data1, data2)macro(2, data1, data2)
#define _MPL_PP_BINARY_REPEAT_4(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_3(macro, data1, data2)macro(3, data1, data2)
#define _MPL_PP_BINARY_REPEAT_5(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_4(macro, data1, data2)macro(4, data1, data2)
#define _MPL_PP_BINARY_REPEAT_6(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_5(macro, data1, data2)macro(5, data1, data2)
#define _MPL_PP_BINARY_REPEAT_7(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_6(macro, data1, data2)macro(6, data1, data2)
#define _MPL_PP_BINARY_REPEAT_8(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_7(macro, data1, data2)macro(7, data1, data2)
#define _MPL_PP_BINARY_REPEAT_9(macro, data1, data2)  _MPL_PP_BINARY_REPEAT_8(macro, data1, data2)macro(8, data1, data2)
#define _MPL_PP_BINARY_REPEAT_10(macro, data1, data2) _MPL_PP_BINARY_REPEAT_9(macro, data1, data2)macro(9, data1, data2)
#define _MPL_PP_BINARY_REPEAT_11(macro, data1, data2) _MPL_PP_BINARY_REPEAT_10(macro, data1, data2)macro(10, data1, data2)
#define _MPL_PP_BINARY_REPEAT_12(macro, data1, data2) _MPL_PP_BINARY_REPEAT_11(macro, data1, data2)macro(11, data1, data2)
#define _MPL_PP_BINARY_REPEAT_13(macro, data1, data2) _MPL_PP_BINARY_REPEAT_12(macro, data1, data2)macro(12, data1, data2)
#define _MPL_PP_BINARY_REPEAT_14(macro, data1, data2) _MPL_PP_BINARY_REPEAT_13(macro, data1, data2)macro(13, data1, data2)
#define _MPL_PP_BINARY_REPEAT_15(macro, data1, data2) _MPL_PP_BINARY_REPEAT_14(macro, data1, data2)macro(14, data1, data2)
#define _MPL_PP_BINARY_REPEAT_16(macro, data1, data2) _MPL_PP_BINARY_REPEAT_15(macro, data1, data2)macro(15, data1, data2)
#define _MPL_PP_BINARY_REPEAT_17(macro, data1, data2) _MPL_PP_BINARY_REPEAT_16(macro, data1, data2)macro(16, data1, data2)
#define _MPL_PP_BINARY_REPEAT_18(macro, data1, data2) _MPL_PP_BINARY_REPEAT_17(macro, data1, data2)macro(17, data1, data2)
#define _MPL_PP_BINARY_REPEAT_19(macro, data1, data2) _MPL_PP_BINARY_REPEAT_18(macro, data1, data2)macro(18, data1, data2)
#define _MPL_PP_BINARY_REPEAT_20(macro, data1, data2) _MPL_PP_BINARY_REPEAT_19(macro, data1, data2)macro(19, data1, data2)


// PP_INC
#define _MPL_PP_INC(x)		_MPL_PP_INC_I(x)
#define _MPL_PP_INC_I(x)    _MPL_PP_INC_##x

#define _MPL_PP_INC_0 1
#define _MPL_PP_INC_1 2
#define _MPL_PP_INC_2 3
#define _MPL_PP_INC_3 4
#define _MPL_PP_INC_4 5
#define _MPL_PP_INC_5 6
#define _MPL_PP_INC_6 7
#define _MPL_PP_INC_7 8
#define _MPL_PP_INC_8 9
#define _MPL_PP_INC_9 10
#define _MPL_PP_INC_10 11
#define _MPL_PP_INC_11 12
#define _MPL_PP_INC_12 13
#define _MPL_PP_INC_13 14
#define _MPL_PP_INC_14 15
#define _MPL_PP_INC_15 16
#define _MPL_PP_INC_16 17
#define _MPL_PP_INC_17 18
#define _MPL_PP_INC_18 19
#define _MPL_PP_INC_19 20
#define _MPL_PP_INC_20 PP_ERROR(0x0006)


// PP_DEC
#define _MPL_PP_DEC(x) _MPL_PP_DEC_I(x)
#define _MPL_PP_DEC_I(x) _MPL_PP_DEC_##x

#define _MPL_PP_DEC_0 PP_ERROR(0x0006)
#define _MPL_PP_DEC_1 0
#define _MPL_PP_DEC_2 1
#define _MPL_PP_DEC_3 2
#define _MPL_PP_DEC_4 3
#define _MPL_PP_DEC_5 4
#define _MPL_PP_DEC_6 5
#define _MPL_PP_DEC_7 6
#define _MPL_PP_DEC_8 7
#define _MPL_PP_DEC_9 8
#define _MPL_PP_DEC_10 9
#define _MPL_PP_DEC_11 10
#define _MPL_PP_DEC_12 11
#define _MPL_PP_DEC_13 12
#define _MPL_PP_DEC_14 13
#define _MPL_PP_DEC_15 14
#define _MPL_PP_DEC_16 15
#define _MPL_PP_DEC_17 16
#define _MPL_PP_DEC_18 17
#define _MPL_PP_DEC_19 18
#define _MPL_PP_DEC_20 19

#if 1
// PP_WHILE
#define _MPL_PP_WHILE(p, o, s)		    _MPL_PP_WHILE_0(p, o, s)

#define _MPL_PP_WHILE__(p, o, s)		s
#define _MPL_PP_WHILE_0(p, o, s)		_MPL_PP_WHILE_N(1, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_1(p, o, s)		_MPL_PP_WHILE_N(2, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_2(p, o, s)		_MPL_PP_WHILE_N(3, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_3(p, o, s)		_MPL_PP_WHILE_N(4, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_4(p, o, s)		_MPL_PP_WHILE_N(5, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_5(p, o, s)		_MPL_PP_WHILE_N(6, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_6(p, o, s)		_MPL_PP_WHILE_N(7, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_7(p, o, s)		_MPL_PP_WHILE_N(8, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_8(p, o, s)		_MPL_PP_WHILE_N(9, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_9(p, o, s)		_MPL_PP_WHILE_N(10, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_10(p, o, s)	_MPL_PP_WHILE_N(11, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_11(p, o, s)	_MPL_PP_WHILE_N(12, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_12(p, o, s)	_MPL_PP_WHILE_N(13, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_13(p, o, s)	_MPL_PP_WHILE_N(14, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_14(p, o, s)	_MPL_PP_WHILE_N(15, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_15(p, o, s)	_MPL_PP_WHILE_N(16, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_16(p, o, s)	_MPL_PP_WHILE_N(17, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_17(p, o, s)	_MPL_PP_WHILE_N(18, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_18(p, o, s)	_MPL_PP_WHILE_N(19, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_19(p, o, s)	_MPL_PP_WHILE_N(20, p, s)(p, o, _MPL_PP_WHILE_S(p, o, s))
#define _MPL_PP_WHILE_20(p, o, s)	_MPL_PP_ERROR(0x0001)

#define _MPL_PP_WHILE_N(n, p, s)		_MPL_PP_CAT(PP_WHILE_, PP_IF(p(s), n, _))
#define _MPL_PP_WHILE_S(p, o, s)		_MPL_PP_IF(p(s), o(s), s)
#endif


// PP_TUPLE
#define _MPL_PP_TUPLE(...)			(__VA_ARGS__)

// PP_TUPLE_SIZE
#define _MPL_PP_TUPLE_SIZE(tuple)	_MPL_PP_VARIADIC_SIZE tuple

// PP_TUPLE_EAT
#define _MPL_PP_TUPLE_EAT(...)

// PP_TUPLE_REM
#define _MPL_PP_TUPLE_REM(tuple)		_MPL_PP_TUPLE_REM_I tuple
#define _MPL_PP_TUPLE_REM_I(...)		__VA_ARGS__


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
#define _MPL_PP_TUPLE_AT(i, tuple)       _MPL_PP_VARIDIC_AT(i, _MPL_PP_TUPLE_REM(tuple))

#define _MPL_PP_VARIDIC_AT(i, ...)       _MPL_PP_VARIDIC_AT_I(i, __VA_ARGS__)
#define _MPL_PP_VARIDIC_AT_I(i, ...)     _MPL_PP_EXPAND(_MPL_PP_CAT(PP_VARIDIC_AT_I_, i)(__VA_ARGS__,))

#define _MPL_PP_VARIDIC_AT_I_0(_0, ...) _0
#define _MPL_PP_VARIDIC_AT_I_1(_0, _1, ...) _1
#define _MPL_PP_VARIDIC_AT_I_2(_0, _1, _2, ...) _2
#define _MPL_PP_VARIDIC_AT_I_3(_0, _1, _2, _3, ...) _3
#define _MPL_PP_VARIDIC_AT_I_4(_0, _1, _2, _3, _4, ...) _4
#define _MPL_PP_VARIDIC_AT_I_5(_0, _1, _2, _3, _4, _5, ...) _5
#define _MPL_PP_VARIDIC_AT_I_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define _MPL_PP_VARIDIC_AT_I_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
#define _MPL_PP_VARIDIC_AT_I_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define _MPL_PP_VARIDIC_AT_I_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define _MPL_PP_VARIDIC_AT_I_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10
#define _MPL_PP_VARIDIC_AT_I_11(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) _11
#define _MPL_PP_VARIDIC_AT_I_12(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...) _12
#define _MPL_PP_VARIDIC_AT_I_13(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...) _13
#define _MPL_PP_VARIDIC_AT_I_14(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...) _14
#define _MPL_PP_VARIDIC_AT_I_15(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define _MPL_PP_VARIDIC_AT_I_16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...) _16
#define _MPL_PP_VARIDIC_AT_I_17(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, ...) _17
#define _MPL_PP_VARIDIC_AT_I_18(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, ...) _18
#define _MPL_PP_VARIDIC_AT_I_19(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _19
#endif
#endif

// PP_TUPLE_REPEAT
#define _MPL_PP_TUPLE_REPEAT(macro, tuple)				_MPL_PP_TUPLE_REPEAT_I(macro, _MPL_PP_TUPLE_SIZE(tuple), tuple)
#define _MPL_PP_TUPLE_REPEAT_I(macro, count, tuple)		_MPL_PP_REPEAT(count, macro, tuple)

// PP_TUPLE_FOREACH
#if 0
#define PP_TUPLE_FOREACH(macro, tuple)              PP_TUPLE_FOREACH_I(macro, tuple)
#define PP_TUPLE_FOREACH_I(macro, tuple)            PP_REPEAT(PP_TUPLE_SIZE(tuple), PP_TUPLE_FOREACH_M, (macro, tuple))
#define PP_TUPLE_FOREACH_M(i, tuple)                PP_TUPLE_FOREACH_M_I(i, PP_TUPLE_REM(tuple))
#define PP_TUPLE_FOREACH_M_I(i, ...)                PP_TUPLE_FOREACH_M_II(i, __VA_ARGS__)
#define PP_TUPLE_FOREACH_M_II(i, macro, tuple)      macro(i, PP_TUPLE_AT(i, tuple))
#else
#define _MPL_PP_TUPLE_FOREACH(macro, tuple)              _MPL_PP_TUPLE_FOREACH_I(macro, tuple)
#define _MPL_PP_TUPLE_FOREACH_I(macro, tuple)            _MPL_PP_BINARY_REPEAT(_MPL_PP_TUPLE_SIZE(tuple), PP_TUPLE_FOREACH_M, macro, tuple)
#define _MPL_PP_TUPLE_FOREACH_M(i, macro, tuple)         macro(i, _MPL_PP_TUPLE_AT(i, tuple))
#endif

// PP_FOREACH
#define _MPL_PP_FOREACH(macro, ...)                      _MPL_PP_TUPLE_FOREACH(macro, _MPL_PP_TUPLE(__VA_ARGS__))


// PP_ADD
#define _MPL_PP_ADD(x, y)			_MPL_PP_TUPLE_AT(0, _MPL_PP_WHILE(PP_ADD_P, PP_ADD_O, (x, y)))
#define _MPL_PP_ADD_P(xy)			_MPL_PP_TUPLE_AT(1, xy)
#define _MPL_PP_ADD_O(xy)			_MPL_PP_ADD_O_I xy
#define _MPL_PP_ADD_O_I(x, y)		(_MPL_PP_INC(x), _MPL_PP_DEC(y))

#define _MPL_PP_SUB_I(x, y)			_MPL_PP_WHILE(PP_SUB_I_P, PP_SUB_I_O, (x, y))
#define _MPL_PP_SUB_I_P(xy)			_MPL_PP_SUB_I_P_I xy
#define _MPL_PP_SUB_I_P_I(x, y)		_MPL_PP_AND(x, y)
#define _MPL_PP_SUB_I_O(xy)			_MPL_PP_SUB_I_O_I xy
#define _MPL_PP_SUB_I_O_I(x, y)		(_MPL_PP_DEC(x), _MPL_PP_DEC(y))

// PP_SUB
#define _MPL_PP_SUB(x, y)			_MPL_PP_TUPLE_AT(0, _MPL_PP_SUB_I(x, y))

// PP_EQUAL
#define _MPL_PP_EQUAL(x, y)			_MPL_PP_AND(\
									_MPL_PP_NOT(_MPL_PP_TUPLE_AT(0, _MPL_PP_SUB_I(x, y))),\
									_MPL_PP_NOT(_MPL_PP_TUPLE_AT(1, _MPL_PP_SUB_I(x, y)))\
								)

// PP_NOT_EQUAL
#define _MPL_PP_NOT_EQUAL(x, y)		_MPL_PP_NOT(_MPL_PP_EQUAL(x, y))

// PP_LESS
#define _MPL_PP_LESS(x, y)			_MPL_PP_AND(\
									_MPL_PP_NOT(_MPL_PP_TUPLE_AT(0, _MPL_PP_SUB_I(x, y))),\
									_MPL_PP_TUPLE_AT(1, _MPL_PP_SUB_I(x, y))\
								)

// PP_GREATER
#define _MPL_PP_GREATER(x, y)		_MPL_PP_BOOL(_MPL_PP_SUB(x, y))

// PP_LESS_EQUAL
#define _MPL_PP_LESS_EQUAL(x, y)		_MPL_PP_NOT(_MPL_PP_GREATER(x, y))

// PP_GREATER_EQUAL
#define _MPL_PP_GREATER_EQUAL(x, y)	_MPL_PP_NOT(_MPL_PP_LESS(x, y))

// PP_MIN
#define _MPL_PP_MIN(x, y)			_MPL_PP_IF(_MPL_PP_GREATER(x, y), y, x)

// PP_MAX
#define _MPL_PP_MAX(x, y)			_MPL_PP_IF(_MPL_PP_GREATER(x, y), x, y)


//PP_RANGE
#define _MPL_PP_RANGE(begin, end)	_MPL_PP_RANGE_I(begin, end)
#define _MPL_PP_RANGE_I(begin, end)	_MPL_PP_REPEAT(PP_SUB(end, begin), PP_RANGE_M, begin)
#define _MPL_PP_RANGE_M(i, n)		_MPL_PP_COMMA_IF(i) PP_ADD(i, n)

//PP_LINE_VAR
#define _MPL_PP_LINE_VAR(x)			_MPL_PP_CAT(x, __LINE__)

#endif /* _MPL_PREPROCESSOR_H_ */
