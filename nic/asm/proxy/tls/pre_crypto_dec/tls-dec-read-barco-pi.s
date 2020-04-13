/*
 *      Read the BARCO GCM1 PI from global table (locked) in HBM
 * Stage 4, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s4_t0_k k;
struct phv_ p;
struct tx_table_s4_t0_tls_read_barco_pi_d d;
        
%%
        
tls_dec_read_barco_pi_process:        
    .param      tls_dec_bld_barco_req_process
	
table_read_BARCO_PI:
    CAPRI_CLEAR_TABLE0_VALID

    smeqb           c1, k.tls_global_phv_debug_dol, TLS_DDOL_BYPASS_BARCO, TLS_DDOL_BYPASS_BARCO
    seq             c2, k.tls_global_phv_write_arq, 1
    bcf             [c1 | c2], tls_dec_read_barco_pi_process_skip_alloc

    add             r1, d.{pi}.hx, 0
    mincr           r1, ASIC_BARCO_RING_SLOTS_SHIFT, 1
    seq             c1, r1, d.{ci}.hx
    b.c1            tls_dec_barco_full

    phvwr           p.s4_s6_t0_phv_sw_barco_pi, d.{pi}.hx
    tblwr.f         d.{pi}, r1.hx

tls_dec_read_barco_pi_process_skip_alloc:

    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_bld_barco_req_process,
                                 k.tls_global_phv_qstate_addr, TLS_TCB_CONFIG,
                                 TABLE_SIZE_512_BITS)
	
    nop.e
    nop

tls_dec_barco_full:
    tbladd.f        d.{stat_qfull}, 1
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    illegal
    nop.e
    nop
