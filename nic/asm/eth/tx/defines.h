
#include "../defines.h"

#define LG2_TX_DESC_SIZE            (4)
#define LG2_TX_CMPL_DESC_SIZE       (4)
#define LG2_TX_SG_ELEM_SIZE         (4)
#define LG2_TX_SG_MAX_READ_SIZE     (6)
#define TX_SG_MAX_READ_SIZE         (64)    // 4 sg elements
#define TX_SG_MAX_READ_ELEM         (4)
#define LG2_TX_SG_DESC_SIZE         (8)
#define LG2_TX_QSTATE_SIZE          (6)

// TX limits
#define MAX_DESC_PER_PHV            (4)
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
#define ETH_DMA_CMD_START_FLIT      8  // DMA commands flit
#define ETH_DMA_CMD_START_INDEX     0

#define GET_BUF_ADDR(n, _r) \
    or          _r, k.eth_tx_to_s2_addr_lo##n, k.eth_tx_to_s2_addr_hi##n, sizeof(k.eth_tx_to_s2_addr_lo##n); \
    add         _r, r0, _r.dx; \
    or          _r, _r[63:16], _r[11:8], sizeof(k.eth_tx_to_s2_addr_lo##n);

#define GET_FRAG_ADDR(n, _r) \
    or          _r, d.addr_lo##n, d.addr_hi##n, sizeof(d.addr_lo##n); \
    add         _r, r0, _r.dx; \
    or          _r, _r[63:16], _r[11:8], sizeof(d.addr_lo##n);

#define GET_CSUM_START(n, _r) \
    or          _r, k.eth_tx_to_s2_hdr_len_lo##n, k.eth_tx_to_s2_hdr_len_hi##n, sizeof(k.eth_tx_to_s2_hdr_len_lo##n); \
    add         _r, r0, _r.dx; \
    or          _r, _r[63:56], _r[49:48];

#define GET_CSUM_OFF(n, _r) \
    or          _r, k.eth_tx_to_s2_mss_or_csumoff_lo##n, k.eth_tx_to_s2_mss_or_csumoff_hi##n, sizeof(k.eth_tx_to_s2_mss_or_csumoff_lo##n); \
    add         _r, r0, _r.dx; \
    or          _r, _r[63:56], _r[53:48], sizeof(k.eth_tx_to_s2_mss_or_csumoff_lo##n);

#define GET_MSS(n, _r) \
    or          _r, k.eth_tx_to_s2_mss_or_csumoff_lo##n, k.eth_tx_to_s2_mss_or_csumoff_hi##n, sizeof(k.eth_tx_to_s2_mss_or_csumoff_lo##n); \
    add         _r, r0, _r.dx; \
    or          _r, _r[63:56], _r[53:48], sizeof(k.eth_tx_to_s2_mss_or_csumoff_lo##n);

#define GET_HDR_LEN(n, _r) \
    or          _r, k.eth_tx_to_s2_hdr_len_lo##n, k.eth_tx_to_s2_hdr_len_hi##n, sizeof(k.eth_tx_to_s2_hdr_len_lo##n); \
    add         _r, r0, _r.dx; \
    or          _r, _r[63:56], _r[49:48]; \

#define DMA_INTRINSIC(n, _r) \
    DMA_PHV2PKT_3(_r, CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport), CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type), CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid), CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv), CAPRI_PHV_START_OFFSET(eth_tx_app_hdr##n##_p4plus_app_id), CAPRI_PHV_END_OFFSET(eth_tx_app_hdr##n##_vlan_tag), r7);

#define DMA_PKT(n, _r_addr, _r_ptr) \
    GET_BUF_ADDR(n, _r_addr); \
    DMA_HOST_MEM2PKT(_r_ptr, c0, _r_addr, k.{eth_tx_to_s2_len##n}.hx);

#define DMA_HDR(n, _r_addr, _r_ptr) \
    GET_BUF_ADDR(n, _r_addr); \
    DMA_HOST_MEM2PKT(_r_ptr, !c0, _r_addr, k.{eth_tx_to_s2_len##n}.hx);

#define DMA_FRAG(n, _c, _r_addr, _r_ptr) \
    GET_FRAG_ADDR(n, _r_addr); \
    DMA_HOST_MEM2PKT(_r_ptr, _c, _r_addr, d.{len##n}.hx);

#define BUILD_APP_HEADER(n) \
    phvwri      p.eth_tx_app_hdr##n##_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC; \
    phvwr       p.eth_tx_app_hdr##n##_insert_vlan_tag, k.eth_tx_to_s2_vlan_insert##n##;\
    phvwr       p.eth_tx_app_hdr##n##_vlan_tag, k.{eth_tx_to_s2_vlan_tci##n}.hx; \
eth_tx_opcode_start:; \
    add         r7, r0, k.eth_tx_to_s2_opcode##n##; \
.brbegin; \
    br          r7[1:0]; \
    nop; \
    .brcase     TXQ_DESC_OPCODE_CALC_NO_CSUM; \
        b       eth_tx_opcode_done; \
        nop; \
    .brcase     TXQ_DESC_OPCODE_CALC_CSUM; \
        b       eth_tx_calc_csum; \
        nop; \
    .brcase     TXQ_DESC_OPCODE_CALC_CSUM_TCPUDP; \
        b       eth_tx_calc_csum_tcpudp; \
        nop; \
    .brcase     TXQ_DESC_OPCODE_TSO; \
        b       eth_tx_tso; \
        nop; \
.brend; \
eth_tx_calc_csum:; \
    GET_CSUM_START(n, r2); \
    addi        r2, r2, 46;\
    GET_CSUM_OFF(n, r3); \
    add         r3, r3, r2;\
    phvwr       p.eth_tx_app_hdr##n##_gso_valid, 1; \
    phvwrpair   p.eth_tx_app_hdr##n##_gso_start, r2, p.eth_tx_app_hdr##n##_gso_offset, r3; \
eth_tx_calc_csum_tcpudp:; \
    seq         c1, k.eth_tx_to_s2_encap##n, 1; \
    bcf         [!c1], eth_tx_calc_csum_tcpudp_noencap; \
    nop; \
    phvwrpair   p.eth_tx_app_hdr##n##_compute_l4_csum, 1, p.eth_tx_app_hdr##n##_compute_ip_csum, 1; \
    phvwrpair   p.eth_tx_app_hdr##n##_compute_inner_l4_csum, k.eth_tx_to_s2_csum_l4##n, p.eth_tx_app_hdr##n##_compute_inner_ip_csum, k.eth_tx_to_s2_csum_l3##n; \
eth_tx_calc_csum_tcpudp_noencap:; \
    phvwrpair   p.eth_tx_app_hdr##n##_compute_l4_csum, k.eth_tx_to_s2_csum_l4##n, p.eth_tx_app_hdr##n##_compute_ip_csum, k.eth_tx_to_s2_csum_l3##n; \
eth_tx_tso:; \
eth_tx_opcode_done:;

#define DEBUG_DESCR_FLD(name) \
    add         r7, r0, d.##name

#define DEBUG_DESCR(n) \
    DEBUG_DESCR_FLD(addr_lo##n); \
    DEBUG_DESCR_FLD(addr_hi##n); \
    DEBUG_DESCR_FLD(rsvd##n); \
    DEBUG_DESCR_FLD(num_sg_elems##n); \
    DEBUG_DESCR_FLD(opcode##n); \
    DEBUG_DESCR_FLD(len##n); \
    DEBUG_DESCR_FLD(vlan_tci##n); \
    DEBUG_DESCR_FLD(hdr_len_lo##n); \
    DEBUG_DESCR_FLD(hdr_len_hi##n); \
    DEBUG_DESCR_FLD(rsvd2##n); \
    DEBUG_DESCR_FLD(vlan_insert##n); \
    DEBUG_DESCR_FLD(cq_entry##n); \
    DEBUG_DESCR_FLD(csum##n); \
    DEBUG_DESCR_FLD(mss_or_csumoff_lo##n); \
    DEBUG_DESCR_FLD(mss_or_csumoff_hi##n); \
    DEBUG_DESCR_FLD(rsvd3_or_rsvd4##n)
