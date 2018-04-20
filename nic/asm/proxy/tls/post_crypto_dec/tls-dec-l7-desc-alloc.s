/*
 *  Stage 5 Table 3
 */
#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct tx_table_s5_t3_k k;
struct phv_ p;
struct tx_table_s5_t3_d d;

%%
    .align
tls_read_l7_descr_alloc:
    CAPRI_SET_DEBUG_STAGE4_7(p.stats_debug_stage4_7_thread, CAPRI_MPU_STAGE_5, CAPRI_MPU_TABLE_3)
    CAPRI_CLEAR_TABLE3_VALID
    
    CAPRI_OPERAND_DEBUG(d.u.tls_l7_desc_alloc_d.desc)
    phvwr       p.to_s7_l7_desc, d.u.tls_l7_desc_alloc_d.desc
    nop.e
    nop
