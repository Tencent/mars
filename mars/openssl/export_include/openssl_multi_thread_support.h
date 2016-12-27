/*
 * openssl_multi_thread_support.h
 *
 *  Created on: 2016��6��30��
 *      Author: wutianqiang
 */

#ifndef EXPORT_INCLUDE_OPENSSL_MULTI_THREAD_SUPPORT_H_
#define EXPORT_INCLUDE_OPENSSL_MULTI_THREAD_SUPPORT_H_
 #include "mars/comm/bootrun.h"
void OpenSSLMultiThreadSetup();
void OpenSSLMultiThreadCleanup();

int export_openssl_mutithread_support();


#endif /* EXPORT_INCLUDE_OPENSSL_MULTI_THREAD_SUPPORT_H_ */
