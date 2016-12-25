/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
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