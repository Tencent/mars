//
//  log_base_buffer.h
//  app
//
//  Created by 黄伟烽 on 2020/2/28.
//

#ifndef log_base_buffer_h
#define log_base_buffer_h

#include <zlib.h>
#include <string>
#include <stdint.h>

#include "mars/comm/ptrbuffer.h"
#include "mars/comm/autobuffer.h"

class LogCrypt;

class LogBaseBuffer {
public:
    LogBaseBuffer(void* _pbuffer, size_t _len, bool _is_compress, const char* _pubkey);
    int extracted();

    virtual ~LogBaseBuffer();

public:
    static bool GetPeriodLogs(const char* _log_path, int _begin_hour, int _end_hour, unsigned long& _begin_pos, unsigned long& _end_pos, std::string& _err_msg);

public:
    virtual PtrBuffer& GetData() = 0;

    virtual void Flush(AutoBuffer& _buff) = 0;
    bool Write(const void* _data, size_t _inputlen, AutoBuffer& _out_buff) ;
    virtual bool Write(const void* _data, size_t _length)   = 0;

protected:

    bool __Reset();
    void __Flush();
    void __Clear();
    void __Fix();

protected:
    PtrBuffer buff_;
    bool is_compress_;

    class LogCrypt* log_crypt_;
    size_t remain_nocrypt_len_;

};


#endif /* log_base_buffer_h */
