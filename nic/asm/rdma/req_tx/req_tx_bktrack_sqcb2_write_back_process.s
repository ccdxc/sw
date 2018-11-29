#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s3_t1_k k;
struct sqcb2_t d;

#define IN_P t1_s2s_bktrack_sqcb2_write_back_info

#define K_SSN             CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit7, ssn_sbit16_ebit23)
#define K_TX_PSN          CAPRI_KEY_RANGE(IN_P, tx_psn_sbit0_ebit7, tx_psn_sbit16_ebit23)
#define K_MSG_PSN         CAPRI_KEY_RANGE(IN_P, msg_psn_sbit0_ebit0, msg_psn_sbit17_ebit23)
#define K_IMM_DATA        CAPRI_KEY_RANGE(IN_P, imm_data_or_inv_key_sbit0_ebit6, imm_data_or_inv_key_sbit31_ebit31)
#define K_INV_KEY         K_IMM_DATA
#define K_OP_TYPE         CAPRI_KEY_FIELD(IN_P, op_type)
#define K_SQ_C_INDEX      CAPRI_KEY_RANGE(IN_P, sq_cindex_sbit0_ebit2, sq_cindex_sbit11_ebit15)
#define K_BKTRACK_IN_PROGRESS CAPRI_KEY_FIELD(IN_P, bktrack_in_progress)
%%

.align
req_tx_bktrack_sqcb2_write_back_process:
     tblwr         d.tx_psn, K_TX_PSN
     tblwr         d.ssn, K_SSN
     tblwr         d.imm_data, K_IMM_DATA
     tblwr         d.inv_key, K_INV_KEY
     tblwr         d.curr_op_type, K_OP_TYPE

     // Revert LSN based on last received ACK's MSN and credits
     //DECODE_ACK_SYNDROME_CREDITS(r1, d.credits, c1)
     //mincr         r1, 24, d.msn
     // TODO Need to fix test cases to have credits as 0x1F (currently SQ
     // is configured for disabled credits), so that RxDMA will not not copy
     // lsn to TXDMA's SQCB
     //tblwr         d.lsn_tx, d.lsn_rx
     //tblwr         d.lsn, d.lsn_rx

     seq           c1, CAPRI_KEY_FIELD(IN_P, skip_wqe_start_psn), 1
     tblwr.!c1     d.wqe_start_psn, K_TX_PSN

     // set rrq_pindex to start from 0
     tblwr         d.rrq_pindex, 0
     tblwr         d.sq_cindex, K_SQ_C_INDEX

     // Update tx_psn, ssn and lsn in sqcb1 for RXDMA
     SQCB1_ADDR_GET(r1)
     add            r2, FIELD_OFFSET(sqcb1_t, tx_psn), r1
     add            r3, r0, d.{tx_psn, ssn}, 16
     memwr.d        r2, r3
     // Also, empty rrq ring, which will then get posted
     // with retransmit requests
     add            r2, FIELD_OFFSET(sqcb1_t, rrq_pindex), r1
     memwr.h        r2, 0

     // Clear rrq_spec_cindex
     add            r2, FIELD_OFFSET(sqcb1_t, rrq_spec_cindex), r1
     memwr.h        r2, 0

     add            r2, FIELD_OFFSET(sqcb1_t, msg_psn), r1
     memwr.b        r2, K_MSG_PSN[23:16]
     add            r2, r2, 1
     memwr.b        r2, K_MSG_PSN[15:8]
     add            r2, r2, 1
     memwr.b        r2, K_MSG_PSN[7:0]

     tblwr          d.{rrq_pindex, rrq_cindex}, 0
 
     tblwr          d.need_credits, 0

     // Update exp_rsp_psn to be one less than rexmit_psn
     sub            r3, d.rexmit_psn, 1
     tblwr          d.exp_rsp_psn, r3

     // Finally, clear bktrack_in_progress
     bbeq           K_BKTRACK_IN_PROGRESS, 1, exit
     add            r2, FIELD_OFFSET(sqcb1_t, bktrack_in_progress), r1
     memwr.b        r2, 0

exit:
     tblwr.e         d.busy, 0
     CAPRI_SET_TABLE_1_VALID(0)
