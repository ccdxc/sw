#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_flow_hash_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct flow_hash_k_ k;
struct flow_hash_d  d;
struct phv_         p;

%%

flow_hash_info:
  // bbne        k.control_metadata_nic_mode, NIC_MODE_SMART, flow_hash_exit
  phvwr       p.rewrite_metadata_entropy_hash, r1
  or          r7, k.flow_lkp_metadata_lkp_type, k.flow_lkp_metadata_lkp_inst, CPU_LKP_FLAGS_LKP_INST
  or          r7, r7, k.flow_lkp_metadata_lkp_dir, CPU_LKP_FLAGS_LKP_DIR
  phvwr       p.control_metadata_lkp_flags_egress, r7
  seq         c2, d.flow_hash_info_d.entry_valid, 1
  bcf         [c1&c2], flow_hash_hit
  seq         c1, r1[31:21], d.flow_hash_info_d.hash1
  // or          r7,  d.flow_hash_info_d.hint1_sbit4_ebit17, \
  //               d.flow_hash_info_d.hint1_sbit0_ebit3, 14
  // or          r7, d.flow_hash_info_d.hash1_sbit7_ebit10, \
  //                  d.flow_hash_info_d.hash1_sbit0_ebit6, 4
  // seq         c1, r1[31:21], r7
  sne         c3, d.flow_hash_info_d.hint1, r0
  // or          r7,  d.flow_hash_info_d.hint1_sbit4_ebit17, \
  //               d.flow_hash_info_d.hint1_sbit0_ebit3, 14
  // or          r7, d.flow_hash_info_d.hash1_sbit7_ebit10, \
  //                  d.flow_hash_info_d.hash1_sbit0_ebit6, 4
  // seq         c1, r1[31:21], r7
  // sne         c3, d.flow_hash_info_d.hint1, r0
  // sne         c3, r7, r0
  bcf         [c1&c2&c3], flow_hash_hint1
  seq         c1, r1[31:21], d.flow_hash_info_d.hash2
  sne         c3, d.flow_hash_info_d.hint2, r0
  bcf         [c1&c2&c3], flow_hash_hint2
  seq         c1, r1[31:21], d.flow_hash_info_d.hash3
  sne         c3, d.flow_hash_info_d.hint3, r0
  bcf         [c1&c2&c3], flow_hash_hint3
  seq         c1, r1[31:21], d.flow_hash_info_d.hash4
  sne         c3, d.flow_hash_info_d.hint4, r0
  bcf         [c1&c2&c3], flow_hash_hint4
  seq         c1, r1[31:21], d.flow_hash_info_d.hash5
  sne         c3, d.flow_hash_info_d.hint5, r0
  bcf         [c1&c2&c3], flow_hash_hint5
  seq         c1, d.flow_hash_info_d.more_hashes, 1
  bcf         [c1&c2], flow_hash_more_hashes
  phvwr       p.recirc_header_valid, 0
  phvwr       p.control_metadata_flow_miss_ingress, 1
  phvwr.e     p.control_metadata_i2e_flags[P4_I2E_FLAGS_FLOW_MISS], 1
  phvwr.f     p.flow_info_metadata_flow_index, 0

flow_hash_hit:
  phvwr.e     p.recirc_header_valid, 0
  phvwr.f     p.flow_info_metadata_flow_index, d.flow_hash_info_d.flow_index

flow_hash_hint1:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint1

flow_hash_hint2:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint2

flow_hash_hint3:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint3

flow_hash_hint4:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint4

flow_hash_hint5:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint5

flow_hash_more_hashes:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.more_hints

flow_hash_recirc:
  .assert(offsetof(p, tcp_option_eol_valid) - offsetof(p, tcp_option_mss_valid) == 11)
  phvwr         p.{tcp_option_eol_valid...tcp_option_mss_valid}, r0

  // recirc_header_valid (TRUE), capri_i2e_metadata_valid (FALSE),
  // capri_p4_intrinsic_valid, (TRUE)
  phvwr         p.{recirc_header_valid, \
                    capri_i2e_metadata_valid, \
                    capri_p4_intrinsic_valid}, 0x5
  phvwrpair     p.recirc_header_src_tm_iport, k.control_metadata_tm_iport[3:0], \
                    p.recirc_header_reason, RECIRC_FLOW_HASH_OVERFLOW
  ori           r2, r2, 0x80000000
  phvwrpair.e   p.control_metadata_ingress_bypass, 1, \
                    p.recirc_header_overflow_entry_index, r2
  phvwr.f       p.capri_intrinsic_tm_iq, k.qos_metadata_qos_class_id[4:0]

flow_hash_exit:
  nop.e
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
flow_hash_error:
  nop.e
  nop
