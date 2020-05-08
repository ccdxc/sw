#include "egress.h"
#include "EGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct decode_roce_opcode_k k;
struct decode_roce_opcode_d d;
struct phv_                 p;

%%

nop:
  nop.e
  nop

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
decode_roce_opcode:
  seq         c1, k.capri_intrinsic_tm_oport, TM_PORT_DMA
  seq.c1      c1, k.control_metadata_rdma_enabled, TRUE
  nop.!c1.e
  phvwr.c1    p.p4_to_p4plus_roce_valid, TRUE
  phvwr       p.p4_to_p4plus_roce_raw_flags, d.u.decode_roce_opcode_d.raw_flags
  phvwr       p.p4_to_p4plus_roce_rdma_hdr_len, d.u.decode_roce_opcode_d.len
  phvwr       p.control_metadata_p4plus_app_id, P4PLUS_APPTYPE_RDMA

  seq         c1, d.u.decode_roce_opcode_d.tm_oq_overwrite, TRUE
  phvwr.c1    p.capri_intrinsic_tm_oq, d.u.decode_roce_opcode_d.tm_oq

  phvwrpair   p.capri_intrinsic_payload, FALSE, p.capri_rxdma_intrinsic_valid, TRUE
  seq         c1, k.capri_intrinsic_tm_instance_type, TM_INSTANCE_TYPE_MULTICAST
  cmov        r1, c1, k.control_metadata_qid, k.roce_bth_destQP
  phvwr       p.capri_rxdma_intrinsic_qid, r1
  phvwr       p.capri_rxdma_intrinsic_qtype, d.u.decode_roce_opcode_d.qtype
  phvwr       p.{p4_to_p4plus_roce_table0_valid...p4_to_p4plus_roce_table1_valid}, 0x3
  add         r1, d.u.decode_roce_opcode_d.len, \
                  (ASICPD_GLOBAL_INTRINSIC_HDR_SZ + \
                   ASICPD_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_ROCE_HDR_SZ)

  smeqb       c1, k.roce_bth_opCode, 0xE0, 0x60

  // increment splitter offset to skip eth and vlan headers for UD RDMA
  add.c1      r1, r1, 18
  phvwr.c1    p.{p4_to_p4plus_roce_vlan_valid,p4_to_p4plus_roce_eth_valid}, 0x3
  phvwr.c1    p.control_metadata_rdma_ud, TRUE
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, r1

  // subtract udp header length 8 and icrc length 4
  sub         r1, k.udp_len, 12
  // add conditionally 40B for ip header length for UD RDMA
  // for IPv6, store 40B of IPv6 header
  // for IPv4, store 20B of hdr into last 20B, the first 20B are zeros
  add.c1      r1, r1, 40
  sub.e       r1, r1, d.u.decode_roce_opcode_d.len
  phvwr       p.p4_to_p4plus_roce_payload_len, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
decode_roce_opcode_error:
  nop.e
  nop
