#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

struct req_tx_phv_t p;
struct req_tx_add_headers_process_k_t k;
struct sqcb1_t d;

%%

.align
req_tx_add_headers_2_process:
    // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_PHV_INTRINSIC)

    // dma_cmd[0] : addr1 - p4_intr
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, common.p4_intr_global_tm_iport, common.p4_intr_global_tm_instance_type, 3)
    phvwrpair      p.common.p4_intr_global_tm_iport, TM_PORT_DMA, p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS

    // No need to fill p4_txdma_intr fields as they are already filled before stage0
    // dma_cmd[0] : addr2 - p4_txdma_intr
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, common.p4_txdma_intr_qid, common.p4_txdma_intr_txdma_rsv, 1)

    // dma_cmd[0] : addr3 - p4plus_to_p4_header
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, p4plus_to_p4, p4plus_to_p4, 2);
  
    #c3 - UD service. Needed only for send & send_imm
    seq            c3, k.args.service, RDMA_SERV_TYPE_UD

    //For UD, ah_handle comes in send req.
    sll            r3, k.args.header_template_addr, HDR_TEMP_ADDR_SHIFT
    // dma_cmd[1] - header_template
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_HEADER_TEMPLATE)
    DMA_HBM_MEM2PKT_SETUP(r6, k.args.header_template_size, r3)

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_HEADERS)
    // dma_cmd[2] : addr2 - deth only if it is UD service (bth setup in add_headers_process)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N_C(r6, deth, deth, 1, c3)

    // For PAD and ICRC
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_PAD_ICRC)
    // dma_cmd[0] : addr1 - pad/icrc
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, immeth, immeth, 1)
    // For ICRC, can point to any 4 bytes of PHV so point to immeth to create 4B hdr
    //  space for ICRC Deparser in P4 calculate and fills ICRC here

    crestore       [c6, c5], k.{args.roce_opt_ts_enable...args.roce_opt_mss_enable}, 0x3
    #c5 - mss_enable
    #c6 - ts_enable
    bcf            [!c5 & !c6],  skip_roce_udp_options
    CAPRI_SET_TABLE_3_VALID(0) //BD slot
    phvwrpair      p.roce_options.OCS_kind, ROCE_OPT_KIND_OCS, p.roce_options.OCS_value, 0
    phvwrpair      p.roce_options.TS_kind, ROCE_OPT_KIND_TS, p.roce_options.TS_len, ROCE_OPT_LEN_TS
    phvwrpair      p.roce_options.MSS_kind, ROCE_OPT_KIND_MSS, p.roce_options.MSS_len, ROCE_OPT_LEN_MSS

#define ROCE_MAX_OPTION_BYTES (sizeof(p.roce_options)/8)
#define ROCE_TOTAL_OPTION_BYTES_TS (ROCE_MAX_OPTION_BYTES - ROCE_OPT_LEN_MSS)
#define ROCE_TOTAL_OPTION_BYTES_MSS (ROCE_MAX_OPTION_BYTES - ROCE_OPT_LEN_TS)
#define ROCE_TOTAL_OPTION_BYTES_TS_MSS (ROCE_MAX_OPTION_BYTES)

    .csbegin
    cswitch [c6, c5]
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_UDP_OPTS) //BDS
    .brcase 0 #both options not enabled
        #should not have come this far if both flags were false
        b              roce_udp_options_done
        phvwr          P4PLUS_TO_P4_UDP_OPT_BYTES, r0 //BDS

    .brcase 1 #MSS enable
        DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, roce_options.OCS_kind, roce_options.OCS_value, 3)       #OCS kind, OCS value
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, roce_options.MSS_kind, roce_options.MSS_value, 1) #MSS kind, MSS len, MSS value
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, roce_options.EOL_kind, roce_options.EOL_kind, 2)        #EOL kind
        b              roce_udp_options_done
        phvwr          P4PLUS_TO_P4_UDP_OPT_BYTES, ROCE_TOTAL_OPTION_BYTES_MSS //BDS

    .brcase 2 #TS enable
        DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, roce_options.OCS_kind, roce_options.OCS_value, 3)       #OCS kind, OCS value
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, roce_options.TS_kind, roce_options.TS_echo, 1)    #TS kind, TS len, TS value
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, roce_options.EOL_kind, roce_options.EOL_kind, 2)        #EOL kind
        b              roce_udp_options_done
        phvwr          P4PLUS_TO_P4_UDP_OPT_BYTES, ROCE_TOTAL_OPTION_BYTES_TS //BDS

    .brcase 3 #TS enable, MSS enable
        DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, roce_options.OCS_kind, roce_options.OCS_value, 4)       #OCS kind, OCS value
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, roce_options.TS_kind, roce_options.TS_echo, 1)          #TS kind, TS len, TS value
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, roce_options.MSS_kind, roce_options.MSS_value, 2)       #MSS kind, MSS len, MSS value
        DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, roce_options.EOL_kind, roce_options.EOL_kind, 3)        #EOL kind
        b              roce_udp_options_done
        phvwr          P4PLUS_TO_P4_UDP_OPT_BYTES, ROCE_TOTAL_OPTION_BYTES_TS_MSS //BDS

    .csend

roce_udp_options_done:
skip_roce_udp_options:
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)

    bcf            [!c3], exit
    add            r1, r0, offsetof(struct req_tx_phv_t, p4_to_p4plus)
    phvwrp         r1, 0, CAPRI_SIZEOF_RANGE(struct req_tx_phv_t, p4_intr_global, p4_to_p4plus), r0
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_FEEDBACK) // Branch Delay Slot
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, p4_intr_global, p4_to_p4plus, 2)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, rdma_feedback, rdma_feedback, 1)

    phvwrpair      p.p4_intr_global.tm_iport, TM_PORT_INGRESS, p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwrpair      p.p4_intr_global.tm_iq, 0, p.p4_intr_global.lif, k.global.lif
    SQCB0_ADDR_GET(r1)
    phvwrpair      p.p4_intr_rxdma.intr_qid, k.global.qid, p.p4_intr_rxdma.intr_qstate_addr, r1
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET 

    phvwrpair      p.p4_intr_rxdma.intr_qtype, k.global.qtype, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK

    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)

exit:
    // This gets executed in both UD and non-UD cases
    // In case of UD, EOC set for Feedback command, otherwise set for ICRC_PAD command
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)
    nop.e
    nop

