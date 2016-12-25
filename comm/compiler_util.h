/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * compiler_util.h
 *
 *  Created on: 2013-11-15
 *      Author: yerungui
 */

#ifndef COMM_COMPILER_UTIL_H_
#define COMM_COMPILER_UTIL_H_

#if defined(__GNUC__)
#define WEAK_FUNC     __attribute__((weak))
#elif defined(_MSC_VER) && !defined(_LIB)
#define WEAK_FUNC __declspec(selectany)
#else
#define WEAK_FUNC
#endif

#if defined(__GNUC__)
#define EXPORT_FUNC __attribute__ ((visibility ("default")))
#elif defined(_MSC_VER)
#define EXPORT_FUNC __declspec(dllexport)
#else
#error "export"
#endif

#ifndef VARIABLE_IS_NOT_USED
#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif
#endif

#endif /* COMM_COMPILER_UTIL_H_ */
