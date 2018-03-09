#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct tunnel_decap_copy_inner_k k;
struct tunnel_decap_copy_inner_d d;
struct phv_                      p;

k = {
  inner_ipv4_version = 4;
  inner_ipv4_srcAddr = 192.168.100.1;
  inner_ipv4_dstAddr = 0x0a0b0c0d;
  inner_udp_srcPort = 0xbaba;
  inner_udp_checksum = 0xc0c0;
  inner_ipv6_valid = 1;
  inner_udp_valid = 1;
  inner_ethernet_valid = 1;

  inner_ipv6_version = 0x6;
  inner_ipv6_trafficClass_sbit0_ebit3 = 0xa;
  inner_ipv6_trafficClass_sbit4_ebit7 = 0xb;
  inner_ipv6_flowLabel_sbit0_ebit3 = 0xf;
  inner_ipv6_flowLabel_sbit4_ebit19 = 0xedcb;
  inner_ipv6_payloadLen = 0x12;
  inner_ipv6_nextHdr = 0x34;
  inner_ipv6_hopLimit = 0x56;

  inner_ipv6_srcAddr_sbit0_ebit7 = 0xa1;
  inner_ipv6_srcAddr_sbit8_ebit15 = 0xa2;
  inner_ipv6_srcAddr_sbit16_ebit31 = 0xa3a4;
  inner_ipv6_srcAddr_sbit32_ebit63 = 0xa5a6a7a8;
  inner_ipv6_srcAddr_sbit64_ebit95 = 0xa9aaabac;
  inner_ipv6_srcAddr_sbit96_ebit127 = 0xadaeaf10;
  inner_ipv6_dstAddr_sbit0_ebit119 = 0x02030405060708090a0b0c0d0e0f10;
  inner_ipv6_dstAddr_sbit120_ebit127 = 0x01;
};
