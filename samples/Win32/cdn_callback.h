// Tencent is pleased to support the open source community by making GAutomator available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "mars/cdn/cdn.h"
#include "mars/cdn/cdn_logic.h"

namespace mars {
	namespace cdn {
	
class CdnCallBack : public Callback
{
public:

	CdnCallBack() {}
	~CdnCallBack() {}

	virtual void OnProgressChanged(const std::string& filekey, size_t completed_length, size_t total_length);

	virtual void OnC2CUploadCompleted(const std::string& filekey, const C2CUploadResult& result);

	virtual void OnC2CDownloadCompleted(const std::string& filekey, const C2CDownloadResult& result);

	virtual void OnCheckFileIDCompleted(const std::string& filekey, const CheckFileIDResult& result);

};

	}
}