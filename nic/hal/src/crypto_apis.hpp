#ifndef __CRYPT_ASYM_APIS_HPP__
#define __CRYPT_ASYM_APIS_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/crypto_apis.pb.h"

using cryptoapis::CryptoApiRequest;
using cryptoapis::CryptoApiRequestMsg;
using cryptoapis::CryptoApiResponse;
using cryptoapis::CryptoApiResponseMsg;


namespace hal {


#define ECC_MAX_KEY_SIZE 32
#define RSA_MAX_KEY_SIZE 256

#define CRYPTO_MAX_HASH_DIGEST_LEN 64

#define CRYPTO_SHA1_DIGEST_LEN   20
#define CRYPTO_SHA224_DIGEST_LEN 28
#define CRYPTO_SHA256_DIGEST_LEN 32
#define CRYPTO_SHA384_DIGEST_LEN 48
#define CRYPTO_SHA512_DIGEST_LEN 64

hal_ret_t cryptoapi_invoke(cryptoapis::CryptoApiRequest &req, 
        cryptoapis::CryptoApiResponse *resp);

}    // namespace hal

#endif  /* __CRYPT_ASYM_APIS_HPP__ */

