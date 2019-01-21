#ifndef _TLS_CONSTANTS_H_
#define _TLS_CONSTANTS_H_

#include "proxy-constants.h"

#define MAX_ENTRIES_PER_DESC 2
#define MAX_ENTRIES_PER_DESC_MASK 0xF

#define NIC_BRQ_ENTRY_SIZE                   128
#define NIC_BRQ_ENTRY_SIZE_SHIFT             7          /* for 128B */

#define NIC_DESC_DATA_LEN_OFFSET             0x1c       /* &((nic_desc_t *)0)->data_len */
#define NIC_DESC_ENTRY0_OFFSET               0x20       /* &((nic_desc_t *)0)->entry[0] */

#define PKT_DESC_STATUS_OFFSET               0
#define PKT_DESC_ODESC_OFFSET                4
#define PKT_DESC_OPAGE_OFFSET                (PKT_DESC_ODESC_OFFSET + 8)
#define PKT_DESC_AOL_OFFSET                  64
#define PKT_DESC_AOL_OFFSET_A0               (PKT_DESC_AOL_OFFSET)
#define PKT_DESC_AOL_OFFSET_O0               (PKT_DESC_AOL_OFFSET_A0 + 8)
#define PKT_DESC_AOL_OFFSET_L0               (PKT_DESC_AOL_OFFSET_O0 + 4)
#define PKT_DESC_AOL_OFFSET_A1               (PKT_DESC_AOL_OFFSET_L0 + 4)
#define PKT_DESC_AOL_OFFSET_O1               (PKT_DESC_AOL_OFFSET_A1 + 8)
#define PKT_DESC_AOL_OFFSET_L1               (PKT_DESC_AOL_OFFSET_O1 + 4)
#define PKT_DESC_AOL_OFFSET_A2               (PKT_DESC_AOL_OFFSET_L1 + 4)
#define PKT_DESC_AOL_OFFSET_O2               (PKT_DESC_AOL_OFFSET_A2 + 8)
#define PKT_DESC_AOL_OFFSET_L2               (PKT_DESC_AOL_OFFSET_O2 + 4)
#define PKT_DESC_AOL_OFFSET_NXT_ADDR         (PKT_DESC_AOL_OFFSET_L2 + 4)
#define PKT_DESC_AOL_OFFSET_NXT_PKT          (PKT_DESC_AOL_OFFSET_NXT_ADDR + 8)

/* Offsets to Barco inputs stored/DMAed into opage */
#define BARCO_INPUT_OPAGE_AAD                0
#define BARCO_INPUT_OPAGE_IV                 (BARCO_INPUT_OPAGE_AAD + 16)
#define BARCO_INPUT_OPAGE_AUTHTAG            (BARCO_INPUT_OPAGE_IV + 16)

#define ENC_FLOW_ID_MASK 0x8000

#define BARCO_SYM_CMD_CORE_AES         0
#define BARCO_SYM_CMD_CORE_DESC        1
#define BARCO_SYM_CMD_CORE_HASH        2
#define BARCO_SYM_CMD_CORE_AES_GCM     3
#define BARCO_SYM_CMD_CORE_AES_XTS     4
#define BARCO_SYM_CMD_CORE_CHACHA      5
#define BARCO_SYM_CMD_CORE_SHA3        6
#define BARCO_SYM_CMD_CORE_AES_HASH    7

#define BARCO_SYM_CMD_KEYSIZE_AES128 0
#define BARCO_SYM_CMD_KEYSIZE_AES256 1

#define BARCO_SYM_CMD_MODE_AES_CBC     1
#define BARCO_SYM_CMD_MODE_AES_CCM     5

#define BARCO_SYM_CMD_MODE_DES_ECB     0
#define BARCO_SYM_CMD_MODE_DES_CBC     1

#define BARCO_SYM_CMD_MODE_HASH_MD5    0
#define BARCO_SYM_CMD_MODE_HASH_SHA1   1
#define BARCO_SYM_CMD_MODE_HASH_SHA224 2
#define BARCO_SYM_CMD_MODE_HASH_SHA256 3
#define BARCO_SYM_CMD_MODE_HASH_SHA384 4
#define BARCO_SYM_CMD_MODE_HASH_SHA512 5

#define BARCO_SYM_CMD_MODE_AES_GCM     0
#define BARCO_SYM_CMD_MODE_AES_XTS     0

#define BARCO_SYM_CMD_MODE_CHACHA20    0
#define BARCO_SYM_CMD_MODE_CHACHA20_POLY1305 1

#define BARCO_SYM_CMD_MODE_SHA3_224    0
#define BARCO_SYM_CMD_MODE_SHA3_256    1
#define BARCO_SYM_CMD_MODE_SHA3_384    2
#define BARCO_SYM_CMD_MODE_SHA3_512    3

#define BARCO_SYM_CMD_MODE_CBC_SHA256  1
#define BARCO_SYM_CMD_MODE_CBC_SHA384  2
#define BARCO_SYM_CMD_MODE_SHA256_CBC  3
#define BARCO_SYM_CMD_MODE_SHA384_CBC  4



#define BARCO_SYM_CMD_OP_ENCRYPT       0
#define BARCO_SYM_CMD_OP_DECRYPT       1
#define BARCO_SYM_CMD_OP_GEN_HASH      0
#define BARCO_SYM_CMD_OP_VERIFY_HASH   1
#define BARCO_SYM_CMD_OP_GENERATE_HMAC 2
#define BARCO_SYM_CMD_OP_VERIFY_HMAC   3

#define NTLS_RECORD_DATA                0x17

/* Constants */
#define NTLS_AES_GCM_128_IV_SIZE        (8)
#define NTLS_AES_GCM_128_KEY_SIZE       (16)
#define NTLS_AES_GCM_128_SALT_SIZE      (4)

#define NTLS_KEY_SIZE                   NTLS_AES_GCM_128_KEY_SIZE
#define NTLS_SALT_SIZE                  NTLS_AES_GCM_128_SALT_SIZE
#define NTLS_TAG_SIZE                   16
#define NTLS_IV_SIZE                    NTLS_AES_GCM_128_IV_SIZE
#define NTLS_NONCE_SIZE                 8

#define NTLS_DATA_PAGES                 (NTLS_MAX_PAYLOAD_SIZE / PAGE_SIZE)
/* +1 for aad, +1 for tag, +1 for chaining */
#define NTLS_SG_DATA_SIZE               (NTLS_DATA_PAGES + 3)

#define NTLS_AAD_SPACE_SIZE             21
#define NTLS_AAD_SIZE                   13

/* TLS
 */
#define NTLS_TLS_HEADER_SIZE            5
#define NTLS_HEADROOM                   128
#define NTLS_TLS_PREPEND_SIZE           (NTLS_TLS_HEADER_SIZE + NTLS_NONCE_SIZE)
#define NTLS_TLS_OVERHEAD               (NTLS_TLS_PREPEND_SIZE + NTLS_TAG_SIZE)

#define NTLS_TLS_1_2_MAJOR              0x03
#define NTLS_TLS_1_2_MINOR              0x03

#define TLS_AES_GCM_AUTH_TAG_SIZE             16
#define TLS_AES_CBC_RANDOM_IV_SIZE            16
#define TLS_AES_CBC_BLOCK_SIZE                16
#define TLS_AES_CBC_HMAC_SHA256_AUTH_TAG_SIZE 32

/*
 * For TLS CCM ciphers, we use 2 16-byte blocks B_0 and B_1 to specify Header to Barco:
 *
 * CCM header block B_0 encoding, based on RFC 3610, Sec. 2.2
 *   M = 16 (TLS_AES_CCM_AUTH_TAG_SIZE), Flags encoding (M-2/2)
 *   L = 3  (24-bit field for specifying length of plaintext), Flags encoding (L-1)
 *   Nonce-size = 12 ((15 - L), with 4 byte Salt + 8 byte explicit-IV)
 * B_0 (16-byte block): 1-byte Flags + 12-byte Nonce + 3-byte Length-field
 *
 * B_1 (16-byte block): 2-byte AAD-size (value 13) + 13-byte AAD (8-byte seq-num + 5-byte TLS header) + 1-byte zero-pad
 *
 */
#define TLS_AES_CCM_AUTH_TAG_SIZE             16
#define TLS_AES_CCM_LENGTH_FIELD_SIZE          3
#define TLS_AES_CCM_AAD_PRESENT                1
#define TLS_AES_CCM_NONCE_SIZE                12
#define TLS_AES_CCM_NONCE_SALT_SIZE            4
#define TLS_AES_CCM_NONCE_EXPLICIT_SIZE        8
#define TLS_AES_CCM_HEADER_BLOCK_SIZE         TLS_AES_CBC_BLOCK_SIZE
#define TLS_AES_CCM_HEADER_SIZE               (TLS_AES_CCM_HEADER_BLOCK_SIZE * 2)
#define TLS_AES_CCM_HEADER_AAD_OFFSET         18 // 3rd byte of B_1

#define TLS_AES_CCM_HDR_B0_FLAGS  \
    (TLS_AES_CCM_AAD_PRESENT << 6 | \
    (((TLS_AES_CCM_AUTH_TAG_SIZE - 2)/2) << 3) | \
    (TLS_AES_CCM_LENGTH_FIELD_SIZE -1))

/* nonce explicit offset in a record */
#define NTLS_TLS_NONCE_OFFSET           NTLS_TLS_HEADER_SIZE


#define NTLS_RECORD_DATA                0x17
#define NTLS_RECORD_HANDSHAKE           0x16

#define BARCO_SYM_STATUS_SUCCESS 0

/* debug dol encoding to affect the runtime TLS processing */
#define TLS_DDOL_BYPASS_BARCO           0x1    /* Enqueue the request to BRQ, but bypass updating the PI of barco and 
                                              * ring BSQ doorbell 
                                              */
#define TLS_DDOL_SESQ_STOP              0x2    /* Enqueue the request to SESQ, but donot ring the doorbell to TCP */

#define TLS_DDOL_BYPASS_PROXY           0x4    /* Don't queue to other flow , keep in same flow */
#define TLS_DDOL_LEAVE_IN_ARQ           0x8    /* Don't queue to ARQ (arm) */
/* MAKE SURE that this is in sync with the define in hal/plugin/proxy/proxy_tls.cc*/
#define TLS_DDOL_ARM_LOOP_CTRL_PKTS     0x10   /* Loopback control packets in ARM */
#define TLS_DDOL_EXPLICIT_IV_USE_RANDOM 0x20   /* Use DRBG random value for explicit IV */   
#define TLS_DDOL_DEC_REASM_PATH         0x40   /* Use the new decrypt reassembly path */
/* HACK used until the issue with payload generation is fixed in DoL framework */
#define TLS_DDOL_FAKE_HANDSHAKE_MSG     0x80

/*

TLS CB is split into 8 64B chunks as below.
PRE_CRYPTO atomic stats start at offset 2*64 and use up 128B
POST_CRYPTO atomic stats start at offset 5*64 and use up 128B

typedef enum tlscb_hwid_order_ {
    P4PD_HWID_TLS_TX_S0_T0_READ_TLS_STG0 = 0,
    P4PD_HWID_TLS_TX_S3_T0_READ_TLS_ST1_7 = 1,
    P4PD_HWID_TLS_TX_PRE_CRYPTO_STATS_U16 = 2,

    P4PD_HWID_TLS_TX_PRE_CRYPTO_STATS1_U64 = 3,           <===  pre-crypto atomic stats start block
    P4PD_HWID_TLS_TX_PRE_CRYPTO_STATS2_U64 = 4,           <===  pre-crypto atomic stats end block

    P4PD_HWID_TLS_TX_POST_CRYPTO_STATS_U16 = 5,


    P4PD_HWID_TLS_TX_POST_CRYPTO_STATS1_U64 = 6,           <===  post-crypto atomic stats start block          
    P4PD_HWID_TLS_TX_POST_CRYPTO_STATS2_U64 = 7,           <===  post-crypto atomic stats end block
} tlscb_hwid_order_t;

*/


#define TLS_PRE_CRYPTO_STAT_OFFSET(_num)            ((64*2) + (_num * 8))
#define TLS_PRE_CRYPTO_STAT_TNMDR_ALLOC_OFFSET      TLS_PRE_CRYPTO_STAT_OFFSET(0)
#define TLS_PRE_CRYPTO_STAT_TNMPR_ALLOC_OFFSET      TLS_PRE_CRYPTO_STAT_OFFSET(1)
#define TLS_PRE_CRYPTO_STAT_ENC_REQUESTS_OFFSET     TLS_PRE_CRYPTO_STAT_OFFSET(2)
#define TLS_PRE_CRYPTO_STAT_DEC_REQUESTS_OFFSET     TLS_PRE_CRYPTO_STAT_OFFSET(3)
#define TLS_PRE_CRYPTO_STAT_MAC_REQUESTS_OFFSET     TLS_PRE_CRYPTO_STAT_OFFSET(4)

#define TLS_POST_CRYPTO_STAT_OFFSET(_num)           ((64*5) + (_num * 8))
#define TLS_POST_CRYPTO_STAT_RNMDR_FREE_OFFSET      TLS_POST_CRYPTO_STAT_OFFSET(0)
#define TLS_POST_CRYPTO_STAT_RNMPR_FREE_OFFSET      TLS_POST_CRYPTO_STAT_OFFSET(1)
#define TLS_POST_CRYPTO_STAT_ENC_COMPLETIONS_OFFSET TLS_POST_CRYPTO_STAT_OFFSET(2)
#define TLS_POST_CRYPTO_STAT_DEC_COMPLETIONS_OFFSET TLS_POST_CRYPTO_STAT_OFFSET(3)
#define TLS_POST_CRYPTO_STAT_MAC_COMPLETIONS_OFFSET TLS_POST_CRYPTO_STAT_OFFSET(4)

#endif /* #ifndef _TLS_CONSTANTS_H_ */
