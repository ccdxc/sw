#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <linux/ipv6.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <net/ethernet.h>
#include <net/route.h>
#include <zmq.h>
#include <assert.h>
#include <sys/stat.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include "nic/e2etests/lib/helpers.hpp"
#include "nic/e2etests/lib/packet.hpp"


long checksum(unsigned short *addr, unsigned int count) {
  /* Compute Internet Checksum for "count" bytes
   *         beginning at location "addr".
   */
  register long sum = 0;


  while( count > 1 )  {
    /*  This is the inner loop */
    sum += * addr++;
    if(sum & 0x80000000)   /* if high order bit set, fold */
      sum = (sum & 0xFFFF) + (sum >> 16);
    count -= 2;
  }
  /*  Add left-over byte, if any */
  if( count > 0 )
    sum += * (unsigned char *) addr;

  /*  Fold 32-bit sum to 16 bits */
  while (sum>>16)
    sum = (sum & 0xffff) + (sum >> 16);

  return ~sum;
}

long get_tcp_checksumv6(struct ipv6_header_t * myip, struct tcp_header_t * mytcp)
{
  uint16_t total_len = htons(myip->payload_len);
  uint16_t cksum = 0;
  struct tcp6_pseudohdr pseudoheadv6 = {0};
  int tcpopt_len = mytcp->doff*4 - 20;
  int tcpdatalen = total_len - (mytcp->doff*4);

  memcpy(pseudoheadv6.saddr.in6_u.u6_addr8, myip->saddr.in6_u.u6_addr8, sizeof(struct in6_addr));
  memcpy(pseudoheadv6.daddr.in6_u.u6_addr8, myip->daddr.in6_u.u6_addr8, sizeof(struct in6_addr));
  pseudoheadv6.protocol=htonl(IPPROTO_TCP);
  pseudoheadv6.len=htonl(sizeof(struct tcp_header_t) + tcpopt_len + tcpdatalen);

  int totaltcp_len = sizeof(struct tcp6_pseudohdr) + sizeof(struct tcp_header_t) + tcpopt_len + tcpdatalen;
  unsigned short * tcp = (unsigned short*) malloc(sizeof(unsigned short) * (totaltcp_len));
  memcpy((unsigned char *)tcp,&pseudoheadv6,sizeof(struct tcp6_pseudohdr));
  memcpy((unsigned char *)tcp+sizeof(struct tcp6_pseudohdr),(unsigned char *)mytcp,sizeof(struct tcp_header_t));
  memcpy((unsigned char *)tcp+sizeof(struct tcp6_pseudohdr)+sizeof(struct tcp_header_t), (unsigned char *)myip+(40)+(sizeof(struct tcp_header_t)), tcpopt_len);
  memcpy((unsigned char *)tcp+sizeof(struct tcp6_pseudohdr)+sizeof(struct tcp_header_t)+tcpopt_len, (unsigned char *)mytcp+(mytcp->doff*4), tcpdatalen);

  cksum = (uint16_t)(checksum(tcp,totaltcp_len));
  //free(tcp);
  return cksum;
}

long get_tcp_checksum(struct ipv4_header_t * myip, struct tcp_header_t * mytcp) {

  uint16_t total_len = ntohs(myip->tot_len);
  struct tcp_pseudo pseudohead = { 0 };

  int tcpopt_len = mytcp->doff*4 - 20;
  int tcpdatalen = total_len - (mytcp->doff*4) - (myip->ihl*4);

  pseudohead.src_addr=myip->saddr;
  pseudohead.dst_addr=myip->daddr;
  pseudohead.zero=0;
  pseudohead.proto=IPPROTO_TCP;
  pseudohead.length=htons(sizeof(struct tcp_header_t) + tcpopt_len + tcpdatalen);

  int totaltcp_len = sizeof(struct tcp_pseudo) + sizeof(struct tcp_header_t) + tcpopt_len + tcpdatalen;
  unsigned short * tcp = new unsigned short[totaltcp_len];


  memcpy((unsigned char *)tcp,&pseudohead,sizeof(struct tcp_pseudo));
  memcpy((unsigned char *)tcp+sizeof(struct tcp_pseudo),(unsigned char *)mytcp,sizeof(struct tcp_header_t));
  memcpy((unsigned char *)tcp+sizeof(struct tcp_pseudo)+sizeof(struct tcp_header_t), (unsigned char *)myip+(myip->ihl*4)+(sizeof(struct tcp_header_t)), tcpopt_len);
  memcpy((unsigned char *)tcp+sizeof(struct tcp_pseudo)+sizeof(struct tcp_header_t)+tcpopt_len, (unsigned char *)mytcp+(mytcp->doff*4), tcpdatalen);


  return checksum(tcp,totaltcp_len);

}

int
dump_pkt(char *pkt, int len, uint32_t hntap_port)
{
  int i;
  for (i = 0; i< len; i++) {
    if (i % 16 == 0) {
      printf("\n");
    }
    printf(" 0x%02x", (unsigned char)pkt[i]);
  }
  printf("\n");

  struct ether_header_t *eth;
  struct vlan_header_t *vlan;
  struct ipv4_header_t *ip;
  struct tcp_header_t *tcp;
  uint16_t etype;
  eth = (struct ether_header_t *)pkt;
  if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
    vlan = (struct vlan_header_t*)pkt;
    TLOG(" ETH-VLAN: DMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x SMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x vlan=%d proto=0x%x\n",
           vlan->dmac[0], vlan->dmac[1], vlan->dmac[2], vlan->dmac[3], vlan->dmac[4], vlan->dmac[5],
           vlan->smac[0], vlan->smac[1], vlan->smac[2], vlan->smac[3], vlan->smac[4], vlan->smac[5],
           ntohs(vlan->vlan_tag),
           ntohs(vlan->etype));
    etype = ntohs(vlan->etype);
    ip = (ipv4_header_t *)(vlan+1);
  } else {
    TLOG(" ETH: DMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x SMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x proto=0x%x\n",
           eth->dmac[0], eth->dmac[1], eth->dmac[2], eth->dmac[3], eth->dmac[4], eth->dmac[5],
           eth->smac[0], eth->smac[1], eth->smac[2], eth->smac[3], eth->smac[4], eth->smac[5],
           ntohs(eth->etype));
    etype = ntohs(eth->etype);
    ip = (ipv4_header_t *)(eth+1);
  }

  if (etype == ETHERTYPE_IP) {

    TLOG(" IP: tos=%d tot_len=%d id=0x%x frag_off=0x%x ttl=%d protocol=%d check=0x%x saddr=0x%x daddr=0x%x\n",
            ip->tos, ntohs(ip->tot_len), ntohs(ip->id), ntohs(ip->frag_off),
            ip->ttl, ip->protocol, ntohs(ip->check), ntohl(ip->saddr), ntohl(ip->daddr));

    if (ip->protocol == IPPROTO_TCP) {
      tcp = (struct tcp_header_t*)(ip+1);
      TLOG(" TCP: sp=0x%x dp=0x%x seq=0x%x ack_seq=0x%x doff=%d res1=%d %s%s%s%s%s%s%s%s wnd=0x%x check=0x%x urg_ptr=0x%x\n",
              ntohs(tcp->sport), ntohs(tcp->dport), ntohl(tcp->seq), ntohl(tcp->ack_seq),
              tcp->doff, tcp->res1,
              tcp->fin ? "F" : "",
              tcp->syn ? "S" : "",
              tcp->rst ? "R" : "",
              tcp->psh ? "P" : "",
              tcp->ack ? "A" : "",
              tcp->urg ? "U" : "",
              tcp->ece ? "E" : "",
              tcp->cwr ? "C" : "",
              ntohs(tcp->window),
              ntohs(tcp->check),
              ntohs(tcp->urg_ptr));

        if (hntap_port && (hntap_port != ntohs(tcp->sport) && hntap_port != ntohs(tcp->dport))) {
          TLOG("TCP port mismatch %d %d %d\n", hntap_port, ntohs(tcp->sport), ntohs(tcp->dport));
                      return(-1);
       }
    } //else if (!hntap_allow_udp || (ip->protocol != IPPROTO_UDP)) return -1;
  }
  return 0;
}

static char buf[64];
static char* print_v6_addr(void* addr)
{
    inet_ntop(AF_INET6, addr, buf, sizeof(buf));
    return buf;
}

int
dump_pkt6(char *pkt, int len, uint32_t hntap_port)
{
  int i;
  for (i = 0; i< len; i++) {
    if (i % 16 == 0) {
      printf("\n");
    }
    printf(" 0x%02x", (unsigned char)pkt[i]);
  }
  printf("\n");

  struct ether_header_t *eth;
  struct vlan_header_t *vlan;
  struct ipv6_header_t *ip;
  struct tcp_header_t *tcp;
  uint16_t etype;
  eth = (struct ether_header_t *)pkt;
  if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
    vlan = (struct vlan_header_t*)pkt;
    TLOG(" ETH-VLAN: DMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x SMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x vlan=%d proto=0x%x\n",
           vlan->dmac[0], vlan->dmac[1], vlan->dmac[2], vlan->dmac[3], vlan->dmac[4], vlan->dmac[5],
           vlan->smac[0], vlan->smac[1], vlan->smac[2], vlan->smac[3], vlan->smac[4], vlan->smac[5],
           ntohs(vlan->vlan_tag),
           ntohs(vlan->etype));
    etype = ntohs(vlan->etype);
    ip = (ipv6_header_t *)(vlan+1);
  } else {
    TLOG(" ETH: DMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x SMAC=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x proto=0x%x\n",
           eth->dmac[0], eth->dmac[1], eth->dmac[2], eth->dmac[3], eth->dmac[4], eth->dmac[5],
           eth->smac[0], eth->smac[1], eth->smac[2], eth->smac[3], eth->smac[4], eth->smac[5],
           ntohs(eth->etype));
    etype = ntohs(eth->etype);
    ip = (ipv6_header_t *)(eth+1);
  }

  if (etype == ETHERTYPE_IPV6) {
    TLOG(" IP: tot_len=%d protocol=%d",
            ntohs(ip->payload_len), ip->nexthdr);
    TLOG(" saddr=%s", print_v6_addr(ip->saddr.s6_addr));
    TLOG(" daddr=%s\n", print_v6_addr(ip->daddr.s6_addr));

    if (ip->nexthdr == IPPROTO_TCP) {
      tcp = (struct tcp_header_t*)(ip+1);
      TLOG(" TCP: sp=0x%x dp=0x%x seq=0x%x ack_seq=0x%x doff=%d res1=%d %s%s%s%s%s%s%s%s wnd=0x%x check=0x%x urg_ptr=0x%x\n",
              ntohs(tcp->sport), ntohs(tcp->dport), ntohl(tcp->seq), ntohl(tcp->ack_seq),
              tcp->doff, tcp->res1,
              tcp->fin ? "F" : "",
              tcp->syn ? "S" : "",
              tcp->rst ? "R" : "",
              tcp->psh ? "P" : "",
              tcp->ack ? "A" : "",
              tcp->urg ? "U" : "",
              tcp->ece ? "E" : "",
              tcp->cwr ? "C" : "",
              ntohs(tcp->window),
              ntohs(tcp->check),
              ntohs(tcp->urg_ptr));

        if (hntap_port && (hntap_port != ntohs(tcp->sport) && hntap_port != ntohs(tcp->dport))) {
          TLOG("TCP port mismatch %d %d %d\n", hntap_port, ntohs(tcp->sport), ntohs(tcp->dport));
                      return(-1);
       }
    } else {
 //else if (!hntap_allow_udp || (ip->protocol != IPPROTO_UDP)) return -1;
       return -1;
    }
  }
  return 0;
}

uint16_t
hntap_get_etype(struct ether_header_t *eth)
{
  struct vlan_header_t *vlan;
  if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
    vlan = (struct vlan_header_t *)eth;
    return ntohs(vlan->etype);
  }
  return ntohs(eth->etype);
}

