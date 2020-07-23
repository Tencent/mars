// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 ============================================================================
 Name		: xlogger.h
 ============================================================================
 */

#include "xlogger.h"

XLogger::~XLogger() {
	if (!m_isassert && m_message.empty()) return;

	gettimeofday(&m_info.timeval, NULL);
	if (m_hook && !m_hook(m_info, m_message)) return;

	xlogger_filter_t filter = xlogger_GetFilter();
	if (filter && filter(&m_info, m_message.c_str()) <= 0)  return;

	if (m_isassert)
		xlogger_Assert(m_isinfonull?NULL:&m_info, m_exp, m_message.c_str());
	else
		xlogger_Write(m_isinfonull?NULL:&m_info, m_message.c_str());
}
