#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_validate_packet_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nw.h"

struct validate_packet_k_ k;
struct phv_               p;

%%

validate_packet:
  seq         c1, k.capri_p4_intrinsic_parser_err, TRUE
  balcf       r7, [c1], f_check_parser_errors
  seq         c1, k.tunnel_metadata_tunnel_terminate, TRUE
  bcf         [c1], validate_tunneled_packet

validate_native_packet:
  sne         c1, k.capri_intrinsic_tm_iport, TM_PORT_NCSI
  seq.c1      c1, k.ethernet_srcAddr, r0
  seq.!c1     c1, k.ethernet_dstAddr, r0
  seq.!c1     c1, k.ethernet_srcAddr, k.ethernet_dstAddr
  seq.!c1     c1, k.ethernet_srcAddr[40], 1
  sub         r1, r0, 1
  seq.!c1     c1, k.ethernet_srcAddr, r1[47:0]
  phvwr.c1    p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  nop.e
  phvwr.c1    p.capri_intrinsic_drop, 1

validate_tunneled_packet:
  seq         c1, k.ethernet_srcAddr, r0
  seq.!c1     c1, k.ethernet_dstAddr, r0
  seq.!c1     c1, k.ethernet_srcAddr, k.ethernet_dstAddr
  seq.!c1     c1, k.ethernet_srcAddr[40], 1
  bbne        k.inner_ethernet_valid, TRUE, validate_tunneled_packet_ip
  sub         r1, r0, 1
  seq.!c1     c1, k.ethernet_srcAddr, r1[47:0]
  seq.!c1     c1, k.inner_ethernet_srcAddr, r0
  seq.!c1     c1, k.inner_ethernet_dstAddr, r0
  seq.!c1     c1, k.inner_ethernet_srcAddr, k.inner_ethernet_dstAddr
  seq.!c1     c1, k.inner_ethernet_srcAddr[40], 1
  seq.!c1     c1, k.inner_ethernet_srcAddr, r1[47:0]
validate_tunneled_packet_ip:
  phvwr.c1    p.control_metadata_drop_reason[DROP_MALFORMED_PKT], 1
  nop.e
  phvwr.c1    p.capri_intrinsic_drop, 1

f_check_parser_errors:
  // do not use c1 register in this function
  bbeq        k.control_metadata_uplink, TRUE, check_parser_errors_uplink
  seq         c2, k.capri_p4_intrinsic_len_err, 0
  smneb.!c2   c2, k.{p4plus_to_p4_update_udp_len, p4plus_to_p4_update_tcp_seq_no, \
                     p4plus_to_p4_update_ip_len}, 0x5, 0
  jrcf        [c2], r7
  phvwr.!c2.e p.control_metadata_drop_reason[DROP_PARSER_LEN_ERR], 1
  phvwr       p.capri_intrinsic_drop, 1

check_parser_errors_uplink:
  phvwr       p.control_metadata_checksum_results, k.capri_intrinsic_csum_err
  seq         c3, k.capri_p4_intrinsic_crc_err, 0
  jrcf        [c2 & c3], r7
  phvwr.!c2   p.control_metadata_drop_reason[DROP_PARSER_LEN_ERR], 1
  phvwr.!c3   p.control_metadata_drop_reason[DROP_PARSER_ICRC_ERR], 1
  phvwr.e     p.capri_intrinsic_drop, 1
  nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
validate_packet_error:
  nop.e
  nop
