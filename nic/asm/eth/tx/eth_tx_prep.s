

#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "defines.h"

struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_eth_tx_prep_d d;

%%

.param  eth_tx_commit
.param  eth_tx_commit_tso

#define  _c_cq            c1  // Generate completion
#define  _c_sg            c2  // SG
#define  _c_tso           c3  // TSO

#define  _r_t1            r1  // Opcode processing register
#define  _r_t2            r2  // Opcode processing register
#define  _r_tbl_size      r3  // Table size
// #define  _r_stats         r6  // Stats


.align
eth_tx_prep:
  // LOAD_STATS(_r_stats)

  bcf         [c2 | c3 | c7], eth_tx_prep_error

  // Set intrinsics
#ifndef GFT
  phvwrpair   p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_INGRESS
#else
  phvwrpair   p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_EGRESS
#endif

  // Setup DMA CMD PTR
  phvwr       p.p4_txdma_intr_dma_cmd_ptr, ETH_DMA_CMD_START_OFFSET
  phvwr       p.eth_tx_global_dma_cur_index, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

  // Set APP type
  seq         c7, k.eth_tx_global_cpu_queue, 1
  cmov        r7, c7, P4PLUS_APPTYPE_CPU, P4PLUS_APPTYPE_CLASSIC_NIC
  phvwr       p.eth_tx_app_hdr_p4plus_app_id, r7

  // Vlan offload
  sne         c7, d.vlan_insert, 0
  phvwr.c7    p.eth_tx_app_hdr_insert_vlan_tag, d.vlan_insert
  phvwr.c7    p.eth_tx_app_hdr_vlan_tag, d.{vlan_tci}.hx
  // SET_STAT(_r_stats, c7, oper_vlan_insert)

  // Opcode handling
  add         r7, r0, d.opcode
.brbegin
  br          r7[1:0]
  seq         _c_tso, r7[1:0], TXQ_DESC_OPCODE_TSO
  .brcase TXQ_DESC_OPCODE_CSUM_NONE
      b             eth_tx_opcode_done
      setcf         _c_cq, [c0]
  .brcase TXQ_DESC_OPCODE_CSUM_PARTIAL
      b             eth_tx_calc_csum
      setcf         _c_cq, [c0]
  .brcase TXQ_DESC_OPCODE_CSUM_TCPUDP
      b             eth_tx_calc_csum_tcpudp
      setcf         _c_cq, [c0]
  .brcase TXQ_DESC_OPCODE_TSO
      b             eth_tx_opcode_tso
      seq           _c_cq, d.csum_l4_or_eot, 1
.brend
  // SET_STAT(_r_stats, _C_TRUE, desc_opcode_invalid)
  b               eth_tx_opcode_done
  nop
eth_tx_opcode_csum_none:
  // SET_STAT(_r_stats, _C_TRUE, desc_opcode_csum_none)
  b               eth_tx_opcode_done
  nop
eth_tx_calc_csum:
  // SET_STAT(_r_stats, _C_TRUE, desc_opcode_csum_partial)
  add             _r_t1, d.{csum_start_or_hdr_len}.hx, 46
  add             _r_t2, _r_t1, d.{csum_offset_or_mss}.hx
  phvwri          p.eth_tx_app_hdr_gso_valid, 1
  b               eth_tx_opcode_done
  phvwrpair       p.eth_tx_app_hdr_gso_start, _r_t1, p.eth_tx_app_hdr_gso_offset, _r_t2
eth_tx_calc_csum_tcpudp:
  // SET_STAT(_r_stats, _C_TRUE, desc_opcode_csum_hw)
  // SET_STAT(_r_stats, _C_TRUE, oper_csum_hw)
  seq             c7, d.encap, 1
  // SET_STAT(_r_stats, c7, oper_csum_hw_inner)
  phvwrpair       p.eth_tx_app_hdr_compute_l4_csum, d.csum_l4_or_eot, p.eth_tx_app_hdr_compute_ip_csum, d.csum_l3_or_sot
  b               eth_tx_opcode_done
  phvwrpair.c7    p.eth_tx_app_hdr_compute_inner_l4_csum, d.csum_l4_or_eot, p.eth_tx_app_hdr_compute_inner_ip_csum, d.csum_l3_or_sot
eth_tx_opcode_tso:
  // SET_STAT(_r_stats, _C_TRUE, desc_opcode_tso)
  phvwri          p.eth_tx_t0_s2s_do_tso, 1
  phvwri          p.eth_tx_app_hdr_tso_valid, 1
  phvwr           p.{eth_tx_global_tso_eot,eth_tx_global_tso_sot}, d.{csum_l4_or_eot,csum_l3_or_sot}
  phvwr           p.{eth_tx_app_hdr_tso_last_segment,eth_tx_app_hdr_tso_first_segment}, d.{csum_l4_or_eot,csum_l3_or_sot}
  bbeq            d.csum_l3_or_sot, 1, eth_tx_opcode_tso_sot
  phvwri          p.{eth_tx_app_hdr_update_tcp_seq_no...eth_tx_app_hdr_update_ip_id}, 0x7
  bbne            d.csum_l4_or_eot, 1, eth_tx_opcode_tso_cont
  nop
eth_tx_opcode_tso_eot:
  // SET_STAT(_r_stats, _C_TRUE, oper_tso_eot)
eth_tx_opcode_tso_cont:
  b               eth_tx_opcode_tso_done
  phvwr           p.eth_tx_to_s2_tso_hdr_addr[13:0], d.{csum_offset_or_mss}.hx
eth_tx_opcode_tso_sot:
  // SET_STAT(_r_stats, _C_TRUE, oper_tso_sot)
  BUF_ADDR_FROM_DATA(_r_t1)
  add             _r_t2, r0, d.{csum_start_or_hdr_len}.hx
  b               eth_tx_opcode_tso_done
  phvwrpair       p.eth_tx_to_s2_tso_hdr_addr, _r_t1, p.eth_tx_to_s2_tso_hdr_len, _r_t2
eth_tx_opcode_tso_done:
  // SET_STAT(_r_stats, _C_TRUE, oper_csum_hw)
  seq             c7, d.encap, 1
  // SET_STAT(_r_stats, c7, oper_csum_hw_inner)
  phvwrpair       p.eth_tx_app_hdr_compute_l4_csum, 1, p.eth_tx_app_hdr_compute_ip_csum, 1
  b               eth_tx_opcode_done
  phvwrpair.c7    p.eth_tx_app_hdr_compute_inner_l4_csum, 1, p.eth_tx_app_hdr_compute_inner_ip_csum, 1
eth_tx_opcode_done:

  // Save descriptor in PHV
  phvwr           p.to_stage_3_to_stage_data, d[511:384]

  // Set number of bytes to tx (for rate limiter)
  phvwr           p.p4_intr_packet_len, d.{len}.hx

  // Launch tso commit stage if tso
  b.!_c_tso       eth_tx_prep_done_no_tso
  add._c_tso      r7, k.eth_tx_to_s1_qstate_addr, 1, LG2_TX_QSTATE_SIZE
  phvwri          p.app_header_table1_valid, 1
  phvwri          p.common_te1_phv_table_lock_en, 1
  phvwrpair       p.common_te1_phv_table_raw_table_size, LG2_TX_QSTATE_SIZE, p.common_te1_phv_table_addr, r7
  phvwri          p.common_te1_phv_table_pc, eth_tx_commit_tso[38:6]

eth_tx_prep_done_no_tso:
  // Set number of sg elements to process
  sne             _c_sg, d.num_sg_elems, 0
  phvwr._c_sg     p.eth_tx_t0_s2s_do_sg, 1
  phvwr._c_sg     p.eth_tx_global_num_sg_elems, d.num_sg_elems
  // SET_STAT(_r_stats, _c_sg, oper_sg)

  phvwri._c_cq    p.eth_tx_global_do_cq, 1

  // SAVE_STATS(_r_stats)

  // Launch commit stage
  phvwri          p.app_header_table0_valid, 1
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, LG2_TX_QSTATE_SIZE, p.common_te0_phv_table_addr, k.eth_tx_to_s1_qstate_addr
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_commit[38:6]

eth_tx_prep_error:
  phvwri          p.eth_tx_global_stats[STAT_desc_fetch_error], 1

  // Don't drop the phv, because, we have claimed a descriptor.
  // Generate an error completion.
  phvwr           p.cq_desc_status, ETH_TX_DESC_ADDR_ERROR
  phvwr           p.eth_tx_global_do_cq, 1
  phvwr           p.eth_tx_global_drop, 1     // increment pkt drop counters

  // Launch commit stage
  phvwri          p.common_te0_phv_table_lock_en, 1
  phvwrpair.e     p.common_te0_phv_table_raw_table_size, LG2_TX_QSTATE_SIZE, p.common_te0_phv_table_addr, k.eth_tx_to_s1_qstate_addr
  phvwri.f        p.common_te0_phv_table_pc, eth_tx_commit[38:6]
