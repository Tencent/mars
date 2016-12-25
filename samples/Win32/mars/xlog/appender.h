/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * appender.h
 *
 *  Created on: 2013-3-7
 *      Author: yerungui
 */

#ifndef APPENDER_H_
#define APPENDER_H_

#include <string>
#include <vector>

enum TAppenderMode
{
    kAppednerAsync,
    kAppednerSync,
};

void appender_open(TAppenderMode _mode, const char* _dir, const char* _nameprefix);
void appender_open_with_cache(TAppenderMode _mode, const std::string& _cachedir, const std::string& _logdir, const char* _nameprefix);
void appender_flush();
void appender_flush_sync();
void appender_close();
void appender_setmode(TAppenderMode _mode);
bool appender_getfilepath_from_timespan(int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec);
bool appender_geterrfilepath_from_timespan(int _timespan, const char* _prefix, char* _filepath, unsigned int _len);
bool appender_get_current_log_path(char* _log_path, unsigned int _len);
bool appender_get_err_log_path(char* _log_path, unsigned int _len);
void appender_set_console_log(bool _is_open);
void appender_seterrlog(bool _is_open);


#endif /* APPENDER_H_ */
