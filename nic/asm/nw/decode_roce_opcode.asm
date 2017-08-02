#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

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
  nop.!c1.e
  phvwr.c1    p.p4_to_p4plus_roce_valid, 1
  phvwr       p.p4_to_p4plus_roce_opcode, k.roce_bth_opCode
  phvwr       p.p4_to_p4plus_roce_raw_flags, d.u.decode_roce_opcode_d.raw_flags
  phvwr       p.p4_to_p4plus_roce_rdma_hdr_len, d.u.decode_roce_opcode_d.len
  phvwr       p.capri_rxdma_intrinsic_qid, k.roce_bth_destQP
  phvwr       p.capri_rxdma_intrinsic_qtype, d.u.decode_roce_opcode_d.qtype
  add         r1, d.u.decode_roce_opcode_d.len, (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + \
                                                 CAPRI_RXDMA_INTRINSIC_HDR_SZ + \
                                                 P4PLUS_ROCE_HDR_SZ)
  phvwr       p.capri_rxdma_intrinsic_rx_splitter_offset, r1
  smeqb       c1, k.roce_bth_opCode, 0xE0, 0x60
  phvwr.c1    p.p4_to_p4plus_roce_ip_valid, 1
  phvwr       p.ipv4_valid, 0
  phvwr.e     p.ipv6_valid, 0
  phvwr       p.udp_valid, 0
