#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aqwqe_t d;
struct aq_tx_s2_t1_k k;

#define IN_TO_S_P to_s2_info
    
#define K_COMMON_GLOBAL_QID CAPRI_KEY_RANGE(phv_global_common, qid_sbit0_ebit4, qid_sbit21_ebit23)
#define K_COMMON_GLOBAL_QTYPE CAPRI_KEY_FIELD(phv_global_common, qtype)
#define K_AH_BASE_ADDR_PAGE_ID CAPRI_KEY_RANGE(IN_TO_S_P, ah_base_addr_page_id_sbit0_ebit15, ah_base_addr_page_id_sbit16_ebit21)
#define K_RRQ_BASE_ADDR_PAGE_ID CAPRI_KEY_RANGE(IN_TO_S_P, rrq_base_addr_page_id_sbit0_ebit1, rrq_base_addr_page_id_sbit18_ebit21)
#define K_RSQ_BASE_ADDR_PAGE_ID CAPRI_KEY_RANGE(IN_TO_S_P, rsq_base_addr_page_id_sbit0_ebit3, rsq_base_addr_page_id_sbit20_ebit21)
#define K_SQCB_BASE_ADDR_HI CAPRI_KEY_RANGE(IN_TO_S_P, sqcb_base_addr_hi_sbit0_ebit5, sqcb_base_addr_hi_sbit22_ebit23) 
#define K_RQCB_BASE_ADDR_HI CAPRI_KEY_RANGE(IN_TO_S_P, rqcb_base_addr_hi_sbit0_ebit5, rqcb_base_addr_hi_sbit22_ebit23)

%%

    .param      dummy
    
.align
rdma_aq_tx_modify_qp_2_process:

    add         r3, r0, d.{id_ver}.wx  //TODO: Need to optimize 
    SQCB_ADDR_GET(r1, r3[23:0], K_SQCB_BASE_ADDR_HI)
    RQCB_ADDR_GET(r2, r3[23:0], K_RQCB_BASE_ADDR_HI)

    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_AV], 1, rrq_base

hdr_update:
    add         r4, d.mod_qp.ah_id_len, r0 //BD Slot
    muli        r5, r4, AT_ENTRY_SIZE_BYTES
    nop
    add         r5, r5, K_AH_BASE_ADDR_PAGE_ID, HBM_PAGE_SIZE_SHIFT
    srl         r7, r5, HDR_TEMP_ADDR_SHIFT
 
    //Invoke rqcb0
    add         r4, r2, r0
    add         r5, r4, FIELD_OFFSET(rqcb0_t, header_template_addr)
    memwr.d     r5, r7
    add         r5, r4, FIELD_OFFSET(rqcb0_t, header_template_size)
    memwr.b     r5, d.mod_qp.ah_id_len

    //Invoke rqcb1
    add         r4, r2, (CB_UNIT_SIZE_BYTES)
    add         r5, r4, FIELD_OFFSET(rqcb1_t, header_template_addr)
    memwr.d     r5, r7
    add         r5, r4, FIELD_OFFSET(rqcb1_t, header_template_size)
    memwr.b     r5, d.mod_qp.ah_id_len

    //Invoke sqcb1
    add         r4, r1, (CB_UNIT_SIZE_BYTES)
    add         r5, r4, FIELD_OFFSET(sqcb1_t, header_template_addr)
    memwr.d     r5, r7
    add         r5, r4, FIELD_OFFSET(sqcb1_t, header_template_size)
    memwr.b     r5, d.mod_qp.ah_id_len

    //Invoke sqcb2
    add         r4, r1, (CB_UNIT_SIZE_BYTES * 2)
    add         r5, r4, FIELD_OFFSET(sqcb2_t, header_template_addr)
    memwr.d     r5, r7
    add         r5, r4, FIELD_OFFSET(sqcb2_t, header_template_size)
    memwr.b     r5, d.mod_qp.ah_id_len

    //Invoke sqcb0
    add         r4, r1, r0
    add         r5, r4, FIELD_OFFSET(sqcb0_t, header_template_addr)
    memwr.d     r5, r7

rrq_base:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_MAX_QP_RD_ATOMIC], 1, rsq_base
    add         r7, r0, K_RRQ_BASE_ADDR_PAGE_ID, HBM_PAGE_SIZE_SHIFT //BD Slot
    add         r4, r0, d.{id_ver}.wx  //TODO: Need to optimize
    add         r7, r7, r4[23:0], LOG_RRQ_QP_SIZE

    //Invode sqcb1
    add         r4, r1, (CB_UNIT_SIZE_BYTES)
    add         r5, r4, FIELD_OFFSET(sqcb1_t, rrq_base_addr)
    memwr.d     r5, r7
    add         r5, r4, FIELD_OFFSET(sqcb1_t, log_rrq_size)
    memwr.b     r5, d.mod_qp.rrq_depth

    //Invode sqcb2
    add         r4, r1, (CB_UNIT_SIZE_BYTES * 2)
    add         r5, r4, FIELD_OFFSET(sqcb2_t, rrq_base_addr)
    memwr.d     r5, r7
    add         r5, r4, FIELD_OFFSET(sqcb2_t, log_rrq_size)
    memwr.b     r5, d.mod_qp.rrq_depth

rsq_base:
    bbne        d.mod_qp.attr_mask[RDMA_UPDATE_QP_OPER_SET_MAX_DEST_RD_ATOMIC], 1, done
    add         r7, r0, K_RSQ_BASE_ADDR_PAGE_ID, HBM_PAGE_SIZE_SHIFT // BD slot
    add         r4, r0, d.{id_ver}.wx  //TODO: Need to optimize
    add         r7, r7, r4[23:0], LOG_RSQ_QP_SIZE

    //Invoke rqcb0
    add         r4, r2, r0
    add         r5, r4, FIELD_OFFSET(rqcb0_t, rsq_base_addr)
    memwr.d     r5, r7
    add         r5, r4, FIELD_OFFSET(rqcb0_t, log_rsq_size)
    memwr.b     r5, d.mod_qp.rsq_depth

    //Invoke rqcb1
    add         r4, r2, (CB_UNIT_SIZE_BYTES)
    add         r5, r4, FIELD_OFFSET(rqcb1_t, rsq_base_addr)
    memwr.d     r5, r7
    add         r5, r4, FIELD_OFFSET(rqcb1_t, log_rsq_size)
    memwr.b     r5, d.mod_qp.rsq_depth

done:
    nop.e
    nop
