#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s3_t1_k k;
struct sqcb2_t d;

#define IN_P t1_s2s_bktrack_sqcb2_write_back_info

#define K_SSN             CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit7, ssn_sbit16_ebit23)
#define K_WQE_START_PSN   CAPRI_KEY_FIELD(IN_P, wqe_start_psn)
#define K_TBL_ID          CAPRI_KEY_FIELD(IN_P, tbl_id)
#define K_IMM_DATA        CAPRI_KEY_RANGE(IN_P, imm_data_sbit0_ebit6, imm_data_sbit31_ebit31)
#define K_INV_KEY         CAPRI_KEY_RANGE(IN_P, inv_key_sbit0_ebit6, inv_key_sbit31_ebit31)
#define K_OP_TYPE         CAPRI_KEY_FIELD(IN_P, op_type)
#define K_SQ_C_INDEX      CAPRI_KEY_RANGE(IN_P, sq_cindex_sbit0_ebit2, sq_cindex_sbit11_ebit15)
%%

.align
req_tx_bktrack_sqcb2_write_back_process:
     tblwr         d.tx_psn, CAPRI_KEY_FIELD(IN_P, tx_psn)
     tblwr         d.ssn, K_SSN
     tblwr         d.imm_data, K_IMM_DATA
     tblwr         d.inv_key, K_INV_KEY
     tblwr         d.curr_op_type, K_OP_TYPE

     // Revert LSN based on last received ACK's MSN and credits
     DECODE_ACK_SYNDROME_CREDITS(r1, d.credits, c1)
     mincr         r1, 24, d.msn
     tblwr         d.lsn, r1

     seq           c1, CAPRI_KEY_FIELD(IN_P, skip_wqe_start_psn), 1
     tblwr.!c1     d.wqe_start_psn, CAPRI_KEY_FIELD(IN_P, tx_psn)

     // set rrq_pindex to start from 0
     tblwr         d.rrq_pindex, 0
     tblwr         d.sq_cindex, K_SQ_C_INDEX

     // Update tx_psn, ssn and lsn in sqcb1 for RXDMA
     SQCB1_ADDR_GET(r1)
     add            r2, FIELD_OFFSET(sqcb1_t, tx_psn), r1
     memwr.d        r2, d.{tx_psn...lsn[23:8]}
     add            r2, r2, 8
     memwr.b        r2, d.lsn[7:0]
     // Also, empty rrq ring, which will then get posted
     // with retransmit requests
     add            r2, FIELD_OFFSET(sqcb1_t, ring5), r1
     memwr.w        r2, 0        
 
     SQCB0_ADDR_GET(r1)
     add            r2, FIELD_OFFSET(sqcb0_t, cb1_byte), r1
     tblwr          d.need_credits, 0
     tblwr          d.in_progress, CAPRI_KEY_FIELD(IN_P, in_progress)
     or             r1, r0, CAPRI_KEY_FIELD(IN_P, in_progress), SQCB0_IN_PROGRESS_BIT_OFFSET
     memwr.b        r2, r1

     CAPRI_SET_TABLE_1_VALID(0)

     nop.e
     nop
