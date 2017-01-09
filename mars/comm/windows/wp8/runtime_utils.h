// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


#ifndef RUNTIME_UTILS
#define RUNTIME_UTILS

#ifdef WP8

#include <stddef.h>
#include <string>
using namespace std;

#define MAX_PATH          260




using namespace Platform;
static char* toMbs(char* dst, const wchar_t* src, int len) {
    if (dst == NULL)
        dst = new char[len + 1];

    for (int i = 0 ; i < len ; i++) {
        dst[i] = (char)src[i];
    }

    dst[len] = 0;

    return dst;
}

static char* getIsoPath(char root_path[MAX_PATH]) {
    Windows::Storage::StorageFolder^ local = Windows::Storage::ApplicationData::Current->LocalFolder;

    toMbs(root_path, local->Path->Data(), local->Path->Length());

    strcat_s(root_path, MAX_PATH, "\\");

    return root_path;
}

static string toMbs(const wchar_t* src, int len) {
    char* dst = new char[len + 1];

    for (int i = 0 ; i < len ; i++) {
        dst[i] = (char)src[i];
    }

    dst[len] = 0;

    string out = dst;
    delete[] dst;
    dst = NULL;
    return out;
}

static string String2stdstring(String^ inString) {
    const wchar_t* src = inString->Data();
    int len = inString->Length();
    char* dst = new char[len + 1];

    for (int i = 0 ; i < len ; i++) {
        dst[i] = (char)src[i];
    }

    dst[len] = 0;

    string out = dst;
    delete[] dst;
    dst = NULL;
    return out;
}

static String^ stdstring2String(string instring) {
    const char* src = instring.c_str();
    int len = instring.length();

    wchar_t* dst = new wchar_t[len + 1];

    for (int i = 0 ; i < len ; i++) {
        dst[i] = src[i];
    }

    dst[len] = 0;

    String^ out = ref new String(dst);
    delete[] dst;
    dst = NULL;

    return out;
}

#endif

#endif