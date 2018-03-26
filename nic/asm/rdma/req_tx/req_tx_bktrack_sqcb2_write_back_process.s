#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s3_t1_k k;
struct sqcb2_t d;

#define IN_P t1_s2s_bktrack_sqcb2_write_back_info

#define K_TX_PSN          CAPRI_KEY_RANGE(IN_P, tx_psn_sbit0_ebit7, tx_psn_sbit16_ebit23)
#define K_SSN             CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit7, ssn_sbit16_ebit23)
#define K_WQE_START_PSN   CAPRI_KEY_FIELD(IN_P, wqe_start_psn)
#define K_TBL_ID          CAPRI_KEY_FIELD(IN_P, tbl_id)
#define K_IMM_DATA        CAPRI_KEY_RANGE(IN_P, imm_data_sbit0_ebit3, imm_data_sbit28_ebit31)
#define K_INV_KEY         CAPRI_KEY_RANGE(IN_P, inv_key_sbit0_ebit3, inv_key_sbit28_ebit31)
%%

.align
req_tx_bktrack_sqcb2_write_back_process:
     tblwr         d.tx_psn, K_TX_PSN
     tblwr         d.ssn, K_SSN
     tblwr         d.imm_data, K_IMM_DATA
     tblwr         d.inv_key, K_INV_KEY

     // Revert LSN based on last received ACK's MSN and credits
     DECODE_ACK_SYNDROME_CREDITS(r1, d.credits, c1)
     mincr         r1, 24, d.msn
     tblwr         d.lsn, r1

     seq           c1, CAPRI_KEY_FIELD(IN_P, skip_wqe_start_psn), 1
     tblwr.!c1     d.wqe_start_psn, K_WQE_START_PSN
    
     add           r1, K_TBL_ID, r0
     CAPRI_SET_TABLE_I_VALID(r1, 0)

     nop.e
     nop
