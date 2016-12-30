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
 * urlparser.h
 *
 *  Created on: 2014年6月27日
 *      Author: wutianqiang
 */

#ifndef SDT_SRC_CHECKIMPL_URLPARSER_H_
#define SDT_SRC_CHECKIMPL_URLPARSER_H_

#include <string>
#include <algorithm>
#include <locale>

#include "mars/comm/strutil.h"
#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace sdt {
	class HttpUrlParser {
	public:
		explicit HttpUrlParser(const char* url, size_t length)
		: port_(80) {
			url_.assign(url, length);
			strutil::Trim(url_);
			Parse();
		}

		explicit HttpUrlParser(const char* url)
		: port_(80) {
			url_ = url;
			strutil::Trim(url_);
			Parse();
		}

		explicit HttpUrlParser(const std::string& url)
		: port_(80), url_(url) {
			strutil::Trim(url_);
			Parse();
		}

		const char* Host() {
			return host_.c_str();
		}

		uint16_t Port() {
			return port_;
		}

		const char* Path() {
			return path_.c_str();
		}

	private:
		bool Parse() {
			if (url_.empty()) {
				xwarn2(TSF"url_ is empty.");
				return false;
			}

			const std::string kHttpSchema("http://");

			size_t schema_start = 0;

			if (0 == ci_find_substr(url_, kHttpSchema, 0))
				schema_start = kHttpSchema.length();

			// only support http
			if (0 == schema_start || schema_start >= url_.length()) {
				xwarn2(TSF"schema_start=%_, url_=%_", schema_start, url_);
				return false;
			}
			size_t schema_end = ci_find_substr(url_, std::string("/"), schema_start + 1); // tmpurl.find("/", schema_start+1);

			if (std::string::npos == schema_end)
				schema_end = url_.length();

			std::string hoststr = url_.substr(schema_start, schema_end - schema_start);
			strutil::Trim(hoststr);

			// user name
			size_t host_start = 0;
			size_t userpwd_start = ci_find_substr(hoststr, std::string("@"), 0);  // hoststr.find("@");

			if (userpwd_start != std::string::npos) {
				host_start = userpwd_start + 1;
			}

			// port
			size_t portstart = ci_find_substr(hoststr, std::string(":"), host_start);  // hoststr.find(":", host_start);

			if (std::string::npos == portstart) {
				host_ = hoststr.substr(host_start);
				port_ = 80;
			} else if (hoststr.length() - 1 == portstart) {
				host_ = hoststr.substr(host_start, portstart - host_start);
				port_ = 80;
			} else {
				host_ = hoststr.substr(host_start, portstart - host_start);
				port_ = (uint16_t)atoi(hoststr.substr(portstart + 1).c_str());
			}

			if (0 == port_)
				port_ = 80;

			strutil::Trim(host_);

			// path
			path_ = url_.substr(schema_end);

			if (path_.empty())
				path_ = "/";

			return !host_.empty();
		}

	private:
		// templated version of my_equal so it could work with both char and wchar_t
		template<typename charT>
		struct my_equal {
			my_equal(const std::locale& loc) : loc_(loc) {}
			bool operator()(charT ch1, charT ch2) {
				return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
			}
		private:
			const std::locale& loc_;
		};

		// find substring (case insensitive)
		template<typename T>
		size_t ci_find_substr(const T& str1, const T& str2, size_t pos, const std::locale& loc = std::locale()) {
			typename T::const_iterator it = std::search(str1.begin() + pos, str1.end(),
														str2.begin(), str2.end(), my_equal<typename T::value_type>(loc));

			if (it != str1.end()) return it - str1.begin();
			else return std::string::npos;  // not found
		}

	private:
		uint16_t port_;
		std::string url_;
		std::string host_;
		std::string path_;
	};

} // namespace sdt
} //namespace mars

#endif /* SDT_SRC_CHECKIMPL_URLPARSER_H_ */
