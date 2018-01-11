#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"
#include "../../include/capri_common.h"

struct rx_policer_action_k k;
struct rx_policer_action_d d;
struct phv_                    p;

%%

rx_policer_action:
  seq         c1, k.policer_metadata_rx_policer_color, POLICER_COLOR_GREEN
  bcf         [!c1], rx_policer_deny
  add         r7, d.rx_policer_action_d.permitted_packets, 1
  bgti        r7, 0xF, rx_policer_permitted_stats_overflow
  tblwr       d.rx_policer_action_d.permitted_packets, r6[3:0]
  tbladd.e    d.rx_policer_action_d.permitted_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  nop

rx_policer_permitted_stats_overflow:
  add         r7, d.rx_policer_action_d.permitted_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  addi        r6, r0, 0x100000F
  or          r7, r7, r6, 32
  or          r7, r7, r5[31:27], 58

  add         r5, r5, k.{capri_intrinsic_lif_sbit0_ebit2, capri_intrinsic_lif_sbit3_ebit10}, 5
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]

  memwr.dx.e  r6, r7
  tblwr       d.rx_policer_action_d.permitted_bytes, 0

rx_policer_deny:
  add         r7, d.rx_policer_action_d.denied_packets, 1
  bgti        r7, 0xF, rx_policer_denied_stats_overflow
  tblwr       d.rx_policer_action_d.denied_packets, r6[3:0]
  tbladd.e    d.rx_policer_action_d.denied_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  nop

rx_policer_denied_stats_overflow:
  add         r7, d.rx_policer_action_d.denied_bytes, \
                k.{capri_p4_intrinsic_packet_len_sbit0_ebit5, \
                   capri_p4_intrinsic_packet_len_sbit6_ebit13}
  addi        r6, r0, 0x100000F
  or          r7, r7, r6, 32
  or          r7, r7, r5[31:27], 58

  add         r5, r5, k.{capri_intrinsic_lif_sbit0_ebit2, capri_intrinsic_lif_sbit3_ebit10}, 5
  add         r5, r5, 16
  addi        r6, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
  add         r6, r6, r5[26:0]

  memwr.dx.e  r6, r7
  tblwr       d.rx_policer_action_d.denied_bytes, 0

/*
 * stats allocation in the atomic add region:
 * 8B permit bytes, 8B permit packets, 8B deny bytes, 8B deby packets
 * total per policer index = 32B
 */

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
rx_policer_action_error:
  nop.e
  nop
