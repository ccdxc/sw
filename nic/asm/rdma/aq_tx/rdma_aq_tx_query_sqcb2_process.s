#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct sqcb2_t d;
struct aq_tx_s5_t2_k k;

#define IN_TO_S_P to_s5_info

#define K_RQCB2_ADDRESS CAPRI_KEY_RANGE(IN_TO_S_P, cb_addr_sbit0_ebit31, cb_addr_sbit32_ebit33)

%%

    .param      rdma_aq_tx_query_sqcb0_process
    .param      rdma_aq_tx_query_rqcb2_process

.align
rdma_aq_tx_query_sqcb2_process:

    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_5
    bcf         [!c1], bubble_to_next_stage

    mfspr       r2, spr_tbladdr     // BD Slot
    sub         r2, r2, (2 * CB_UNIT_SIZE_BYTES)
    CAPRI_RESET_TABLE_2_ARG()
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_query_sqcb0_process, r2)

    add         r2, d.service, r0
    beqi        r2, RDMA_SERV_TYPE_UD, exit
    add         r2, K_RQCB2_ADDRESS, r0     // BD Slot
    phvwr       p.query_sq.qkey_dest_qpn, d.dst_qp
    CAPRI_RESET_TABLE_1_ARG()
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_query_rqcb2_process, r2)

exit:
    nop.e
    nop

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_4
    bcf         [!c1], exit

    CAPRI_GET_TABLE_2_K(aq_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS)
