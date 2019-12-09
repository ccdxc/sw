#include "capri.h"
#include "aq_rx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "sqcb.h"    
#include "common_phv.h"
#include "types.h"

struct aq_rx_phv_t p;
struct rqcb1_t d;
struct aq_rx_s5_t2_k k;

#define IN_P t2_s2s_sqcb1_to_rqcb1_info
#define IN_TO_S_P to_s5_info

#define R_RQCB_ADDR     r2  // RQCB1 Address

#define K_SQCB_ADDR   CAPRI_KEY_RANGE(IN_P, sqcb_addr_sbit0_ebit6, sqcb_addr_sbit31_ebit31)
#define K_CONGESTION_MGMT_ENABLED CAPRI_KEY_FIELD(IN_P, congestion_mgmt_enable)
#define K_ACCESS_FLAGS_VALID CAPRI_KEY_FIELD(IN_P, access_flags_valid)
#define K_ACCESS_FLAGS CAPRI_KEY_FIELD(IN_P, access_flags)
#define K_DCQCN_CFG_ID CAPRI_KEY_RANGE(IN_P, dcqcn_cfg_id_sbit0_ebit2, dcqcn_cfg_id_sbit3_ebit3)

%%

.align
rdma_aq_rx_rqcb1_process:

//TODO: Remove  redundant labels later

    bbne        CAPRI_KEY_FIELD(IN_P, cur_state_valid), 1, hdr_update
    mfspr       R_RQCB_ADDR, spr_tbladdr    // BD Slot
    
    add         r3, CAPRI_KEY_FIELD(IN_P, cur_state), r0

    seq         c1, d.serv_type, RDMA_SERV_TYPE_UD
    add         r4, d.state, r0
    bne.!c1     r4, r3, done
    bne.c1      r4, r3, restore_ud
    nop     // BD Slot

hdr_update:
    bbne        CAPRI_KEY_FIELD(IN_P, av_valid), 1, rsq_base
    seq         c2, K_CONGESTION_MGMT_ENABLED, 1    // BD Slot

    tblwr       d.congestion_mgmt_type, K_CONGESTION_MGMT_ENABLED
    tblwr.c2    d.dcqcn_cfg_id, K_DCQCN_CFG_ID
    tblwr       d.header_template_addr, CAPRI_KEY_FIELD(IN_P, ah_addr)
    tblwr       d.header_template_size, CAPRI_KEY_FIELD(IN_P, ah_len)

rsq_base:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P, rsq_valid), 1, rq_psn
    nop

    tblwr       d.rsq_base_addr, CAPRI_KEY_FIELD(IN_TO_S_P, q_key_rsq_base_addr)
    tblwr       d.log_rsq_size, CAPRI_KEY_FIELD(IN_TO_S_P, rsq_depth_log2)

rq_psn:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P, rq_psn_valid), 1, state
    nop

    tblwr       d.e_psn, CAPRI_KEY_RANGE(IN_TO_S_P, rq_psn_sbit0_ebit0, rq_psn_sbit17_ebit23)

state:
    bbne        CAPRI_KEY_FIELD(IN_P, state_valid), 1, pmtu
    nop

    // Set up DMA instructions to update tx CB states
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_MODIFY_QP_SQCB0_STATE)
    add         r3, FIELD_OFFSET(sqcb0_t, service), K_SQCB_ADDR
    DMA_HBM_PHV2MEM_SETUP(r6, mod_qp.service, mod_qp.state, r3)
 
    phvwrpair   p.mod_qp.rq_state, CAPRI_KEY_FIELD(IN_P, state), p.mod_qp.log_rsq_size, d.log_rsq_size
    sub         r3, R_RQCB_ADDR, CB_UNIT_SIZE_BYTES
    add         r3, r3, FIELD_OFFSET(rqcb0_t, state)
    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_MODIFY_QP_RQCB0_STATE)
    DMA_HBM_PHV2MEM_SETUP(r6, mod_qp.rq_state, mod_qp.log_rsq_size, r3)

    seq         c2, CAPRI_KEY_FIELD(IN_P, state), QP_STATE_INIT
    bcf         [!c2], pmtu
    tblwr       d.state, CAPRI_KEY_FIELD(IN_P, state)  //BD slot

    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_CLEAR_STATS_CB)
    add         r3, R_RQCB_ADDR, CB3_OFFSET_BYTES

    DMA_HBM_PHV2MEM_SETUP(r6, rqcb4, rqcb5, r3)
    
pmtu:
    bbne        CAPRI_KEY_FIELD(IN_P , pmtu_valid), 1, access_flags
    nop

    tblwr       d.log_pmtu, CAPRI_KEY_RANGE(IN_P, pmtu_log2_sbit0_ebit2, pmtu_log2_sbit3_ebit4)

access_flags:
    seq         c1, K_ACCESS_FLAGS_VALID, 1
    tblwr.c1    d.access_flags, K_ACCESS_FLAGS

q_key:
    bbne        CAPRI_KEY_FIELD(IN_TO_S_P , q_key_valid), 1, done
    nop

    tblwr       d.q_key, CAPRI_KEY_FIELD(IN_TO_S_P, q_key_rsq_base_addr)

done:
    nop.e
    nop

restore_ud:
	bbne        CAPRI_KEY_FIELD(IN_TO_S_P , q_key_valid), 1, done
    phvwr       p.mod_qp.q_key, d.q_key		// BD Slot

    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_MODIFY_QP_UD_RESTORE1)
    add         r3, K_SQCB_ADDR, FIELD_OFFSET(sqcb0_t, q_key)
    DMA_PHV2MEM_SETUP(r6, c1, mod_qp.q_key, mod_qp.q_key, r3)

    DMA_CMD_STATIC_BASE_GET(r6, AQ_RX_DMA_CMD_START_FLIT_ID, AQ_RX_DMA_CMD_MODIFY_QP_UD_RESTORE2)
    sub         r3, R_RQCB_ADDR, CB_UNIT_SIZE_BYTES    
    add         r3, r3, FIELD_OFFSET(rqcb0_t, q_key)
    DMA_PHV2MEM_SETUP_E(r6, c1, mod_qp.q_key, mod_qp.q_key, r3)
