#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"
#include "../../include/capri_common.h"

#define TX_UCAST_BYTES_OVERFLOW_OFFSET  0
#define TX_MCAST_BYTES_OVERFLOW_OFFSET  (TX_UCAST_BYTES_OVERFLOW_OFFSET + 16)
#define TX_BCAST_BYTES_OVERFLOW_OFFSET  (TX_UCAST_BYTES_OVERFLOW_OFFSET + 32)
#define TX_EGRESS_DROPS_OVERFLOW_OFFSET (TX_UCAST_BYTES_OVERFLOW_OFFSET + 46)

struct tx_stats_k k;
struct tx_stats_d d;
struct phv_       p;

%%

tx_stats:
  bbeq        k.capri_intrinsic_drop, 1, tx_egress_drops
  add         r3, r0, k.flow_lkp_metadata_pkt_type
  .brbegin
  br          r3[1:0]
  add         r4, r0, k.control_metadata_src_lif, 6
  .brcase 0
  add         r7, d.tx_stats_d.tx_ucast_pkts, 1
  bgti        r7, 0xF, tx_ucast_overflow
  tblwr.e     d.tx_stats_d.tx_ucast_pkts, r7[3:0]
  tbladd.f    d.tx_stats_d.tx_ucast_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  .brcase 1
  add         r7, d.tx_stats_d.tx_mcast_pkts, 1
  bgti        r7, 0xF, tx_mcast_overflow
  tblwr.e     d.tx_stats_d.tx_mcast_pkts, r7[3:0]
  tbladd.f    d.tx_stats_d.tx_mcast_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  .brcase 2
  add         r7, d.tx_stats_d.tx_bcast_pkts, 1
  bgti        r7, 0xF, tx_bcast_overflow
  tblwr.e     d.tx_stats_d.tx_bcast_pkts, r7[3:0]
  tbladd.f    d.tx_stats_d.tx_bcast_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  .brcase 3
  nop.e
  nop
  .brend

tx_egress_drops:
  add         r6, d.tx_stats_d.tx_egress_drops, 1
  bgti        r6, 0xF, tx_egress_drops_overflow
  add.e       r4, r0, k.control_metadata_src_lif, 6
  tblwr.f     d.tx_stats_d.tx_egress_drops, r6

tx_ucast_overflow:
  add         r7, d.tx_stats_d.tx_ucast_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  addi        r6, r0, 0x100000F
  or          r7, r7, r6, 32
  or          r7, r7, r5[31:27], 58

  add         r5, r5, r4
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]

  memwr.dx.e  r6, r7
  tblwr.f     d.tx_stats_d.tx_ucast_bytes, r0

tx_mcast_overflow:
  add         r7, d.tx_stats_d.tx_mcast_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  addi        r6, r0, 0x100000F
  or          r7, r7, r6, 32
  or          r7, r7, r5[31:27], 58

  add         r5, r5, r4
  add         r5, r5, TX_MCAST_BYTES_OVERFLOW_OFFSET
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]

  memwr.dx.e  r6, r7
  tblwr.f     d.tx_stats_d.tx_mcast_bytes, r0

tx_bcast_overflow:
  add         r7, d.tx_stats_d.tx_bcast_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  addi        r6, r0, 0x100000F
  or          r7, r7, r6, 32
  or          r7, r7, r5[31:27], 58

  add         r5, r5, r4
  add         r5, r5, TX_BCAST_BYTES_OVERFLOW_OFFSET
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]

  memwr.dx.e  r6, r7
  tblwr.f     d.tx_stats_d.tx_mcast_bytes, r0

tx_egress_drops_overflow:
  or          r7, r7, r5[31:27], 58

  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r5, r5, r4
  addi        r5, r5, TX_EGRESS_DROPS_OVERFLOW_OFFSET
  add         r6, r6, r5[26:0]

  memwr.dx.e  r6, r7
  tblwr.f     d.tx_stats_d.tx_egress_drops, r0

/*
 * stats allocation in the atomic add region:
 * Unicast : 8B bytes, 8B packets, Multicast : 8B bytes, 8B packets,
 * Broadcast : 8B bytes, 8B packets, Drops : 8B, pad (to align) : 8B
 * total per index = 64B
 */

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tx_stats_error:
  phvwr.e     p.capri_intrinsic_drop, TRUE
  nop
