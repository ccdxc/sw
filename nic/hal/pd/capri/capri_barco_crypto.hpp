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

/* Barco Crypto Asym specific definitions */
typedef struct capri_barco_asym_key_desc_s {
    uint64_t                key_param_list;
    uint32_t                command_reg;
    uint32_t                reserved;
} __PACK__ capri_barco_asym_key_desc_t;

/* Asymmetric/PKE Command Definitions */
#define CAPRI_BARCO_ASYM_CMD_CALCR2         0x80000000

#define CAPRI_BARCO_ASYM_CMD_FLAGB          0x40000000

#define CAPRI_BARCO_ASYM_CMD_FLAGA          0x20000000

#define CAPRI_BARCO_ASYM_CMD_SWAP_BYTES     0x10000000

#define CAPRI_BARCO_ASYM_CMD_BUFFER_SEL     0x08000000

#define CAPRI_BARCO_ASYM_CMD_R_AND_PROJ     0x02000000

#define CAPRI_BARCO_ASYM_CMD_R_AND_KE       0x01000000

#define CAPRI_BARCO_ASYM_CMD_SEL_CURVE_NO_ACCEL     0x00000000
#define CAPRI_BARCO_ASYM_CMD_SEL_CURVE_P256         0x00100000
#define CAPRI_BARCO_ASYM_CMD_SEL_CURVE_P384         0x00200000
#define CAPRI_BARCO_ASYM_CMD_SEL_CURVE_P521_E521    0x00300000
#define CAPRI_BARCO_ASYM_CMD_SEL_CURVE_P192         0x00400000
#define CAPRI_BARCO_ASYM_CMD_SEL_CURVE_CURVE25519_ED25519   0x00500000

#define CAPRI_BARCO_ASYM_CMD_SIZE_OF_OP_SHIFT   8
#define CAPRI_BARCO_ASYM_CMD_SIZE_OF_OP(size)   ((size - 1) << CAPRI_BARCO_ASYM_CMD_SIZE_OF_OP_SHIFT)

#define CAPRI_BARCO_ASYM_CMD_FIELD_GFP      0x00000000
#define CAPRI_BARCO_ASYM_CMD_FIELD_GF2M     0x00000080

/* Primitive Arithmetic Operations GF(p) and GF(2^m) */
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_NA             0x00
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_ADD        0x01
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_SUB        0x02
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_MUL_ODD_N  0x03
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_RED_ODD_N  0x04
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_DIV_ODD_N  0x05
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_INV_ODD_N  0x06
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_SQRT       0x07
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MUL            0x08
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_INV_EVEN_N 0x09
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_MOD_RED_EVEN_N 0x0a
#define CAPRI_BARCO_ASYM_CMD_OP_TYPE_CLEAR_DATA_MEM 0x0f

/* High-level RSA, CRT & DSA Operations - GF(p) only */
#define CAPRI_BARCO_ASYM_CMD_MOD_EXPO               0x10
#define CAPRI_BARCO_ASYM_CMD_RSA_PK_GEN             0x11
#define CAPRI_BARCO_ASYM_CMD_RSA_CRT_KEY_PARAM_GEN  0x12
#define CAPRI_BARCO_ASYM_CMD_RSA_CRT_DECRYPT        0x13
#define CAPRI_BARCO_ASYM_CMD_RSA_ENCRYPT            0x14
#define CAPRI_BARCO_ASYM_CMD_RSA_DECRYPT            0x15
#define CAPRI_BARCO_ASYM_CMD_RSA_SIG_GEN            0x16
#define CAPRI_BARCO_ASYM_CMD_RSA_SIG_VERIFY         0x17
#define CAPRI_BARCO_ASYM_CMD_DSA_KEY_GEN            0x18
#define CAPRI_BARCO_ASYM_CMD_DSA_SIG_GEN            0x19
#define CAPRI_BARCO_ASYM_CMD_DSA_SIG_VERIFY         0x1a
#define CAPRI_BARCO_ASYM_CMD_SRP_SERVER_SESS_KEY    0x1b
#define CAPRI_BARCO_ASYM_CMD_SRP_CLIENT_SESS_KEY    0x1c
#define CAPRI_BARCO_ASYM_CMD_RSA_HALF_CRT_RECOMB    0x1d
#define CAPRI_BARCO_ASYM_CMD_SRP_SERVER_PUB_KEY     0x1e
#define CAPRI_BARCO_ASYM_CMD_RSA_HALF_CRT_DECRYPT   0x1f

/* Primitive ECC & Check Point Operations GF(p) & GF(2m) */
#define CAPRI_BARCO_ASYM_CMD_ECC_POINT_DOUBLE       0x20
#define CAPRI_BARCO_ASYM_CMD_ECC_POINT_ADD          0x21
#define CAPRI_BARCO_ASYM_CMD_ECC_POINT_MUL          0x22
#define CAPRI_BARCO_ASYM_CMD_ECC_CHECK_A_AND_B      0x23
#define CAPRI_BARCO_ASYM_CMD_ECC_CHECK_N_NE_Q       0x24
#define CAPRI_BARCO_ASYM_CMD_ECC_CHECK_X_Y_LT_Q     0x25
#define CAPRI_BARCO_ASYM_CMD_ECC_CHECK_POINT_ON_CURVE   0x26
#define CAPRI_BARCO_ASYM_CMD_ECC_POINT_DECOMPRESS   0x27
#define CAPRI_BARCO_ASYM_CMD_ECC_MONT_POINT_MUL     0x28
#define CAPRI_BARCO_ASYM_CMD_SM2_SIG_GEN            0x2d
#define CAPRI_BARCO_ASYM_CMD_SM2_SIG_VERIFY         0x2e
#define CAPRI_BARCO_ASYM_CMD_SM2_KEX                0x2f


/* High-level ECC – ECDSA Operations GF(p) & GF(2m) */
#define CAPRI_BARCO_ASYM_ECDSA_SIG_GEN              0x30
#define CAPRI_BARCO_ASYM_ECDSA_SIG_VERIFY           0x31
#define CAPRI_BARCO_ASYM_ECDSA_DOMAIN_PARAM_VALIDATION  0x32
#define CAPRI_BARCO_ASYM_ECKCDSA_PUB_KEY_GEN        0x33
#define CAPRI_BARCO_ASYM_ECKCDSA_SIG_GEN            0x34
#define CAPRI_BARCO_ASYM_ECKCDSA_SIG_VERIFY         0x35
#define CAPRI_BARCO_ASYM_JPAKE_GEN_ZKP              0x36
#define CAPRI_BARCO_ASYM_JPAKE_VERIFY_ZKP           0x37
#define CAPRI_BARCO_ASYM_JPAKE_2_POINT_ADD          0x38
#define CAPRI_BARCO_ASYM_JPAKE_GEN_SESS_KEY         0x39
#define CAPRI_BARCO_ASYM_JPAKE_GEN_STEP_2           0x3a
#define CAPRI_BARCO_ASYM_EDDSA_POINT_MUL            0x3b
#define CAPRI_BARCO_ASYM_EDDSA_SIG_GEN              0x3c
#define CAPRI_BARCO_ASYM_EDDSA_SIG_VERIFY           0x3d
#define CAPRI_BARCO_ASYM_EDDSA_GEN_SESS_KEY         0x3e

/* Primality Test – Rabin-Miller */
#define CAPRI_BARCO_ASYM_ROUND_RABIN_MILLER         0x40
#define CAPRI_BARCO_ASYM_INIT_RABIN_MILLER          0x41





}    // namespace pd
}    // namespace hal

#endif /*  __CAPRI_BARCO_CRYPTO_HPP__ */
