/*
 *      Read the BARCO GCM0 PI from global table (locked) in HBM
 * Stage 4, Table 2
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
    barco_pi  : 16;
    pad       : 496;
};
        
struct tx_table_s4_t2_k k;
struct phv_ p;
struct d_struct d;
        
%%
        
tls_enc_read_barco_pi_process:        

table_read_BARCO_PI:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s6_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_2)
    CAPRI_CLEAR_TABLE2_VALID

    /*
     * Currently the barco-PI we have in global table is only for GCM0 Ring.
     * TODO: Add all barco ring PI's in this HBM global table to be shared across programs.
     */
    phvwr           p.to_s5_sw_barco_pi, d.{barco_pi}.hx
    smeqb           c4, k.to_s4_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO
    seq             c5, k.to_s4_do_pre_ccm_enc, 1
    setcf           c6, [!c4 & !c5]
    tblmincri.c6.f  d.{barco_pi}.hx, CAPRI_BARCO_RING_SLOTS_SHIFT, 1

    nop.e
    nop
