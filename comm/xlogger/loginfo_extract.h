/*
 * loginfo_extract.h
 *
 *  Created on: 2016年10月13日
 *      Author: yanguoyue
 */

#ifndef XLOGGER_LOGINFO_EXTRACT_H_
#define XLOGGER_LOGINFO_EXTRACT_H_

#ifdef __cplusplus
extern "C" {
#endif

const char* ExtractFileName(const char* _path);

void ExtractFunctionName(const char* _func, char* _func_ret, int _len);


#ifdef __cplusplus
}
#endif
#endif /* XLOGGER_LOGINFO_EXTRACT_H_ */
