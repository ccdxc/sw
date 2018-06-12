#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct rqcb1_t d;
struct common_p4plus_stage0_app_header_table_k k;

#define TO_S_STATS_INFO_P to_s7_stats_info

#define TO_S_CQCB_INFO_P to_s5_cqcb_info

#define PHV_GLOBAL_COMMON_P phv_global_common

#define DMA_CMD_BASE r1

%%
    .param    resp_rx_stats_process

.align
resp_rx_rqcb_process_ext:

    // is this a fresh packet ?
    seq     c1, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 0
    bcf     [!c1], recirc_pkt

    //fresh packet
    // populate global fields
    phvwrpair   CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, lif), \
                CAPRI_RXDMA_INTRINSIC_LIF, \
                CAPRI_PHV_FIELD(PHV_GLOBAL_COMMON_P, cb_addr), \
                CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(RQCB_ADDR_SHIFT) //BD Slot

    phvwr       CAPRI_PHV_FIELD(TO_S_CQCB_INFO_P, bth_se), CAPRI_APP_DATA_BTH_SE 

    //Temporary code to test UDP options
    //For now, checking on ts flag for both options ts and mss to avoid performance cost
    bbeq     CAPRI_APP_DATA_ROCE_OPT_TS_VALID, 0, skip_roce_opt_parsing
    CAPRI_SET_FIELD_RANGE2(PHV_GLOBAL_COMMON_P, qid, qtype, CAPRI_RXDMA_INTRINSIC_QID_QTYPE) //BD Slot

    //get rqcb3 address
    add      r6, CAPRI_RXDMA_INTRINSIC_QSTATE_ADDR, (CB_UNIT_SIZE_BYTES * 3)
    add      r5, r6, FIELD_OFFSET(rqcb3_t, roce_opt_ts_value)
    memwr.d  r5, CAPRI_APP_DATA_ROCE_OPT_TS_VALUE_AND_ECHO
    add      r5, r6, FIELD_OFFSET(rqcb3_t, roce_opt_mss)
    memwr.h  r5, CAPRI_APP_DATA_ROCE_OPT_MSS

skip_roce_opt_parsing:

    // stats should not include pad bytes
    sub     r2, CAPRI_APP_DATA_PAYLOAD_LEN, CAPRI_APP_DATA_BTH_PAD
    CAPRI_SET_FIELD2(TO_S_STATS_INFO_P, pyld_bytes, r2)

    seq     c1, CAPRI_APP_DATA_BTH_PAD, r0
    bcf     [c1], exit

    // If PAD is not 0, introduce a skip command. This command is statically placed after
    // the payload transfer DMA commands, hence skips any PAD bytes from getting transfered.
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_RX_DMA_CMD_START_FLIT_ID, RESP_RX_DMA_CMD_SKIP_PLD) //BD Slot
    DMA_SKIP_CMD_SETUP(DMA_CMD_BASE, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/)

recirc_pkt:
    seq     c1, CAPRI_RXDMA_INTRINSIC_RECIRC_COUNT, 7
    bcf     [!c1], exit
    nop // BD Slot

    // invoke stats_process as mpu_only program and it keeps
    // bubbling down and handles actual stats in stage 7.
    CAPRI_SET_FIELD2(TO_S_STATS_INFO_P, incr_recirc_drop, 1)
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_stats_process, r0)
exit:
    nop.e
    nop
