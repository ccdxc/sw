//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CRYPT_ASYM_APIS_HPP__
#define __CRYPT_ASYM_APIS_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "gen/proto/internal.pb.h"

using internal::CryptoApiRequest;
using internal::CryptoApiRequestMsg;
using internal::CryptoApiResponse;
using internal::CryptoApiResponseMsg;


namespace hal {


#define ECC_MAX_KEY_SIZE 32
#define RSA_MAX_KEY_SIZE 256

#define CRYPTO_MAX_HASH_DIGEST_LEN 64

#define CRYPTO_SHA1_DIGEST_LEN   20
#define CRYPTO_SHA224_DIGEST_LEN 28
#define CRYPTO_SHA256_DIGEST_LEN 32
#define CRYPTO_SHA384_DIGEST_LEN 48
#define CRYPTO_SHA512_DIGEST_LEN 64

enum CryptoApiHashType {
  CRYPTOAPI_HASHTYPE_NONE = 0,
  CRYPTOAPI_HASHTYPE_MD5 = 1,
  CRYPTOAPI_HASHTYPE_SHA1 = 2,
  CRYPTOAPI_HASHTYPE_SHA224 = 3,
  CRYPTOAPI_HASHTYPE_SHA256 = 4,
  CRYPTOAPI_HASHTYPE_SHA384 = 5,
  CRYPTOAPI_HASHTYPE_SHA512 = 6,
  CRYPTOAPI_HASHTYPE_SHA512_224 = 7,
  CRYPTOAPI_HASHTYPE_SHA512_256 = 8,
  CRYPTOAPI_HASHTYPE_HMAC_MD5 = 9,
  CRYPTOAPI_HASHTYPE_HMAC_SHA1 = 10,
  CRYPTOAPI_HASHTYPE_HMAC_SHA224 = 11,
  CRYPTOAPI_HASHTYPE_HMAC_SHA256 = 12,
  CRYPTOAPI_HASHTYPE_HMAC_SHA384 = 13,
  CRYPTOAPI_HASHTYPE_HMAC_SHA512 = 14
};

static inline const char *
CryptoApiHashType_Name(CryptoApiHashType value)
{
    switch(value) {
     case CRYPTOAPI_HASHTYPE_NONE:
         return("None");
     case CRYPTOAPI_HASHTYPE_MD5:
         return("MD5");
     case CRYPTOAPI_HASHTYPE_SHA1:
         return("SHA1");
     case CRYPTOAPI_HASHTYPE_SHA224:
         return("SHA224");
     case CRYPTOAPI_HASHTYPE_SHA256:
         return("SHA256");
     case CRYPTOAPI_HASHTYPE_SHA384:
         return("SHA384");
     case CRYPTOAPI_HASHTYPE_SHA512:
         return("SHA512");
     case CRYPTOAPI_HASHTYPE_SHA512_224:
         return("SHA512-224");
     case CRYPTOAPI_HASHTYPE_SHA512_256:
         return("SHA512-256");
     case CRYPTOAPI_HASHTYPE_HMAC_MD5:
         return("HMAC-MD5");
     case CRYPTOAPI_HASHTYPE_HMAC_SHA1:
         return("HMAC-SHA1");
     case CRYPTOAPI_HASHTYPE_HMAC_SHA224:
         return("HMAC-SHA224");
     case CRYPTOAPI_HASHTYPE_HMAC_SHA256:
         return("HMAC-SHA256");
     case CRYPTOAPI_HASHTYPE_HMAC_SHA384:
         return("HMAC-SHA384");
     case CRYPTOAPI_HASHTYPE_HMAC_SHA512:
         return("HMAC-SHA512");
     default:
       return("Unknown");
  }
  return("Unknown");
}

}    // namespace hal

#endif  /* __CRYPT_ASYM_APIS_HPP__ */

