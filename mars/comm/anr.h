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

#include <string>

#include "mars/boost/signals2.hpp"

#if !defined(ANDROID) && !defined(__APPLE__)
#define ANR_CHECK_DISABLE
#endif

namespace mars {
    namespace comm {
        struct check_content {
            uintptr_t ptr;
            std::string file;
            std::string func;
            int line;
            int timeout;
            intmax_t tid;
            
            uint64_t start_time;
            uint64_t end_time;
            
            uint64_t start_tickcount;
            uint64_t used_cpu_time; //ms
            
            int call_id;;
            void* extra_info;
            
            bool operator<(const check_content& _ref) const {
                return end_time > _ref.end_time;
            }
        };
    }
}


class scope_anr {
  public:
    scope_anr(const char* _file = "", const char* _func = "", int _line = 0, int _id = 0, void* _extra_info = NULL);
    ~scope_anr();

    void anr(int _timeout = 15 * 60 * 1000);

  private:
    scope_anr(const scope_anr&);
    scope_anr& operator=(const scope_anr&);

  private:
    const char* file_;
    const char* func_;
    int line_;
    int call_id_;
    void* extra_info_;
};


extern boost::signals2::signal<void (bool _iOS_style, const mars::comm::check_content& _content)>& GetSignalCheckHit();

#define SCOPE_ANR_AUTO(timeout, id, extra_info) scope_anr __anr__var__anonymous_variable__(__FILE__, __func__, __LINE__, id, extra_info); __anr__var__anonymous_variable__.anr(timeout)
#define SCOPE_ANR_OBJ(objname) scope_anr objname(__FILE__, __func__, __LINE__)

#endif /* COMM_ANR_H_ */
