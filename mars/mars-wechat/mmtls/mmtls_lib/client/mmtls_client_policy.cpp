#include "mmtls_client_policy.h"

namespace mmtls {

ClientPolicy& ClientPolicy::GetInstance() {
    static ClientPolicy instance;
    return instance;
}

}  // namespace mmtls
