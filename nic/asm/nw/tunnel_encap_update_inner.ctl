#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tunnel_encap_update_inner_k k;
struct phv_                        p;

k = {
  ipv4_version = 4;
  ipv4_srcAddr = 11.12.14.15;
  ipv4_dstAddr = 10.1.1.1;
  ipv4_totalLen = 0x256;
  udp_srcPort = 0xbaba;
  udp_dstPort = 0xdada;
  udp_checksum = 0x4545;
  ipv4_valid = 1;
  udp_valid = 1;
  
  ipv6_version = 0x6;
  ipv6_trafficClass_sbit0_ebit3 = 0xa;
  ipv6_trafficClass_sbit4_ebit7 = 0xb;
  ipv6_flowLabel_sbit0_ebit3 = 0xa;
  ipv6_flowLabel_sbit4_ebit19 = 0xbcde;
  ipv6_payloadLen = 0x12;
  ipv6_nextHdr = 0x34;
  ipv6_hopLimit = 0x56;

  ipv6_srcAddr_sbit0_ebit7 = 0xa1;
  ipv6_srcAddr_sbit8_ebit15 = 0xa2;
  ipv6_srcAddr_sbit16_ebit31 = 0xa3a4;
  ipv6_srcAddr_sbit32_ebit63 = 0xa5a6a7a8;
  ipv6_srcAddr_sbit64_ebit95 = 0xa9aaabac;
  ipv6_srcAddr_sbit96_ebit127 = 0xadaeaf10;
  ipv6_dstAddr_sbit0_ebit7 = 0x01;
  ipv6_dstAddr_sbit8_ebit127 = 0x02030405060708090a0b0c0d0e0f10;
};
