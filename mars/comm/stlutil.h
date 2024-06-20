// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
// Created by tiemuhuaguo on 4/29/24.
//

#ifndef MARS_STLUTIL_H
#define MARS_STLUTIL_H

#include <algorithm>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <list>

namespace stlutil {
template <typename C, typename T>
bool Exist(const C& c, const T& v) {
    // std::map、std::unordered_map不支持std::find(std::begin(c), std::end(c), v)
    return c.find(v) != c.end();
}
}  // namespace stlutil

#endif  // MARS_STLUTIL_H
