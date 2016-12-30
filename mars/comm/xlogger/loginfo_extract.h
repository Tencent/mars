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
 * loginfo_extract.h
 *
 *  Created on: 2016年10月13日
 *      Author: yanguoyue
 */

#ifndef XLOGGER_LOGINFO_EXTRACT_H_
#define XLOGGER_LOGINFO_EXTRACT_H_

#ifdef __cplusplus
extern "C" {
#endif

const char* ExtractFileName(const char* _path);

void ExtractFunctionName(const char* _func, char* _func_ret, int _len);


#ifdef __cplusplus
}
#endif
#endif /* XLOGGER_LOGINFO_EXTRACT_H_ */
