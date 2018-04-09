/*
 * 	Implements the reading of input descriptor to retirve the page addresses
 *  Stage 4, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_                 p;
struct tx_table_s4_t3_d     d;
struct tx_table_s4_t3_k     k;


%%
#if 0
    .param      tls_enc_update_desc_q_process
#endif

tls_enc_post_read_idesc:
    CAPRI_SET_DEBUG_STAGE4_7(p.to_s7_debug_stage4_7_thread, CAPRI_MPU_STAGE_4, CAPRI_MPU_TABLE_3)

    CAPRI_CLEAR_TABLE3_VALID
    phvwrpair   p.to_s5_ipage,  d.u.tls_read_idesc_d.A0[31:0], \
                p.to_s5_next_idesc, d.u.tls_read_idesc_d.next_pkt[31:0]
    CAPRI_OPERAND_DEBUG(d.u.tls_read_idesc_d.next_pkt)
#if 0
    CAPRI_NEXT_TABLE_READ_OFFSET(3, TABLE_LOCK_DIS, tls_enc_update_desc_q_process,
                                 k.tls_global_phv_qstate_addr,
                                 TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)
#endif

tls_enc_post_read_idesc_done:
    nop.e
    nop.e

