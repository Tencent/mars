#include "mmtls_client_stat.h"

namespace mmtls {

mmtls::String ClientStat::ToString() {
    std::stringstream ss;
    ss << mode() << ',' << stage() << ',' << alert() << ',' << init_time() << ',' << active_time() << ','
       << close_time() << ',' <<
        // send/receive record size/count
        send_data_size() << ',' << receive_data_size() << ',' << send_data_cnt() << ',' << receive_data_cnt() << ','
       << send_handshake_data_size() << ',' << receive_handshake_data_size() << ',' << send_handshake_data_cnt() << ','
       << receive_handshake_data_cnt() << ',' << send_early_data_size() << ',' << receive_early_data_size() << ','
       << send_early_data_cnt() << ',' << receive_early_data_cnt() << ',' <<
        // psk
        access_psk_cnt() << ',' << refresh_psk_cnt() << ',' << new_access_psk_cnt() << ',' << new_refresh_psk_cnt()
       << ',' << delete_access_psk_cnt() << ',' << get_access_psk_fail() << ',' << get_refresh_psk_fail();

    return mmtls::String(ss.str());
}

}  // namespace mmtls
