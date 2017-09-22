/*
 * 	Stats accounting
 *  Stage 6, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s6_t0_k     k;
struct phv_                 p;
struct tx_table_s6_t0_d     d;
	
%%
    .align
tls_dec_post_crypto_stats_process:
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_OPERAND_DEBUG(k.to_s6_rnmdr_free)
    CAPRI_OPERAND_DEBUG(k.to_s6_rnmpr_free)
    /* FIXME: Dec completions accounting */
    CAPRI_OPERAND_DEBUG(k.to_s6_enc_completions)
    CAPRI_COUNTER16_INC(u.tls_post_crypto_stats5_d.rnmdr_free,TLS_POST_CRYPTO_STAT_RNMDR_FREE_OFFSET, k.to_s6_rnmdr_free)
    CAPRI_COUNTER16_INC(u.tls_post_crypto_stats5_d.rnmpr_free, TLS_POST_CRYPTO_STAT_RNMPR_FREE_OFFSET, k.to_s6_rnmpr_free)
    CAPRI_COUNTER16_INC(u.tls_post_crypto_stats5_d.enc_completions, TLS_POST_CRYPTO_STAT_ENC_COMPLETIONS_OFFSET, k.to_s6_enc_completions)
    CAPRI_COUNTER16_INC(u.tls_post_crypto_stats5_d.dec_completions, TLS_POST_CRYPTO_STAT_DEC_COMPLETIONS_OFFSET, k.to_s6_dec_completions)
    tblwr    d.u.tls_post_crypto_stats5_d.debug_stage0_3_thread, k.to_s6_debug_stage0_3_thread
    tblwr    d.u.tls_post_crypto_stats5_d.debug_stage4_7_thread, k.to_s6_debug_stage4_7_thread
    nop.e
    nop
