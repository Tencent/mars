// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by LOGlicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * compiler_util.h
 *
 *  Created on: 2017-3-28
 *      Author: jehoochen
 */

#ifndef LOG_COMPILER_UTIL_H_
#define LOG_COMPILER_UTIL_H_

#if defined(_MSC_VER) && defined(MARS_USE_DLLS)
#ifdef MARS_LOG_EXPORTS
#define MARS_LOG_EXPORT __declspec(dllexport)
#else
#define MARS_LOG_EXPORT __declspec(dllimport)
#endif
#else
#define MARS_LOG_EXPORT
#endif

#endif /* LOG_COMPILER_UTIL_H_ */
