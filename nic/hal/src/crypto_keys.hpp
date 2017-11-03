#ifndef __CRYPTO_SESSION_KEYS_HPP__
#define __CRYPTO_SESSION_KEYS_HPP__
#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/crypto_keys.pb.h"
#include "nic/gen/proto/hal/types.pb.h"


using cryptokey::CryptoKeySpec;

using cryptokey::CryptoKeyCreateRequest;
using cryptokey::CryptoKeyCreateRequestMsg;
using cryptokey::CryptoKeyCreateResponse;
using cryptokey::CryptoKeyCreateResponseMsg;

using cryptokey::CryptoKeyReadRequest;
using cryptokey::CryptoKeyReadRequestMsg;
using cryptokey::CryptoKeyReadResponse;
using cryptokey::CryptoKeyReadResponseMsg;

using cryptokey::CryptoKeyUpdateRequest;
using cryptokey::CryptoKeyUpdateRequestMsg;
using cryptokey::CryptoKeyUpdateResponse;
using cryptokey::CryptoKeyUpdateResponseMsg;


using cryptokey::CryptoKeyDeleteRequest;
using cryptokey::CryptoKeyDeleteRequestMsg;
using cryptokey::CryptoKeyDeleteResponse;
using cryptokey::CryptoKeyDeleteResponseMsg;

namespace hal {

// TODO: Needs a flexible allocation based on key sizes
#define CRYPTO_KEY_SIZE_MAX     64  /* 2 * 256 bit key */

typedef struct crypto_key_s {
    types::CryptoKeyType    key_type;
    uint32_t                key_size;
    uint8_t                 key[CRYPTO_KEY_SIZE_MAX];
} __PACK__ crypto_key_t;

hal_ret_t   crypto_key_create(cryptokey::CryptoKeyCreateRequest &request,
        cryptokey::CryptoKeyCreateResponse *response);

hal_ret_t   crypto_key_read(cryptokey::CryptoKeyReadRequest &request,
        cryptokey::CryptoKeyReadResponse *response);

hal_ret_t   crypto_key_update(cryptokey::CryptoKeyUpdateRequest &request,
        cryptokey::CryptoKeyUpdateResponse *response);

hal_ret_t   crypto_key_delete(cryptokey::CryptoKeyDeleteRequest &request,
        cryptokey::CryptoKeyDeleteResponse *response);



typedef struct crypto_asym_key_s {
    uint64_t                key_param_list;     /* Address to the DMA 
                                                   descriptor that contains the
                                                   key/param */
    uint32_t                command_reg;        /* Command associated with this
                                                   key descriptor */


} __PACK__ crypto_asym_key_t;

}    // namespace hal
#endif  /* __CRYPTO_SESSION_KEYS_HPP__ */

