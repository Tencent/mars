/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ijkplayer_napi_utils_H
#define ijkplayer_napi_utils_H
#include <string>
#include <napi/native_api.h>
#include "native_common.h"

class NapiUtil {
public:
    static void JsValueToString(const napi_env &env, const napi_value &value, const int32_t bufLen,
        std::string &target);
    static napi_value SetNapiCallInt32(const napi_env & env,  const int32_t intValue);
    static napi_value SetNapiCallBool(const napi_env env, bool value);
    static int StringToInt(std::string value);
    static int StringToLong(std::string value);
    static float StringToFloat(std::string value);
    static bool StringToBool(std::string value);
};


#endif //ijkplayer_napi_utils_H
