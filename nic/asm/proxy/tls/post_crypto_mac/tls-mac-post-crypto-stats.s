/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt post-mac pipeline:
 * 	Update stats for this pipeline.
 *  Stage 5, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s5_t0_k     k;
struct phv_                 p;
struct tx_table_s5_t0_tls_post_crypto_stats5_d d	;
	
%%
    .align
tls_mac_post_crypto_stats_process:
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_OPERAND_DEBUG(k.to_s5_tnmdr_alloc)
    CAPRI_OPERAND_DEBUG(k.to_s5_tnmpr_alloc)
    CAPRI_OPERAND_DEBUG(k.to_s5_mac_completions)
    CAPRI_OPERAND_DEBUG(k.to_s5_enc_requests)
    CAPRI_COUNTER16_INC(tnmdr_alloc, TLS_PRE_CRYPTO_STAT_TNMDR_ALLOC_OFFSET, k.to_s5_tnmdr_alloc)
    CAPRI_COUNTER16_INC(tnmpr_alloc, TLS_PRE_CRYPTO_STAT_TNMPR_ALLOC_OFFSET, k.to_s5_tnmpr_alloc)
    CAPRI_COUNTER16_INC(mac_completions, TLS_POST_CRYPTO_STAT_MAC_COMPLETIONS_OFFSET, k.to_s5_mac_completions)
    CAPRI_COUNTER16_INC(enc_requests, TLS_PRE_CRYPTO_STAT_ENC_REQUESTS_OFFSET, k.to_s5_enc_requests)
    tblwr    d.debug_stage0_3_thread, k.to_s5_debug_stage0_3_thread
    tblwr    d.debug_stage4_7_thread, k.to_s5_debug_stage4_7_thread
    nop.e
    nop
