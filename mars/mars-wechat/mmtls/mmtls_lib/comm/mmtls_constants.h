#pragma once

#include "mmtls_types.h"
#if defined(LINUX) || defined(ANDROID)
#include <endian.h>
#elif defined(__APPLE__)
#include <machine/endian.h>
#endif
#ifdef __APPLE__
#define __BYTE_ORDER BYTE_ORDER
#define __BIG_ENDIAN BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#endif
#ifdef WIN32
static union {
    int i;
    char c[sizeof(int)];
} EndianTest = {1};

#define __BYTE_ORDER EndianTest.c[0]
#define __BIG_ENDIAN 0
#define __LITTLE_ENDIAN 1
#endif
namespace mmtls {

// Constants for MMTLS
// These constants are from TLS1.3.
// For new types in TLS1.3 see https://tlswg.github.io/tls13-spec/#rfc.appendix.A
// For already registered types see http://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml

// For new types in TLS1.3 see https://tlswg.github.io/tls13-spec/#record-layer
// For already registered types see http://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-5
typedef byte RecordType;

// 0 ~ 20 are reserved
const RecordType RECORD_ALERT = 21;
const RecordType RECORD_HANDSHAKE = 22;
const RecordType RECORD_APPLICATION_DATA = 23;
const RecordType RECORD_HEARTBEAT = 24;
const RecordType RECORD_EARLY_HANDSHAKE = 25;
const RecordType RECORD_NONE = 255;

// For alert types in TLS1.3 see https://tlswg.github.io/tls13-spec/#alert-protocol
// For already registered alert types see
// http://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-6
typedef uint16 AlertType;

// These is a normal close alert.
const AlertType CLOSE_NOTIFY = 0;

// These types are fatal. Client and server should close connection after receive them.

// Unknown record type.
const AlertType ALERT_UNEXPECTED_MESSAGE = 10;
// RecordHead size too big.
const AlertType ALERT_RECORD_OVERFLOW = 22;
// The handshake state machine is invalid. E.g. received an unexpected handshake message.
const AlertType ALERT_HANDSHAKE_FAILURE = 40;
// Parameter in record, handshake messages, extensions is invalid.
const AlertType ALERT_ILLEGAL_PARAMETER = 47;
// Server reject client
const AlertType ALERT_ACCESS_DENIED = 49;
// 1. Decode a handshake message fail 2. Decode a record fail.
const AlertType ALERT_DECODE_ERROR = 50;
// 1. Decrypt a record fail 2. Verify a signature fail 3. Verify a finished fail.
const AlertType ALERT_DECRYPT_ERROR = 51;
// Error occur internally. E.g. Encrypt a record fail.
const AlertType ALERT_INTERNAL_ERROR = 52;
// Misssing required extension in handshake message.
const AlertType ALERT_MISSING_EXTENSION = 109;
// Unknown extension.
const AlertType ALERT_UNSUPPORTED_EXTENSION = 110;
// Server doesn't accept PSK handshake.
const AlertType ALERT_UNKNOWN_PSK_IDENTITY = 115;
// Server want client to fallback to no mmtls.
const AlertType ALERT_FALLBACK_NO_MMTLS = 116;

// Only one warning alert in mmtls.
// Indicate end of early data.
const AlertType ALERT_END_OF_EARLY_DATA = 1;

const AlertType ALERT_NONE = 255;

// Alert level
typedef byte AlertLevel;

const AlertLevel ALERT_LEVEL_WARNING = 1;
const AlertLevel ALERT_LEVEL_FATAL = 2;
const AlertLevel ALERT_LEVEL_NONE = 255;

// For new types in TLS1.3 see https://tlswg.github.io/tls13-spec/#handshake-protocol
// For already registered types see http://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-7
typedef byte HandShakeType;

// 0 is reserved
const HandShakeType CLIENT_HELLO = 1;
const HandShakeType SERVER_HELLO = 2;
// 3 is hello verify request. It's not used in MMTLS.
const HandShakeType NEW_SESSION_TICKET = 4;
// 6 is hello retry. It's not used in MMTLS.
// 7 is server key share SHARE.
// ServerKeyShare message is going to be an extension. So we move it to extension now.
// https://github.com/tlswg/tls13-spec/pull/268
const HandShakeType ENCRYPTED_EXTENSIONS = 8;
// 11 is certificate. It's not used in MMTLS because certificate is pre-installed in client.
// 13 is certificate request, It's not used in MMTLS because we don't verify client.
// 14 is reserved
const HandShakeType CERTIFICATE_VERIFY = 15;
// 16 is reserved
// 17 is server configuration. It's not used in MMTLS because ECDH key is pre-installed in client.
const HandShakeType FINISHED = 20;
const HandShakeType HANDSHAKE_NONE = 255;

// The handshake mode that mmtls supported.
typedef byte HandShakeMode;

const HandShakeMode HS_MODE_ONE_RTT_ECDH = 1;
const HandShakeMode HS_MODE_ONE_RTT_PSK = 2;
const HandShakeMode HS_MODE_ZERO_RTT_PSK = 3;
const HandShakeMode HS_MODE_NONE = 255;

// The sub handshake mode.
typedef byte HandShakeSubMode;

const HandShakeSubMode HS_SUB_MODE_NORMAL = 0;
const HandShakeSubMode HS_SUB_MODE_STATIC_ECDH = 1;
const HandShakeSubMode HS_SUB_MODE_REFRESH_PSK = 2;
const HandShakeSubMode HS_SUB_MODE_NONE = 255;

// Most of the new extension types in TLS1.3 are not defined.
// So we try our best to be compatiable with the specification.
// For extension types in TLS1.3 see https://tlswg.github.io/tls13-spec/#hello-extensions
// IANA doesn't specify particular extension type, see https://tools.ietf.org/html/rfc6066#page-3
typedef uint16 ExtensionType;

// 0 ~ 9 are reserved
// 10 is supported_groups. It's not used in MMTLS
// 11 ~ 12 are reserved
// 13 is signature_algorithms. It's not used in MMTLS
// We define the new extension type from 14.
const ExtensionType EARLY_DATA = 14;
const ExtensionType PRE_SHAKE_KEY = 15;
const ExtensionType CLIENT_KEY_SHARE = 16;
// ServerKeyShare message is going to be an extension. So we move it to extension now.
// https://github.com/tlswg/tls13-spec/pull/268
// TODO: The number may need to change.
const ExtensionType SERVER_KEY_SHARE = 17;
// EarlyEncryptData is an mmtls only extension. It's for 0-RTT application anti-replay attack.
// TODO: The number may need to change.
const ExtensionType EARLY_ENCRYPT_DATA = 18;
const ExtensionType CERT_REGION = 19;
const ExtensionType EXTENSION_NONE = 65535;

// See https://tlswg.github.io/tls13-spec/#early-data-indication
typedef byte EarlyDataType;

// 1 is client_authentication
const EarlyDataType EARLY_APP_DATA = 2;
// 3 is client_authentication_and_data
const EarlyDataType EARLY_DATA_NONE = 255;

// Specify the early data anti-replay algorithm
typedef byte EarlyDataAntiReplayType;

const EarlyDataAntiReplayType EARLY_DATA_ANTI_REPLY_BY_TIME = 1;
const EarlyDataAntiReplayType EARLY_DATA_ANTI_REPLY_NONE = 255;

enum ConnectionSide { CONN_SIDE_CLIENT = 1, CONN_SIDE_SERVER = 2 };

// See record layer for these size limits https://tlswg.github.io/tls13-spec/#record-layer

typedef size_t MaxSize;

const MaxSize MAX_DATA_PACK_SIZE = 134217728;  // 128MB
const MaxSize MAX_PLAINTEXT_SIZE = 64 * 1024 - 16 - 1;
const MaxSize MAX_CIPHERTEXT_SIZE = 64 * 1024;
const MaxSize MAX_PUBLIC_VALUE_SIZE = 256;
const MaxSize MAX_CIPHERSUITE_SIZE = 2;
const MaxSize MAX_EXTENSION_SIZE = 256;
const MaxSize MAX_KEY_OFFER_SIZE = 256;
const MaxSize MAX_CLIENT_RANDOM_SIZE = 32;
const MaxSize MAX_SERVER_RANDOM_SIZE = 32;
const MaxSize MAX_SIGNATURE_SIZE = 2048;
const MaxSize MAX_FINISHED_VERIFY_DATA_SIZE = 2048;
const MaxSize MAX_NEW_SESSION_TICKET_PSK_SIZE = 2;
const MaxSize MAX_PRE_SHARED_KEY_MAC_SIZE = 256;
const MaxSize MAX_HEARTBEAT_PAYLOAD_SIZE = 16 * 1024 - 32 - 2 - 1;

// Definition for fixed sizes.

typedef size_t FixedSize;

const FixedSize FIXED_RECORD_HEAD_SIZE = 5;  // type_[8bit],version_[16bit],payloadsize[16bit]
const FixedSize FIXED_RANDOM_SIZE = 32;
const FixedSize FIXED_SECRET_SIZE = 32;  // secret size equal to sha256 size
const FixedSize FIXED_CLIENT_IDENTITY = 16;
const FixedSize FIXED_STRING_UNIT_SIZE = 8;

// Supported cipher type.
enum CipherType {
    // Some handshake messages need encryption. So use null_encrypter to "encrypt" them.
    CIPHER_NULL = 1,
    CIPHER_AES_128_GCM_12 = 2
};

// Connection key type
// See https://tlswg.github.io/tls13-spec/#traffic-key-calculation
enum ConnectionKeyType {
    CON_KEY_NONE = 0,
    CON_KEY_EARLY_DATA = 1,  // early data
    CON_KEY_HANDSHAKE = 2,
    CON_KEY_APPLICATION = 3,

    CON_KEY_NUM = 4  // total count
};

// Connection key derivation lable
// See https://tlswg.github.io/tls13-spec/#traffic-key-calculation
#define CON_KEY_LABLE_EARLY_DATA "early data key expansion"
#define CON_KEY_LABLE_HANDSHAKE "handshake key expansion"
#define CON_KEY_LABLE_APPLICATION "application data key expansion"

// Secret derivation label
#define EXPAND_SS_LABLE "expanded static secret"
#define EXPAND_ES_LABLE "expanded ephemeral secret"
#define EXPAND_LABLE "expanded secret"
#define EXPAND_ACCESS_PSK_SECRET_LABEL "PSK_ACCESS\0resumption master secret"
#define EXPAND_REFRESH_PSK_SECRET_LABEL "PSK_REFRESH\0resumption master secret"
#define EXPAND_REFRESH_PSK_MAC_LABEL "PSK_REFRESH\0mac secret"

#define EARLY_DATA_DIGEST_FILED "digest"
#define FALLBACK_NO_MMTLS "fallback_no_mmtls\0"

// The HandShake Stage.
enum HandShakeStage {
    STAGE_INIT = 1,

    // client stages
    STAGE_SEND_CLIENT_HELLO = 2,
    STAGE_SEND_ENCRYPTED_EXTENSIONS = 3,
    STAGE_SEND_EARLY_APP_DATA = 4,
    STAGE_RECV_SERVER_HELLO = 5,
    STAGE_RECV_CERTIFICATE_VERIFY = 6,
    STAGE_RECV_NEW_SESSION_TICKET = 7,
    STAGE_RECV_SERVER_FINISHED = 8,
    STAGE_RECV_EARLY_APP_DATA = 9,
    STAGE_SEND_CLIENT_FINISHED = 10,

    // server stages
    STAGE_RECV_CLIENT_HELLO = 101,
    STAGE_RECV_ENCRYPTED_EXTENSIONS = 102,
    STAGE_SEND_SERVER_HELLO = 103,
    STAGE_SEND_CERTIFICATE_VERIFY = 104,
    STAGE_SEND_NEW_SESSION_TICKET = 105,
    STAGE_RECV_CLIENT_EARLY_DATA = 106,
    STAGE_SEND_SERVER_EARLY_DATA = 107,
    STAGE_SEND_SERVER_FINISHED = 108,
    STAGE_RECV_CLIENT_FINISHED = 109,

    STAGE_COMPLETED = 201,

    STAGE_CLOSED = 301,

    STAGE_NONE = 65535,
};

// ACCESS_PSK the PSK used in PSK handshake. It has shorter life time and is saved in memory.
// REFRESH_PSK the PSK used in ECDH handshake for disaster recovery. It has longer life time and is saved in persistent
// storage. REFRESH_PSK has a signature in its ticket part, in order to avoid MITM attack when it's leaked.
typedef byte PskType;
const PskType PSK_NONE = 0;
const PskType PSK_ACCESS = 1;
const PskType PSK_REFRESH = 2;
const PskType PSK_MAX = 15;

typedef byte PskTicketVersion;
// version 0 is no longer supported in server
const PskTicketVersion PSK_TICKET_VERSION_0 = 0;
const PskTicketVersion PSK_TICKET_VERSION_1 = 1;
const PskTicketVersion PSK_TICKET_VERSION_MAX = 15;

enum ErrorCode {
    OK = 0,

    // -1 ~ -999 are for external usage.
    // E.g. channel Init, Send, Receive
    ERR_APP_DATA_NOT_SENT = -1,
    ERR_INVALID_RECORD = -2,
    ERR_INVALID_HANDSHAKE_MESSAGE = -3,
    ERR_INVALID_ALERT = -4,
    ERR_PROCESS_HANDSHAKE_FAIL = -5,
    ERR_PROCESS_EARLY_DATA_FAIL = -6,
    ERR_PROCESS_APP_DATA_FAIL = -7,
    ERR_TLS_CLOSED = -8,
    ERR_RECEIVE_FATAL_ALERT = -9,
    ERR_INTERNAL_ERROR = -10,
    // ERR_RECEIVE_WARNING_ALERT = -11, // No longer used.
    ERR_APP_DATA_EXCEED_LIMIT = -12,
    ERR_ONE_TIME_CONNECT_FOR_0RTT_PSK_MODE = -13,
    ERR_FALLBACK_NO_MMTLS = -14,
    ERR_HEARTBEAT_NOT_SENT = -15,
    ERR_PROCESS_HEARTBEAT_FAIL = -16,

    // -100 ~ -INF are for internal usage.

    // For assertion.
    ERR_UNEXPECT_CHECK_FAIL = -10000,

    // Internal errros occurred in channel [-19999, -10001]
    ERR_RECEIVE_APP_DATA_STAGE_ERR = -10003,
    ERR_INVLID_CLIENT_HELLO = -10004,
    ERR_INVALID_CIPHERSUITE = -10005,
    ERR_RECEIVE_VERIFY_DATA_INVALID = -10006,
    ERR_INVALID_SERVER_HELLO = -10007,
    ERR_INVLID_CLIENT_KEY_SHARE = -10008,
    ERR_REPLAY_AT_PROXY = -10009,
    ERR_UNEXPECTED_HANDSHAKE_MSG = -10010,
    ERR_UNKNOWN_RECORD_TYPE = -10011,
    ERR_UNKNOWN_HANDSHAKE_STAGE = -10012,
    ERR_UNEXPECT_RECORD = -10013,
    ERR_RECEIVE_UNKNOWN_HANDSHAKE_MESSAGE = -10014,
    ERR_INVALID_CLIENT_PRE_SHARE_KEY = -10015,
    ERR_INVALID_CLIENT_KEY_SHARE_EXTENSION = -10016,
    ERR_INTENTION = -10017,
    ERR_NO_ECDH_WITHOUT_REFRESH_PSK_QUOTA = -10018,
    ERR_REPLAY_AT_LOGIC = -10018,
    ERR_NO_ECDH_WITH_REFRESH_PSK_QUOTA = -10019,
    ERR_NOT_ALLOW_ECDH_WITH_INVALID_REFRESH_PSK = -10020,
    ERR_RECEIVE_RECORD_VERSION_UNACCEPTED = -10021,
    ERR_UNEXPECTED_HEARTBEAT = -10022,
    ERR_INVALID_HEARTBEAT = -10023,
    ERR_INVALID_CERT_REGION = -10024,

    // Internal crypto error region [-20099, -20000]
    ERR_ENCRYPT_FAIL = -20000,
    ERR_DECRYPT_FAIL = -20001,
    ERR_DERIVE_KEY_FAIL = -20002,
    ERR_GEN_MAC_FAIL = -20003,
    ERR_COMPUTE_ECDH_FAIL = -20004,
    ERR_ECDSA_SIGN_FAIL = -20005,
    ERR_ECDSA_VEIRFY_FAIL = -20006,
    ERR_HASH_FAILD = -20007,
    ERR_GEN_RANDOM_FAIL = -20008,
    ERR_CREATE_CONN_CIPHER_STATE_FAIL = -20009,
    ERR_MISSING_EXTENSION = -20010,
    ERR_GEN_ECDH_KEY_FAIL = -20011,
    ERR_HKDF_FAIL = -20012,
    ERR_DIGEST_FAILD = -20013,

    // Internal key management error region [-20199, -20100]
    ERR_ECDH_KEY_INVALID = -20100,
    ERR_ECDSA_KEY_INVALID = -20101,
    // ERR_PSK_INVALID = -20102, // No longer used.
    ERR_PSK_EXPIRED = -20103,
    ERR_INTERFACE_NOT_IMPLEMENT = -20104,
    ERR_NO_CON_KEY_TYPE = -20105,
    ERR_NO_FIND_CON_KEY = -20106,
    ERR_NO_CIPHER_SUITE = -20107,
    ERR_PSK_NOT_EXIST = -20108,
    ERR_READ_KEY_FILE_FAILED = -20109,
    ERR_KEY_NOT_EXIST = -20110,
    ERR_KEY_VESION_NOT_MATCH = -20111,
    ERR_PSK_TICKET_DECRYPT_FAIL = -20112,
    ERR_PSK_TICKET_DECODE_FAIL = -20113,
    ERR_PSK_FROM_LOW_SECURITY_IDC = -20114,
    ERR_PSK_TICKET_CONTENT_INVALID = -20115,
    ERR_UNSUPPORTED_PSK_TICKET_VERSION = -20116,

    ERR_PSK_EMPTY = -20198,         // added by elviswu
    ERR_PSK_INVALID_TYPE = -20199,  // added by elviswu

    // Internal parameter, encodeing, decoding error region [-20299, -20200]
    ERR_DECODE_ERROR = -20299,
    ERR_ENCODE_ERROR = -20300,
    ERR_ILLEGAL_PARAM = -20301,
    ERR_NO_ENOUGH_DATA = -20302,
    ERR_LENGTH_EXCEED_LIMIT = -20303,
    ERR_AUDIT_CHECK_FAIL = -20304,
    ERR_NO_ENOUGH_MEM = -20305,
};

// Heartbeat Message
// See https://tools.ietf.org/html/rfc6520
typedef byte HeartbeatType;

const HeartbeatType HEARTBEAT_REQUEST = 1;
const HeartbeatType HEARTBEAT_RESPONSE = 2;
const HeartbeatType HEARTBEAT_NONE = 255;

};  // namespace mmtls
