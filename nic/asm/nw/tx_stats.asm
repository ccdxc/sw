#include "egress.h"
#include "EGRESS_p.h"

#define TX_UCAST_BYTES_OVERFLOW_ADDRESS  0xabcd0000
#define TX_MCAST_BYTES_OVERFLOW_ADDRESS  TX_UCAST_BYTES_OVERFLOW_ADDRESS + 16
#define TX_BCAST_BYTES_OVERFLOW_ADDRESS  TX_UCAST_BYTES_OVERFLOW_ADDRESS + 32
#define TX_EGRESS_DROPS_OVERFLOW_ADDRESS TX_UCAST_BYTES_OVERFLOW_ADDRESS + 46

struct tx_stats_k k;
struct tx_stats_d d;
struct phv_       p;

%%

tx_stats:
  seq         c2, k.capri_intrinsic_drop, 1
  bcf         [c2], tx_egress_drops
  add         r3, r0, k.flow_lkp_metadata_pkt_type
  .brbegin
  br          r3[1:0]
  mul         r4, k.control_metadata_src_lif, 7, 3
  .brcase 0
  add         r6, d.tx_stats_d.tx_ucast_bytes, k.control_metadata_packet_len
  bgei        r6, 0x3FFF, tx_ucast_bytes_overflow
  tbladd      d.tx_stats_d.tx_ucast_pkts, 1
  tblwr.e     d.tx_stats_d.tx_ucast_bytes, r6
  nop
  .brcase 1
  add         r6, d.tx_stats_d.tx_mcast_bytes, k.control_metadata_packet_len
  bgei        r6, 0x3FFF, tx_mcast_bytes_overflow
  tbladd      d.tx_stats_d.tx_mcast_pkts, 1
  tblwr.e     d.tx_stats_d.tx_mcast_bytes, r6
  nop
  .brcase 2
  add         r6, d.tx_stats_d.tx_bcast_bytes, k.control_metadata_packet_len
  bgei        r6, 0x3FFF, tx_bcast_bytes_overflow
  tbladd      d.tx_stats_d.tx_bcast_pkts, 1
  tblwr.e     d.tx_stats_d.tx_bcast_bytes, r6
  nop
  .brcase 3
  nop.e
  nop
  .brend

tx_egress_drops:
  add         r6, d.tx_stats_d.tx_egress_drops, 1
  beqi        r6, 0xFFFF, tx_egress_drops_overflow
  mul         r4, k.control_metadata_src_lif, 7, 3
  tblwr.e     d.tx_stats_d.tx_egress_drops, r6
  nop

tx_ucast_bytes_overflow:
  addi        r5, r4, TX_UCAST_BYTES_OVERFLOW_ADDRESS
  memwr.d     r5, r6
  add         r5, r5, 8
  memwr.d     r5, d.tx_stats_d.tx_ucast_pkts
  tblwr.e     d.tx_stats_d.tx_ucast_bytes, r0
  tblwr       d.tx_stats_d.tx_ucast_pkts, r0

tx_mcast_bytes_overflow:
  addi        r5, r4, TX_MCAST_BYTES_OVERFLOW_ADDRESS
  memwr.d     r5, r6
  add         r5, r5, 8
  memwr.d     r5, d.tx_stats_d.tx_mcast_pkts
  tblwr.e     d.tx_stats_d.tx_mcast_bytes, r0
  tblwr       d.tx_stats_d.tx_mcast_pkts, r0

tx_bcast_bytes_overflow:
  addi        r5, r4, TX_BCAST_BYTES_OVERFLOW_ADDRESS
  memwr.d     r5, r6
  add         r5, r5, 8
  memwr.d     r5, d.tx_stats_d.tx_bcast_pkts
  tblwr.e     d.tx_stats_d.tx_bcast_bytes, r0
  tblwr       d.tx_stats_d.tx_bcast_pkts, r0

tx_egress_drops_overflow:
  addi        r5, r4, TX_EGRESS_DROPS_OVERFLOW_ADDRESS
  memwr.d.e   r5, r6
  tblwr       d.tx_stats_d.tx_egress_drops, r0

