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

hal_ret_t crypto_api_invoke(cryptoapis::CryptoApiRequest &req, 
        cryptoapis::CryptoApiResponse *resp);

}    // namespace hal

#endif  /* __CRYPT_ASYM_APIS_HPP__ */

