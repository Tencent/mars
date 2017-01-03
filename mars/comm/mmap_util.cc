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
 * mmap_util.c
 *
 *  Created on: 2016-2-22
 *      Author: yanguoyue
 */

#include "mmap_util.h"

#include <unistd.h>
#include <stdio.h>

#include "boost/filesystem.hpp"

bool IsMmapFileOpenSucc(const boost::iostreams::mapped_file& _mmmap_file) {
    return !_mmmap_file.operator !() && _mmmap_file.is_open();
}

bool OpenMmapFile(const char* _filepath, unsigned int _size, boost::iostreams::mapped_file& _mmmap_file) {

    if (NULL == _filepath || 0 == strnlen(_filepath, 128) || 0 == _size) {
        return false;
    }

    if (IsMmapFileOpenSucc(_mmmap_file)) {
        CloseMmapFile(_mmmap_file);
    }
    
    if (_mmmap_file.is_open() && _mmmap_file.operator!()) {
        return false;
    }

    boost::iostreams::mapped_file_params param;
    param.path = _filepath;
    param.flags = boost::iostreams::mapped_file_base::readwrite;

    bool file_exist = boost::filesystem::exists(_filepath);
    if (!file_exist) {
        param.new_file_size = _size;
    }

    _mmmap_file.open(param);

    bool is_open = IsMmapFileOpenSucc(_mmmap_file);

    if (!file_exist && is_open) {

        //Extending a file with ftruncate, thus creating a big hole, and then filling the hole by mod-ifying a shared mmap() can lead to SIGBUS when no space left
        //the boost library uses ftruncate, so we pre-allocate the file's backing store by writing zero.
        FILE* file = fopen(_filepath, "rb+");
        if (NULL == file) {
            _mmmap_file.close();
            remove(_filepath);
            return false;
        }

        char* zero_data = new char[_size];
        memset(zero_data, 0, _size);

        if (_size != fwrite(zero_data, sizeof(char), _size, file)) {
            _mmmap_file.close();
            fclose(file);
            remove(_filepath);
            delete[] zero_data;
            return false;
        }
        fclose(file);
        delete[] zero_data;
    }

    return is_open;
}

void CloseMmapFile(boost::iostreams::mapped_file& _mmmap_file) {
    if (_mmmap_file.is_open()) {
        _mmmap_file.close();
    }
}
