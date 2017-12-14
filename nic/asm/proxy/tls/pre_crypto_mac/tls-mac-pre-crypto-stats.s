/*
 * AES-CBC-HMAC-SHA2 Mac-then-encrypt pre-mac pipeline:
 * 	Update stats for this pipeline.
 *  Stage 6, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct tx_table_s6_t0_k k                  ;
struct phv_ p	;
struct tx_table_s6_t0_tls_pre_crypto_stats6_d d	;
	
%%
    .align
tls_mac_pre_crypto_stats_process:
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_OPERAND_DEBUG(k.to_s6_mac_requests)
    CAPRI_COUNTER16_INC(mac_requests, TLS_PRE_CRYPTO_STAT_MAC_REQUESTS_OFFSET, k.to_s6_mac_requests)
    tblwr    d.debug_stage0_3_thread, k.to_s6_debug_stage0_3_thread
    tblwr    d.debug_stage4_7_thread, k.to_s6_debug_stage4_7_thread
    nop.e
    nop
        
