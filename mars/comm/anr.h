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
 * anr.h
 *
 *  Created on: 2014-7-18
 *      Author: yerungui
 */

#ifndef COMM_ANR_H_
#define COMM_ANR_H_

#include "mars/boost/signals2.hpp"

class scope_anr {
  public:
    scope_anr(const char* _file = "", const char* _func = "", int _line = 0);
    ~scope_anr();

    void anr(int _timeout = 15 * 60 * 1000);

  private:
    scope_anr(const scope_anr&);
    scope_anr& operator=(const scope_anr&);

  private:
    const char* file_;
    const char* func_;
    int line_;
};


extern boost::signals2::signal<void (bool _iOS_style)>& GetSignalCheckHit();

#define SCOPE_ANR_AUTO(...) scope_anr __anr__var__anonymous_variable__(__FILE__, __func__, __LINE__); __anr__var__anonymous_variable__.anr(__VA_ARGS__)
#define SCOPE_ANR_OBJ(objname) scope_anr objname(__FILE__, __func__, __LINE__)

#endif /* COMM_ANR_H_ */
