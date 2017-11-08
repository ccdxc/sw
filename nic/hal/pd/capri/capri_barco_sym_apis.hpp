#ifndef __CAPRI_BARCO_SYM_APIS_H__
#define __CAPRI_BARCO_SYM_APIS_H__

#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/gen/proto/hal/crypto_apis.pb.h"

namespace hal {
namespace pd {

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

hal_ret_t capri_barco_sym_hash_process_request(cryptoapis::CryptoApiHashType hash_type, bool generate,
					       unsigned char *key, int key_len,
					       unsigned char *data, int data_len,
					       uint8_t *output_digest, int digest_len);


}    // namespace pd
}    // namespace hal

#endif /* __CAPRI_BARCO_SYM_APIS_H__ */
