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
 * baseprj.cpp
 *
 *  Created on: 2014-7-7
 *      Author: yerungui
 */

#include "mars/baseevent/baseprjevent.h"
#include "mars/baseevent/base_logic.h"

#include "mars/comm/bootregister.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/thread/lock.h"
#include "mars/tsmssl/tssl.h"
#include "mars/tsmssl/tssl_crypto.h"


using namespace mars::comm;

namespace mars{
    namespace baseevent{

        void OnCreate()
        {
            GetSignalOnCreate()();
            tssl_X509_NAME_oneline(nullptr,nullptr,0);
            tssl_ASN1_TIME_print(nullptr,nullptr);
            tssl_BIO_new_fd(0, 0);
            tssl_BIO_new_file(nullptr, nullptr);
            tssl_BIO_number_read(nullptr);
            tssl_BIO_number_written(nullptr);
            tssl_BIO_printf(nullptr, nullptr);
            tssl_BIO_puts(nullptr, nullptr);
            tssl_BIO_write(nullptr, nullptr, 0);

            tssl_CRYPTO_free(nullptr, nullptr, 0);
            tssl_EC_curve_nid2nist(0);
            tssl_ERR_print_errors(nullptr);
            tssl_EVP_PKEY_bits(nullptr);
            tssl_OBJ_nid2sn(0);
            tssl_OPENSSL_die(nullptr, nullptr, 0);
            tssl_OPENSSL_sk_num(nullptr);
            tssl_OPENSSL_sk_value(nullptr, 0);
        }
        
        void OnInitBeforeOnCreate(int _encoder_status) {
            GetSignalOnInitBeforeOnCreate()(_encoder_status);
        }
        
        void OnDestroy()
        {
            GetSignalOnDestroy()();
        }
        
        void OnSingalCrash(int _sig)
        {
            GetSignalOnSingalCrash()(_sig);
        }
        
        void OnExceptionCrash()
        {
            GetSignalOnExceptionCrash()();
        }
        
        void OnForeground(bool _isforeground)
        {
            GetSignalOnForeground()(_isforeground);
        }
        
        void OnNetworkChange()
        {
#ifdef __APPLE__
            FlushReachability();
#endif
#ifdef ANDROID
            g_NetInfo = 0;
            
            ScopedLock lock(g_net_mutex);
            g_wifi_info.ssid.clear();
            g_wifi_info.bssid.clear();
            g_sim_info.isp_code.clear();
            g_sim_info.isp_name.clear();
            g_apn_info.nettype = kNoNet -1;
            g_apn_info.sub_nettype = 0;
            g_apn_info.extra_info.clear();
            lock.unlock();
#endif
            GetSignalOnNetworkChange()();
        }
        
        void OnNetworkDataChange(const char* _tag, int32_t _send, int32_t _recv) {
            GetSignalOnNetworkDataChange()(_tag, _send, _recv);
        }

#ifdef ANDROID
        void OnAlarm(int64_t _id) {
            GetSignalOnAlarm()(_id);
        }
#endif
    }
}

