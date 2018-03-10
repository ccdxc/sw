/*
 * 	Implements the reading of input descriptor to retirve the page addresses
 *  Stage 3, Table 3
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_                 p;
struct tx_table_s3_t3_d     d;
struct tx_table_s3_t3_k     k;


%%
    .param      tls_enc_update_desc_q_process

tls_enc_post_read_idesc:

    CAPRI_CLEAR_TABLE3_VALID
    phvwrpair   p.to_s4_ipage,  d.u.tls_read_idesc_d.A0[31:0], \
                p.to_s4_next_idesc, d.u.tls_read_idesc_d.next_pkt[31:0]
    CAPRI_OPERAND_DEBUG(d.u.tls_read_idesc_d.next_pkt)

    CAPRI_NEXT_TABLE_READ_OFFSET(3, TABLE_LOCK_DIS, tls_enc_update_desc_q_process,
                                 k.tls_global_phv_qstate_addr,
                                 TLS_TCB_CRYPT_OFFSET, TABLE_SIZE_512_BITS)

tls_enc_post_read_idesc_done:
    nop.e
    nop.e

