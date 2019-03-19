/*
 *    Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "nic/p4/common/defines.h"
#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t2_tcp_rx_k.h"

struct phv_ p;
struct s6_t2_tcp_rx_k_ k;

%%
    .param          tcp_rx_stats_stage_start
    .align

    /*
     * Global conditional variables
     * c7 drop packet and don't send to arm
     */
tcp_rx_write_ooq_stage_start:

dma_cmd_data:
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, TCP_PHV_RXDMA_COMMANDS_START

    /* Set the DMA_WRITE CMD for data */
    add         r3, k.to_s6_page, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)

    CAPRI_DMA_CMD_PKT2MEM_SETUP(pkt_dma_dma_cmd, r3, k.t2_s2s_payload_len)

dma_cmd_descr:
    /* Set the DMA_WRITE CMD for descr */
    add         r1, k.to_s6_descr, NIC_DESC_ENTRY_0_OFFSET

    addi        r3, r0, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr       p.aol_A0, k.{to_s6_page}.dx
    phvwr       p.aol_O0, r3.wx
    phvwr       p.aol_L0, k.{to_s6_payload_len}.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(pkt_descr_dma_dma_cmd, r1, aol_A0, aol_next_pkt)

dma_tcp_hdr:
    add         r1, k.to_s6_descr, NIC_DESC_ENTRY_OOO_TCP_HDR_OFFSET
    phvwr       p.tcp_app_header_from_ooq_txdma, 1

    // HACK, 1+8 bytes following tcp_app_header is ooq_header which contains the
    // descriptor address. Until we can unionize this header correctly in p4,
    // hardcoding the PHV location for now. This is prone to error, but
    // hopefully if something breaks, we have DOL test cases to catch it.
    // (refer to iris/gen/p4gen/tcp_proxy_rxdma/asm_out/INGRESS_p.h)
    // HACK descriptor spans 1 byte in flit 0 and 7 bytes in flit 1
    add         r2, r0, 1024 - (7*8)
    phvwrp      r2, 0, 34, k.to_s6_descr

    // HACK: penultimate byte in flit 0 is feedback type
    // (immediately following p4 to p4plus header)
    phvwrp      r0, 8, 8, TCP_TX2RX_FEEDBACK_OOO_PKT

    CAPRI_DMA_CMD_PHV2MEM_SETUP_WITH_LEN(tcp_flags_dma_dma_cmd, r1,
                    tcp_app_header_p4plus_app_id,
                    P4PLUS_TCP_PROXY_BASE_HDR_SZ + P4PLUS_TCP_PROXY_OOQ_HDR_SZ)
dma_cmd_ooq_slot:
    phvwr       p.ring_entry_descr_addr, k.to_s6_descr

    add         r1, k.t2_s2s_ooo_qbase_addr, k.t2_s2s_ooo_tail_index, 3
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ring_slot_dma_cmd, r1, ring_entry_pad, ring_entry_descr_addr)

dma_cmd_set_eop:
    seq         c1, k.common_phv_pending_txdma, 0
    phvwr.c1    p.rx2tx_extra_dma_dma_cmd_eop, 1

dma_cmd_ooq_next_table:
    CAPRI_CLEAR_TABLE_VALID(2)
    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_rx_stats_stage_start)
    nop.e
    nop

