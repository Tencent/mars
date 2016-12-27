/*
 * shortlink_packer.cc
 *
 *  Created on: 2016-03-15
 *      Author: yanguoyue
 */

#include "shortlink_packer.h"


using namespace http;

void shortlink_pack(const std::string& _url, const std::map<std::string, std::string>& _headers, const AutoBuffer& _body, AutoBuffer& _out_buff) {

	Builder req_builder(kRequest);
	req_builder.Request().Method(RequestLine::kPost);
	req_builder.Request().Version(kVersion_1_1);

	req_builder.Fields().HeaderFiled(HeaderFields::MakeAcceptAll());
	req_builder.Fields().HeaderFiled(HeaderFields::KStringUserAgent, HeaderFields::KStringMicroMessenger);
	req_builder.Fields().HeaderFiled(HeaderFields::MakeCacheControlNoCache());
	req_builder.Fields().HeaderFiled(HeaderFields::MakeContentTypeOctetStream());
	req_builder.Fields().HeaderFiled(HeaderFields::MakeConnectionClose());

    char len_str[32] = {0};
	snprintf(len_str, sizeof(len_str), "%u", (unsigned int)_body.Length());
	req_builder.Fields().HeaderFiled(HeaderFields::KStringContentLength, len_str);

	for (std::map<std::string, std::string>::const_iterator iter = _headers.begin(); iter != _headers.end(); ++iter) {
		req_builder.Fields().HeaderFiled(iter->first.c_str(), iter->second.c_str());
	}

	req_builder.Request().Url(_url);
	req_builder.HeaderToBuffer(_out_buff);
	_out_buff.Write(_body.Ptr(), _body.Length());
}



