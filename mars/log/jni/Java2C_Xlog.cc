// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#include <jni.h>

#include <vector>
#include <string>

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/jni/util/var_cache.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/comm_function.h"

#include "mars/log/appender.h"

#define LONGTHREADID2INT(a) ((a >> 32)^((a & 0xFFFF)))
DEFINE_FIND_CLASS(KXlog, "com/tencent/mars/xlog/Xlog")

extern "C" {

DEFINE_FIND_STATIC_METHOD(KXlog_appenderOpenWithMultipathWithLevel, KXlog, "appenderOpen", "(IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V")
JNIEXPORT void JNICALL Java_com_tencent_mars_xlog_Xlog_appenderOpen
	(JNIEnv *env, jclass, jint level, jint mode, jstring _cache_dir, jstring _log_dir, jstring _nameprefix, jstring _pubkey) {
	if (NULL == _log_dir || NULL == _nameprefix) {
		return;
	}

	std::string cache_dir;
	if (NULL != _cache_dir) {
		ScopedJstring cache_dir_jstr(env, _cache_dir);
		cache_dir = cache_dir_jstr.GetChar();
	}

	const char* pubkey = NULL;
	if (NULL != _pubkey) {
		pubkey = ScopedJstring(env, _pubkey).GetChar();
	}

	ScopedJstring log_dir_jstr(env, _log_dir);
	ScopedJstring nameprefix_jstr(env, _nameprefix);
	appender_open_with_cache((TAppenderMode)mode, cache_dir.c_str(), log_dir_jstr.GetChar(), nameprefix_jstr.GetChar(), pubkey);
	xlogger_SetLevel((TLogLevel)level);

}

JNIEXPORT void JNICALL Java_com_tencent_mars_xlog_Xlog_appenderClose(JNIEnv *env, jobject) {
	appender_close();
}

JNIEXPORT void JNICALL Java_com_tencent_mars_xlog_Xlog_appenderFlush(JNIEnv *env, jobject, jboolean _is_sync) {
	if (_is_sync) {
		appender_flush_sync();
	} else {
		appender_flush();
	}
}

DEFINE_FIND_STATIC_METHOD(KXlog_logWrite, KXlog, "logWrite", "(Lcom/tencent/mars/xlog/Xlog$XLoggerInfo;Ljava/lang/String;)V")
JNIEXPORT void JNICALL Java_com_tencent_mars_xlog_Xlog_logWrite
  (JNIEnv *env, jclass, jobject _log_info, jstring _log) {

	if (NULL == _log_info || NULL == _log) {
		xerror2(TSF"loginfo or log is null");
		return;
	}

	jint level = JNU_GetField(env, _log_info, "level", "I").i;

	if (!xlogger_IsEnabledFor((TLogLevel)level)) {
		return;
	}

	jstring tag = (jstring)JNU_GetField(env, _log_info, "tag", "Ljava/lang/String;").l;
	jstring filename = (jstring)JNU_GetField(env, _log_info, "filename", "Ljava/lang/String;").l;
	jstring funcname = (jstring)JNU_GetField(env, _log_info, "funcname", "Ljava/lang/String;").l;
	jint line = JNU_GetField(env, _log_info, "line", "I").i;
	jlong pid = JNU_GetField(env, _log_info, "pid", "J").i;
	jlong tid = JNU_GetField(env, _log_info, "tid", "J").j;
	jlong maintid = JNU_GetField(env, _log_info, "maintid", "J").j;

	XLoggerInfo xlog_info;
	gettimeofday(&xlog_info.timeval, NULL);
	xlog_info.level = (TLogLevel)level;
	xlog_info.line = line;
	xlog_info.pid = (int)pid;
	xlog_info.tid = LONGTHREADID2INT(tid);
	xlog_info.maintid = LONGTHREADID2INT(maintid);;

	ScopedJstring tag_jstr(env, tag);
	ScopedJstring filename_jstr(env, filename);
	ScopedJstring funcname_jstr(env, funcname);
	ScopedJstring log_jst(env, _log);

	xlog_info.tag = tag_jstr.GetChar();
	xlog_info.filename = filename_jstr.GetChar();
	xlog_info.func_name = funcname_jstr.GetChar();

	xlogger_Write(&xlog_info, log_jst.GetChar());

}

DEFINE_FIND_STATIC_METHOD(KXlog_logWrite2, KXlog, "logWrite2", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;IIJJLjava/lang/String;)V")
JNIEXPORT void JNICALL Java_com_tencent_mars_xlog_Xlog_logWrite2
  (JNIEnv *env, jclass, int _level, jstring _tag, jstring _filename,
		  jstring _funcname, jint _line, jint _pid, jlong _tid, jlong _maintid, jstring _log) {

	if (!xlogger_IsEnabledFor((TLogLevel)_level)) {
		return;
	}

	XLoggerInfo xlog_info;
	gettimeofday(&xlog_info.timeval, NULL);
	xlog_info.level = (TLogLevel)_level;
	xlog_info.line = (int)_line;
	xlog_info.pid = (int)_pid;
	xlog_info.tid = LONGTHREADID2INT(_tid);
	xlog_info.maintid = LONGTHREADID2INT(_maintid);

	const char* tag_cstr = NULL;
	const char* filename_cstr = NULL;
	const char* funcname_cstr = NULL;
	const char* log_cstr = NULL;

	if (NULL != _tag) {
		tag_cstr = env->GetStringUTFChars(_tag, NULL);
	}

	if (NULL != _filename) {
		filename_cstr = env->GetStringUTFChars(_filename, NULL);
	}

	if (NULL != _funcname) {
		funcname_cstr = env->GetStringUTFChars(_funcname, NULL);
	}

	if (NULL != _log) {
		log_cstr = env->GetStringUTFChars(_log, NULL);
	}

	xlog_info.tag = NULL == tag_cstr ? "" : tag_cstr;
	xlog_info.filename = NULL == filename_cstr ? "" : filename_cstr;
	xlog_info.func_name = NULL == funcname_cstr ? "" : funcname_cstr;

	xlogger_Write(&xlog_info, NULL == log_cstr ? "NULL == log" : log_cstr);

	if (NULL != _tag) {
		env->ReleaseStringUTFChars(_tag, tag_cstr);
	}

	if (NULL != _filename) {
		env->ReleaseStringUTFChars(_filename, filename_cstr);
	}

	if (NULL != _funcname) {
		env->ReleaseStringUTFChars(_funcname, funcname_cstr);
	}

	if (NULL != _log) {
		env->ReleaseStringUTFChars(_log, log_cstr);
	}
}

JNIEXPORT jint JNICALL Java_com_tencent_mars_xlog_Xlog_getLogLevel
  (JNIEnv *, jobject) {
	return xlogger_Level();
}

DEFINE_FIND_STATIC_METHOD(KXlog_setLogLevel, KXlog, "setLogLevel", "(I)V")
JNIEXPORT void JNICALL Java_com_tencent_mars_xlog_Xlog_setLogLevel
  (JNIEnv *, jclass, jint _log_level) {
	xlogger_SetLevel((TLogLevel)_log_level);
}

DEFINE_FIND_STATIC_METHOD(KXlog_setAppenderMode, KXlog, "setAppenderMode", "(I)V")
JNIEXPORT void JNICALL Java_com_tencent_mars_xlog_Xlog_setAppenderMode
  (JNIEnv *, jclass, jint _mode) {
	appender_setmode((TAppenderMode)_mode);
}

DEFINE_FIND_STATIC_METHOD(KXlog_setConsoleLogOpen, KXlog, "setConsoleLogOpen", "(Z)V")
JNIEXPORT void JNICALL Java_com_tencent_mars_xlog_Xlog_setConsoleLogOpen
  (JNIEnv *env, jclass, jboolean _is_open) {
	appender_set_console_log((bool)_is_open);
}


}

void ExportXlog() {}
