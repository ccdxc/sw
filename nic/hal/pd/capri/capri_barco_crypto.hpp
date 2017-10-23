#ifndef __CAPRI_BARCO_CRYPTO_HPP__
#define __CAPRI_BARCO_CRYPTO_HPP__

#include "nic/gen/proto/hal/types.pb.h"

namespace hal {
namespace pd {
#define BARCO_CRYPTO_DESC_SZ                128 /* 1024 bits */
#define BARCO_CRYPTO_DESC_ALIGN_BYTES       128

#define BARCO_CRYPTO_KEY_DESC_SZ            16 /* 128 bits */
#define BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES   16

/* FIXME: this needs to be driven from HAL PD, but the includes do not make it to capri */
#define CRYPTO_KEY_COUNT_MAX                (64 * 1024)

hal_ret_t capri_barco_rings_init(void);
hal_ret_t capri_barco_res_allocator_init(void);
hal_ret_t capri_barco_crypto_init(void);
hal_ret_t capri_barco_init_key(uint32_t key_idx, uint64_t key_addr);
hal_ret_t capri_barco_setup_key(uint32_t key_idx, types::CryptoKeyType key_type, uint8_t *key,
        uint32_t key_size);
hal_ret_t capri_barco_read_key(uint32_t key_idx, types::CryptoKeyType *key_type,
        uint8_t *key, uint32_t *key_size);

/* Barco Crypto specific definitions */
typedef struct capri_barco_key_desc_s {
    uint64_t                key_address;
    uint32_t                key_type;
    uint32_t                reserved;
} __PACK__ capri_barco_key_desc_t;

#define CAPRI_BARCO_KEYTYPE_SHIFT       28
#define CAPRI_BARCO_KEYTYPE_AES         (0x0 << CAPRI_BARCO_KEYTYPE_SHIFT)
#define CAPRI_BARCO_KEYTYPE_AES128      (CAPRI_BARCO_KEYTYPE_AES | 0x0000010)
#define CAPRI_BARCO_KEYTYPE_AES192      (CAPRI_BARCO_KEYTYPE_AES | 0x0000018)
#define CAPRI_BARCO_KEYTYPE_AES256      (CAPRI_BARCO_KEYTYPE_AES | 0x0000020)
#define CAPRI_BARCO_KEYTYPE_DES         ((0x1 << CAPRI_BARCO_KEYTYPE_SHIFT) | (0x0000070))
#define CAPRI_BARCO_KEYTYPE_CHACHA20    ((0x2 << CAPRI_BARCO_KEYTYPE_SHIFT) | (0x0000020))
#define CAPRI_BARCO_KEYTYPE_POY1305     ((0x3 << CAPRI_BARCO_KEYTYPE_SHIFT) | (0x0000020))
#define CAPRI_BARCO_KEYTYPE_HMAC_TYPE   (0x4 << CAPRI_BARCO_KEYTYPE_SHIFT)
#define CAPRI_BARCO_KEYTYPE_HMAC_LEN_MASK   (0xfffffff)
#define CAPRI_BARCO_KEYTYPE_HMAC(len)   ((CAPRI_BARCO_KEYTYPE_HMAC_TYPE) | \
                                        (len & CAPRI_BARCO_KEYTYPE_HMAC_LEN_MASK))


}    // namespace pd
}    // namespace hal

#endif /*  __CAPRI_BARCO_CRYPTO_HPP__ */
