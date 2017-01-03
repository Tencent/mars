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
 * HttpRequest.h
 *
 *  Created on: 2013-12-5
 *      Author: yerungui
 */

#ifndef HTTP_H_
#define HTTP_H_

#include <string>
#include <map>

#include "autobuffer.h"

namespace http {

struct less {
    bool operator()(const std::string& __x, const std::string& __y) const
    { return 0 > strcasecmp(__x.c_str(), __y.c_str()); }
};

enum THttpVersion {
    kVersion_0_9,
    kVersion_1_0,
    kVersion_1_1,
    kVersion_2_0,
    kVersion_Unknow,
};

const char* const kHttpVersionString[] = {
    "HTTP/0.9",
    "HTTP/1.0",
    "HTTP/1.1",
    "HTTP/2",
    "version_unknown",
};

enum TCsMode {
    kRequest,
    kRespond,
};

class RequestLine {
  public:
    enum THttpMethod {
        kUnknown = 0,
        kGet,
        kPost,
        kOptions,
        kHead,
        kPut,
        kDelete,
        kTrace,
        kConnect,
        kMax,
    };

    static const char* const kHttpMethodString[kMax];

  public:
    RequestLine();
    RequestLine(THttpMethod _httpMethod, const char* _url, THttpVersion _httpVersion);
    // RequestLine(const RequestLine&);
    // RequestLine& operator=(const RequestLine&);

  public:
    void Method(THttpMethod _method);
    THttpMethod Method() const;

    void Version(THttpVersion _version);
    THttpVersion Version() const;

    void Url(const std::string& _url);
    std::string Url() const;

    std::string ToString() const;
    bool FromString(const std::string& _requestline);

  private:
    THttpMethod http_method_;
    std::string req_url_;
    THttpVersion http_version_;
};

class StatusLine {
  public:
    StatusLine();
    StatusLine(THttpVersion _httpversion, int _statuscode, const std::string& _reasonphrase);
    // StatusLine(const StatusLine&);
    // StatusLine& operator=(const StatusLine&);

  public:
    void Version(THttpVersion _version);
    THttpVersion Version() const;

    void StatusCode(int _statuscode);
    int StatusCode() const;

    void ReasonPhrase(const std::string& _reasonphrase);
    std::string ReasonPhrase() const;

    std::string ToString() const;
    bool FromString(const std::string& _statusline);

  private:
    THttpVersion http_version_;
    int statuscode_;
    std::string reason_phrase_;
};


class HeaderFields {
  public:
    // HeaderFields(const HeaderFields&);
    // HeaderFields& operator=(const HeaderFields&);

  public:
    static std::pair<const std::string, std::string> MakeContentLength(int _len);
    static std::pair<const std::string, std::string> MakeTransferEncodingChunked();
    static std::pair<const std::string, std::string> MakeConnectionClose();
    static std::pair<const std::string, std::string> MakeConnectionKeepalive();
    static std::pair<const std::string, std::string> MakeAcceptAll();
    static std::pair<const std::string, std::string> MakeAcceptEncodingDefalte();
    static std::pair<const std::string, std::string> MakeCacheControlNoCache();
    static std::pair<const std::string, std::string> MakeContentTypeOctetStream();

    static const char* const KStringHost;
    static const char* const KStringAccept;
    static const char* const KStringUserAgent;
    static const char* const KStringCacheControl;
    static const char* const KStringConnection;
    static const char* const KStringContentType;
    static const char* const KStringContentLength;
    static const char* const KStringTransferEncoding;
    static const char* const kStringContentEncoding;
    static const char* const KStringAcceptEncoding;
    static const char* const KStringContentRange;
    static const char* const KStringMicroMessenger;
    static const char* const KStringRange;
    static const char* const KStringLocation;
    static const char* const KStringReferer;

    void HeaderFiled(const char* _name, const char* _value);
    void HeaderFiled(const std::pair<const std::string, std::string>& _headerfield);
    void HeaderFiled(const HeaderFields& _headerfields);
    const char* HeaderField(const char* _key) const;
    std::map<const std::string, std::string, less>& GetHeaders() {return headers_;}

    bool IsTransferEncodingChunked();
    int ContentLength();

    bool ContentRange(int* start, int* end, int* total);

    const std::string ToStrig() const;

  private:
    std::map<const std::string, std::string, less> headers_;
};

class IBlockBodyProvider {
  public:
    virtual ~IBlockBodyProvider() {}

    virtual bool Data(AutoBuffer& _body) = 0;
    virtual bool FillData(AutoBuffer& _body) = 0;
    virtual size_t Length() const = 0;
};

class BufferBodyProvider : public IBlockBodyProvider {
  public:
    bool Data(AutoBuffer& _body) {
        if (!buffer_.Ptr()) return false;

        _body.Write(buffer_.Ptr(), buffer_.Length());
        buffer_.Reset();
        return true;
    }
    bool FillData(AutoBuffer& _body) {
        if (!buffer_.Ptr()) return false;

        _body.Write(buffer_.Ptr(), buffer_.Length());
        buffer_.Reset();
        return true;
    }
    size_t Length() const {return buffer_.Length();}
    AutoBuffer& Buffer() {return buffer_;}
  private:
    AutoBuffer buffer_;
};

class IStreamBodyProvider {
  public:
    virtual ~IStreamBodyProvider() {}

    virtual bool HaveData() const = 0;
    virtual bool Data(AutoBuffer& _body) = 0;

    virtual bool Eof() const = 0;
    const char* EofData();

  protected:
    static void AppendHeader(AutoBuffer& _body, size_t _length);
    static void AppendTail(AutoBuffer& _body);
};

class Builder {
  public:
    Builder(TCsMode _csmode);
    ~Builder();

  private:
    Builder(const Builder&);
    Builder& operator=(const Builder&);

  public:
    RequestLine& Request();
    StatusLine& Status();
    const RequestLine& Request() const;
    const StatusLine& Status() const;

    HeaderFields& Fields();
    const HeaderFields& Fields() const;

    void BlockBody(IBlockBodyProvider* _body, bool _manage);
    void StreamBody(IStreamBodyProvider* _body, bool _manage);
    IBlockBodyProvider* BlockBody();
    IStreamBodyProvider* StreamBody();
    const IBlockBodyProvider* BlockBody() const;
    const IStreamBodyProvider* StreamBody() const;

    bool HeaderToBuffer(AutoBuffer& _header);
    bool HttpToBuffer(AutoBuffer& _http);

  private:
    TCsMode csmode_;

    StatusLine statusline_;
    RequestLine requestline_;

    HeaderFields headfields_;

    IBlockBodyProvider* blockbody_;
    IStreamBodyProvider* streambody_;
    bool is_manage_body_;
};

class BodyReceiver {
  public:
    BodyReceiver(): total_length_(0) {}
    virtual ~BodyReceiver() {}

    virtual void AppendData(const void* _body, size_t _length) { total_length_ += _length;}
    virtual void EndData() {}
    size_t Length() const {return total_length_;}

  private:
    size_t total_length_;
};

class MemoryBodyReceiver : public BodyReceiver {
  public:
	MemoryBodyReceiver(AutoBuffer& _buf)
    : body_(_buf) {}
    virtual void AppendData(const void* _body, size_t _length) {
        BodyReceiver::AppendData(_body, _length);
        body_.Write(_body, _length);
    }
    virtual void EndData() {}

  private:
    AutoBuffer& body_;
};

class Parser {
  public:
    enum TRecvStatus {
        kStart,
        kFirstLine,
        kFirstLineError,
        kHeaderFields,
        kHeaderFieldsError,
        kBody,
        kBodyError,
        kEnd,
    };

  public:
    Parser(BodyReceiver* _body = new BodyReceiver(), bool _manage = true);
    ~Parser();

  private:
    Parser(const Parser&);
    Parser& operator=(const Parser&);

  public:
    TRecvStatus Recv(const void* _buffer, size_t _length);
    TRecvStatus Recv(AutoBuffer& _recv_buffer);
    TRecvStatus RecvStatus() const;

    TCsMode CsMode() const;
    bool FirstLineReady() const;
    const RequestLine& Request() const;
    const StatusLine& Status() const;

    bool FieldsReady() const;
    HeaderFields& Fields();
    const HeaderFields& Fields() const;
    size_t HeaderLength() const;

    bool BodyReady() const;
    bool BodyRecving() const;
    BodyReceiver& Body();
    const BodyReceiver& Body() const;

    bool Error() const;
    bool Success() const;

  private:
    TRecvStatus recvstatus_;
    AutoBuffer    recvbuf_;
    TCsMode csmode_;

    StatusLine statusline_;
    RequestLine requestline_;

    HeaderFields headfields_;

    BodyReceiver* bodyreceiver_;
    bool is_manage_body_;
    size_t headerlength_;
};

// void testChunk();


} /* namespace http */
#endif /* HTTPREQUEST_H_ */
