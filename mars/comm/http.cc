// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  http.cpp
//  PublicComponent
//
//  Created by liucan on 1/22/14.
//

#include "http.h"

#include <cstddef>
#include <stdlib.h>
#ifdef WIN32
#include <algorithm>
#endif //WIN32
#include "comm/strutil.h"
#include "comm/xlogger/xlogger.h"

namespace http {

static const char* const KStringSpace = " ";
static const char* const KStringCRLF = "\r\n";
static const char* const KStringColon = ":";

bool less::operator()(const std::string& __x, const std::string& __y) const {
    return 0 > strcasecmp(__x.c_str(), __y.c_str());
}

inline char* string_strnstr(const char* src, const char* sfind, int pos1) {
    xassert2(src != NULL && sfind != NULL);

    if (src == NULL || sfind == NULL)
        return NULL;

    int l1, l2;
    l2 = (int)strlen(sfind);

    if (!l2)
        return (char*)src;

    l1 = (int)strnlen(src, pos1);
    pos1 = (pos1 > l1) ? l1 : pos1;

    while (pos1 >= l2) {
        pos1--;

        if (!memcmp(src, sfind, (size_t)l2))
            return (char*)src;

        src++;
    }

    return NULL;
}

static THttpVersion __GetHttpVersion(const std::string& _strVersion) {
    for (size_t i = 0; i < sizeof(kHttpVersionString) / sizeof(kHttpVersionString[0]); ++i) {
        if (0 == strcmp(_strVersion.c_str(), kHttpVersionString[i])) {
            return (THttpVersion)i;
        }
    }

    xerror2(TSF"invalid httpversion:%_", _strVersion);
    return kVersion_Unknow;
}

static bool __ParserHeaders(const std::string& _strHeaders, HeaderFields& _headers) {
    
    std::string::size_type linebegin = 0;
    std::string::size_type lineend = 0;
    
    while ((lineend = _strHeaders.find(KStringCRLF, lineend, strlen(KStringCRLF))) != std::string::npos){
        
        std::string line = _strHeaders.substr(linebegin, lineend - linebegin);
        
        std::string::size_type namebegin = line.find_first_not_of(KStringColon, 0, strlen(KStringColon));
        if (namebegin != std::string::npos){
            std::string::size_type nameend = line.find_first_of(KStringColon, 0, strlen(KStringColon));
            std::string name = line.substr(0, nameend);
            if (line.length() > nameend + strlen(KStringColon)){
                std::string value = line.substr(nameend + strlen(KStringColon));
                
                strutil::Trim(name);
                strutil::Trim(value);
                
                _headers.HeaderFiled(name.c_str(), value.c_str());
            }
        }
        
        lineend += strlen(KStringCRLF);
        linebegin = lineend;
    }
    
    return true;
}

// implement of RequestLine

const char* const RequestLine::kHttpMethodString[kMax] = {
    "UNKNOWN",
    "GET",
    "POST",
    "OPTIONS",
    "HEAD",
    "PUT",
    "DELETE",
    "TRACE",
    "CONNECT"
};

RequestLine::RequestLine(): http_method_(kGet), http_version_(kVersion_1_0) { }

RequestLine::RequestLine(THttpMethod _httpMethod, const char* _url, THttpVersion _httpVersion)
    : http_method_(_httpMethod)
    , req_url_(_url)
    , http_version_(_httpVersion)
{ }

void RequestLine::Method(RequestLine::THttpMethod _method) {
    http_method_ = _method;
}

RequestLine::THttpMethod RequestLine::Method() const {
    return http_method_;
}


void RequestLine::Version(THttpVersion _version) {
    http_version_ = _version;
}

THttpVersion RequestLine::Version() const {
    return http_version_;
}

void RequestLine::Url(const std::string& _url) {
    req_url_ = _url;
}

std::string RequestLine::Url() const {
    return req_url_;
}

std::string RequestLine::ToString() const {
    std::string str;
    str += kHttpMethodString[http_method_];
    str += KStringSpace;
    str += req_url_;
    str += KStringSpace;
    str += kHttpVersionString[http_version_];
    str += KStringCRLF;

    return str;
}

bool RequestLine::FromString(const std::string& _requestline) {
    std::string::size_type pos = _requestline.find(KStringCRLF);

    if (pos == std::string::npos) return false;

    std::string str = _requestline.substr(0, pos);
    std::vector<std::string> strVer;
    strutil::SplitToken(str, KStringSpace, strVer);

    xassert2(strVer.size() >= 3);

    if (strVer.size() < 3) {
//        xerror2(TSF"requestline:%_, strver:%_", _requestline.c_str(), str.c_str());
        xassert2(false, "requestline:%s, strver:%s", _requestline.c_str(), str.c_str());
        return false;
    }

    THttpMethod httpmethod = kUnknown;

    for (size_t i = 0; i < sizeof(kHttpMethodString) / sizeof(kHttpMethodString[0]); ++i) {
        if (0 == strcmp(strVer[0].c_str(), kHttpMethodString[i])) {
            httpmethod = (THttpMethod)i;
            break;
        }
    }

    if (kUnknown == httpmethod) {
        xerror2(TSF"invalid http method:%_", strVer[0].c_str());
        return false;
    }

    http_method_ = httpmethod;
    req_url_ = strVer[1];
    http_version_ = __GetHttpVersion(strVer[2]);

    if (http_version_ == kVersion_Unknow) return false;

    return true;
}


// implement of StatusLine

StatusLine::StatusLine()
    : http_version_(kVersion_1_0)
    , statuscode_(0)
{}

StatusLine::StatusLine(THttpVersion _httpversion, int _statuscode, const std::string& _reasonphrase) {
    http_version_ = _httpversion;
    statuscode_ = _statuscode;
    reason_phrase_ = _reasonphrase;
}

void StatusLine::Version(THttpVersion _version) {
    http_version_ = _version;
}

THttpVersion StatusLine::Version() const {
    return http_version_;
}

void StatusLine::StatusCode(int _statuscode) {
    statuscode_ = _statuscode;
}

int StatusLine::StatusCode() const {
    return statuscode_;
}

void StatusLine::ReasonPhrase(const std::string& _reasonphrase) {
    reason_phrase_ = _reasonphrase;
}

std::string StatusLine::ReasonPhrase() const {
    return reason_phrase_;
}

std::string StatusLine::ToString() const {
    char strStatusCode[16] = {0};
    snprintf(strStatusCode, sizeof(strStatusCode), "%d", statuscode_);

    std::string str;
    str += kHttpVersionString[http_version_];
    str += KStringSpace;
    str += strStatusCode;
    str += KStringSpace;
    str += reason_phrase_;
    str += KStringCRLF;

    return str;
}

bool StatusLine::FromString(const std::string& _statusline) {
    std::string::size_type pos = _statusline.find(KStringCRLF);

    if (pos == std::string::npos) return false;

    std::string str = _statusline.substr(0, pos);
    std::vector<std::string> strVer;
    strutil::SplitToken(str, KStringSpace, strVer);

    xassert2(strVer.size() >= 2);

    if (strVer.size() < 2) {
        xerror2(TSF"strVer.size() < 2, _statusline=%0", _statusline.c_str());
        return false;
    }

    http_version_ = __GetHttpVersion(strVer[0]);

    if (http_version_ == kVersion_Unknow) return false;

    statuscode_ = (int)strtol(strVer[1].c_str(), NULL, 10);

    if (strVer.size() == 3) {
        reason_phrase_ = strVer[2];
    }

    return true;
}



// implement of HeaderFields


const char* const HeaderFields::KStringHost = "Host";
const char* const HeaderFields::KStringAccept = "Accept";
const char* const HeaderFields::KStringUserAgent = "User-Agent";
const char* const HeaderFields::KStringCacheControl = "Cache-Control";
const char* const HeaderFields::KStringConnection = "Connection";
const char* const HeaderFields::kStringProxyConnection = "Proxy-Connection";
const char* const HeaderFields::kStringProxyAuthorization = "Proxy-Authorization";
const char* const HeaderFields::KStringContentType = "Content-Type";
const char* const HeaderFields::KStringContentLength = "Content-Length";
const char* const HeaderFields::KStringTransferEncoding = "Transfer-Encoding";
const char* const HeaderFields::kStringContentEncoding = "Content-Encoding";
const char* const HeaderFields::KStringAcceptEncoding = "Accept-Encoding";
const char* const HeaderFields::KStringContentRange = "Content-Range";
const char* const HeaderFields::KStringMicroMessenger = "MicroMessenger Client";
const char* const HeaderFields::KStringRange = "Range";
const char* const HeaderFields::KStringLocation = "Location";
const char* const HeaderFields::KStringReferer = "Referer";

const char* const KStringChunked = "chunked";
const char* const KStringClose = "close";
const char* const KStringKeepalive = "Keep-Alive";
const char* const KStringAcceptAll = "*/*";
const char* const KStringAcceptEncodingDeflate = "deflate";
const char* const KStringAcceptEncodingGzip = "gzip";
const char* const KStringNoCache = "no-cache";
const char* const KStringOctetType = "application/octet-stream";


std::pair<const std::string, std::string> HeaderFields::MakeContentLength(int _len) {
    char strLength[16] = {0};
    snprintf(strLength, sizeof(strLength), "%d", _len);
    return std::make_pair(KStringContentLength, strLength);
}

std::pair<const std::string, std::string> HeaderFields::MakeTransferEncodingChunked() {
    return std::make_pair(KStringTransferEncoding, KStringChunked);
}

std::pair<const std::string, std::string> HeaderFields::MakeConnectionClose() {
    return std::make_pair(KStringConnection, KStringClose);
}

std::pair<const std::string, std::string> HeaderFields::MakeConnectionKeepalive() {
    return std::make_pair(KStringConnection, KStringKeepalive);
}
std::pair<const std::string, std::string> HeaderFields::MakeAcceptAll() {
    return std::make_pair(KStringAccept, KStringAcceptAll);
}

std::pair<const std::string, std::string> HeaderFields::MakeAcceptEncodingDefalte() {
    return std::make_pair(KStringAcceptEncoding, KStringAcceptEncodingDeflate);
}
std::pair<const std::string, std::string> HeaderFields::MakeAcceptEncodingGzip() {
    return std::make_pair(KStringAcceptEncoding, KStringAcceptEncodingGzip);
}
std::pair<const std::string, std::string> HeaderFields::MakeCacheControlNoCache() {
    return std::make_pair(KStringCacheControl, KStringNoCache);
}

std::pair<const std::string, std::string> HeaderFields::MakeContentTypeOctetStream() {
    return std::make_pair(KStringContentType, KStringOctetType);
}


void HeaderFields::HeaderFiled(const char* _name, const char* _value) {
    headers_.insert(std::pair<const std::string, std::string>(_name, _value));
}

void HeaderFields::HeaderFiled(const std::pair<const std::string, std::string>& _headerfield) {
    headers_.insert(_headerfield);
}

void HeaderFields::HeaderFiled(const http::HeaderFields& _headerfields) {
    headers_.insert(_headerfields.headers_.begin(), _headerfields.headers_.end());
}

const char* HeaderFields::HeaderField(const char* _key) const {
    std::map<const std::string, std::string, less>::const_iterator iter = headers_.find(_key);

    if (iter != headers_.end()) {
        return iter->second.c_str();
    }

    return NULL;
}

bool HeaderFields::IsTransferEncodingChunked() {
    const char* transferEncoding = HeaderField(HeaderFields::KStringTransferEncoding);

    if (transferEncoding && 0 == strcasecmp(transferEncoding, KStringChunked)) return true;

    return false;
}

int HeaderFields::ContentLength() {
    const char* strContentLength = HeaderField(HeaderFields::KStringContentLength);
    int contentLength = 0;

    if (strContentLength) {
        contentLength = (int)strtol(strContentLength, NULL, 10);
    }

    return contentLength;
}


bool HeaderFields::ContentRange(int* start, int* end, int* total) {
    // Content-Range: bytes 0-102400/102399

    *start = 0;
    *end = 0;
    *total = 0;

    const char* strContentRange = HeaderField(HeaderFields::KStringContentRange);

    if (strContentRange) {
        // bytes 0-102400/102399
        std::string range(strContentRange);

        if (!strutil::StartsWith(range, std::string("bytes ")))
            return false;

        std::string bytes = range.substr(6);
        strutil::Trim(bytes);

        size_t range_start = bytes.find("-");

        if (std::string::npos != range_start) {
            std::string startstr = bytes.substr(0, range_start);
            *start = (int)strtol(startstr.c_str(), NULL, 10);

            size_t range_end = bytes.find("/", range_start + 1);

            if (range_end != std::string::npos) {
                std::string endstr = bytes.substr(range_start + 1, range_end - range_start - 1);
                *end = (int)strtol(endstr.c_str(), NULL, 10);


                std::string totalstr = bytes.substr(range_end + 1);
                *total = (int)strtol(totalstr.c_str(), NULL, 10);

                return true;
            }
        }
    }

    return false;
}

const std::string HeaderFields::ToString() const {
    if (headers_.empty()) return "";

    std::string str;

    for (std::map<const std::string, std::string, less>::const_iterator iter = headers_.begin(); iter != headers_.end(); ++iter) {
        str += iter->first + KStringColon + KStringSpace + iter->second + KStringCRLF;
    }

    return str;
}




// implement of IStreamBodyProvider
const char* IStreamBodyProvider::EofData() {
    static const char* eofdata = "0\r\n\r\n";

    return eofdata;
}

void IStreamBodyProvider::AppendHeader(AutoBuffer& _body, size_t _length) {
    char head[16] = {0};
    snprintf(head, sizeof(head), "%x\r\n", (unsigned int)_length);
    _body.Write(head, strlen(head));
}

void IStreamBodyProvider::AppendTail(AutoBuffer& _body) {
    _body.Write(KStringCRLF, strlen(KStringCRLF));
}




// implement of Builder
Builder::Builder(TCsMode _csmode)
    : csmode_(_csmode)
    , blockbody_(NULL)
    , streambody_(NULL)
    , is_manage_body_(false) {}

Builder::~Builder() {
    if (is_manage_body_) {
        if (blockbody_) {
            delete blockbody_;
            blockbody_ = NULL;
        }

        if (streambody_) {
            delete streambody_;
            streambody_ = NULL;
        }
    }
}

RequestLine& Builder::Request() {
    return requestline_;
}

StatusLine& Builder::Status() {
    return statusline_;
}

const RequestLine& Builder::Request() const {
    return requestline_;
}

const StatusLine& Builder::Status() const {
    return statusline_;
}

HeaderFields& Builder::Fields() {
    return headfields_;
}

const HeaderFields& Builder::Fields() const {
    return headfields_;
}

void Builder::BlockBody(IBlockBodyProvider* _body, bool _manage) {
    xassert2(streambody_ == NULL);

    if (NULL != streambody_) {
        xerror2(TSF"setBlockBody, but streambody had set");
        return;
    }


    if (NULL != blockbody_ && _manage) {
        delete blockbody_;
        blockbody_ = NULL;
    }

    blockbody_ = _body;
    is_manage_body_ = _manage;
}

void Builder::StreamBody(IStreamBodyProvider* _body, bool _manage) {
    xassert2(blockbody_ == NULL);

    if (NULL != blockbody_) {
        xerror2(TSF"setStreamBody, but blockbody had set");
        return;
    }

    if (NULL != streambody_ && _manage) {
        delete streambody_;
        streambody_ = NULL;
    }


    streambody_ = _body;
    is_manage_body_ = _manage;
}

IBlockBodyProvider* Builder::BlockBody() {
    return blockbody_;
}

IStreamBodyProvider* Builder::StreamBody() {
    return streambody_;
}

const IBlockBodyProvider* Builder::BlockBody() const {
    return blockbody_;
}

const IStreamBodyProvider* Builder::StreamBody() const {
    return streambody_;
}

bool Builder::HeaderToBuffer(AutoBuffer& _header) {
    std::string firstline;

    if (kRequest == csmode_)
        firstline = requestline_.ToString();
    else
        firstline = statusline_.ToString();

    if (firstline.empty()) return false;

    const std::string strheaders = headfields_.ToString();

    if (strheaders.empty()) return false;

    _header.Write(firstline.c_str(), firstline.size());
    _header.Write(strheaders.c_str(), strheaders.size());
    _header.Write(KStringCRLF, strlen(KStringCRLF));
    return true;
}

bool Builder::HttpToBuffer(AutoBuffer& _http) {

    if (blockbody_) {
        if (blockbody_->Length() > 0) {
            headfields_.MakeContentLength((int)blockbody_->Length());
            if (!HeaderToBuffer(_http) || !blockbody_->FillData(_http)) return false;
        }

    } else if (streambody_) {
    		headfields_.MakeTransferEncodingChunked();
    		if (!HeaderToBuffer(_http)) return false;
        if (streambody_->HaveData()) {
            if (!streambody_->Data(_http)) return false;
        }
    } else {
    		return HeaderToBuffer(_http);
    }

    return true;
}


// implement of Parser
Parser::Parser(BodyReceiver* _body, bool _manage)
    : recvstatus_(kStart)
    , response_header_ready_(false)
    , csmode_(kRespond)
    , headfields_()
    , bodyreceiver_(_body)
    , is_manage_body_(_manage)
    , headerlength_(0){
}

Parser::~Parser() {
    if (is_manage_body_) {
        if (bodyreceiver_) {
            delete bodyreceiver_;
            bodyreceiver_ = NULL;
        }
    }
}

Parser::TRecvStatus Parser::Recv(const void* _buffer, size_t _length) {
    xassert2(_buffer);
    
    if (NULL == _buffer || 0 == _length) {
        xwarn2(TSF"Recv(%_, %_), status:%_", NULL==_buffer?"NULL":_buffer, _length, recvstatus_);
        return recvstatus_;
    }
    
    if (recvstatus_ < kBody && headerbuf_.Length() < 4096 && !response_header_ready_){
        
        headerbuf_.Write(_buffer, std::min(_length, (size_t)4096));
        
        const char* pszbuf = (const char*)headerbuf_.Ptr();
        size_t length = headerbuf_.Length();
    
        if (length > 4){
            char* pos = string_strnstr(pszbuf, "\r\n\r\n", (int)length);
            if (pos != NULL){
                headerbuf_.Length(0, pos - pszbuf + 4);
                response_header_ready_ = true;
            }
        }
    }
    
    recvbuf_.Write(_buffer, _length);
    
    while (true) {
        switch (recvstatus_) {
            case kStart:
            case kFirstLine: {
                char* pBuf = (char*)recvbuf_.Ptr();
                char* pos = string_strnstr(pBuf, KStringCRLF, (int)recvbuf_.Length());
                
                if (NULL == pos && 8 * 1024 < recvbuf_.Length()) {
                    xerror2(TSF"wrong first line 8k buffer no found CRLF");
                    recvstatus_ = kFirstLineError;
                    return recvstatus_;
                }
                
                if (NULL == pos) {
                    recvstatus_ = kFirstLine;
                    return recvstatus_;
                }
                
                ptrdiff_t firstlinelength = pos - pBuf + 2;
                std::string firstline = std::string(pBuf, (size_t)firstlinelength);
                
                bool parseFirstlineSuc = false;
                
                if (strutil::StartsWith(firstline, "HTTP/")) {
                    if (statusline_.FromString(firstline)) {
                        csmode_ = kRespond;
                        parseFirstlineSuc = true;
                    }
                } else {
                    if (requestline_.FromString(firstline)) {
                        csmode_ = kRequest;
                        parseFirstlineSuc = true;
                    }
                }
                
                if (!parseFirstlineSuc) {
                    xerror2(TSF"wrong first line: %0", firstline);
                    recvstatus_ = kFirstLineError;
                    return recvstatus_;
                }
                
                headerbuf_.Write(recvbuf_.Ptr(), firstlinelength);
                // HTTP/1.1 4.7 Unauthorized\r\n\r\n
                char* pos_2crlf = string_strnstr(pBuf, "\r\n\r\n", (int)recvbuf_.Length());
                if (NULL != pos_2crlf && pos_2crlf == pos) {
                    recvstatus_ = kBody;
                    recvbuf_.Move(- (firstlinelength + 2));
                } else {
                    recvstatus_ = kHeaderFields;
                    recvbuf_.Move(- firstlinelength);
                }
            }
                break;
                
            case kHeaderFields: {
                char* pBuf = (char*)recvbuf_.Ptr();
                char* pos = string_strnstr(pBuf, "\r\n\r\n", (int)recvbuf_.Length());
                
                if (NULL == pos && 128 * 1024 < recvbuf_.Length()) {
                    xerror2(TSF"wrong header fields 128k buffer no found CRLFCRLF");
                    recvstatus_ = kHeaderFieldsError;
                    return recvstatus_;
                }
                
                if (NULL == pos) return recvstatus_;
                
                ptrdiff_t headerslength = pos - pBuf + 4;
                std::string headers = std::string(pBuf, (size_t)headerslength);
                
                bool ret = __ParserHeaders(headers, headfields_);
                
                if (false == ret) {
                    recvstatus_ = kHeaderFieldsError;
                    return recvstatus_;
                }
                
                recvstatus_ = kBody;
                headerbuf_.Write(recvbuf_.Ptr(), headerslength);
                recvbuf_.Move(-headerslength);
            }
                break;
                
            case kBody: {
                xassert2(bodyreceiver_);
                
                if (bodyreceiver_) {
                    // chunked
                    
                    if (headfields_.IsTransferEncodingChunked()) {
                        char* chunkSizeBegin = (char*)recvbuf_.Ptr();
                        char* chunkSizeEnd = string_strnstr(chunkSizeBegin, KStringCRLF, (int)recvbuf_.Length());
                        
                        if (NULL == chunkSizeEnd) {
                            return recvstatus_;
                        }
                        
                        std::string strChunkSize = std::string(chunkSizeBegin, chunkSizeEnd);
                        strutil::Trim(strChunkSize);
                        
                        int64_t chunkSize = strtol(strChunkSize.c_str(), NULL, 16);
                        
                        ptrdiff_t sizeLen = chunkSizeEnd - chunkSizeBegin;
                        
                        if (0 != chunkSize) {
                            if ((ptrdiff_t)recvbuf_.Length() < chunkSize + sizeLen + 4)  return recvstatus_;
                            
                            char* chunkBegin = chunkSizeEnd + 2;
                            char* chunkEnd = chunkBegin + chunkSize;
                            
                            if (*chunkEnd != '\r' || *(chunkEnd + 1) != '\n') {
                                recvstatus_ = kBodyError;
                                return recvstatus_;
                            }
                            
                            bodyreceiver_->AppendData(chunkBegin, (size_t)chunkSize);
                            
                            recvbuf_.Move(-(chunkEnd - chunkSizeBegin + 2));
                        } else {  // last chunk
                            char* trailerBegin = chunkSizeEnd + 2;
                            
                            if (recvbuf_.Length() < (unsigned int)(sizeLen + 4)) return recvstatus_;
                            
                            char* trailerEnd = string_strnstr(trailerBegin, KStringCRLF, (int)recvbuf_.Length());
                            
                            if (NULL == trailerEnd)
                                return recvstatus_;
                            
                            recvstatus_ = kEnd;
                            bodyreceiver_->EndData();
                            
                            
                            recvbuf_.Move(-(trailerEnd - chunkSizeBegin + 2));
                        }
                    } else {  // no chunk
                        int contentLength = headfields_.ContentLength();
                        int appendlen = 0;
                        
                        if (int(recvbuf_.Length() + bodyreceiver_->Length()) <= contentLength)
                            appendlen = int(recvbuf_.Length());
                        else
                            appendlen = contentLength - int(bodyreceiver_->Length());
                        
                        
                        bodyreceiver_->AppendData(recvbuf_.Ptr(), (size_t)appendlen);
                        recvbuf_.Move(-appendlen);
                        
                        if ((int)bodyreceiver_->Length() == contentLength) {
                            recvstatus_ = kEnd;
                            bodyreceiver_->EndData();
                            return  recvstatus_;
                        }
                    }
                }
                
                if (0 == recvbuf_.Length())
                    return recvstatus_;
            }
                break;
                
            case kEnd: {
                return recvstatus_;
            }
                break;
                
            default:
                break;
        }
    }
    
    xassert2(false, TSF"status:%_", recvstatus_);
    return recvstatus_;
}

    
Parser::TRecvStatus Parser::Recv(AutoBuffer& _recv_buffer) {

    if (NULL == _recv_buffer.Ptr() || 0 == _recv_buffer.Length()) {
        xwarn2(TSF"Recv(%_, %_), status:%_", _recv_buffer.Ptr() , _recv_buffer.Length(), recvstatus_);
        return recvstatus_;
    }

    if (recvstatus_ < kBody && headerbuf_.Length() < 4096 && !response_header_ready_){
        
        headerbuf_.Write(_recv_buffer.Ptr(), std::min(_recv_buffer.Length(), (size_t)4096));
        
        const char* pszbuf = (const char*)headerbuf_.Ptr();
        size_t length = headerbuf_.Length();
        
        if (length > 4){
            char* pos = string_strnstr(pszbuf, "\r\n\r\n", (int)length);
            if (pos != NULL){
                headerbuf_.Length(0, pos - pszbuf + 4);
                response_header_ready_ = true;
            }
        }
    }

    while (true) {
        switch (recvstatus_) {
        case kStart:
        case kFirstLine: {
            char* pBuf = (char*)_recv_buffer.Ptr();
            char* pos = string_strnstr(pBuf, KStringCRLF, (int)_recv_buffer.Length());

            if (NULL == pos && 8 * 1024 < _recv_buffer.Length()) {
                xerror2(TSF"wrong first line 8k buffer no found CRLF");
                recvstatus_ = kFirstLineError;
                return recvstatus_;
            }

            if (NULL == pos) {
                recvstatus_ = kFirstLine;
                return recvstatus_;
            }

            ptrdiff_t firstlinelength = pos - pBuf + 2;
            std::string firstline = std::string(pBuf, (size_t)firstlinelength);

            bool parseFirstlineSuc = false;

            if (strutil::StartsWith(firstline, "HTTP/")) {
                if (statusline_.FromString(firstline)) {
                    csmode_ = kRespond;
                    parseFirstlineSuc = true;
                }
            } else {
                if (requestline_.FromString(firstline)) {
                    csmode_ = kRequest;
                    parseFirstlineSuc = true;
                }
            }

            if (!parseFirstlineSuc) {
                xerror2(TSF"wrong first line: %0", firstline);
                recvstatus_ = kFirstLineError;
                return recvstatus_;
            }

            recvstatus_ = kHeaderFields;
            _recv_buffer.Move(- firstlinelength);
        }
        break;

        case kHeaderFields: {
            char* pBuf = (char*)_recv_buffer.Ptr();
            char* pos = string_strnstr(pBuf, "\r\n\r\n", (int)_recv_buffer.Length());

            if (NULL == pos && 128 * 1024 < _recv_buffer.Length()) {
                xerror2(TSF"wrong header fields 128k buffer no found CRLFCRLF");
                recvstatus_ = kHeaderFieldsError;
                return recvstatus_;
            }

            if (NULL == pos) return recvstatus_;

            ptrdiff_t headerslength = pos - pBuf + 4;
            std::string headers = std::string(pBuf, (size_t)headerslength);

            bool ret = __ParserHeaders(headers, headfields_);

            if (false == ret) {
                recvstatus_ = kHeaderFieldsError;
                return recvstatus_;
            }

            recvstatus_ = kBody;
            _recv_buffer.Move(-headerslength);
            headerlength_ = headerslength;
        }
        break;

        case kBody: {
            xassert2(bodyreceiver_);

            if (bodyreceiver_) {
                // chunked

                if (headfields_.IsTransferEncodingChunked()) {
                    char* chunkSizeBegin = (char*)_recv_buffer.Ptr();
                    char* chunkSizeEnd = string_strnstr(chunkSizeBegin, KStringCRLF, (int)_recv_buffer.Length());

                    if (NULL == chunkSizeEnd) {
                        return recvstatus_;
                    }

                    std::string strChunkSize = std::string(chunkSizeBegin, chunkSizeEnd);
                    strutil::Trim(strChunkSize);

                    int64_t chunkSize = strtol(strChunkSize.c_str(), NULL, 16);

                    ptrdiff_t sizeLen = chunkSizeEnd - chunkSizeBegin;

                    if (0 != chunkSize) {
                        if ((ptrdiff_t)_recv_buffer.Length() < chunkSize + sizeLen + 4)  return recvstatus_;

                        char* chunkBegin = chunkSizeEnd + 2;
                        char* chunkEnd = chunkBegin + chunkSize;

                        if (*chunkEnd != '\r' || *(chunkEnd + 1) != '\n') {
                            recvstatus_ = kBodyError;
                            return recvstatus_;
                        }

                        bodyreceiver_->AppendData(chunkBegin, (size_t)chunkSize);

                        _recv_buffer.Move(-(chunkEnd - chunkSizeBegin + 2));
                    } else {  // last chunk
                        char* trailerBegin = chunkSizeEnd + 2;

                        if (recvbuf_.Length() < (unsigned int)(sizeLen + 4)) return recvstatus_;

                        char* trailerEnd = string_strnstr(trailerBegin, KStringCRLF, (int)_recv_buffer.Length());

                        if (NULL == trailerEnd)
                            return recvstatus_;

                        recvstatus_ = kEnd;
                        bodyreceiver_->EndData();


                        _recv_buffer.Move(-(trailerEnd - chunkSizeBegin + 2));
                    }
                } else {  // no chunk
                    int contentLength = headfields_.ContentLength();
                    int appendlen = 0;

                    if (int(_recv_buffer.Length() + bodyreceiver_->Length()) <= contentLength)
                        appendlen = int(_recv_buffer.Length());
                    else
                        appendlen = contentLength - int(bodyreceiver_->Length());


                    bodyreceiver_->AppendData(_recv_buffer.Ptr(), (size_t)appendlen);
                    _recv_buffer.Move(-appendlen);

                    if ((int)bodyreceiver_->Length() == contentLength) {
                        recvstatus_ = kEnd;
                        bodyreceiver_->EndData();
                        return  recvstatus_;
                    }
                }
            }

            if (0 == _recv_buffer.Length())
                return recvstatus_;
        }
        break;

        case kEnd: {
            return recvstatus_;
        }
        break;

        default:
            break;
        }
    }

    xassert2(false, TSF"status:%_", recvstatus_);
    return recvstatus_;
}

Parser::TRecvStatus Parser::RecvStatus() const {
    return recvstatus_;

}

TCsMode Parser::CsMode() const {
    return csmode_;
}

const RequestLine& Parser::Request() const {
    return requestline_;
}

const StatusLine& Parser::Status() const {
    return statusline_;
}

HeaderFields& Parser::Fields() {
    return headfields_;
}

const HeaderFields& Parser::Fields() const {
    return headfields_;
}

BodyReceiver& Parser::Body() {
    return *bodyreceiver_;
}

const BodyReceiver& Parser::Body() const {
    return *bodyreceiver_;
}

const AutoBuffer& Parser::HeaderBuffer() const{
    return headerbuf_;
}

bool Parser::FirstLineReady() const {
    return kFirstLineError < recvstatus_;
}

bool Parser::FieldsReady() const {
    return kHeaderFieldsError < recvstatus_;
}
size_t Parser::HeaderLength() const{
    return headerlength_;
}

bool Parser::BodyReady() const {
    return kBodyError < recvstatus_;
}
bool Parser::BodyRecving() const{
    return kBody == recvstatus_;
}

bool Parser::Error() const {
    return kFirstLineError == recvstatus_
           || kHeaderFieldsError == recvstatus_
           || kBodyError == recvstatus_;
}

bool Parser::Success() const {
    return kEnd == recvstatus_;
}

///////////////////////// test code

class TestChunkProvider : public IStreamBodyProvider {
  public:
    TestChunkProvider()
        : m_isEnd(false) {}

    bool HaveData() const {
        return m_buffer.Length() > 0;
    }

    bool Data(AutoBuffer& _body) {
        if (m_buffer.Length() == 0) return false;

        _body.Write(m_buffer.Ptr(), m_buffer.Length());
        m_buffer.Reset();
        return true;
    }

    bool Eof() const {
        return m_isEnd;
    }

    void End() {
        m_isEnd = true;
    }

    void AddData(const char* _buf, size_t _len) {
        AutoBuffer buffer;
        AppendHeader(buffer, _len);
        buffer.Write(_buf, _len);
        AppendTail(buffer);

        m_buffer.Write(buffer.Ptr(), buffer.Length());
    }

  private:
    AutoBuffer m_buffer;
    bool m_isEnd;
};

class TestBodyReceiver : public BodyReceiver {
  public:
    TestBodyReceiver() {}
    ~TestBodyReceiver() {}


    void AppendData(const void* _body, size_t _length) {
        BodyReceiver::AppendData(_body, _length);
        buffer.Write(_body, _length);
    }

    AutoBuffer& Buffer() {
        return buffer;
    }

  private:
    AutoBuffer buffer;
};


}
