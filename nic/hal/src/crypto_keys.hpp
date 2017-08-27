#ifndef __CRYPTO_SESSION_KEYS_HPP__
#define __CRYPTO_SESSION_KEYS_HPP__
#include <base.h>
#include <pd.hpp>
#include <hal_state.hpp>
#include <crypto_keys.pb.h>
#include <types.pb.h>

namespace hal {


#define CRYPTO_KEY_SIZE_MAX     32  /* 256 bit key */

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

}    // namespace hal
#endif  /* __CRYPTO_SESSION_KEYS_HPP__ */

