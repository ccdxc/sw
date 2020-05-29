// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_BARCO_SYM_APIS_HPP__
#define __ELBA_BARCO_SYM_APIS_HPP__

#include "platform/elba/elba_barco_crypto.hpp"

namespace sdk {
namespace platform {
namespace elba {

typedef enum {
    ELBA_SYMM_ENCTYPE_NONE = 0,
    ELBA_SYMM_ENCTYPE_AES_GCM = 1,
    ELBA_SYMM_ENCTYPE_AES_CCM = 2,
    ELBA_SYMM_ENCTYPE_AES_SHA256_CBC = 3,
    ELBA_SYMM_ENCTYPE_AES_SHA384_CBC = 4,
    ELBA_SYMM_ENCTYPE_AES_CBC_SHA256 = 5,
    ELBA_SYMM_ENCTYPE_AES_CBC_SHA384 = 6,
    ELBA_SYMM_ENCTYPE_AES_HMAC_SHA256_CBC = 7,
    ELBA_SYMM_ENCTYPE_AES_HMAC_SHA384_CBC = 8,
    ELBA_SYMM_ENCTYPE_AES_CBC_HMAC_SHA256 = 9,
    ELBA_SYMM_ENCTYPE_AES_CBC_HMAC_SHA384 = 10
} elba_barco_symm_enctype_e;

#define TLS_AES_CCM_AUTH_TAG_SIZE             16
#define TLS_AES_CCM_LENGTH_FIELD_SIZE          3
#define TLS_AES_CCM_AAD_PRESENT                1
#define TLS_AES_CCM_NONCE_SIZE                12
#define TLS_AES_CCM_NONCE_SALT_SIZE            4
#define TLS_AES_CCM_NONCE_EXPLICIT_SIZE        8
#define TLS_AES_CCM_AAD_SIZE                  13
#define TLS_AES_CCM_HEADER_BLOCK_SIZE         16
#define TLS_AES_CCM_HEADER_SIZE               (TLS_AES_CCM_HEADER_BLOCK_SIZE * 2)
#define TLS_AES_CCM_HEADER_AAD_OFFSET         18 // 3rd byte of B_1

#define TLS_AES_CCM_HDR_B0_FLAGS  \
    (TLS_AES_CCM_AAD_PRESENT << 6 | \
    (((TLS_AES_CCM_AUTH_TAG_SIZE - 2)/2) << 3) | \
    (TLS_AES_CCM_LENGTH_FIELD_SIZE -1))

static inline const char *
elba_barco_symm_enctype_name (elba_barco_symm_enctype_e encaptype)
{
    switch(encaptype) {
        case ELBA_SYMM_ENCTYPE_NONE:
            return("None");
        case  ELBA_SYMM_ENCTYPE_AES_GCM:
            return("AES-GCM");
        case ELBA_SYMM_ENCTYPE_AES_CCM:
            return("AES-CCM");
        case ELBA_SYMM_ENCTYPE_AES_SHA256_CBC:
            return("AES-HASH-SHA256-CBC");
        case ELBA_SYMM_ENCTYPE_AES_SHA384_CBC:
            return("AES-HASH-SHA384-CBC");
        case ELBA_SYMM_ENCTYPE_AES_CBC_SHA256:
            return("AES-HASH-CBC-SHA256");
        case ELBA_SYMM_ENCTYPE_AES_CBC_SHA384:
            return("AES-HASH-CBC-SHA384");
        case ELBA_SYMM_ENCTYPE_AES_HMAC_SHA256_CBC:
            return("AES-HASH-HMAC-SHA256-CBC");
        case ELBA_SYMM_ENCTYPE_AES_HMAC_SHA384_CBC:
            return("AES-HASH-HMAC-SHA384-CBC");
        case ELBA_SYMM_ENCTYPE_AES_CBC_HMAC_SHA256:
            return("AES-HASH-CBC-HMAC-SHA256");
        case ELBA_SYMM_ENCTYPE_AES_CBC_HMAC_SHA384:
            return("AES-HASH-CBC-HMAC-SHA384");
        default:
            return("Unknown");
    }
    return("Unknown");
}

static inline const char *
barco_symm_err_status_str (uint64_t status)
{
    uint32_t err = (status & (ELBA_BARCO_SYM_ERR_BUS_ERR |
                              ELBA_BARCO_SYM_ERR_GEN_PUSH_ERR |
                              ELBA_BARCO_SYM_ERR_GEN_FETCH_ERR |
                              ELBA_BARCO_SYM_ERR_BUS_UNSUP_MODE |
                              ELBA_BARCO_SYM_ERR_BUS_RSVD |
                              ELBA_BARCO_SYM_ERR_BUS_BAD_CMD |
                              ELBA_BARCO_SYM_ERR_BUS_UNK_STATE |
                              ELBA_BARCO_SYM_ERR_BUS_AXI_BUS_RESP |
                              ELBA_BARCO_SYM_ERR_BUS_WRONG_KEYTYPE |
                              ELBA_BARCO_SYM_ERR_BUS_KEYTYPE_RANGE));

    switch (err) {
        case  ELBA_BARCO_SYM_ERR_BUS_ERR:
            return("Bus error when writing status");
        case  ELBA_BARCO_SYM_ERR_GEN_PUSH_ERR:
            return("Generic Pusher error");
        case  ELBA_BARCO_SYM_ERR_GEN_FETCH_ERR:
            return("Generic Fetcher error");
        case  ELBA_BARCO_SYM_ERR_BUS_UNSUP_MODE:
            return("Unsupported mode in this Hw");
        case  ELBA_BARCO_SYM_ERR_BUS_RSVD:
            return("Reserved");
        case  ELBA_BARCO_SYM_ERR_BUS_BAD_CMD:
            return("Bad command");
        case  ELBA_BARCO_SYM_ERR_BUS_UNK_STATE:
            return("Unknown state reached");
        case  ELBA_BARCO_SYM_ERR_BUS_AXI_BUS_RESP:
            return("AXI bus resp not ok");
        case  ELBA_BARCO_SYM_ERR_BUS_WRONG_KEYTYPE:
            return("Wrong key type");
        case  ELBA_BARCO_SYM_ERR_BUS_KEYTYPE_RANGE:
            return("Index key out of range");
        default:
            return(status == 0 ? "success" : "Unknown error");
    }
}

sdk_ret_t elba_barco_sym_hash_process_request(CryptoApiHashType hash_type,
                                              bool generate,
                                              unsigned char *key, int key_len,
                                              unsigned char *data, int data_len,
                                              uint8_t *output_digest,
                                              int digest_len);

sdk_ret_t elba_barco_sym_aes_encrypt_process_request(elba_barco_symm_enctype_e enc_type, bool encrypt,
                                                     uint8_t *key, int key_len,
                                                     uint8_t *header,
                                                     int header_len,
                                                     uint8_t *plaintext,
                                                     int plaintext_len,
                                                     uint8_t *iv,
                                                     int iv_len,
                                                     uint8_t *ciphertext,
                                                     int ciphertext_len,
                                                     uint8_t *auth_tag,
                                                     int auth_tag_len,
                                                     bool schedule_barco);

void elba_barco_init_drbg(void);

}   // namespace elba
}   // namespace platform
}   // namespace sdk

#endif //   __ELBA_BARCO_SYM_APIS_HPP__
