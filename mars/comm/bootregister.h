// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#ifndef COMM_BOOTREGISTER_H_
#define COMM_BOOTREGISTER_H_

#include <vector>
#include "comm/compiler_util.h"

template <typename T>
std::vector<T>& BootRegister_Container()
{
    static std::vector<T> s_register;
    return s_register;
}

template <typename T>
bool BootRegister_Add(const T& _data)
{
    BootRegister_Container<T>().push_back(_data);
    return true;
}

#define BOOT_REGISTER(data) BOOT_REGISTER_IMPL_I(data, __LINE__)
#define BOOT_REGISTER_IMPL_I(data, line) BOOT_REGISTER_IMPL_II(data, line)
#define BOOT_REGISTER_IMPL_II(data, line) VARIABLE_IS_NOT_USED static bool __int_anonymous_##line = BootRegister_Add(data)
#define BOOT_REGISTER_CHECK(name, data) VARIABLE_IS_NOT_USED bool __test_##name##_check = BootRegister_Add(data)

#endif	// COMM_BOOTREGISTER_H_
