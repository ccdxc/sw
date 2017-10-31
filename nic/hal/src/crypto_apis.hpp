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


#define MAX_KEY_SIZE 32

typedef struct ecc_point_s {
    uint8_t                 x[MAX_KEY_SIZE];
    uint8_t                 y[MAX_KEY_SIZE];
} ecc_point_t;

typedef struct ecc_domain_params_s {
    uint32_t                key_size;
    uint8_t                 p[MAX_KEY_SIZE];
    uint8_t                 a[MAX_KEY_SIZE];
    uint8_t                 b[MAX_KEY_SIZE];
    ecc_point_t             G;
    uint8_t                 n[MAX_KEY_SIZE];
    uint8_t                 h[MAX_KEY_SIZE];
} ecc_domain_params_t;

typedef struct crypto_asym_api_ecc_point_mul_fp_s {
    ecc_domain_params_t     ecc_domain_params;
    uint8_t                 k[MAX_KEY_SIZE];
    ecc_point_t             q;                      /* Output */
} crypto_asym_api_ecc_point_mul_fp_t;

typedef struct crypto_apis_s {
    cryptoapis::CryptoApiType                api_type;
    union {
        crypto_asym_api_ecc_point_mul_fp_t  ecc_point_mul_fp;
    } u;
} crypto_apis_t;

hal_ret_t crypto_api_invoke(cryptoapis::CryptoApiRequest &req, 
        cryptoapis::CryptoApiResponse *resp);

}    // namespace hal

#endif  /* __CRYPT_ASYM_APIS_HPP__ */

