#pragma once

#include <string>

#include "mmtls_client_handshake_state.h"
#include "mmtls_stat.h"
#include "mmtls_string.h"

namespace mmtls {
class ClientStat : public Stat {
 public:
    ClientStat(const ClientHandShakeState* state)
    : state_(state)
    , access_psk_cnt_(0)
    , refresh_psk_cnt_(0)
    , new_access_psk_cnt_(0)
    , new_refresh_psk_cnt_(0)
    , delete_access_psk_cnt_(0)
    , get_access_psk_fail_(0)
    , get_refresh_psk_fail_(0)
    , no_mmtls_(false) {
    }

    virtual ~ClientStat() {
    }
    mmtls::String ToString();
    HandShakeMode handshake_mode() const {
        return state()->mode();
    }

 protected:
    virtual const HandShakeState* state() const {
        return state_;
    }
    ClientStat(const ClientStat&);
    ClientStat& operator=(const ClientStat&);

 private:
    const ClientHandShakeState* state_;

    DECLAR_ADD_ELEMENT(uint32, access_psk_cnt);
    DECLAR_ADD_ELEMENT(uint32, refresh_psk_cnt);
    DECLAR_ADD_ELEMENT(uint32, new_access_psk_cnt);
    DECLAR_ADD_ELEMENT(uint32, new_refresh_psk_cnt);
    DECLAR_ADD_ELEMENT(uint32, delete_access_psk_cnt);
    DECLAR_INC_ELEMENT(uint32, get_access_psk_fail);
    DECLAR_INC_ELEMENT(uint32, get_refresh_psk_fail);
    DECLAR_SET_ELEMENT(bool, no_mmtls);
};

}  // namespace mmtls
