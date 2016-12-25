/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

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
