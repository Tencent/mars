// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 ============================================================================
 Name        : preprocessor.h
 Author        : 范亮亮
 Version    : 1.0
 Created on    : 2010-4-18
 Copyright    : Copyright (C) 1998 - 2009 TENCENT Inc. All Right Reserved
 Description:
 ============================================================================
 */

#ifndef PREPROCESSOR_H_
#define PREPROCESSOR_H_


#define PP_EMPTY()
#define PP_COMMA() ,


// PP_CAT
#define PP_CAT(x, y) PP_CAT_I(x, y)
#define PP_CAT_I(x, y) PP_CAT_II(x, y)
#define PP_CAT_II(x, y) x##y

// PP_ERROR
#define PP_ERROR(code) PP_CAT(PP_ERROR_, code)
#define PP_ERROR_0x0000 PP_ERROR(0x0000, PP_INDEX_OUT_OF_BOUNDS)
#define PP_ERROR_0x0001 PP_ERROR(0x0001, PP_PP_WHILE_OVERFLOW)
#define PP_ERROR_0x0002 PP_ERROR(0x0002, PP_FOR_OVERFLOW)
#define PP_ERROR_0x0003 PP_ERROR(0x0003, PP_REPEAT_OVERFLOW)
#define PP_ERROR_0x0004 PP_ERROR(0x0004, PP_LIST_FOLD_OVERFLOW)
#define PP_ERROR_0x0005 PP_ERROR(0x0005, PP_SEQ_FOLD_OVERFLOW)
#define PP_ERROR_0x0006 PP_ERROR(0x0006, PP_ARITHMETIC_OVERFLOW)
#define PP_ERROR_0x0007 PP_ERROR(0x0007, PP_DIVISION_BY_ZERO)


// PP_BOOL
#define PP_BOOL(x) PP_BOOL_I(x)
#define PP_BOOL_I(x) PP_BOOL_II(x)
#define PP_BOOL_II(x) PP_BOOL_##x

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


// PP_BITNOT
#define PP_BITNOT(x) PP_BITNOT_I(x)
#define PP_BITNOT_I(x) PP_NOT_##x

#define PP_NOT_0 1
#define PP_NOT_1 0


// PP_BITAND
#define PP_BITAND(x, y) PP_BITAND_I(x, y)
#define PP_BITAND_I(x, y) PP_BITAND_##x##y

#define PP_BITAND_00 0
#define PP_BITAND_01 0
#define PP_BITAND_10 0
#define PP_BITAND_11 1


// PP_BITOR
#define PP_BITOR(x, y) PP_BITOR_I(x, y)
#define PP_BITOR_I(x, y) PP_BITOR_##x##y

#define PP_BITOR_00 0
#define PP_BITOR_01 1
#define PP_BITOR_10 1
#define PP_BITOR_11 1


// PP_BITXOR
#define PP_BITXOR(x, y) PP_BITXOR_I(x, y)
#define PP_BITXOR_I(x, y) PP_BITXOR_##x##y

#define PP_BITXOR_00 0
#define PP_BITXOR_01 1
#define PP_BITXOR_10 1
#define PP_BITXOR_11 0


// PP_NOT
#define PP_NOT(x) PP_BITNOT(PP_BOOL(x))


// PP_AND
#define PP_AND(p, q) PP_BITAND(PP_BOOL(p), PP_BOOL(q))


// PP_OR
#define PP_OR(p, q) PP_BITOR(PP_BOOL(p), PP_BOOL(q))


// PP_XOR
#define PP_XOR(p, q) PP_BITXOR(PP_BOOL(p), PP_BOOL(q))


// PP_IF
#define PP_IF(cond, t, f) PP_IF_I(PP_BOOL(cond), t, f)
#define PP_IF_I(b, t, f) PP_IF_II(b, t, f)
#define PP_IF_II(b, t, f) PP_IF_##b(t, f)

#define PP_IF_0(t, f) f
#define PP_IF_1(t, f) t


// PP_COMMA_IF
#define PP_COMMA_IF(cond) PP_IF(cond, PP_COMMA, PP_EMPTY)()


// PP_ENUM
#define PP_ENUM(count, param) PP_ENUM_REPEAT(count, PP_ENUM_M, param)
#define PP_ENUM_M(n, text) PP_COMMA_IF(n) text

// PP_NARG
#define PP_RSEQ_N() 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N

// #ifdef  __GNUC__
// #define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
// #define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)


// for all compiler
// #elif defined _MSC_VER
#define PP_NARG(...) PP_NARG_ ( __VA_ARGS__, PP_RSEQ_N() )
#define PP_NARG_(...) PP_ARG_N PP_BRACKET_L() __VA_ARGS__ PP_BRACKET_R()
#define PP_BRACKET_L() (
#define PP_BRACKET_R() )

// #else
// #error "no supported!!"
// #endif

#define PP_NUM_PARAMS(...) PP_IF(PP_DEC(PP_NARG(__VA_ARGS__)), PP_NARG(__VA_ARGS__), PP_NUM_PARAMS_0_1_TEST(__VA_ARGS__))
#define PP_NUM_PARAMS_0_1_TEST(_1, ...) PP_IF(PP_DEC(PP_NARG(PP_NUM_PARAMS_0 _1 ())), 0, 1)
#define PP_NUM_PARAMS_0()	,

#if 0
#define P00_ARG(                                               \
   _1, _2, _3, _4, _5, _6, _7, _8,                               \
   _9, _10, _11, _12, _13, _14, _15, _16,                        \
   _17, _18, _19, _20, _21, _22, _23, _24,                       \
   _25, _26, _27, _28, _29, _30, _31, _32,                       \
   _33, _34, _35, _36, _37, _38, _39, _40,                       \
   _41, _42, _43, _44, _45, _46, _47, _48,                       \
   _49, _50, _51, _52, _53, _54, _55, _56,                       \
   _57, _58, _59, _60, _61, _62, _63, _64,                       \
   _65, _66, _67, _68, _69, _70, _71, _72,                       \
   _73, _74, _75, _76, _77, _78, _79, _80,                       \
   _81, _82, _83, _84, _85, _86, _87, _88,                       \
   _89, _90, _91, _92, _93, _94, _95, _96,                       \
   _97, _98, _99, _100, _101, _102, _103, _104,                  \
   _105, _106, _107, _108, _109, _110, _111, _112,               \
   _113, _114, _115, _116, _117, _118, _119, _120,               \
   _121, _122, _123, _124, _125, _126, _127, _128,               \
   _129, _130, _131, _132, _133, _134, _135, _136,               \
   _137, _138, _139, _140, _141, _142, _143, _144,               \
   _145, _146, _147, _148, _149, _150, _151, _152,               \
   _153, _154, _155, _156, _157, _158, _159,                     \
   ...) _159

 #define P99_HAS_COMMA(...) P00_ARG(__VA_ARGS__,                \
   1, 1, 1, 1, 1, 1, 1,                                          \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 1, 1,                                       \
   1, 1, 1, 1, 1, 1, 0, 0)

#endif
// PP_ENUM_PARAMS
#define PP_ENUM_PARAMS(count, param) PP_ENUM_REPEAT(count, PP_ENUM_PARAMS_M, param)
#define PP_ENUM_PARAMS_M(n, text) PP_COMMA_IF(n) text##n


// PP_ENUM_BINARY_PARAMS
#define PP_ENUM_BINARY_PARAMS(count, a, b) PP_ENUM_BINARY_REPEAT(count, PP_ENUM_BINARY_PARAMS_M, a, b)
#define PP_ENUM_BINARY_PARAMS_M(n, a, b) PP_ENUM_BINARY_PARAMS_M_I(n, a, b)
#define PP_ENUM_BINARY_PARAMS_M_I(n, a, b) PP_COMMA_IF(n) a##n b##n


// PP_ENUM_TRAILING
#define PP_ENUM_TRAILING(count, param) PP_ENUM_REPEAT(count, PP_ENUM_TRAILING_M, param)
#define PP_ENUM_TRAILING_M(n, text) , text


// PP_ENUM_TRAILING_PARAMS
#define PP_ENUM_TRAILING_PARAMS(count, param) PP_ENUM_REPEAT(count, PP_ENUM_TRAILING_PARAMS_M, param)
#define PP_ENUM_TRAILING_PARAMS_M(n, text) , text##n


// PP_ENUM_TRAILING_BINARY_PARAMS
#define PP_ENUM_TRAILING_BINARY_PARAMS(count, a, b) PP_ENUM_BINARY_REPEAT(count, PP_ENUM_TRAILING_BINARY_PARAMS_M, a, b)
#define PP_ENUM_TRAILING_BINARY_PARAMS_M(n, a, b) PP_ENUM_TRAILING_BINARY_PARAMS_M_I(n, a, b)
#define PP_ENUM_TRAILING_BINARY_PARAMS_M_I(n, a, b) , a##n b##n


// PP_ENUM_REPEAT
#define PP_ENUM_REPEAT(count, macro, data) PP_ENUM_REPEAT_I(count, macro, data)
#define PP_ENUM_REPEAT_I(count, macro, data) PP_ENUM_REPEAT_##count(macro, data)

#define PP_ENUM_REPEAT_0(macro, data)
#define PP_ENUM_REPEAT_1(macro, data) PP_ENUM_REPEAT_0(macro, data)macro(0, data)
#define PP_ENUM_REPEAT_2(macro, data) PP_ENUM_REPEAT_1(macro, data)macro(1, data)
#define PP_ENUM_REPEAT_3(macro, data) PP_ENUM_REPEAT_2(macro, data)macro(2, data)
#define PP_ENUM_REPEAT_4(macro, data) PP_ENUM_REPEAT_3(macro, data)macro(3, data)
#define PP_ENUM_REPEAT_5(macro, data) PP_ENUM_REPEAT_4(macro, data)macro(4, data)
#define PP_ENUM_REPEAT_6(macro, data) PP_ENUM_REPEAT_5(macro, data)macro(5, data)
#define PP_ENUM_REPEAT_7(macro, data) PP_ENUM_REPEAT_6(macro, data)macro(6, data)
#define PP_ENUM_REPEAT_8(macro, data) PP_ENUM_REPEAT_7(macro, data)macro(7, data)
#define PP_ENUM_REPEAT_9(macro, data) PP_ENUM_REPEAT_8(macro, data)macro(8, data)
#define PP_ENUM_REPEAT_10(macro, data) PP_ENUM_REPEAT_9(macro, data)macro(9, data)
#define PP_ENUM_REPEAT_11(macro, data) PP_ENUM_REPEAT_10(macro, data)macro(10, data)
#define PP_ENUM_REPEAT_12(macro, data) PP_ENUM_REPEAT_11(macro, data)macro(11, data)
#define PP_ENUM_REPEAT_13(macro, data) PP_ENUM_REPEAT_12(macro, data)macro(12, data)
#define PP_ENUM_REPEAT_14(macro, data) PP_ENUM_REPEAT_13(macro, data)macro(13, data)
#define PP_ENUM_REPEAT_15(macro, data) PP_ENUM_REPEAT_14(macro, data)macro(14, data)
#define PP_ENUM_REPEAT_16(macro, data) PP_ENUM_REPEAT_15(macro, data)macro(15, data)


// PP_ENUM_BINARY_REPEAT
#define PP_ENUM_BINARY_REPEAT(count, macro, data1, data2) PP_ENUM_BINARY_REPEAT_I(count, macro, data1, data2)
#define PP_ENUM_BINARY_REPEAT_I(count, macro, data1, data2) PP_ENUM_BINARY_REPEAT_##count(macro, data1, data2)

#define PP_ENUM_BINARY_REPEAT_0(macro, data1, data2)
#define PP_ENUM_BINARY_REPEAT_1(macro, data1, data2) PP_ENUM_BINARY_REPEAT_0(macro, data1, data2)macro(0, data1, data2)
#define PP_ENUM_BINARY_REPEAT_2(macro, data1, data2) PP_ENUM_BINARY_REPEAT_1(macro, data1, data2)macro(1, data1, data2)
#define PP_ENUM_BINARY_REPEAT_3(macro, data1, data2) PP_ENUM_BINARY_REPEAT_2(macro, data1, data2)macro(2, data1, data2)
#define PP_ENUM_BINARY_REPEAT_4(macro, data1, data2) PP_ENUM_BINARY_REPEAT_3(macro, data1, data2)macro(3, data1, data2)
#define PP_ENUM_BINARY_REPEAT_5(macro, data1, data2) PP_ENUM_BINARY_REPEAT_4(macro, data1, data2)macro(4, data1, data2)
#define PP_ENUM_BINARY_REPEAT_6(macro, data1, data2) PP_ENUM_BINARY_REPEAT_5(macro, data1, data2)macro(5, data1, data2)
#define PP_ENUM_BINARY_REPEAT_7(macro, data1, data2) PP_ENUM_BINARY_REPEAT_6(macro, data1, data2)macro(6, data1, data2)
#define PP_ENUM_BINARY_REPEAT_8(macro, data1, data2) PP_ENUM_BINARY_REPEAT_7(macro, data1, data2)macro(7, data1, data2)
#define PP_ENUM_BINARY_REPEAT_9(macro, data1, data2) PP_ENUM_BINARY_REPEAT_8(macro, data1, data2)macro(8, data1, data2)
#define PP_ENUM_BINARY_REPEAT_10(macro, data1, data2) PP_ENUM_BINARY_REPEAT_9(macro, data1, data2)macro(9, data1, data2)
#define PP_ENUM_BINARY_REPEAT_11(macro, data1, data2) PP_ENUM_BINARY_REPEAT_9(macro, data1, data2)macro(10, data1, data2)
#define PP_ENUM_BINARY_REPEAT_12(macro, data1, data2) PP_ENUM_BINARY_REPEAT_9(macro, data1, data2)macro(11, data1, data2)
#define PP_ENUM_BINARY_REPEAT_13(macro, data1, data2) PP_ENUM_BINARY_REPEAT_9(macro, data1, data2)macro(12, data1, data2)
#define PP_ENUM_BINARY_REPEAT_14(macro, data1, data2) PP_ENUM_BINARY_REPEAT_9(macro, data1, data2)macro(13, data1, data2)
#define PP_ENUM_BINARY_REPEAT_15(macro, data1, data2) PP_ENUM_BINARY_REPEAT_9(macro, data1, data2)macro(14, data1, data2)
#define PP_ENUM_BINARY_REPEAT_16(macro, data1, data2) PP_ENUM_BINARY_REPEAT_9(macro, data1, data2)macro(15, data1, data2)


// PP_INC
#define PP_INC(x) PP_INC_I(x)
#define PP_INC_I(x) PP_INC_II(x)
#define PP_INC_II(x) PP_INC_##x

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
#define PP_INC_16 16


// PP_DEC
#define PP_DEC(x) PP_DEC_I(x)
#define PP_DEC_I(x) PP_DEC_II(x)
#define PP_DEC_II(x) PP_DEC_##x

#define PP_DEC_0 0
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


// PP_TUPLE_REM
#define PP_TUPLE_REM(size) PP_TUPLE_REM_I(size)
#define PP_TUPLE_REM_I(size) PP_TUPLE_REM_##size

#define PP_TUPLE_REM_0()
#define PP_TUPLE_REM_1(a) a
#define PP_TUPLE_REM_2(a, b) a, b
#define PP_TUPLE_REM_3(a, b, c) a, b, c
#define PP_TUPLE_REM_4(a, b, c, d) a, b, c, d
#define PP_TUPLE_REM_5(a, b, c, d, e) a, b, c, d, e
#define PP_TUPLE_REM_6(a, b, c, d, e, f) a, b, c, d, e, f
#define PP_TUPLE_REM_7(a, b, c, d, e, f, g) a, b, c, d, e, f, g
#define PP_TUPLE_REM_8(a, b, c, d, e, f, g, h) a, b, c, d, e, f, g, h
#define PP_TUPLE_REM_9(a, b, c, d, e, f, g, h, i) a, b, c, d, e, f, g, h, i
#define PP_TUPLE_REM_10(a, b, c, d, e, f, g, h, i, j) a, b, c, d, e, f, g, h, i, j
#define PP_TUPLE_REM_11(a, b, c, d, e, f, g, h, i, j, k) a, b, c, d, e, f, g, h, i, j, k
#define PP_TUPLE_REM_12(a, b, c, d, e, f, g, h, i, j, k, l) a, b, c, d, e, f, g, h, i, j, k, l
#define PP_TUPLE_REM_13(a, b, c, d, e, f, g, h, i, j, k, l, m) a, b, c, d, e, f, g, h, i, j, k, l, m
#define PP_TUPLE_REM_14(a, b, c, d, e, f, g, h, i, j, k, l, m, n) a, b, c, d, e, f, g, h, i, j, k, l, m, n
#define PP_TUPLE_REM_15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o
#define PP_TUPLE_REM_16(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p


// PP_TUPLE_ELEM
#define PP_TUPLE_ELEM(size, i, tuple) PP_TUPLE_ELEM_I(size, i, tuple)
#define PP_TUPLE_ELEM_I(size, i, tuple) PP_TUPLE_ELEM_##size##_##i tuple

#define PP_TUPLE_ELEM_1_0(a) a

#define PP_TUPLE_ELEM_2_0(a, b) a
#define PP_TUPLE_ELEM_2_1(a, b) b

#define PP_TUPLE_ELEM_3_0(a, b, c) a
#define PP_TUPLE_ELEM_3_1(a, b, c) b
#define PP_TUPLE_ELEM_3_2(a, b, c) c

#define PP_TUPLE_ELEM_4_0(a, b, c, d) a
#define PP_TUPLE_ELEM_4_1(a, b, c, d) b
#define PP_TUPLE_ELEM_4_2(a, b, c, d) c
#define PP_TUPLE_ELEM_4_3(a, b, c, d) d

#define PP_TUPLE_ELEM_5_0(a, b, c, d, e) a
#define PP_TUPLE_ELEM_5_1(a, b, c, d, e) b
#define PP_TUPLE_ELEM_5_2(a, b, c, d, e) c
#define PP_TUPLE_ELEM_5_3(a, b, c, d, e) d
#define PP_TUPLE_ELEM_5_4(a, b, c, d, e) e

#define PP_TUPLE_ELEM_6_0(a, b, c, d, e, f) a
#define PP_TUPLE_ELEM_6_1(a, b, c, d, e, f) b
#define PP_TUPLE_ELEM_6_2(a, b, c, d, e, f) c
#define PP_TUPLE_ELEM_6_3(a, b, c, d, e, f) d
#define PP_TUPLE_ELEM_6_4(a, b, c, d, e, f) e
#define PP_TUPLE_ELEM_6_5(a, b, c, d, e, f) f

#define PP_TUPLE_ELEM_7_0(a, b, c, d, e, f, g) a
#define PP_TUPLE_ELEM_7_1(a, b, c, d, e, f, g) b
#define PP_TUPLE_ELEM_7_2(a, b, c, d, e, f, g) c
#define PP_TUPLE_ELEM_7_3(a, b, c, d, e, f, g) d
#define PP_TUPLE_ELEM_7_4(a, b, c, d, e, f, g) e
#define PP_TUPLE_ELEM_7_5(a, b, c, d, e, f, g) f
#define PP_TUPLE_ELEM_7_6(a, b, c, d, e, f, g) g

#define PP_TUPLE_ELEM_8_0(a, b, c, d, e, f, g, h) a
#define PP_TUPLE_ELEM_8_1(a, b, c, d, e, f, g, h) b
#define PP_TUPLE_ELEM_8_2(a, b, c, d, e, f, g, h) c
#define PP_TUPLE_ELEM_8_3(a, b, c, d, e, f, g, h) d
#define PP_TUPLE_ELEM_8_4(a, b, c, d, e, f, g, h) e
#define PP_TUPLE_ELEM_8_5(a, b, c, d, e, f, g, h) f
#define PP_TUPLE_ELEM_8_6(a, b, c, d, e, f, g, h) g
#define PP_TUPLE_ELEM_8_7(a, b, c, d, e, f, g, h) h

#define PP_TUPLE_ELEM_9_0(a, b, c, d, e, f, g, h, i) a
#define PP_TUPLE_ELEM_9_1(a, b, c, d, e, f, g, h, i) b
#define PP_TUPLE_ELEM_9_2(a, b, c, d, e, f, g, h, i) c
#define PP_TUPLE_ELEM_9_3(a, b, c, d, e, f, g, h, i) d
#define PP_TUPLE_ELEM_9_4(a, b, c, d, e, f, g, h, i) e
#define PP_TUPLE_ELEM_9_5(a, b, c, d, e, f, g, h, i) f
#define PP_TUPLE_ELEM_9_6(a, b, c, d, e, f, g, h, i) g
#define PP_TUPLE_ELEM_9_7(a, b, c, d, e, f, g, h, i) h
#define PP_TUPLE_ELEM_9_8(a, b, c, d, e, f, g, h, i) i

#define PP_TUPLE_ELEM_10_0(a, b, c, d, e, f, g, h, i, j) a
#define PP_TUPLE_ELEM_10_1(a, b, c, d, e, f, g, h, i, j) b
#define PP_TUPLE_ELEM_10_2(a, b, c, d, e, f, g, h, i, j) c
#define PP_TUPLE_ELEM_10_3(a, b, c, d, e, f, g, h, i, j) d
#define PP_TUPLE_ELEM_10_4(a, b, c, d, e, f, g, h, i, j) e
#define PP_TUPLE_ELEM_10_5(a, b, c, d, e, f, g, h, i, j) f
#define PP_TUPLE_ELEM_10_6(a, b, c, d, e, f, g, h, i, j) g
#define PP_TUPLE_ELEM_10_7(a, b, c, d, e, f, g, h, i, j) h
#define PP_TUPLE_ELEM_10_8(a, b, c, d, e, f, g, h, i, j) i
#define PP_TUPLE_ELEM_10_9(a, b, c, d, e, f, g, h, i, j) j

#define PP_TUPLE_ELEM_11_0(a, b, c, d, e, f, g, h, i, j, k) a
#define PP_TUPLE_ELEM_11_1(a, b, c, d, e, f, g, h, i, j, k) b
#define PP_TUPLE_ELEM_11_2(a, b, c, d, e, f, g, h, i, j, k) c
#define PP_TUPLE_ELEM_11_3(a, b, c, d, e, f, g, h, i, j, k) d
#define PP_TUPLE_ELEM_11_4(a, b, c, d, e, f, g, h, i, j, k) e
#define PP_TUPLE_ELEM_11_5(a, b, c, d, e, f, g, h, i, j, k) f
#define PP_TUPLE_ELEM_11_6(a, b, c, d, e, f, g, h, i, j, k) g
#define PP_TUPLE_ELEM_11_7(a, b, c, d, e, f, g, h, i, j, k) h
#define PP_TUPLE_ELEM_11_8(a, b, c, d, e, f, g, h, i, j, k) i
#define PP_TUPLE_ELEM_11_9(a, b, c, d, e, f, g, h, i, j, k) j
#define PP_TUPLE_ELEM_11_10(a, b, c, d, e, f, g, h, i, j, k) k

#define PP_TUPLE_ELEM_12_0(a, b, c, d, e, f, g, h, i, j, k, l) a
#define PP_TUPLE_ELEM_12_1(a, b, c, d, e, f, g, h, i, j, k, l) b
#define PP_TUPLE_ELEM_12_2(a, b, c, d, e, f, g, h, i, j, k, l) c
#define PP_TUPLE_ELEM_12_3(a, b, c, d, e, f, g, h, i, j, k, l) d
#define PP_TUPLE_ELEM_12_4(a, b, c, d, e, f, g, h, i, j, k, l) e
#define PP_TUPLE_ELEM_12_5(a, b, c, d, e, f, g, h, i, j, k, l) f
#define PP_TUPLE_ELEM_12_6(a, b, c, d, e, f, g, h, i, j, k, l) g
#define PP_TUPLE_ELEM_12_7(a, b, c, d, e, f, g, h, i, j, k, l) h
#define PP_TUPLE_ELEM_12_8(a, b, c, d, e, f, g, h, i, j, k, l) i
#define PP_TUPLE_ELEM_12_9(a, b, c, d, e, f, g, h, i, j, k, l) j
#define PP_TUPLE_ELEM_12_10(a, b, c, d, e, f, g, h, i, j, k, l) k
#define PP_TUPLE_ELEM_12_11(a, b, c, d, e, f, g, h, i, j, k, l) l

#define PP_TUPLE_ELEM_13_0(a, b, c, d, e, f, g, h, i, j, k, l, m) a
#define PP_TUPLE_ELEM_13_1(a, b, c, d, e, f, g, h, i, j, k, l, m) b
#define PP_TUPLE_ELEM_13_2(a, b, c, d, e, f, g, h, i, j, k, l, m) c
#define PP_TUPLE_ELEM_13_3(a, b, c, d, e, f, g, h, i, j, k, l, m) d
#define PP_TUPLE_ELEM_13_4(a, b, c, d, e, f, g, h, i, j, k, l, m) e
#define PP_TUPLE_ELEM_13_5(a, b, c, d, e, f, g, h, i, j, k, l, m) f
#define PP_TUPLE_ELEM_13_6(a, b, c, d, e, f, g, h, i, j, k, l, m) g
#define PP_TUPLE_ELEM_13_7(a, b, c, d, e, f, g, h, i, j, k, l, m) h
#define PP_TUPLE_ELEM_13_8(a, b, c, d, e, f, g, h, i, j, k, l, m) i
#define PP_TUPLE_ELEM_13_9(a, b, c, d, e, f, g, h, i, j, k, l, m) j
#define PP_TUPLE_ELEM_13_10(a, b, c, d, e, f, g, h, i, j, k, l, m) k
#define PP_TUPLE_ELEM_13_11(a, b, c, d, e, f, g, h, i, j, k, l, m) l
#define PP_TUPLE_ELEM_13_12(a, b, c, d, e, f, g, h, i, j, k, l, m) m

#define PP_TUPLE_ELEM_14_0(a, b, c, d, e, f, g, h, i, j, k, l, m, n) a
#define PP_TUPLE_ELEM_14_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n) b
#define PP_TUPLE_ELEM_14_2(a, b, c, d, e, f, g, h, i, j, k, l, m, n) c
#define PP_TUPLE_ELEM_14_3(a, b, c, d, e, f, g, h, i, j, k, l, m, n) d
#define PP_TUPLE_ELEM_14_4(a, b, c, d, e, f, g, h, i, j, k, l, m, n) e
#define PP_TUPLE_ELEM_14_5(a, b, c, d, e, f, g, h, i, j, k, l, m, n) f
#define PP_TUPLE_ELEM_14_6(a, b, c, d, e, f, g, h, i, j, k, l, m, n) g
#define PP_TUPLE_ELEM_14_7(a, b, c, d, e, f, g, h, i, j, k, l, m, n) h
#define PP_TUPLE_ELEM_14_8(a, b, c, d, e, f, g, h, i, j, k, l, m, n) i
#define PP_TUPLE_ELEM_14_9(a, b, c, d, e, f, g, h, i, j, k, l, m, n) j
#define PP_TUPLE_ELEM_14_10(a, b, c, d, e, f, g, h, i, j, k, l, m, n) k
#define PP_TUPLE_ELEM_14_11(a, b, c, d, e, f, g, h, i, j, k, l, m, n) l
#define PP_TUPLE_ELEM_14_12(a, b, c, d, e, f, g, h, i, j, k, l, m, n) m
#define PP_TUPLE_ELEM_14_13(a, b, c, d, e, f, g, h, i, j, k, l, m, n) n

#define PP_TUPLE_ELEM_15_0(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) a
#define PP_TUPLE_ELEM_15_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) b
#define PP_TUPLE_ELEM_15_2(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) c
#define PP_TUPLE_ELEM_15_3(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) d
#define PP_TUPLE_ELEM_15_4(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) e
#define PP_TUPLE_ELEM_15_5(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) f
#define PP_TUPLE_ELEM_15_6(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) g
#define PP_TUPLE_ELEM_15_7(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) h
#define PP_TUPLE_ELEM_15_8(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) i
#define PP_TUPLE_ELEM_15_9(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) j
#define PP_TUPLE_ELEM_15_10(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) k
#define PP_TUPLE_ELEM_15_11(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) l
#define PP_TUPLE_ELEM_15_12(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) m
#define PP_TUPLE_ELEM_15_13(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) n
#define PP_TUPLE_ELEM_15_14(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) o

#define PP_TUPLE_ELEM_16_0(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) a
#define PP_TUPLE_ELEM_16_1(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) b
#define PP_TUPLE_ELEM_16_2(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) c
#define PP_TUPLE_ELEM_16_3(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) d
#define PP_TUPLE_ELEM_16_4(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) e
#define PP_TUPLE_ELEM_16_5(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) f
#define PP_TUPLE_ELEM_16_6(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) g
#define PP_TUPLE_ELEM_16_7(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) h
#define PP_TUPLE_ELEM_16_8(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) i
#define PP_TUPLE_ELEM_16_9(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) j
#define PP_TUPLE_ELEM_16_10(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) k
#define PP_TUPLE_ELEM_16_11(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) l
#define PP_TUPLE_ELEM_16_12(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) m
#define PP_TUPLE_ELEM_16_13(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) n
#define PP_TUPLE_ELEM_16_14(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) o
#define PP_TUPLE_ELEM_16_15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) p


// PP_WHILE
#define PP_WHILE(pred, op, state) PP_WHILE_0(pred, op, state)

#define PP_WHILE__(pred, op, state) state
#define PP_WHILE_0(pred, op, state) PP_WHILE_NEXT_ITEM(0, 1, pred, state)(pred, op, PP_WHILE_NEXT_STATE(0, pred, op, state))
#define PP_WHILE_1(pred, op, state) PP_WHILE_NEXT_ITEM(1, 2, pred, state)(pred, op, PP_WHILE_NEXT_STATE(1, pred, op, state))
#define PP_WHILE_2(pred, op, state) PP_WHILE_NEXT_ITEM(2, 3, pred, state)(pred, op, PP_WHILE_NEXT_STATE(2, pred, op, state))
#define PP_WHILE_3(pred, op, state) PP_WHILE_NEXT_ITEM(3, 4, pred, state)(pred, op, PP_WHILE_NEXT_STATE(3, pred, op, state))
#define PP_WHILE_4(pred, op, state) PP_WHILE_NEXT_ITEM(4, 5, pred, state)(pred, op, PP_WHILE_NEXT_STATE(4, pred, op, state))
#define PP_WHILE_5(pred, op, state) PP_WHILE_NEXT_ITEM(5, 6, pred, state)(pred, op, PP_WHILE_NEXT_STATE(5, pred, op, state))
#define PP_WHILE_6(pred, op, state) PP_WHILE_NEXT_ITEM(6, 7, pred, state)(pred, op, PP_WHILE_NEXT_STATE(6, pred, op, state))
#define PP_WHILE_7(pred, op, state) PP_WHILE_NEXT_ITEM(7, 8, pred, state)(pred, op, PP_WHILE_NEXT_STATE(7, pred, op, state))
#define PP_WHILE_8(pred, op, state) PP_WHILE_NEXT_ITEM(8, 9, pred, state)(pred, op, PP_WHILE_NEXT_STATE(8, pred, op, state))
#define PP_WHILE_9(pred, op, state) PP_WHILE_NEXT_ITEM(9, 10, pred, state)(pred, op, PP_WHILE_NEXT_STATE(9, pred, op, state))
#define PP_WHILE_10(pred, op, state) PP_WHILE_NEXT_ITEM(10, 11, pred, state)(pred, op, PP_WHILE_NEXT_STATE(10, pred, op, state))
#define PP_WHILE_11(pred, op, state) PP_WHILE_NEXT_ITEM(11, 12, pred, state)(pred, op, PP_WHILE_NEXT_STATE(11, pred, op, state))
#define PP_WHILE_12(pred, op, state) PP_WHILE_NEXT_ITEM(12, 13, pred, state)(pred, op, PP_WHILE_NEXT_STATE(12, pred, op, state))
#define PP_WHILE_13(pred, op, state) PP_WHILE_NEXT_ITEM(13, 14, pred, state)(pred, op, PP_WHILE_NEXT_STATE(13, pred, op, state))
#define PP_WHILE_14(pred, op, state) PP_WHILE_NEXT_ITEM(14, 15, pred, state)(pred, op, PP_WHILE_NEXT_STATE(14, pred, op, state))
#define PP_WHILE_15(pred, op, state) PP_WHILE_NEXT_ITEM(15, 16, pred, state)(pred, op, PP_WHILE_NEXT_STATE(15, pred, op, state))
#define PP_WHILE_16(pred, op, state) PP_ERROR(0x0001)

#define PP_WHILE_NEXT_ITEM(i, n, pred, state) PP_CAT(PP_WHILE_, PP_IF(PP_BOOL(pred(i, state)), n, _))
#define PP_WHILE_NEXT_STATE(i, pred, op, state) PP_IF(PP_BOOL(pred(i, state)), op(i, state), state)


#if 0

#define STATE(d, state) state

#define TUPLE4 (1, 2, 3, 4)
#define TUPLE2 (4, 8)
#define PRED(d, state) PP_TUPLE_ELEM(2, 0, state)
#define OP(d, state) (PP_DEC(PP_TUPLE_ELEM(2, 0, state)), PP_INC(PP_TUPLE_ELEM(2, 1, state)))

void _f1() {
    PP_TUPLE_ELEM(4, 2, TUPLE4);
    PP_INC(3);
    PP_DEC(3);

    PRED(1, TUPLE2);
    OP(1, TUPLE2);

    PP_WHILE(PRED, OP, TUPLE2);

    // PP_IF(PP_BOOL(pred(2, state)), 3, 0);

    // PP_TUPLE_ELEM(2, 1, WHILE(PRED, OP, TUPLE2));
}

#define DECL(n, text) text##n = n;
#define DECL2(n, text) , text##n
#define DECL3(n, text) PP_COMMA_IF(n)text##n

void test() {
    PP_BOOL(0);
    PP_BOOL(2);

    PP_NOT(0);
    PP_NOT(2);

    PP_AND(0, 0);
    PP_AND(0, 2);
    PP_AND(2, 0);
    PP_AND(1, 2);

    PP_OR(0, 0);
    PP_OR(0, 2);
    PP_OR(2, 0);
    PP_OR(1, 2);

    PP_XOR(0, 0);
    PP_XOR(0, 2);
    PP_XOR(2, 0);
    PP_XOR(1, 2);

    PP_IF(2, a, b);
    PP_IF(0, a, b);

    int a PP_COMMA_IF(2) b;
    // int a, b

    PP_COMMA_IF(0);

    PP_ENUM_PARAMS(3, class T);
    // class T0, class T1, class T2

    PP_ENUM_REPEAT(3, DECL, int a);
    // int a0 = 0; int a1 = 1; int a2 = 2;

    PP_ENUM(3, class);
    // class, class, class
}

void test2(PP_ENUM_REPEAT(3, DECL3, int a)) {
}

void test3(PP_ENUM_PARAMS(3, int a)) {
}

void test4(int x, int y PP_ENUM_REPEAT(9, DECL2, int a)) {
}

void test5(int x, int y PP_ENUM_TRAILING_PARAMS(9, int a)) {
}

template <PP_ENUM_PARAMS(3, typename A)>
void test6(PP_ENUM_BINARY_PARAMS(3, A, a)) {
}

template <PP_ENUM_PARAMS(3, typename A)>
void test7(int x PP_ENUM_TRAILING_BINARY_PARAMS(3, A, a)) {
}

#endif

#endif /* PREPROCESSOR_H_ */


