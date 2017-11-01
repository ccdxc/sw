#ifndef __CAPRI_BARCO_SYM_APIS_H__
#define __CAPRI_BARCO_SYM_APIS_H__

#include "nic/hal/pd/capri/capri_barco_crypto.hpp"

namespace hal {
namespace pd {

typedef enum {
    CRYPTO_HASH_TYPE_NONE = 0,
    CRYPTO_HASH_TYPE_MD5 = 1,
    CRYPTO_HASH_TYPE_SHA1 = 2,
    CRYPTO_HASH_TYPE_SHA224 = 3,
    CRYPTO_HASH_TYPE_SHA256 = 4,
    CRYPTO_HASH_TYPE_SHA384 = 5,
    CRYPTO_HASH_TYPE_SHA512 = 6,
    CRYPTO_HASH_TYPE_SHA512_224 = 7,
    CRYPTO_HASH_TYPE_SHA512_256 = 8,
    CRYPTO_HASH_TYPE_HMAC_MD5 = 9,
    CRYPTO_HASH_TYPE_HMAC_SHA1 = 10,
    CRYPTO_HASH_TYPE_HMAC_SHA224 = 11,
    CRYPTO_HASH_TYPE_HMAC_SHA256 = 12,
    CRYPTO_HASH_TYPE_HMAC_SHA384 = 13,
    CRYPTO_HASH_TYPE_HMAC_SHA512 = 14
} crypto_hash_type_e;

static inline const char *
crypto_hash_type_str (crypto_hash_type_e hash_type)
{
  switch (hash_type) {
  case CRYPTO_HASH_TYPE_NONE:
    return("None");
  case CRYPTO_HASH_TYPE_MD5:
    return("MD5");
  case CRYPTO_HASH_TYPE_SHA1:
    return("SHA1");
  case CRYPTO_HASH_TYPE_SHA224:
    return("SHA224");
  case CRYPTO_HASH_TYPE_SHA256:
    return("SHA256");
  case CRYPTO_HASH_TYPE_SHA384:
    return("SHA384");
  case CRYPTO_HASH_TYPE_SHA512:
    return("SHA512");
  case CRYPTO_HASH_TYPE_SHA512_224:
    return("SHA512-224");
  case CRYPTO_HASH_TYPE_SHA512_256:
    return("SHA512-256");
  case CRYPTO_HASH_TYPE_HMAC_MD5:
    return("HMAC-MD5");
  case CRYPTO_HASH_TYPE_HMAC_SHA1:
    return("HMAC-SHA1");
  case CRYPTO_HASH_TYPE_HMAC_SHA224:
    return("HMAC-SHA224");
  case CRYPTO_HASH_TYPE_HMAC_SHA256:
    return("HMAC-SHA256");
  case CRYPTO_HASH_TYPE_HMAC_SHA384:
    return("HMAC-SHA384");
  case CRYPTO_HASH_TYPE_HMAC_SHA512:
    return("HMAC-SHA512");
  default:
    return("Unknown");
  }
}

static inline const char *
barco_symm_err_status_str (uint64_t status)
{
  uint32_t err = (status & (CAPRI_BARCO_SYM_ERR_BUS_ERR |
			    CAPRI_BARCO_SYM_ERR_GEN_PUSH_ERR |
			    CAPRI_BARCO_SYM_ERR_GEN_FETCH_ERR |
			    CAPRI_BARCO_SYM_ERR_BUS_UNSUP_MODE |
			    CAPRI_BARCO_SYM_ERR_BUS_RSVD |
			    CAPRI_BARCO_SYM_ERR_BUS_BAD_CMD |
			    CAPRI_BARCO_SYM_ERR_BUS_UNK_STATE |
			    CAPRI_BARCO_SYM_ERR_BUS_AXI_BUS_RESP |
			    CAPRI_BARCO_SYM_ERR_BUS_WRONG_KEYTYPE |
			    CAPRI_BARCO_SYM_ERR_BUS_KEYTYPE_RANGE));


  switch (err) {
  case  CAPRI_BARCO_SYM_ERR_BUS_ERR:
    return("Bus error when writing status");
  case  CAPRI_BARCO_SYM_ERR_GEN_PUSH_ERR:
    return("Generic Pusher error");
  case  CAPRI_BARCO_SYM_ERR_GEN_FETCH_ERR:
    return("Generic Fetcher error");
  case  CAPRI_BARCO_SYM_ERR_BUS_UNSUP_MODE:
    return("Unsupported mode in this Hw");
  case  CAPRI_BARCO_SYM_ERR_BUS_RSVD:
    return("Reserved");
  case  CAPRI_BARCO_SYM_ERR_BUS_BAD_CMD:
    return("Bad command");
  case  CAPRI_BARCO_SYM_ERR_BUS_UNK_STATE:
    return("Unknown state reached");
  case  CAPRI_BARCO_SYM_ERR_BUS_AXI_BUS_RESP:
    return("AXI bus resp not ok");
  case  CAPRI_BARCO_SYM_ERR_BUS_WRONG_KEYTYPE:
    return("Wrong key type");
  case  CAPRI_BARCO_SYM_ERR_BUS_KEYTYPE_RANGE:
    return("Index key out of range");
  default:
    return(status == 0 ? "success" : "Unknown error");
  }
}

hal_ret_t capri_barco_sym_hash_process_request(crypto_hash_type_e hash_type, bool verify,
					       char *key, int key_len,
					       char *data, int data_len,
					       uint8_t *output_digest, int digest_len);


}    // namespace pd
}    // namespace hal

#endif /* __CAPRI_BARCO_SYM_APIS_H__ */
