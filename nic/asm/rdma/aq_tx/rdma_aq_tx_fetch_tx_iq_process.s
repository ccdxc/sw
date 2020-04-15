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

#define K_UPLINK_PORT CAPRI_KEY_RANGE(IN_TO_S_P, local_ack_timeout_or_uplink_port_sbit0_ebit3, local_ack_timeout_or_uplink_port_sbit4_ebit4)
#define K_PCP CAPRI_KEY_FIELD(IN_TO_S_P, err_retry_count_or_pcp)
#define K_DCQCN_CONFIG_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, cb_addr_sbit0_ebit31, cb_addr_sbit32_ebit33)
#define K_CONGESTION_MGMT_ENABLE CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_enable)
#define K_DSCP CAPRI_KEY_RANGE(IN_TO_S_P, tx_psn_or_dscp_sbit0_ebit3, tx_psn_or_dscp_sbit20_ebit23)

#define NUM_DSCP_VALS 64
#define NUM_PCP_VALS 8
#define DROP     0
#define NO_DROP1 1
#define NO_DROP2 2
#define NO_DROP3 3

%%

    .param      rdma_aq_tx_dcqcn_config_process

.align
rdma_aq_tx_fetch_tx_iq_process:

    bbeq        d.is_dscp, 1, dscp_map
    nop // BD-slot

pcp_map:
    //fetch txdma-iq corresponding to PCP value in header-template.
    add         r1, K_PCP, r0

    // Fetch 2-bit no-drop value corresponding to PCP.
    // 0 -> drop TC. 1/2/3 -> no-drop TC

    sub         r4, (NUM_PCP_VALS - 1), r1
    sll         r4, r4, 1
    srl         r2, d.no_drop[127:112], r4
    and         r2, r2, 0x3

    // If drop TC, then fetch txdma_iq1
    beqi        r2, 0, txdma_iq1

    // If no-drop TC and uplink_port is 1, then fetch txdma_iq1
    seq         c2, K_UPLINK_PORT, 1 //BD-slot
    bcf         [c2], txdma_iq1
    nop //BD-slot

    // If no-drop TC and uplink_port is 5, then fetch txdma_iq2
txdma_iq2:
    .brbegin
    br             r2[1:0]
    nop

    .brcase DROP
        b               txdma_iq1
        nop

    .brcase NO_DROP1
        b               iq_fetch_done
        add             r2, r0, d.no_drop1_txdma_iq // Branch Delay slot

    .brcase NO_DROP2
        b               iq_fetch_done
        add             r2, r0, d.no_drop2_txdma_iq // Branch Delay slot
    
    .brcase NO_DROP3
        b               iq_fetch_done
        add             r2, r0, d.no_drop3_txdma_iq // Branch Delay slot

    .brend

txdma_iq1:
    sub         r3, (NUM_PCP_VALS - 1), r1
    sll         r3, r3, 2
    srl         r2, d.txdma_iq[255:224], r3
    and         r2, r2, 0xf

iq_fetch_done:
    //Overload q_key to pass tx-iq info rqcb0/sqcb0 stages.
    phvwr       CAPRI_PHV_FIELD(TO_RQCB0_INFO_P, q_key_or_tm_iq), r2

    add         r3, K_CONGESTION_MGMT_ENABLE, r0
    beqi        r3, 1, setup_dcqcn
    phvwr       CAPRI_PHV_FIELD(TO_SQCB0_INFO_P, q_key_or_tm_iq), r2   // BD Slot

    CAPRI_SET_TABLE_3_VALID_CE(c0, 0)
    nop

dscp_map:
    //TODO: fetch txdma-iq corresponding to DSCP value in header-template

setup_dcqcn:
    CAPRI_RESET_TABLE_3_ARG()
    add         r3, r0, K_DCQCN_CONFIG_ADDR
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, rdma_aq_tx_dcqcn_config_process, r3)

