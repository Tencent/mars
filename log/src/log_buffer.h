/*
 * log_buffer.h
 *
 *  Created on: 2015-7-28
 *      Author: yanguoyue
 */

#ifndef LOGBUFFER_H_
#define LOGBUFFER_H_

#include <zlib.h>
#include <string>
#include <stdint.h>

#include "mars/comm/ptrbuffer.h"
#include "mars/comm/autobuffer.h"

class LogCrypt;

class LogBuffer {
public:
    LogBuffer(void* _pbuffer, size_t _len, bool _is_compress);
    ~LogBuffer();
    
public:
    static bool GetPeriodLogs(const char* _log_path, int _begin_hour, int _end_hour, unsigned long& _begin_pos, unsigned long& _end_pos, std::string& _err_msg);
    static bool Write(const void* _data, size_t _inputlen, void* _output, size_t& _len);

public:
    PtrBuffer& GetData();
    

    void Flush(AutoBuffer& _buff);
    bool Write(const void* _data, size_t _length);

private:
    
    bool __Reset();
    void __Flush();
    void __Clear();
    
    void __Fix();

private:
    PtrBuffer buff_;
    bool is_compress_;
    z_stream cstream_;
    
    static class LogCrypt* s_log_crypt;

};


#endif /* LOGBUFFER_H_ */
