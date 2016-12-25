/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/********************************************************************
    modify:    2012/9/17
    author:     Ray
*********************************************************************/

#ifndef ANDROID_CALLSTACK_H
#define ANDROID_CALLSTACK_H

#include <stdint.h>
#include <sys/types.h>

#ifndef __cplusplus

void android_callstack(char* _out,  unsigned int _len);

#else
#include <string>

extern "C" void android_callstack(char* _out,  unsigned int _len);
// ---------------------------------------------------------------------------

namespace android {

class CallStack {
  public:
    enum {
        kMaxDepth = 31
    };

    CallStack();
    CallStack(const CallStack& rhs);
    ~CallStack();

    CallStack& operator = (const CallStack& rhs);

    bool operator == (const CallStack& rhs) const;
    bool operator != (const CallStack& rhs) const;
    bool operator < (const CallStack& rhs) const;
    bool operator >= (const CallStack& rhs) const;
    bool operator > (const CallStack& rhs) const;
    bool operator <= (const CallStack& rhs) const;

    const void* operator [](int index) const;

    void clear();

    void update(int32_t ignoreDepth = 1, int32_t maxDepth = kMaxDepth);

    // Dump a stack trace to the log
    //    void dump(const char* prefix = 0) const;

    // Return a string (possibly very long) containing the complete stack trace
    std::string toString(const char* prefix = 0) const;
    pid_t  TID() const { return tid_; }
    size_t size() const { return count_; }

    std::string Format(const char*  _str_build_fingerprint = NULL, const char* _str_process_name = NULL) const;

  private:
    // Internal helper function
    std::string toStringSingleLevel(const char* prefix, int32_t level) const;

    size_t      count_;
    pid_t       tid_;
    const void* stack_[kMaxDepth];
};

};  // namespace android

#endif
// ---------------------------------------------------------------------------

#endif  // ANDROID_CALLSTACK_H
