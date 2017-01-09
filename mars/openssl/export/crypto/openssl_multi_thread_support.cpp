/*
 * openssl_multi_thread_support.cpp
 *
 *  Created on: 2016��6��30��
 *      Author: wutianqiang
 */
#include "openssl_multi_thread_support.h"
#include "openssl/crypto.h"
#include "thread/mutex.h"
#include "comm/xlogger/xlogger.h"
static Mutex** mutex_start = NULL;
static long* lock_count = NULL;

void LockingCallback(int mode, int type, const char* file, int line) {
	static int call_counts = 0;
	if (call_counts++%100 == 0) { //call 100 times, log once
		xinfo2(TSF"call_counts=%_, file=%_, line=%_, type=%_, thread_id=%_, lock_mode=(%_,%_)", call_counts, file, line, type,
				CRYPTO_thread_id(), (mode & CRYPTO_LOCK)?"lock":"unlock", (mode & CRYPTO_READ)?"r":"w");
	}
	if (NULL!=mutex_start && NULL!=lock_count) {
		if (mode & CRYPTO_LOCK) {
			mutex_start[type]->lock();
			lock_count[type]++;
		} else {
			mutex_start[type]->unlock();
		}
	} else {
		xassert2(false);
	}
}

void OpenSSLMultiThreadSetup() {
	//this log will NOT be written in file. OpenSSLMultiThreadSetup() will be called when load the xx.so because of BOOT_RUN_STARTUP,
	//but at that time appender_open() haven't been called yet.
	xinfo_function();
	mutex_start = (Mutex**) OPENSSL_malloc(CRYPTO_num_locks() * sizeof(Mutex*));
	lock_count = (long*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
	if (NULL!=mutex_start && NULL!=lock_count) {
		for (int i=0; i<CRYPTO_num_locks(); ++i) {
			lock_count[i] = 0;
			mutex_start[i] = new Mutex();
		}
	} else {
		xassert2(false);
	}
	//use default implementation to get thread id
	//https://www.openssl.org/docs/man1.0.2/crypto/threads.html
	//CRYPTO_THREADID_set_callback(pthreads_thread_id);
	CRYPTO_set_locking_callback(LockingCallback);
}



void OpenSSLMultiThreadCleanup() {
	xinfo_function();
	CRYPTO_set_locking_callback(NULL);
	for (int i=0; i<CRYPTO_num_locks(); ++i) {
		if(NULL!=mutex_start) {
			delete (mutex_start[i]);
		} else {
			xassert2(false);
		}
	}
	OPENSSL_free(mutex_start);
	mutex_start = NULL;
	OPENSSL_free(lock_count);
	lock_count = NULL;

}

BOOT_RUN_STARTUP(OpenSSLMultiThreadSetup);
BOOT_RUN_EXIT(OpenSSLMultiThreadCleanup);
int export_openssl_mutithread_support() {return 0;}
