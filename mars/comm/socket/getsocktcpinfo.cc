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
 * getsocktcpinfo.c
 *
 *  Created on: 2012-9-28
 *      Author: yerungui
 */


#include "getsocktcpinfo.h"

#include <sys/socket.h>

#include <string.h>

#include "mars/comm/assert/__assert.h"
#include "mars/comm/xlogger/xlogger.h"

int getsocktcpinfo(int _sockfd, struct tcp_info* _info)
{
#if defined(__APPLE__) || defined(ANDROID)
	ASSERT(_info);
	int length = sizeof(struct tcp_info);
	return getsockopt( _sockfd, IPPROTO_TCP, TCP_INFO, (void *)_info, (socklen_t *)&length);
#else
    return -1;
#endif
}

char* tcpinfo2str(struct tcp_info* _info, char* _info_str_buf, size_t _buf_len) {
    XMessage ss;
    memset(_info_str_buf, 0, _buf_len);
#ifdef __APPLE__
    ss << "tcpi_state=0x"                  <<  string_cast_hex((uint32_t)_info->tcpi_state)
    << "， tcpi_snd_wscale=0x"              << string_cast_hex((uint32_t)_info->tcpi_snd_wscale)
    << "， tcpi_rcv_wscale=0x"              << string_cast_hex((uint32_t)_info->tcpi_rcv_wscale)
    << "， tcpi_options=0x"                 << string_cast_hex(_info->tcpi_options)
    << "， tcpi_flags=0x"                   << string_cast_hex(_info->tcpi_flags)
    << "， tcpi_rto=0x"                     << string_cast_hex(_info->tcpi_rto)
    << "， tcpi_maxseg=0x"                  << string_cast_hex(_info->tcpi_maxseg)
    << "， tcpi_snd_ssthresh=0x"            << string_cast_hex(_info->tcpi_snd_ssthresh)
    << "， tcpi_snd_cwnd=0x"                << string_cast_hex(_info->tcpi_snd_cwnd)
    << "， tcpi_snd_wnd=0x"                 << string_cast_hex(_info->tcpi_snd_wnd)
    << "， tcpi_snd_sbbytes=0x"             << string_cast_hex(_info->tcpi_snd_sbbytes)
    << "， tcpi_rcv_wnd=0x"                 << string_cast_hex(_info->tcpi_rcv_wnd)
    << "， tcpi_rttcur=0x"                  << string_cast_hex(_info->tcpi_rttcur)
    << "， tcpi_srtt=0x"                    << string_cast_hex(_info->tcpi_srtt)
    << "， tcpi_rttvar=0x"                  << string_cast_hex(_info->tcpi_rttvar)
    << "， tcpi_txpackets=0x"               << string_cast_hex(_info->tcpi_txpackets)
    << "， tcpi_txbytes=0x"                 << string_cast_hex(_info->tcpi_txbytes)
    << "， tcpi_txretransmitbytes=0x"       << string_cast_hex(_info->tcpi_txretransmitbytes)
    << "， tcpi_rxpackets=0x"               << string_cast_hex(_info->tcpi_rxpackets)
    << "， tcpi_rxbytes=0x"                 << string_cast_hex(_info->tcpi_rxbytes)
    << "， tcpi_rxoutoforderbytes=0x"       << string_cast_hex(_info->tcpi_rxoutoforderbytes);

#elif defined(ANDROID)
    ss << "tcpi_state=0x"                  << string_cast_hex((uint32_t)_info->tcpi_state)
    << "， tcpi_ca_state=0x"                << string_cast_hex((uint32_t)_info->tcpi_ca_state)
    << "， tcpi_retransmits=0x"             << string_cast_hex((uint32_t)_info->tcpi_retransmits)
    << "， tcpi_probes=0x"                  << string_cast_hex((uint32_t)_info->tcpi_probes)
    << "， tcpi_backoff=0x"                 << string_cast_hex((uint32_t)_info->tcpi_backoff)
    << "， tcpi_options=0x"                 << string_cast_hex((uint32_t)_info->tcpi_options)
    << "， tcpi_rto=0x"                     << string_cast_hex(_info->tcpi_rto)
    << "， tcpi_snd_mss=0x"                 << string_cast_hex(_info->tcpi_snd_mss)
    << "， tcpi_rcv_mss=0x"                 << string_cast_hex(_info->tcpi_rcv_mss)
    << "， tcpi_unacked=0x"                 << string_cast_hex(_info->tcpi_unacked)
    << "， tcpi_sacked=0x"                  << string_cast_hex(_info->tcpi_sacked)
    << "， tcpi_lost=0x"                    << string_cast_hex(_info->tcpi_lost)
    << "， tcpi_retrans=0x"                 << string_cast_hex(_info->tcpi_retrans)
    << "， tcpi_fackets=0x"                 << string_cast_hex(_info->tcpi_fackets)
    << "， tcpi_last_data_sent=0x"          << string_cast_hex(_info->tcpi_last_data_sent)
    << "， tcpi_last_ack_sent=0x"           << string_cast_hex(_info->tcpi_last_ack_sent)
    << "， tcpi_last_data_recv=0x"          << string_cast_hex(_info->tcpi_last_data_recv)
    << "， tcpi_last_ack_recv=0x"           << string_cast_hex(_info->tcpi_last_ack_recv)
    << "， tcpi_pmtu=0x"                    << string_cast_hex(_info->tcpi_pmtu)
    << "， tcpi_rcv_ssthresh=0x"            << string_cast_hex(_info->tcpi_rcv_ssthresh)
    << "， tcpi_rtt=0x"                     << string_cast_hex(_info->tcpi_rtt)
    << "， tcpi_rttvar=0x"                  << string_cast_hex(_info->tcpi_rttvar)
    << "， tcpi_snd_ssthresh=0x"            << string_cast_hex(_info->tcpi_snd_ssthresh)
    << "， tcpi_snd_cwnd=0x"                << string_cast_hex(_info->tcpi_snd_cwnd)
    << "， tcpi_advmss=0x"                  << string_cast_hex(_info->tcpi_advmss)
    << "， tcpi_reordering=0x"              << string_cast_hex(_info->tcpi_reordering)
    << "， tcpi_rcv_rtt=0x"                 << string_cast_hex(_info->tcpi_rcv_rtt)
    << "， tcpi_rcv_space=0x"               << string_cast_hex(_info->tcpi_rcv_space)
    << "， tcpi_total_retrans=0x"           << string_cast_hex(_info->tcpi_total_retrans);
#endif
    if(ss.String().size()>_buf_len) {
        //buf overflow
        ASSERT(false);
    }
    memcpy(_info_str_buf, ss.String().c_str(), (ss.String().size()>_buf_len?_buf_len:ss.String().size())-1 );
    return _info_str_buf;
}
