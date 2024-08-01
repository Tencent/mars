
#include "mmtls_client_credential_manager.h"

#include "mars/app/app_manager.h"
#include "mars/comm/macro.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mmtls_client_credential_storage.h"

using namespace mars::app;
using namespace mars::comm;
namespace mmtls {

ClientCredentialManager& ClientCredentialManager::GetInstance(mars::boot::Context* _context, const std::string& _name) {
    NO_DESTROY static std::map<std::string, ClientCredentialManager*> instance;
    NO_DESTROY static Mutex s_mutex;
    ScopedLock lock(s_mutex);
    if (instance.find(_name) == instance.end()) {
        instance[_name] = new ClientCredentialManager(_context, _name);
    }

    return *instance[_name];
}

ClientCredentialManager::ClientCredentialManager(mars::boot::Context* _context, const std::string& _name) {
    std::string name = _name.empty() ? "default" : _name;
    std::string dir = (name == "default") ? (_context->GetManager<AppManager>()->GetAppFilePath() + "/")
                                          : (_context->GetManager<AppManager>()->GetAppFilePath() + "/" + _name + "/");
    client_storage_ = new mmtls::ClientCredStorage(_context, dir, _name);
    client_storage_->Init();
    xinfo2(TSF"credential name: %_", _name);
}

ClientCredentialManager::~ClientCredentialManager() {
    delete client_storage_;
}

int ClientCredentialManager::SavePsk(const ClientPsk& psk) {
    // TODO: implement it.
    PskType type = psk.psk()->type();
    xdebug2(TSF "type=%_, save pre_shared_key=%_", type, SafeStrToHex(psk.pre_shared_key()).c_str());
    DataWriter writer;
    ClientPsk psk_copy = psk;
    int ret = psk_copy.Serialize(writer);
    if (ret != OK) {
        xerror2(TSF "Serialize failed.");
        return -1;
    }
    return client_storage_->SavePsk(type, writer.Buffer());
}

int ClientCredentialManager::GetValidPsk(PskType type, ClientPsk** psk) {
    // TODO: implement it.
    mmtls::String serialized_client_psk;
    if (OK == client_storage_->GetPsk(type, serialized_client_psk)) {
        xinfo2(TSF "mmtls: psktype=%_, len=%_" /*, serialized_client_psk=%_"*/,
               type,
               serialized_client_psk.size() /*, StrToHex(serialized_client_psk).c_str()*/);
        DataReader reader(serialized_client_psk);
        *psk = new ClientPsk();  // caller free (*psk)
        if (NULL != (*psk)) {
            (*psk)->Clear();
            (*psk)->Init(reader);
            if ((PSK_ACCESS == type)
                && (uint64)time(NULL) > (*psk)->expired_time()) {  // only access_psk check the expired_time
                xdebug2(TSF "mmtls: timeNow=%_, expired_time=%_.", time(NULL), (*psk)->expired_time());
                return ERR_PSK_EXPIRED;
            }
            return OK;
        }
        return ERR_UNEXPECT_CHECK_FAIL;
    } else {
        return ERR_PSK_EMPTY;
    }
}

int ClientCredentialManager::GetStaticEcdhKeyPairs(std::vector<KeyPair>& key_pairs) {
    return client_storage_->GetStaticEcdhKeyPairs(key_pairs);
}

int ClientCredentialManager::GetStaticEcdhKeyPair(uint32 version, KeyPair& key_pair) {
    return client_storage_->GetStaticEcdhKeyPair(version, key_pair);
}

int ClientCredentialManager::GetEcdsaKeyPairs(std::vector<KeyPair>& key_pairs) {
    return client_storage_->GetEcdsaKeyPairs(key_pairs);
}

int ClientCredentialManager::GetEcdsaKeyPair(uint32 version, KeyPair& key_pair) {
    return client_storage_->GetEcdsaKeyPair(version, key_pair);
}
int ClientCredentialManager::DeletePsk(PskType type) {
    xinfo2(TSF "DeletePsk type=%_", type);
    return client_storage_->DeletePsk(type);
}

NO_DESTROY static std::map<std::string, NewMMTlsKey> tls_keys_;

bool ClientCredentialManager::AddKey(const std::string& _group, const NewMMTlsKey& _key) {
    if (tls_keys_.find(_group) != tls_keys_.end())
        return false;
    tls_keys_[_group] = _key;
    return true;
}

const NewMMTlsKey& ClientCredentialManager::TlsKey(const std::string& _group) {
    return tls_keys_[_group];
}

}  // namespace mmtls
