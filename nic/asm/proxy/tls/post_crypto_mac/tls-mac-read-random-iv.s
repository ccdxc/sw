/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 * 	Read the explicit IV from the Barco DRBG (random number generator) crypto ram
 * Stage 4, Table 1
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

 /* d is the data returned by lookup result */
struct d_struct {
    random_num  : 128;
    pad         : 384;
};

struct tx_table_s4_t1_k k;
struct phv_ p;
struct d_struct d;

%%

tls_mac_read_random_iv:

table_read_RANDOM_IV:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s5_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_1)
    CAPRI_CLEAR_TABLE1_VALID

    /*
     * For CBC encrypt, we use 128-bit random value for the explicit-IV field for the TLS encryption.
     * Fill the explicit IV for the barco-request with the pre-generated random number from the
     * DRBG crypto-ram.
     */
    phvwr    p.crypto_random_iv_explicit_iv, d.random_num
#ifdef ELBA
    /*CAPRI_OPERAND_DEBUG(d.random_num)*/
#else
    CAPRI_OPERAND_DEBUG(d.random_num)
#endif

    nop.e
    nop
