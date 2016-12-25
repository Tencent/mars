#pragma once

#include <string>

namespace mmpaycertlib{

// OpenSsl util for external use. It's a subset copy of comm/mmtls_openssl_crypto_util.
// We hope others can benefit from mmtls's best practice.
// Don't depends on targets in client/server/comm directories.

class OpenSslCryptoUtil
{
public:
	static OpenSslCryptoUtil &GetDefault();

	OpenSslCryptoUtil();

	virtual ~OpenSslCryptoUtil();

    // The ec curve is NID_X9_62_prime256v1
	int GenEcdhKeyPair(std::string& public_material, std::string& private_material);

    // The ec curve is NID_X9_62_prime256v1. Keys are PEM format
    int GenEcdsaKeyPair(std::string& public_key, std::string& private_key);

    // The result is 32 bytes long.
	int Ecdh(const std::string& public_material,
    	 	 const std::string& private_material,
			 std::string& result);

    // The private_key is PEM format.
    // =0 for succ
	int EcdsaSign(const std::string& private_key,
				  const std::string& message,
				  std::string& signature);

    // The public_key is PEM format
    // =0 for succ
    int EcdsaVerify(const std::string& public_key,
                    const std::string& signature,
                    const std::string& message);

private:
	int Ecdh(int nid,
			const unsigned char* public_material, size_t public_material_size,
			const unsigned char* private_material, size_t private_material_size,
			std::string& result);

	int EcdsaSign(const unsigned char* private_key, size_t private_key_size,
				  const unsigned char* message, size_t message_size,
				  std::string& signature);

	int EcdsaVerify(const unsigned char* public_key, size_t public_key_size,
					const unsigned char* signature, size_t signature_size,
					const unsigned char* message, size_t message_size);
};

}
