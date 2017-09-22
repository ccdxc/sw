/*
 * 	Construct the barco request in this stage
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s5_t0_k k                  ;
struct phv_ p	;
struct tx_table_s5_t0_tls_post_crypto_stats5_d d	;
	
%%
    .align
tls_enc_post_crypto_stats_process:
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_OPERAND_DEBUG(k.to_s5_rnmdr_free)
    CAPRI_OPERAND_DEBUG(k.to_s5_rnmpr_free)
    CAPRI_OPERAND_DEBUG(k.to_s5_enc_completions)
    CAPRI_COUNTER16_INC(rnmdr_free,TLS_POST_CRYPTO_STAT_RNMDR_FREE_OFFSET, k.to_s5_rnmdr_free)
    CAPRI_COUNTER16_INC(rnmpr_free, TLS_POST_CRYPTO_STAT_RNMPR_FREE_OFFSET, k.to_s5_rnmpr_free)
    CAPRI_COUNTER16_INC(enc_completions, TLS_POST_CRYPTO_STAT_ENC_COMPLETIONS_OFFSET, k.to_s5_enc_completions)
    CAPRI_COUNTER16_INC(dec_completions, TLS_POST_CRYPTO_STAT_DEC_COMPLETIONS_OFFSET, k.to_s5_dec_completions)
    tblwr    d.debug_stage0_3_thread, k.to_s5_debug_stage0_3_thread
    tblwr    d.debug_stage4_7_thread, k.to_s5_debug_stage4_7_thread
    nop.e
    nop
