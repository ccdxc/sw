
#include "nic/asm/eth/defines.h"

#define LG2_TX_QSTATE_SIZE          (6)

#define LG2_TX_SG_MAX_READ_SIZE     (6)

#define LG2_TX_STATS_BLOCK_SZ       (6)

// TX limits
#define MAX_DESC_SPEC               (64)
#define MAX_BYTES_PER_PHV           (16384)

// TX Spurious Doorbell config
#define MAX_SPURIOUS_DB             (8)
#define LG2_MAX_SPURIOUS_DB         (3)

// TX Descriptor Opcodes
#define TXQ_DESC_OPCODE_CSUM_NONE           0x0
#define TXQ_DESC_OPCODE_CSUM_PARTIAL        0x1
#define TXQ_DESC_OPCODE_CSUM_TCPUDP         0x2
#define TXQ_DESC_OPCODE_TSO                 0x3

// DMA Macros
#define ETH_DMA_CMD_START_OFFSET    (CAPRI_PHV_START_OFFSET(dma_dma_cmd_type) / 16)
#define ETH_DMA_CMD_START_FLIT      ((offsetof(p, dma_dma_cmd_type) / 512) + 1)
#define ETH_DMA_CMD_START_INDEX     0

// Completion error codes
#define ETH_TX_DESC_ADDR_ERROR      1
#define ETH_TX_DESC_DATA_ERROR      2

/*
 * Stats Macros
 */

#define INIT_STATS(_r) \
    add         _r, r0, r0

#define LOAD_STATS(_r) \
    add         _r, r0, k.eth_tx_global_stats;

#define SET_STAT(_r, _c, name) \
    ori._c      _r, _r, (1 << STAT_##name);

#define CLR_STAT(_r, _c, name) \
    andi._c     _r, _r, ~(1 << STAT_##name);

#define SAVE_STATS(_r) \
    phvwr       p.eth_tx_global_stats, _r;

/*
 * Stat position within stats register
 */

// queue & descriptor error counters
#define STAT_queue_disabled                 0
#define STAT_queue_error                    1
#define STAT_desc_fetch_error               2
#define STAT_desc_data_error                3
#define STAT_queue_empty                    4
#define STAT_event_disabled                 5
#define STAT_event_error                    6
// DEBUG: operation counters
#define STAT_oper_csum_hw                   8
#define STAT_oper_csum_hw_inner             9
#define STAT_oper_vlan_insert               10
#define STAT_oper_sg                        11
#define STAT_oper_tso_sg                    12
#define STAT_oper_tso_sot                   13
#define STAT_oper_tso_eot                   14
// DEBUG: descriptor opcode counters
#define STAT_desc_opcode_invalid            16
#define STAT_desc_opcode_csum_none          17
#define STAT_desc_opcode_csum_partial       18
#define STAT_desc_opcode_csum_hw            19
#define STAT_desc_opcode_tso                20
// DEBUG: completion, event, and intr counters (XXX not counted)
#define STAT_arm                            24
#define STAT_cqe                            25
#define STAT_eqe                            26
#define STAT_intr                           27

/*
 * Descriptor decode Macros
 */
#define BUF_ADDR_FROM_KEY(_r, hdr) \
    add         _r, r0, k.{eth_tx_##hdr##_addr_hi...eth_tx_##hdr##_addr_lo}.dx; \
    add         _r, r0, _r[63:12];

#define BUF_ADDR_FROM_DATA(_r) \
    add         _r, r0, d.{addr_hi...addr_lo}.dx; \
    add         _r, r0, _r[63:12];

/*
 * DMA Macros
 */

#define DMA_INTRINSIC(_r) \
    DMA_PHV2PKT_3(_r, CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport), CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type), CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid), CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv), CAPRI_PHV_START_OFFSET(eth_tx_app_hdr_p4plus_app_id), CAPRI_PHV_END_OFFSET(eth_tx_app_hdr_vlan_tag), r7);

#define DMA_PKT(_r_addr, _r_ptr, hdr) \
    BUF_ADDR_FROM_KEY(_r_addr, hdr); \
    DMA_MEM2PKT(_r_ptr, c0, k.eth_tx_global_host_queue, _r_addr, k.{eth_tx_##hdr##_len}.hx, r7);

#define DMA_HDR(_r_addr, _r_ptr, hdr) \
    BUF_ADDR_FROM_KEY(_r_addr, hdr); \
    seq          c7, k.eth_tx_global_num_sg_elems, 0; \
    DMA_MEM2PKT(_r_ptr, c7, k.eth_tx_global_host_queue, _r_addr, k.{eth_tx_##hdr##_len}.hx, r7);

#define DMA_TSO_HDR(_r_addr, _r_ptr, hdr) \
    phvwr      p.{eth_tx_app_hdr_ip_id_delta,eth_tx_app_hdr_tcp_seq_delta}, k.{eth_tx_t1_s2s_tso_ipid_delta,eth_tx_t1_s2s_tso_seq_delta};\
    DMA_MEM2PKT(_r_ptr, !c0, k.eth_tx_global_host_queue, k.eth_tx_t1_s2s_tso_hdr_addr, k.eth_tx_t1_s2s_tso_hdr_len, r7);

#define DMA_FRAG(n, _c, _r_addr, _r_ptr) \
    DMA_MEM2PKT(_r_ptr, _c, k.eth_tx_global_host_queue, d.{addr##n}.dx, d.{len##n}.hx, r7);
