// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LOG_MAGIC_NUM_H_
#define LOG_MAGIC_NUM_H_

namespace mars {
namespace xlog {

class LogMagicNum {
 public:
    LogMagicNum() = delete;

    static const char kMagicSyncZlibStart = '\x06';
    static const char kMagicSyncNoCryptZlibStart = '\x08';
    static const char kMagicAsyncZlibStart = '\x07';
    static const char kMagicAsyncNoCryptZlibStart = '\x09';

    static const char kMagicSyncZstdStart = '\x0A';
    static const char kMagicSyncNoCryptZstdStart = '\x0B';
    static const char kMagicAsyncZstdStart = '\x0C';
    static const char kMagicAsyncNoCryptZstdStart = '\x0D';

    static const char kMagicEnd = '\0';

    static bool MagicStartIsValid(char _magic) {
        return kMagicSyncZlibStart == _magic || kMagicSyncNoCryptZlibStart == _magic || kMagicAsyncZlibStart == _magic
               || kMagicAsyncNoCryptZlibStart == _magic || kMagicSyncZstdStart == _magic
               || kMagicSyncNoCryptZstdStart == _magic || kMagicAsyncZstdStart == _magic
               || kMagicAsyncNoCryptZstdStart == _magic;
    }
};

}  // namespace xlog
}  // namespace mars

#endif /* LOG_MAGIC_NUM_H_ */
