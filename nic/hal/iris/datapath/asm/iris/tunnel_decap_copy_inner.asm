#include "egress.h"
#include "EGRESS_p.h"
#include "EGRESS_tunnel_decap_copy_inner_k.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct tunnel_decap_copy_inner_k_ k;
struct tunnel_decap_copy_inner_d  d;
struct phv_                       p;

%%

nop:
  nop.e
  nop

.align
copy_inner_ipv4_udp:
  phvwr       p.{ipv4_version...ipv4_fragOffset}, \
                    k.{inner_ipv4_version...inner_ipv4_fragOffset}
  phvwr       p.{ipv4_ttl...ipv4_dstAddr}, k.{inner_ipv4_ttl...inner_ipv4_dstAddr}
  phvwr       p.{udp_srcPort,udp_dstPort,udp_len,udp_checksum}, \
                k.{inner_udp_srcPort,inner_udp_dstPort,inner_udp_len,inner_udp_checksum}
  phvwr       p.ipv4_valid, TRUE
  phvwr       p.ipv4_options_blob2_valid, k.inner_ipv4_options_blob_valid
  phvwr       p.udp_valid, TRUE
  phvwr.e     p.{inner_ipv4_options_blob_valid,inner_ipv4_valid}, 0
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_ipv4_other:
  phvwr       p.{ipv4_version...ipv4_fragOffset}, \
                    k.{inner_ipv4_version...inner_ipv4_fragOffset}
  phvwr       p.{ipv4_ttl...ipv4_dstAddr}, k.{inner_ipv4_ttl...inner_ipv4_dstAddr}
  phvwr       p.ipv4_valid, TRUE
  phvwr       p.ipv4_options_blob2_valid, k.inner_ipv4_options_blob_valid
  phvwr.e     p.{inner_ipv4_options_blob_valid,inner_ipv4_valid}, 0
  phvwr       p.udp_valid, FALSE

.align
copy_inner_ipv6_udp:
  phvwr       p.{ipv6_version...ipv6_hopLimit}, k.{inner_ipv6_version...inner_ipv6_hopLimit}
  phvwr       p.{ipv6_srcAddr, ipv6_dstAddr[127:8]}, \
                k.{inner_ipv6_srcAddr, inner_ipv6_dstAddr_s0_e119}
  phvwr       p.{ipv6_dstAddr[7:0]}, k.{inner_ipv6_dstAddr_s120_e127}
  phvwr       p.{udp_srcPort,udp_dstPort,udp_len,udp_checksum}, \
                k.{inner_udp_srcPort,inner_udp_dstPort,inner_udp_len,inner_udp_checksum}
  phvwr       p.ipv6_valid, TRUE
  phvwr       p.udp_valid, TRUE
  phvwr       p.ipv4_valid, FALSE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_ipv6_other:
  phvwr       p.{ipv6_version...ipv6_hopLimit}, k.{inner_ipv6_version...inner_ipv6_hopLimit}
  phvwr       p.{ipv6_srcAddr, ipv6_dstAddr[127:8]}, \
                k.{inner_ipv6_srcAddr, inner_ipv6_dstAddr_s0_e119}
  phvwr       p.{ipv6_dstAddr[7:0]}, k.{inner_ipv6_dstAddr_s120_e127}
  phvwr       p.ipv6_valid, TRUE
  phvwr       p.ipv4_valid, FALSE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.udp_valid, FALSE

.align
copy_inner_eth_ipv4_udp:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv4_version...ipv4_fragOffset}, \
                    k.{inner_ipv4_version...inner_ipv4_fragOffset}
  phvwr       p.{ipv4_ttl...ipv4_dstAddr}, k.{inner_ipv4_ttl...inner_ipv4_dstAddr}
  phvwr       p.{udp_srcPort,udp_dstPort,udp_len,udp_checksum}, \
                k.{inner_udp_srcPort,inner_udp_dstPort,inner_udp_len,inner_udp_checksum}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.ipv4_valid, TRUE
  phvwr       p.ipv4_options_blob2_valid, k.inner_ipv4_options_blob_valid
  phvwr       p.udp_valid, TRUE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr.e     p.{inner_ipv4_options_blob_valid,inner_ipv4_valid}, 0
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_eth_ipv4_other:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv4_version...ipv4_fragOffset}, \
                    k.{inner_ipv4_version...inner_ipv4_fragOffset}
  phvwr       p.{ipv4_ttl...ipv4_dstAddr}, k.{inner_ipv4_ttl...inner_ipv4_dstAddr}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.ipv4_valid, TRUE
  phvwr       p.ipv4_options_blob2_valid, k.inner_ipv4_options_blob_valid
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr.e     p.{inner_ipv4_options_blob_valid,inner_ipv4_valid}, 0
  phvwr       p.udp_valid, FALSE

.align
copy_inner_eth_ipv6_udp:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv6_version...ipv6_hopLimit}, k.{inner_ipv6_version...inner_ipv6_hopLimit}
  phvwr       p.{ipv6_srcAddr, ipv6_dstAddr[127:8]}, \
                k.{inner_ipv6_srcAddr, inner_ipv6_dstAddr_s0_e119}
  phvwr       p.{ipv6_dstAddr[7:0]}, k.{inner_ipv6_dstAddr_s120_e127}
  phvwr       p.{udp_srcPort,udp_dstPort,udp_len,udp_checksum}, \
                k.{inner_udp_srcPort,inner_udp_dstPort,inner_udp_len,inner_udp_checksum}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.ipv6_valid, TRUE
  phvwr       p.udp_valid, TRUE
  phvwr       p.ipv4_valid, FALSE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.inner_udp_valid, FALSE

.align
copy_inner_eth_ipv6_other:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.{ipv6_version...ipv6_hopLimit}, k.{inner_ipv6_version...inner_ipv6_hopLimit}
  phvwr       p.{ipv6_srcAddr...ipv6_dstAddr[127:8]}, \
                    k.{inner_ipv6_srcAddr, inner_ipv6_dstAddr_s0_e119}
  phvwr       p.{ipv6_dstAddr[7:0]}, k.{inner_ipv6_dstAddr_s120_e127}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.ipv6_valid, TRUE
  phvwr       p.ipv4_valid, FALSE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.vlan_tag_valid, FALSE
  phvwr.e     p.inner_ipv6_valid, FALSE
  phvwr       p.udp_valid, FALSE

.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
copy_inner_eth_non_ip:
  phvwr       p.ethernet_dstAddr, k.inner_ethernet_dstAddr
  phvwr       p.{ethernet_srcAddr, ethernet_etherType}, k.{inner_ethernet_srcAddr, inner_ethernet_etherType}
  phvwr       p.ethernet_valid, TRUE
  phvwr       p.inner_ethernet_valid, FALSE
  phvwr       p.ipv4_valid, FALSE
  phvwr       p.ipv6_valid, FALSE
  phvwr.e     p.vlan_tag_valid, FALSE
  phvwr       p.udp_valid, FALSE

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
tunnel_decap_copy_inner_error:
  nop.e
  nop
