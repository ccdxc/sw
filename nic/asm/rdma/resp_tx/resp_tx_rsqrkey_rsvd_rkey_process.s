#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s3_t0_k k;
struct key_entry_aligned_t d;

#define DMA_CMD_BASE r1
#define IN_PROGRESS r2

#define RQCB0_WB_INFO_P t1_s2s_rqcb0_write_back_info

#define IN_P    t0_s2s_rsqwqe_to_rkey_info
#define IN_TO_S_P to_s3_dcqcn_info
#define TO_S4_P  to_s4_dcqcn_info
#define TO_S7_P  to_s7_stats_info

#define K_XFER_VA CAPRI_KEY_RANGE(IN_P, transfer_va_sbit0_ebit7, transfer_va_sbit48_ebit63)
#define K_XFER_BYTES CAPRI_KEY_RANGE(IN_P, transfer_bytes_sbit0_ebit7, transfer_bytes_sbit8_ebit15)
#define K_HDR_TMP CAPRI_KEY_RANGE(IN_P, header_template_addr_sbit0_ebit7, header_template_addr_sbit24_ebit31)
#define K_HDR_TMP_SZ CAPRI_KEY_FIELD(IN_P, header_template_size)
#define K_DCQCN_CB_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, dcqcn_cb_addr_sbit0_ebit15, dcqcn_cb_addr_sbit32_ebit33)

%%
    .param      resp_tx_dcqcn_enforce_process

resp_tx_rsqrkey_rsvd_rkey_process:

    bbeq        CAPRI_KEY_FIELD(IN_P, skip_rkey), 1, add_headers
    CAPRI_SET_TABLE_0_VALID(0) // BD Slot

    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_PYLD_BASE)
    // r1 has DMA_CMD_BASE
    DMA_MEM2PKT_SETUP(DMA_CMD_BASE, c0, K_XFER_BYTES, K_XFER_VA)

add_headers:

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_HDR_TEMPLATE)
    sll r4, K_HDR_TMP, HDR_TEMP_ADDR_SHIFT
    DMA_HBM_MEM2PKT_SETUP(DMA_CMD_BASE, K_HDR_TMP_SZ, r4)
    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_BTH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, bth, bth)

    bbne        CAPRI_KEY_FIELD(IN_P, send_aeth), 1, invoke_dcqcn
    CAPRI_SET_FIELD2(TO_S7_P, pyld_bytes, K_XFER_BYTES) //BD Slot

    DMA_CMD_STATIC_BASE_GET(DMA_CMD_BASE, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_AETH)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, aeth, aeth)

invoke_dcqcn:
    phvwr          p.common.p4_intr_packet_len, K_XFER_BYTES
    // Note: Next stage(DCQCN) does not use stage-to-stage keys. So this will be passed to write-back stage untouched!
    CAPRI_RESET_TABLE_1_ARG()
    seq         c1, CAPRI_KEY_FIELD(IN_P, last_or_only), 1
    cmov        IN_PROGRESS, c1, 0, 1
    phvwrpair   CAPRI_PHV_FIELD(RQCB0_WB_INFO_P, curr_read_rsp_psn), \
                CAPRI_KEY_RANGE(IN_P, curr_read_rsp_psn_sbit0_ebit7, curr_read_rsp_psn_sbit16_ebit23), \
                CAPRI_PHV_FIELD(RQCB0_WB_INFO_P, read_rsp_in_progress), \
                IN_PROGRESS

    seq            c3, CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_type), 1
    bcf            [c3], dcqcn
    add            r3,  r0, K_DCQCN_CB_ADDR // BD slot

dcqcn_mpu_only:
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_dcqcn_enforce_process, r3)

dcqcn:
    CAPRI_SET_FIELD2(TO_S4_P, packet_len, K_XFER_BYTES)
    CAPRI_NEXT_TABLE1_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_tx_dcqcn_enforce_process, r3)
