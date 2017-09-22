/* 
 *  Read odesc to setup the DMA request
 *  Stage 4, Table 0
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s4_t0_d     d;
struct tx_table_s4_t0_k     k;
struct phv_                 p;

%%
    .param      tls_dec_queue_sesq_process

tls_dec_post_read_odesc:
    phvwr       p.odesc_A0, d.u.tls_read_odesc_d.A0
    /* Trim off the AAD from the output */
    addi        r2, r0, NTLS_AAD_SIZE
    add         r1, d.{u.tls_read_odesc_d.O0}.wx, r2
    phvwr       p.odesc_O0, r1.wx
    sub         r1, d.{u.tls_read_odesc_d.L0}.wx, r2
    phvwr       p.odesc_L0, r1.wx


    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tls_dec_queue_sesq_process,
                           k.tls_global_phv_qstate_addr,
                       	   TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)

tls_dec_post_read_odesc_done:
    nop.e
    nop.e
    
