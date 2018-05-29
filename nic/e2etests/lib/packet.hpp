#ifndef LIB_PACKET_H
#define LIB_PACKET_H
#include "nic/include/pkt_hdrs.hpp"

struct tcp6_pseudohdr {
  struct      in6_addr  saddr;
  struct      in6_addr daddr;
  uint32_t    len;
  uint32_t    protocol;  /* including padding */
};

struct tcp_pseudo /*the tcp pseudo header*/
{
  uint32_t src_addr;
  uint32_t dst_addr;
  uint8_t zero;
  uint8_t proto;
  uint16_t length;
};


int dump_pkt(char *pkt, int len, uint32_t htnp_port = 0);
int dump_pkt6(char *pkt, int len, uint32_t htnp_port = 0);
long checksum(unsigned short *addr, unsigned int count);
long get_tcp_checksum(ipv4_header_t * myip, tcp_header_t * mytcp);
long get_tcp_checksumv6(ipv6_header_t * myip, tcp_header_t * mytcp);
uint16_t get_udp_checksum(const void *buff, size_t len, uint32_t src_addr, uint32_t dest_addr);
uint16_t get_tcp_checksum(const void *buff, size_t len, uint32_t src_addr, uint32_t dest_addr);
uint16_t hntap_get_etype(ether_header_t *eth);

#endif
