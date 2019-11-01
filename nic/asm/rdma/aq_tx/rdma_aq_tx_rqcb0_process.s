#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct rqcb0_t d;
struct aq_tx_s5_t1_k k;

#define IN_P t1_s2s_wqe2_to_rqcb0_info
#define IN_TO_S_P to_s5_info

#define K_Q_KEY_TM_IQ CAPRI_KEY_RANGE(IN_TO_S_P, q_key_or_tm_iq_sbit0_ebit1, q_key_or_tm_iq_sbit26_ebit31)
#define K_DST_QP  CAPRI_KEY_RANGE(IN_TO_S_P, dst_qp_sbit0_ebit5, dst_qp_sbit22_ebit23)
#define K_DCQCN_PROFILE CAPRI_KEY_FIELD(IN_P, dcqcn_profile)
#define K_CONGESTION_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_enable)

%%

    .param      rdma_aq_tx_rqcb2_process
    
.align
rdma_aq_tx_rqcb0_process:

    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], bubble_to_next_stage
    nop

hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, dst_qp
    seq         c1, K_CONGESTION_ENABLE, 1  // BD Slot
    
    tblwr     d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    tblwr     d.header_template_size, CAPRI_KEY_FIELD(IN_P, ah_len)
    // Update cos(tm_iq) in qstate.
    tblwr       d.intrinsic.cosB, K_Q_KEY_TM_IQ
    tblwr.c1    d.dcqcn_cfg_id, K_DCQCN_PROFILE
    tblwr.c1    d.congestion_mgmt_type, 1

dst_qp:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P, dst_qp_valid), 1, rsq_base
    nop

    tblwr       d.dst_qp, K_DST_QP

rsq_base:
    bbne        CAPRI_KEY_FIELD(IN_P, rsq_valid), 1, pmtu
    nop
    
    tblwr       d.rsq_base_addr, CAPRI_KEY_RANGE(IN_P, rsq_base_addr_sbit0_ebit0, rsq_base_addr_sbit25_ebit31)
    tblwr       d.log_rsq_size, CAPRI_KEY_RANGE(IN_P, rsq_depth_log2_sbit0_ebit0, rsq_depth_log2_sbit1_ebit4)

pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P , pmtu_valid), 1, q_key
    nop
    
    tblwr       d.log_pmtu, CAPRI_KEY_FIELD(IN_P, pmtu_log2)

q_key:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P , q_key_valid), 1, setup_rqcb2
    nop

    tblwr       d.q_key, K_Q_KEY_TM_IQ

setup_rqcb2:
    mfspr       r2, spr_tbladdr
    add         r2, r2, (2 * CB_UNIT_SIZE_BYTES)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_rqcb2_process, r2)
    nop.e
    nop
    
bubble_to_next_stage:
exit: 
    nop.e
    nop
    
    
    
