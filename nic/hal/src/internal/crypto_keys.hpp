//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CRYPTO_SESSION_KEYS_HPP__
#define __CRYPTO_SESSION_KEYS_HPP__
#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "gen/proto/internal.pb.h"
#include "gen/proto/types.pb.h"


using internal::CryptoKeySpec;

using internal::CryptoKeyCreateRequest;
using internal::CryptoKeyCreateRequestMsg;
using internal::CryptoKeyCreateResponse;
using internal::CryptoKeyCreateResponseMsg;

using internal::CryptoKeyCreateWithIdRequest;
using internal::CryptoKeyCreateWithIdRequestMsg;
using internal::CryptoKeyCreateWithIdResponse;
using internal::CryptoKeyCreateWithIdResponseMsg;

using internal::CryptoKeyReadRequest;
using internal::CryptoKeyReadRequestMsg;
using internal::CryptoKeyReadResponse;
using internal::CryptoKeyReadResponseMsg;

using internal::CryptoKeyUpdateRequest;
using internal::CryptoKeyUpdateRequestMsg;
using internal::CryptoKeyUpdateResponse;
using internal::CryptoKeyUpdateResponseMsg;


using internal::CryptoKeyDeleteRequest;
using internal::CryptoKeyDeleteRequestMsg;
using internal::CryptoKeyDeleteResponse;
using internal::CryptoKeyDeleteResponseMsg;

namespace hal {

// TODO: Needs a flexible allocation based on key sizes
#define CRYPTO_KEY_SIZE_MAX     64  /* 2 * 256 bit key */

typedef struct crypto_key_s {
    types::CryptoKeyType    key_type;
    uint32_t                key_size;
    uint8_t                 key[CRYPTO_KEY_SIZE_MAX];
} __PACK__ crypto_key_t;


typedef struct crypto_asym_key_s {
    uint64_t                key_param_list;     /* Address to the DMA
                                                   descriptor that contains the
                                                   key/param */
    uint32_t                command_reg;        /* Command associated with this
                                                   key descriptor */


} __PACK__ crypto_asym_key_t;

}    // namespace hal
#endif  /* __CRYPTO_SESSION_KEYS_HPP__ */

