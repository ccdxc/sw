#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aq_dscp_cos_map d;
struct aq_tx_s3_t3_k k;

#define IN_TO_S_P to_s3_info

#define TO_RQCB0_INFO_P to_s5_info    
#define TO_SQCB0_INFO_P to_s6_info    

#define K_DSCP CAPRI_KEY_RANGE(IN_TO_S_P, local_ack_timeout_or_dscp_sbit0_ebit3, local_ack_timeout_or_dscp_sbit4_ebit4)
#define K_PCP CAPRI_KEY_FIELD(IN_TO_S_P, err_retry_count_or_pcp)
#define K_DCQCN_CONFIG_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, cb_addr_sbit0_ebit31, cb_addr_sbit32_ebit33)
#define K_CONGESTION_MGMT_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_enable)

#define NUM_DSCP_VALS 64

%%

    .param      rdma_aq_tx_sqcb2_process
    .param      rdma_aq_tx_dcqcn_config_process
.align
rdma_aq_tx_fetch_tx_iq_process:

    // First-bit of qos-default-class (is_dscp) indicates dscp or pcp
    seq         c1, d[504], 1

    sub.c1      r1, (NUM_DSCP_VALS - 1), K_DSCP
    sub.!c1     r1, (NUM_DSCP_VALS - 1), K_PCP

    sll         r1, r1, LOG_BITS_PER_BYTE

    tblrdp      r2, r1, 0, 8

    //Overload q_key to pass tx-iq info rqcb0/sqcb0 stages.
    phvwr       CAPRI_PHV_FIELD(TO_RQCB0_INFO_P, q_key_or_tm_iq), r2[5:2]

    add         r3, K_CONGESTION_MGMT_ENABLE, r0
    beqi        r3, 1, setup_dcqcn
    phvwr       CAPRI_PHV_FIELD(TO_SQCB0_INFO_P, q_key_or_tm_iq), r2[5:2]   // BD Slot

    CAPRI_SET_TABLE_3_VALID_CE(c0, 0)
    nop

setup_dcqcn:
    CAPRI_RESET_TABLE_3_ARG()
    add         r3, r0, K_DCQCN_CONFIG_ADDR
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_dcqcn_config_process, r3)

