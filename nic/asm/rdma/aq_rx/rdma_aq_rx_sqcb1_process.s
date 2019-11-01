#include "capri.h"
#include "aq_rx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "types.h"

struct aq_rx_phv_t p;
struct sqcb1_t d;
struct aq_rx_s4_t2_k k;

#define IN_P t2_s2s_aqcb_to_sqcb1_info
#define IN_TO_S_P to_s4_info
    
#define SQCB1_RQCB1_P t2_s2s_sqcb1_to_rqcb1_info

#define R_SQCB_ADDR r2
#define R_RQCB_ADDR r3

#define K_SQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, sqcb_base_addr_hi)    
#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_FIELD(IN_TO_S_P, rqcb_base_addr_hi)
#define K_TX_PSN CAPRI_KEY_RANGE(IN_TO_S_P, tx_psn_sbit0_ebit4, tx_psn_sbit21_ebit23)
#define K_TX_PSN_VALID CAPRI_KEY_FIELD(IN_TO_S_P, tx_psn_valid)
#define K_ERR_RETRY_COUNT CAPRI_KEY_RANGE(IN_TO_S_P, err_retry_count_sbit0_ebit1, err_retry_count_sbit2_ebit2)
#define K_ERR_RETRY_COUNT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, err_retry_count_valid)
#define K_RQ_ID CAPRI_KEY_RANGE(IN_P, rq_id_sbit0_ebit4, rq_id_sbit21_ebit23)    
#define K_RNR_RETRY_COUNT CAPRI_KEY_FIELD(IN_TO_S_P, rnr_retry_count)
#define K_RNR_RETRY_COUNT_VALID CAPRI_KEY_FIELD(IN_TO_S_P, rnr_retry_count_valid)
#define K_SQD_ASYNC_NOTIFY_EN CAPRI_KEY_FIELD(IN_P, sqd_async_notify_en)
#define K_ACCESS_FLAGS_VALID CAPRI_KEY_FIELD(IN_P, access_flags_valid)
#define K_ACCESS_FLAGS CAPRI_KEY_FIELD(IN_P, access_flags)
#define K_CONGESTION_MGMT_ENABLED CAPRI_KEY_FIELD(IN_P, congestion_mgmt_enable)
%%

    .param      rdma_aq_rx_rqcb1_process
    
.align
rdma_aq_rx_sqcb1_process:

    // Pin cqcb process to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage
    SQCB_ADDR_GET(R_SQCB_ADDR, K_RQ_ID, K_SQCB_BASE_ADDR_HI) // BD Slot
    RQCB_ADDR_GET(R_RQCB_ADDR, K_RQ_ID, K_RQCB_BASE_ADDR_HI)

cur_state:
    bbne        CAPRI_KEY_FIELD(IN_P, cur_state_valid), 1, hdr_update
    add         r4, CAPRI_KEY_FIELD(IN_P, cur_state), r0    // BD Slot

    seq         c1, d.service, RDMA_SERV_TYPE_UD
    add         r5, d.state, r0
    bne.!c1     r5, r4, cur_state_invalid
    bne.c1      r5, r4, setup_rqcb_stages
    nop         // BD Slot
    
hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, rrq_base
    nop

    tblwr       d.congestion_mgmt_type, K_CONGESTION_MGMT_ENABLED
    tblwr       d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    tblwr       d.header_template_size, CAPRI_KEY_FIELD(IN_P, ah_len)
    
rrq_base:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P, rrq_valid), 1, state
    nop
    
    tblwr       d.rrq_base_addr, CAPRI_KEY_FIELD(IN_TO_S_P, rrq_base_addr)
    tblwr       d.log_rrq_size, CAPRI_KEY_FIELD(IN_TO_S_P, rrq_depth_log2)
    
state:
    bbne        CAPRI_KEY_FIELD(IN_P, state_valid), 1, tx_psn
    add         r6, CAPRI_KEY_FIELD(IN_P, state), r0    // BD Slot

    phvwrpair   p.mod_qp.service, d.service, p.{mod_qp.flush_rq...mod_qp.state}, r6
    tblwr       d.state, CAPRI_KEY_FIELD(IN_P, state)

tx_psn:
    bbne        K_TX_PSN_VALID, 1, retry_cnt
    nop

    tblwr       d.tx_psn, K_TX_PSN
    tblwr       d.max_tx_psn, K_TX_PSN
    tblwr       d.rexmit_psn, K_TX_PSN

retry_cnt:
    bbne        K_ERR_RETRY_COUNT_VALID, 1, rnr_retry_count
    nop

    tblwr       d.err_retry_count, K_ERR_RETRY_COUNT

rnr_retry_count:
    bbne        K_RNR_RETRY_COUNT_VALID, 1, pmtu
    nop

    tblwr     d.rnr_retry_count, K_RNR_RETRY_COUNT

pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P , pmtu_valid), 1, access_flags
    nop
                                        
    tblwr       d.log_pmtu, CAPRI_KEY_RANGE(IN_P, pmtu_log2_sbit0_ebit2, pmtu_log2_sbit3_ebit4)

access_flags:
    seq         c1, K_ACCESS_FLAGS_VALID, 1
    //TODO: Enable tblwr after carving space in sqcb1 to hold them.
    //tblwr.c1    d.access_flags, K_ACCESS_FLAGS

sqd_async_notify:
    seq         c1, K_SQD_ASYNC_NOTIFY_EN, 1
    tblwr.c1    d.sqd_async_notify_enable, 1

setup_rqcb_stages: 

    phvwr       CAPRI_PHV_FIELD(SQCB1_RQCB1_P, sqcb_addr), R_SQCB_ADDR 
    add         r4, R_RQCB_ADDR, CB_UNIT_SIZE_BYTES
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_rx_rqcb1_process, r4)

cur_state_invalid:
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop

bubble_to_next_stage:
    seq         c1, r1[4:2], STAGE_3
    bcf         [!c1], exit

    SQCB_ADDR_GET(R_SQCB_ADDR, K_RQ_ID, K_SQCB_BASE_ADDR_HI)
    add         r4, R_SQCB_ADDR, CB_UNIT_SIZE_BYTES
    CAPRI_GET_TABLE_2_K(aq_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r4) //Exit Slot

exit:
    nop.e
    nop
    
