/*
 *      Read the BARCO GCM0 PI from global table (locked) in HBM
 * Stage 1, Table 1
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s1_t1_k k;
struct phv_ p;
struct tx_table_s1_t1_d d;
        
%%
        
tls_enc_free_barco_ci:
    CAPRI_CLEAR_TABLE1_VALID
    smeqb           c4, k.tls_global_phv_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO
    tblmincri.!c4.f d.{u.tls_free_barco_slot_d.ci}.hx, ASIC_BARCO_RING_SLOTS_SHIFT, 1
	
tls_enc_free_barco_ci_done:
    nop.e
    nop
