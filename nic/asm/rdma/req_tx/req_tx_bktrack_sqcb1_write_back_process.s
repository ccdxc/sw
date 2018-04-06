#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s2_t1_k k;
struct sqcb1_t d;

#define IN_P        t1_s2s_bktrack_sqcb1_write_back_info

#define K_TX_PSN    CAPRI_KEY_FIELD(IN_P, tx_psn)
#define K_SSN       CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit7, ssn_sbit16_ebit23)

%%
.align
req_tx_bktrack_sqcb1_write_back_process:
    
    scwlt24        c1, d.max_tx_psn, K_TX_PSN
    tblwr.c1       d.max_tx_psn, d.tx_psn
     
    scwlt24        c1, d.max_ssn, K_SSN
    tblwr.c1       d.max_ssn, d.ssn

    CAPRI_SET_TABLE_1_VALID(0)

    nop.e
    nop
