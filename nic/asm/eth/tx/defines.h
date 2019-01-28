
#include "nic/asm/eth/defines.h"

#define LG2_TX_DESC_SIZE            (4)
#define LG2_TX_CMPL_DESC_SIZE       (4)
#define LG2_TX_SG_ELEM_SIZE         (4)
#define LG2_TX_SG_MAX_READ_SIZE     (6)
#define TX_SG_MAX_READ_SIZE         (64)    // 4 sg elements
#define TX_SG_MAX_READ_ELEM         (4)
#define LG2_TX_SG_DESC_SIZE         (8)
#define LG2_TX_QSTATE_SIZE          (6)

// TX limits
#define MAX_DESC_SPEC               (64)
#define MAX_DESC_PER_PHV            (1)
#define MAX_BYTES_PER_PHV           (16384)

// TX Spurious Doorbell config
#define MAX_SPURIOUS_DB             (8)
#define LG2_MAX_SPURIOUS_DB         (3)

// TX Descriptor Opcodes
#define TXQ_DESC_OPCODE_CALC_NO_CSUM        0x0
#define TXQ_DESC_OPCODE_CALC_CSUM           0x1
#define TXQ_DESC_OPCODE_CALC_CSUM_TCPUDP    0x2
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
#define STAT_cqe                            21
#define STAT_intr                           22

/*
 * Descriptor decode Macros
 */
#define GET_CSUM_OFF(n, _r) \
    add         _r, r0, d.{mss_or_csumoff_lo##n...mss_or_csumoff_hi##n}.hx; \
    add         _r, r0, _r[13:0];

#define GET_MSS(n, _r) \
    add         _r, r0, d.{mss_or_csumoff_lo##n...mss_or_csumoff_hi##n}.hx; \
    add         _r, r0, _r[13:0];

#define GET_HDR_LEN(n, _r) \
    add         _r, r0, d.{hdr_len_lo##n...hdr_len_hi##n}.hx; \
    add         _r, r0, _r[9:0];

#define GET_BUF_ADDR(n, _r, hdr) \
    add         _r, r0, k.{eth_tx_##hdr##_addr_lo##n...eth_tx_##hdr##_addr_hi##n}.dx; \
    add         _r, r0, _r[59:8];

#define GET_FRAG_ADDR(n, _r) \
    add         _r, r0, d.{addr_lo##n...addr_hi##n}.dx; \
    add         _r, r0, _r[59:8];

/*
 * DMA Macros
 */

#define DMA_INTRINSIC(n, _r) \
    DMA_PHV2PKT_3(_r, CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport), CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type), CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid), CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv), CAPRI_PHV_START_OFFSET(eth_tx_app_hdr##n##_p4plus_app_id), CAPRI_PHV_END_OFFSET(eth_tx_app_hdr##n##_vlan_tag), r7);

#define DMA_PKT(n, _r_addr, _r_ptr, hdr) \
    GET_BUF_ADDR(n, _r_addr, hdr); \
    DMA_MEM2PKT(_r_ptr, c0, k.eth_tx_global_host_queue, _r_addr, k.{eth_tx_##hdr##_len##n}.hx);

#define DMA_HDR(n, _r_addr, _r_ptr, hdr) \
    GET_BUF_ADDR(n, _r_addr, hdr); \
    seq          c7, k.eth_tx_global_num_sg_elems, 0; \
    DMA_MEM2PKT(_r_ptr, c7, k.eth_tx_global_host_queue, _r_addr, k.{eth_tx_##hdr##_len##n}.hx);

#define DMA_TSO_HDR(n, _r_addr, _r_ptr, hdr) \
    phvwr      p.{eth_tx_app_hdr##n##_ip_id_delta,eth_tx_app_hdr##n##_tcp_seq_delta}, k.{eth_tx_t2_s2s_tso_ipid_delta,eth_tx_t2_s2s_tso_seq_delta};\
    DMA_MEM2PKT(_r_ptr, !c0, k.eth_tx_global_host_queue, k.eth_tx_t2_s2s_tso_hdr_addr, k.eth_tx_t2_s2s_tso_hdr_len);

#define DMA_FRAG(n, _c, _r_addr, _r_ptr) \
    GET_FRAG_ADDR(n, _r_addr); \
    DMA_MEM2PKT(_r_ptr, _c, k.eth_tx_global_host_queue, _r_addr, d.{len##n}.hx);

/*
 * Opcode Handling
 */

#define BUILD_APP_HEADER(n, _r_t1, _r_t2, _r_stats) \
    phvwri      p.eth_tx_app_hdr##n##_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC; \
    sne         c7, d.vlan_insert##n, 0; \
    phvwr.c7    p.eth_tx_app_hdr##n##_insert_vlan_tag, d.vlan_insert##n; \
    phvwr.c7    p.eth_tx_app_hdr##n##_vlan_tag, d.{vlan_tci##n}.hx; \
    SET_STAT(_r_stats, c7, oper_vlan_insert) \
    add         r7, r0, d.opcode##n##; \
.brbegin; \
    br          r7[1:0]; \
    nop; \
    .brcase         TXQ_DESC_OPCODE_CALC_NO_CSUM; \
        b               eth_tx_opcode_done##n; \
        phvwri          p.eth_tx_global_cq_entry, 1; \
    .brcase         TXQ_DESC_OPCODE_CALC_CSUM; \
        b               eth_tx_calc_csum##n; \
        phvwri          p.eth_tx_global_cq_entry, 1; \
    .brcase         TXQ_DESC_OPCODE_CALC_CSUM_TCPUDP; \
        b               eth_tx_calc_csum_tcpudp##n; \
        phvwri          p.eth_tx_global_cq_entry, 1; \
    .brcase         TXQ_DESC_OPCODE_TSO; \
        b               eth_tx_opcode_tso##n; \
        phvwr           p.eth_tx_global_cq_entry, d.{csum_l4_or_eot##n}; \
.brend; \
    SET_STAT(_r_stats, _C_TRUE, desc_opcode_invalid) \
    b               eth_tx_opcode_done##n; \
    nop; \
eth_tx_opcode_csum_none##n:;\
    SET_STAT(_r_stats, _C_TRUE, desc_opcode_csum_none) \
    b               eth_tx_opcode_done##n; \
    nop; \
eth_tx_calc_csum##n:; \
    SET_STAT(_r_stats, _C_TRUE, desc_opcode_csum_partial) \
    GET_HDR_LEN(n, _r_t1) \
    GET_CSUM_OFF(n, _r_t2) \
    add             _r_t1, _r_t1, 46; \
    add             _r_t2, _r_t2, _r_t1; \
    phvwri          p.eth_tx_app_hdr##n##_gso_valid, 1; \
    b               eth_tx_opcode_done##n; \
    phvwrpair       p.eth_tx_app_hdr##n##_gso_start, _r_t1, p.eth_tx_app_hdr##n##_gso_offset, _r_t2; \
eth_tx_calc_csum_tcpudp##n:; \
    SET_STAT(_r_stats, _C_TRUE, desc_opcode_csum_hw) \
    SET_STAT(_r_stats, _C_TRUE, oper_csum_hw) \
    seq             c7, d.encap##n, 1; \
    SET_STAT(_r_stats, c7, oper_csum_hw_inner) \
    phvwrpair       p.eth_tx_app_hdr##n##_compute_l4_csum, d.csum_l4_or_eot##n, p.eth_tx_app_hdr##n##_compute_ip_csum, d.csum_l3_or_sot##n; \
    b               eth_tx_opcode_done##n; \
    phvwrpair.c7    p.eth_tx_app_hdr##n##_compute_inner_l4_csum, d.csum_l4_or_eot##n, p.eth_tx_app_hdr##n##_compute_inner_ip_csum, d.csum_l3_or_sot##n; \
eth_tx_opcode_tso##n:; \
    SET_STAT(_r_stats, _C_TRUE, desc_opcode_tso) \
    phvwr           p.{eth_tx_global_tso_eot,eth_tx_global_tso_sot}, d.{csum_l4_or_eot##n,csum_l3_or_sot##n}; \
    phvwri          p.eth_tx_t0_s2s_do_tso, 1; \
    phvwri          p.eth_tx_app_hdr##n##_tso_valid, 1; \
    phvwr           p.{eth_tx_app_hdr##n##_tso_last_segment,eth_tx_app_hdr##n##_tso_first_segment}, d.{csum_l4_or_eot##n,csum_l3_or_sot##n}; \
    phvwrpair       p.eth_tx_app_hdr##n##_compute_l4_csum, 1, p.eth_tx_app_hdr##n##_compute_ip_csum, 1; \
    phvwrpair.c7    p.eth_tx_app_hdr##n##_compute_inner_l4_csum, 1, p.eth_tx_app_hdr##n##_compute_inner_ip_csum, 1; \
    bbeq            d.csum_l3_or_sot##n, 1, eth_tx_opcode_tso_sot##n; \
    phvwri          p.{eth_tx_app_hdr##n##_update_tcp_seq_no...eth_tx_app_hdr##n##_update_ip_id}, 0x7; \
    bbne            d.csum_l4_or_eot##n, 1, eth_tx_opcode_tso_cont##n; \
    nop; \
eth_tx_opcode_tso_eot##n:; \
    SET_STAT(_r_stats, _C_TRUE, oper_tso_eot) \
eth_tx_opcode_tso_cont##n:; \
    GET_MSS(n, _r_t1) \
    b               eth_tx_opcode_tso_done##n; \
    phvwr           p.eth_tx_to_s2_tso_hdr_addr[13:0], _r_t1[13:0]; \
eth_tx_opcode_tso_sot##n:;\
    SET_STAT(_r_stats, _C_TRUE, oper_tso_sot) \
    GET_FRAG_ADDR(n, _r_t1) \
    GET_HDR_LEN(n, _r_t2) \
    b               eth_tx_opcode_tso_done##n; \
    phvwrpair       p.eth_tx_to_s2_tso_hdr_addr, _r_t1, p.eth_tx_to_s2_tso_hdr_len, _r_t2; \
eth_tx_opcode_tso_done##n:;\
    SET_STAT(_r_stats, _C_TRUE, oper_csum_hw) \
    seq             c7, d.encap##n, 1; \
    SET_STAT(_r_stats, c7, oper_csum_hw_inner) \
    phvwrpair       p.eth_tx_app_hdr##n##_compute_l4_csum, 1, p.eth_tx_app_hdr##n##_compute_ip_csum, 1; \
    b               eth_tx_opcode_done##n; \
    phvwrpair.c7    p.eth_tx_app_hdr##n##_compute_inner_l4_csum, 1, p.eth_tx_app_hdr##n##_compute_inner_ip_csum, 1; \
eth_tx_opcode_done##n:;
