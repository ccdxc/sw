#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "capri-macros.h"

struct req_tx_phv_t p;
struct dcqcn_config_cb_t d;
struct req_tx_s3_t3_k k;

#define SQCB_TO_DCQCN_CFG_T t3_s2s_dcqcn_config_info
#define TO_S_STATS_INFO_P to_s7_stats_info
#define TO_S2_SQWQE_P  to_s2_sqwqe_info
#define TO_S3_SQWQE_P  to_s3_sqwqe_info

#define K_DCQCN_CFG_ID CAPRI_KEY_FIELD(SQCB_TO_DCQCN_CFG_T, dcqcn_config_id)
#define K_DCQCN_CFG_BASE CAPRI_KEY_RANGE(SQCB_TO_DCQCN_CFG_T, dcqcn_config_base_sbit0_ebit1, dcqcn_config_base_sbit2_ebit33)
#define K_HEAD_TEMPLATE_ADDR CAPRI_KEY_RANGE(SQCB_TO_DCQCN_CFG_T, header_template_addr_sbit0_ebit1, header_template_addr_sbit26_ebit31)

#define K_IPV6_FLAG CAPRI_KEY_FIELD(TO_S3_SQWQE_P, ipv6_flag)
#define K_IPV6_TAG CAPRI_KEY_FIELD(TO_S3_SQWQE_P, ipv6_tag)
#define K_LOG_NUM_KT_ENTRIES CAPRI_KEY_RANGE(TO_S3_SQWQE_P, log_num_kt_entries_sbit0_ebit0, log_num_kt_entries_sbit1_ebit4)
#define DMA_CMD_BASE        r1

%%
    .param      req_tx_stats_process
    .param      req_tx_dcqcn_cnp_add_header_process

.align
req_tx_dcqcn_config_load_process:
    // Pin dcqcn_cfg to stage 1 table 3, stage2 table 3
    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_2
    bcf         [c1], config_load
    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7) //BD Slot

    seq         c1, r1[4:2], STAGE_3
    bcf         [!c1], exit
    nop

add_header_dscp_value:
    // Because ipv4 and ipv6 has different header format, handel them differently
    // ipv4: version(4bits) + IHL(4bits) + TOS(8bits) + Total Length(16bits)
    // ipv6: version(4bits) + TC(8bits) + Flow Label(20bits) 
    bbeq        K_IPV6_FLAG, 1, ipv6
    add         r3, d.np_cnp_dscp, r0 //BD Slot

ipv4:
    // if ipv4 and DSCP(tos), the whole header_template will be separated into 3 parts
    // part I(152bits) + tos value(8bits) + part III(208bits)
    // the first part and last part DMA in req_tx_sqcb2_cnp_proces
    // invoke stats as mpu only, branch to dcqcn_cnp_sent in req_tx_stats
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_stats_process, r0)
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_CNP_HEADER_TEMPLATE_2)
    phvwr.e     p.cnp_tos.dscp, r3
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, cnp_tos, cnp_tos) //BD Slot

ipv6:
    // if ipv6 and DSCP(tos), the whole header_template will be also separated into 3 parts
    // part I(144bits) + [ version + tc + (first 4 bit of flow label)](16bits) + part III (368)
    // the first part and last part DMA in req_tx_sqcb2_cnp_proces, the middle part will be handled in req_tx_dcqcn_cnp_add_header_process
    seq       c1, K_IPV6_TAG, 1
    add.c1         r1, CNP_IPV6_HEADER_TEMPLATE_1_LEN_TAG, K_HEAD_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    add.!c1         r1, CNP_IPV6_HEADER_TEMPLATE_1_LEN_UNTAG, K_HEAD_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT
    phvwr       CAPRI_PHV_FIELD(SQCB_TO_DCQCN_CFG_T, np_cnp_dscp), d.np_cnp_dscp
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_16_BITS, req_tx_dcqcn_cnp_add_header_process, r1)

config_load:
    SQ_TX_DCQCN_CONFIG_BASE_ADDR_GET2(r2, r4, K_LOG_NUM_KT_ENTRIES)
    add         r2, r2, K_DCQCN_CFG_ID, LOG_SIZEOF_DCQCN_CONFIG_T
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r2)
    nop

exit:
   nop.e
   nop
