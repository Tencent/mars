/*
 * mmtls_client_credential_storage.cpp
 *
 *  Created on: 2015��11��16��
 *      Author: elviswu
 */

#include "mmtls_client_credential_storage.h"

#include <string>

#include "mars/app/app.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mmtls_aead_crypter_aes_gcm.h"
#include "mmtls_client_static_keys_util.h"
#include "mmtls_log.h"
#include "openssl/export_include/aes_crypt.h"
#include "openssl/md5.h"
// #include "mmtls_client_static_keys.h"
#include "mars/app/app_manager.h"

using namespace mars::comm;
using namespace mars::boot;
using namespace mars::app;

namespace mmtls {

// ClientCredStorage* ClientCredStorage::s_Instance = NULL;
// Mutex* ClientCredStorage::s_mutex = (new Mutex());

static mmtls::String GenerateSaveFileKey(const std::string& _devicename) {
    mmtls::String device_info(_devicename);  // mars::app::GetDeviceInfo().devicename
    MMTLSLOG_IMPT("device_info=%s.", device_info.c_str());
    unsigned char sig[16] = {0};
    MD5((const unsigned char*)device_info.c_str(), device_info.size(), sig);

    std::string des = strutil::MD5DigestToBase16(sig);

    mmtls::String key(des.data(), 16);
    MMTLSLOG_DEBUG("device_info md5=%s. key=%s", des.c_str(), key.c_str());  // TODO DELETE LOG

    return key;
}
ClientCredStorage::ClientCredStorage(Context* _context, const std::string& _dir, const std::string& _group)
: context_(_context), member_mutex(new Mutex()) {
    group_ = _group;
    if (_dir.empty()) {
        key_dir_ = context_->GetManager<AppManager>()->GetAppFilePath() + "/";
    } else {
        key_dir_ = _dir;
    }
    MMTLSLOG_IMPT("new ClientCredStorage. key_dir_=%s.", key_dir_.c_str());
}
ClientCredStorage::~ClientCredStorage() {
    if (member_mutex != NULL)
        delete member_mutex, member_mutex = NULL;
}

int ClientCredStorage::SavePsk(PskType type, const mmtls::String& serialized_client_psk) {
    ScopedLock member_lock(*member_mutex);
    int retcode = OK;
    if (PSK_ACCESS == type) {
        // only store in memory
        serialized_acess_psk_ = serialized_client_psk;
    } else if (PSK_REFRESH == type) {
        serialized_refresh_psk_ = serialized_client_psk;
        SaveRefreshPskToFile();
    } else {
        MMTLSLOG_ERR("psk_type is error. type=%d.", type);
        retcode = ERR_PSK_INVALID_TYPE;
    }
    MMTLSLOG_IMPT("PskType=%d, serialized_client_psk length = %zu.", type, serialized_client_psk.size());
    return retcode;
}

int ClientCredStorage::GetPsk(PskType type, mmtls::String& serialized_client_psk) {
    ScopedLock member_lock(*member_mutex);
    int retcode = OK;
    if (PSK_ACCESS == type) {
        if (!serialized_acess_psk_.empty()) {
            serialized_client_psk = serialized_acess_psk_;
        } else {
            MMTLSLOG_ERR("serialized_acess_psk_ is empty.");
            retcode = ERR_PSK_EMPTY;
        }
    } else if (PSK_REFRESH == type) {
        if (!serialized_refresh_psk_.empty()) {
            serialized_client_psk = serialized_refresh_psk_;
        } else {
            MMTLSLOG_ERR("serialized_refresh_psk_ is empty.");
            retcode = ERR_PSK_EMPTY;
        }
    } else {
        MMTLSLOG_ERR("psk_type is error. type=%d.", type);
        retcode = ERR_PSK_EMPTY;
    }
    return retcode;
}

int ClientCredStorage::DeletePsk(PskType type) {
    ScopedLock member_lock(*member_mutex);
    int retcode = 0;
    if (PSK_ACCESS == type) {
        serialized_acess_psk_.clear();
    } else if (PSK_REFRESH == type) {
        serialized_refresh_psk_.clear();
        retcode = SaveRefreshPskToFile(true);

    } else {
        MMTLSLOG_ERR("psk_type is error. type=%d.", type);
        retcode = -2;
    }
    return retcode;
}

int ClientCredStorage::GetStaticEcdhKeyPairs(std::vector<KeyPair>& key_pairs) {
    ScopedLock member_lock(*member_mutex);
    key_pairs.clear();

    std::map<uint32, KeyPair>::const_iterator iter = ecdh_keys_.keys_.begin();

    for (; iter != ecdh_keys_.keys_.end(); ++iter) {
        key_pairs.push_back(iter->second);
    }

    return OK;
}

int ClientCredStorage::GetStaticEcdhKeyPair(uint32 version, KeyPair& key_pair) {
    ScopedLock member_lock(*member_mutex);
    std::map<uint32, KeyPair>::const_iterator iter = ecdh_keys_.keys_.find(version);

    if (iter == ecdh_keys_.keys_.end()) {
        return ERR_KEY_NOT_EXIST;
    }

    key_pair = iter->second;

    return OK;
}

int ClientCredStorage::GetEcdsaKeyPairs(std::vector<KeyPair>& key_pairs) {
    ScopedLock member_lock(*member_mutex);
    key_pairs.clear();

    std::map<uint32, KeyPair>::const_iterator iter = ecdsa_keys_.keys_.begin();

    for (; iter != ecdsa_keys_.keys_.end(); ++iter) {
        key_pairs.push_back(iter->second);
    }

    return OK;
}

int ClientCredStorage::GetEcdsaKeyPair(uint32 version, KeyPair& key_pair) {
    ScopedLock member_lock(*member_mutex);
    std::map<uint32, KeyPair>::const_iterator iter = ecdsa_keys_.keys_.find(version);

    if (iter == ecdsa_keys_.keys_.end()) {
        return ERR_KEY_NOT_EXIST;
    }

    key_pair = iter->second;

    return OK;
}

// get singleton
// ClientCredStorage &ClientCredStorage::GetInstance()
// {
//     if (s_Instance) return (*s_Instance);

//     ScopedLock lock(*s_mutex);

//     if (s_Instance) return (*s_Instance);

//     MMTLSLOG_IMPT("s_Instance = new ClientCredStorage()----------------------------");
//     s_Instance = new ClientCredStorage();

//     s_Instance->Init();
//     return (*s_Instance);
// }

// void ClientCredStorage::Release() {
//     ScopedLock lock(*s_mutex);
//     MMTLSLOG_IMPT("Release ClientCredStorage-----------------------------");

//     if (NULL != s_Instance) {
//         delete s_Instance;
//         s_Instance = NULL;
//     } else {
//     	MMTLSLOG_IMPT("NULL==s_Instance");
//     }

// }
// initialize client credential storage
void ClientCredStorage::Init() {
    ScopedLock member_lock(*member_mutex);
    InitPskKeys();
    InitEcdhKeys();
    InitEcdsaKeys();
    // InitTestKeys();
}

void ClientCredStorage::InitTestKeys() {
    // ecdh_keys_.update_time_ = NowTime();
    // for (unsigned int i=0; i<sizeof(mmtls::mmtls_g_ecdh_keys) / sizeof(mmtls::mmtls_g_ecdh_keys[0]); i++) {
    // 	unsigned char* p_static_key = str2hex(mmtls::mmtls_g_ecdh_keys[i]);
    //     mmtls::String key((char*)p_static_key, std::string(mmtls::mmtls_g_ecdh_keys[i]).length()/2);
    //     mmtls::DataReader reader(key);
    //     mmtls::KeyPair keyPair;
    //     keyPair.Deserialize(reader);
    //     ecdh_keys_.keys_.insert(std::make_pair(keyPair.version(), keyPair));
    // }

    // ecdsa_keys_.update_time_ = NowTime();
    // for (unsigned int i=0; i<sizeof(mmtls_g_ecdsa_keys) / sizeof(mmtls_g_ecdsa_keys[0]); i++) {
    // 	unsigned char* p_static_key = str2hex(mmtls_g_ecdsa_keys[i]);
    //     mmtls::String key((char*)p_static_key, std::string(mmtls_g_ecdsa_keys[i]).length()/2);
    //     mmtls::DataReader reader(key);
    //     mmtls::KeyPair keyPair;
    //     keyPair.Deserialize(reader);
    //     ecdsa_keys_.keys_.insert(std::make_pair(keyPair.version(), keyPair));
    // }
}

int ClientCredStorage::InitPskKeys()  // don't need lock, because locked Init()
{
    xassert2(serialized_acess_psk_.empty() && serialized_refresh_psk_.empty());
    LoadRefreshPskFromFile();
    xassert2(serialized_acess_psk_.empty());
    return 0;
}

int ClientCredStorage::InitEcdhKeys() {
    std::vector<KeyPair> ecdh_static_key_list = GetEcdhStaticKey(group_);
    // KeyPair ecdh_normal_key = GetEcdhNormalKey(group_);
    ecdh_keys_.update_time_ = NowTime();
    for (size_t i = 0; i < ecdh_static_key_list.size(); i++) {
        ecdh_keys_.keys_.insert(std::make_pair(ecdh_static_key_list[i].version(), ecdh_static_key_list[i]));
    }
    // ecdh_keys_.keys_.insert(std::make_pair(ecdh_normal_key.version(), ecdh_normal_key));
    return 0;
}

int ClientCredStorage::InitEcdsaKeys() {
    std::vector<KeyPair> ecdsa_key_list = GetEcdsaKey(group_);
    ecdsa_keys_.update_time_ = NowTime();
    for (size_t i = 0; i < ecdsa_key_list.size(); i++) {
        ecdsa_keys_.keys_.insert(std::make_pair(ecdsa_key_list[i].version(), ecdsa_key_list[i]));
    }
    return 0;
}
static const int NONCE_LEN = 12;
static const int AES_KEY_LEN = 16;
static const int MAC_TAG_LEN = 16;
int ClientCredStorage::SaveRefreshPskToFile(bool is_delete) {
    int ret = 0;
    mmtls::String path = (key_dir_ + PSK_KEY_FILENAME);
    FILE* file = fopen(path.c_str(), "wb");
    if (NULL == file) {
        MMTLSLOG_ERR("open file failed, errno:(%d, %s). path=%s.", errno, strerror(errno), path.c_str());
        return -1;
    }
    if (!is_delete) {
        mmtls::String encrypted_refresh_psk;
        mmtls::String aes_key = GenerateSaveFileKey(context_->GetManager<AppManager>()->GetDeviceInfo().devicename);
        mmtls::String nonce(aes_key.c_str(), NONCE_LEN);
        xassert2(aes_key.size() == 16 && nonce.size() == 12);

        AesGcmCrypter aes_gcm_encrypter(NONCE_LEN, AES_KEY_LEN, MAC_TAG_LEN, true);  // true for encrypt
        ret = aes_gcm_encrypter.SetIv((const byte*)nonce.c_str(), NONCE_LEN);
        MMTLS_CHECK_EQ(ret, OK, (fclose(file), ret), "set iv failed");
        ret = aes_gcm_encrypter.SetKey((const byte*)aes_key.c_str(), AES_KEY_LEN);
        MMTLS_CHECK_EQ(ret, OK, (fclose(file), ret), "set key failed");

        ret = aes_gcm_encrypter.Crypt((const byte*)nonce.c_str(),
                                      NONCE_LEN,
                                      NULL,
                                      0,
                                      (const byte*)serialized_refresh_psk_.c_str(),
                                      serialized_refresh_psk_.size(),
                                      encrypted_refresh_psk);
        MMTLS_CHECK_EQ(ret, OK, (fclose(file), ret), "encrypt failed");
        xassert2(encrypted_refresh_psk.size() == aes_gcm_encrypter.GetCipherTextSize(serialized_refresh_psk_.size()));

        if (OK == ret) {
            size_t size = fwrite(encrypted_refresh_psk.c_str(), 1, encrypted_refresh_psk.size(), file);
            xassert2(size == encrypted_refresh_psk.size());
            xdebug2(TSF "encrypted_refresh_psk len=%_", encrypted_refresh_psk.size());
        }
    }
    fclose(file);
    return ret;
}
#define MAX_SERIALIZED_PSK_LEN 8192
int ClientCredStorage::LoadRefreshPskFromFile() {
    int ret = 0;
    mmtls::String path = (key_dir_ + PSK_KEY_FILENAME);
    FILE* file = fopen(path.c_str(), "rb");
    if (NULL == file) {
        MMTLSLOG_ERR("open file failed, errno:(%d, %s). path=%s.", errno, strerror(errno), path.c_str());
        return -1;
    }
    unsigned char* buf = (unsigned char*)malloc(MAX_SERIALIZED_PSK_LEN);
    memset(buf, 0, MAX_SERIALIZED_PSK_LEN);
    size_t size = fread(buf, 1, MAX_SERIALIZED_PSK_LEN, file);
    MMTLSLOG_IMPT("mmtls: MAX_SERIALIZED_PSK_LEN=%d, read read size=%zu", MAX_SERIALIZED_PSK_LEN, size);
    if (ferror(file)) {
        ret = -1;
        MMTLSLOG_IMPT("fread error.");
    } else {
        ///
        mmtls::String decrypted_refresh_psk;
        mmtls::String aes_key = GenerateSaveFileKey(context_->GetManager<AppManager>()->GetDeviceInfo().devicename);
        mmtls::String nonce(aes_key.c_str(), NONCE_LEN);
        xassert2(aes_key.size() == 16 && nonce.size() == 12);

        AesGcmCrypter aes_gcm_encrypter(NONCE_LEN, AES_KEY_LEN, MAC_TAG_LEN, false);  // false for decrypt
        ret = aes_gcm_encrypter.SetIv((const byte*)nonce.c_str(), NONCE_LEN);
        MMTLS_CHECK_EQ(ret, OK, (fclose(file), ret), "set iv failed");
        ret = aes_gcm_encrypter.SetKey((const byte*)aes_key.c_str(), AES_KEY_LEN);
        MMTLS_CHECK_EQ(ret, OK, (fclose(file), ret), "set key failed");

        ret = aes_gcm_encrypter
                  .Crypt((const byte*)nonce.c_str(), NONCE_LEN, NULL, 0, (const byte*)buf, size, decrypted_refresh_psk);
        MMTLS_CHECK_EQ(ret, OK, (fclose(file), ret), "decrypt failed");
        if (0 == ret) {
            serialized_refresh_psk_.clear();
            serialized_refresh_psk_.append((char*)decrypted_refresh_psk.c_str(), decrypted_refresh_psk.size());
            xdebug2(TSF "decrypted_refresh_psk len=%_", decrypted_refresh_psk.size());
        }
    }

    free(buf);
    fclose(file);
    return ret;
}

}  // end namespace mmtls
