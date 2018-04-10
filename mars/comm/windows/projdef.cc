// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#include "projdef.h"
#include <Windows.h>

#include "strutil.h"

#define ENCODING CP_UTF8 

FILE* mars_fopen_utf8(const char* _path, const char* _model) {
	return _wfopen(strutil::String2WString(_path, ENCODING).c_str(), strutil::String2WString(_model, ENCODING).c_str());
}

int mars_mkdir_utf8(const char* _path, unsigned short _model) {
	return _wmkdir(strutil::String2WString(_path, ENCODING).c_str());
}
int mars_access_utf8(char const* _path, int _model) {
	return _waccess(strutil::String2WString(_path, ENCODING).c_str(), _model);
}

int mars_remove_utf8(char const* _path) {
	return _wremove(strutil::String2WString(_path, ENCODING).c_str());
}