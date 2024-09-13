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
#include <list>
#include <map>
#include <unordered_map>

namespace stlutil {
template <typename C, typename T>
bool Exist(const C& c, const T& v) {
    return std::find(std::begin(c), std::end(c), v) != std::end(c);
}

template <typename K, typename V>
bool Exist(const std::map<K, V>& c, const K& v) {
    return c.find(v) != c.end();
}

template <typename K, typename V>
bool Exist(const std::unordered_map<K, V>& c, const K& v) {
    return c.find(v) != c.end();
}

template <typename C, typename T>
void Erase(C& c, const T& v) {
    c.erase(std::find(std::begin(c), std::end(c), v));
}
template <typename C, typename F>
void RemoveIfAndErase(C& c, const F& f) {
    c.erase(std::remove_if(std::begin(c), std::end(c), f), c.end());
}
template <typename C, typename F>
bool AnyOf(C& c, const F& f) {
    return std::any_of(c.begin(), c.end(), f);
}
template <typename T>
T GetAndPopFront(std::list<T>& list) {
    T t = list.front();
    list.pop_front();
    return t;
}
template <typename T>
T DereferenceSafe(const T* ptr) {
    if (ptr) {
        return *ptr;
    }
    return {};
}
}  // namespace stlutil

#endif  // MARS_STLUTIL_H
