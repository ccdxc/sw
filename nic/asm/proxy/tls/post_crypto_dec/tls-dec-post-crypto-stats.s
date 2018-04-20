/*
 * 	Stats accounting
 *  Stage 7, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s7_t3_k     k;
struct phv_                 p;
struct tx_table_s7_t3_d     d;
	
%%
    .align
tls_dec_post_crypto_stats_process:
    CAPRI_OPERAND_DEBUG(k.stats_rnmdr_free)
    CAPRI_OPERAND_DEBUG(k.stats_rnmpr_free)
    /* FIXME: Dec completions accounting */
    CAPRI_OPERAND_DEBUG(k.stats_enc_completions)
    CAPRI_COUNTER16_INC(u.tls_post_crypto_stats5_d.rnmdr_free,TLS_POST_CRYPTO_STAT_RNMDR_FREE_OFFSET, k.stats_rnmdr_free)
    CAPRI_COUNTER16_INC(u.tls_post_crypto_stats5_d.rnmpr_free, TLS_POST_CRYPTO_STAT_RNMPR_FREE_OFFSET, k.stats_rnmpr_free)
    CAPRI_COUNTER16_INC(u.tls_post_crypto_stats5_d.enc_completions, TLS_POST_CRYPTO_STAT_ENC_COMPLETIONS_OFFSET, k.stats_enc_completions)
    CAPRI_COUNTER16_INC(u.tls_post_crypto_stats5_d.dec_completions, TLS_POST_CRYPTO_STAT_DEC_COMPLETIONS_OFFSET, k.stats_dec_completions)
    tblwr    d.u.tls_post_crypto_stats5_d.debug_stage0_3_thread, k.stats_debug_stage0_3_thread
    tblwr    d.u.tls_post_crypto_stats5_d.debug_stage4_7_thread, k.stats_debug_stage4_7_thread
    nop.e
    nop
