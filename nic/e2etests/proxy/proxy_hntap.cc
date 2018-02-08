#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <net/ethernet.h>
#include <net/route.h>
#include <assert.h>
#include <sys/stat.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "nic/e2etests/lib/helpers.hpp"
#include "nic/e2etests/hntap/dev.hpp"

#define HNTAP_HOST_TAPIF        "hntap_host0"
#define HNTAP_HOST_TAPIF_IP     "64.1.0.4"
#define HNTAP_HOST_TAPIF_IPMASK "255.255.255.0"
#define HNTAP_HOST_ROUTE_DESTIP "64.0.0.1"
#define HNTAP_HOST_ROUTE_GWIP   "64.1.0.4"

#define HNTAP_NET_TAPIF         "hntap_net0"
#define HNTAP_NET_TAPIF_IP      "64.0.0.2"
#define HNTAP_NET_TAPIF_IPMASK  "255.255.255.0"
#define HNTAP_NET_ROUTE_DESTIP  "64.1.0.3"
#define HNTAP_NET_ROUTE_GWIP    "64.0.0.2"


extern uint32_t nw_retries;
extern uint16_t hntap_port;
extern bool hntap_drop_rexmit;


void
hntap_nat_worker(char *pkt, int len, bool source_ip, uint32_t orig_addr, uint32_t to_addr)
{

#if 0
  int i;
  for (i = 0; i< len; i++) {
    if (i % 16 == 0) {
      printf("\n");
    }
    printf(" 0x%02x", (unsigned char)pkt[i]);
  }
  printf("\n");
#endif

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
    TLOG(" IP BEFORE1: tos=%d tot_len=%d id=0x%x frag_off=0x%x ttl=%d protocol=%d check=0x%x saddr=0x%x daddr=0x%x\n",
            ip->tos, ntohs(ip->tot_len), ntohs(ip->id), ntohs(ip->frag_off),
            ip->ttl, ip->protocol, ntohs(ip->check), ntohl(ip->saddr), ntohl(ip->daddr));

    ip->check = 0;
    ip->check = checksum((unsigned short *)ip, 20);


    TLOG(" IP BEFORE2: tos=%d tot_len=%d id=0x%x frag_off=0x%x ttl=%d protocol=%d check=0x%x saddr=0x%x daddr=0x%x\n",
            ip->tos, ntohs(ip->tot_len), ntohs(ip->id), ntohs(ip->frag_off),
            ip->ttl, ip->protocol, ntohs(ip->check), ntohl(ip->saddr), ntohl(ip->daddr));

    if (ip->protocol == IPPROTO_TCP) {
      tcp = (struct tcp_header_t*)(ip+1);
      TLOG(" TCP BEFORE1: sp=0x%x dp=0x%x seq=0x%x ack_seq=0x%x doff=%d res1=%d %s%s%s%s%s%s%s%s wnd=0x%x check=0x%x urg_ptr=0x%x\n",
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
    }

    if (ip->protocol == IPPROTO_TCP) {
      tcp = (struct tcp_header_t*)(ip+1);
      tcp->check = 0;
      tcp->check = get_tcp_checksum(ip, tcp);

      TLOG(" TCP BEFORE2: sp=0x%x dp=0x%x seq=0x%x ack_seq=0x%x doff=%d res1=%d %s%s%s%s%s%s%s%s wnd=0x%x check=0x%x urg_ptr=0x%x\n",
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
    }

    if (source_ip) {
      if (ntohl(ip->saddr) == orig_addr) {
        TLOG("NAT IPSA: 0x%x -> 0x%x\n", orig_addr, to_addr);

        ip->saddr = htonl(to_addr);
        ip->check = 0;
        ip->check = checksum((unsigned short *)ip, 20);
      } else {
        TLOG("No NAT\n");
      }
    } else {

      if (ntohl(ip->daddr) == orig_addr) {
        TLOG("NAT IPDA: 0x%x -> 0x%x\n", orig_addr, to_addr);

        ip->daddr = htonl(to_addr);
        ip->check = 0;
        ip->check = checksum((unsigned short *)ip, 20);

      } else {
        TLOG("No NAT\n");
      }
    }
    TLOG(" IP AFTER: tos=%d tot_len=%d id=0x%x frag_off=0x%x ttl=%d protocol=%d check=0x%x saddr=0x%x daddr=0x%x\n",
            ip->tos, ntohs(ip->tot_len), ntohs(ip->id), ntohs(ip->frag_off),
            ip->ttl, ip->protocol, ntohs(ip->check), ntohl(ip->saddr), ntohl(ip->daddr));

    if (ip->protocol == IPPROTO_TCP) {
      tcp = (struct tcp_header_t*)(ip+1);
#if 0
      TLOG("NAT DP: 80 -> 10080\n");
      tcp->dport = htons(10080);
#endif
      tcp->check = 0;
      tcp->check = get_tcp_checksum(ip, tcp);

      TLOG(" TCP AFTER: sp=0x%x dp=0x%x seq=0x%x ack_seq=0x%x doff=%d res1=%d %s%s%s%s%s%s%s%s wnd=0x%x check=0x%x urg_ptr=0x%x\n",
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
    }
  }
}

void
hntap_host_client_to_server_nat(char *pkt, int len)
{
  hntap_nat_worker(pkt, len, true, 0x40010004, 0x40010003);
}
void
hntap_net_client_to_server_nat(char *pkt, int len)
{
  hntap_nat_worker(pkt, len, false, 0x40000001, 0x40000002);
}
void
hntap_net_server_to_client_nat(char *pkt, int len)

{
  hntap_nat_worker(pkt, len, true, 0x40000002, 0x40000001);
}
void
hntap_host_server_to_client_nat(char *pkt, int len)
{
  hntap_nat_worker(pkt, len, false, 0x40010003, 0x40010004);
}


void
hntap_net_ether_header_add(char *pkt)
{
  struct ether_header_t *eth = (struct ether_header_t *)pkt;

  eth->dmac[0] = 0x00;
  eth->dmac[1] = 0xEE;
  eth->dmac[2] = 0x00;
  eth->dmac[3] = 0x00;
  eth->dmac[4] = 0x00;
  eth->dmac[5] = 0x04;

  eth->smac[0] = 0x00;
  eth->smac[1] = 0xEE;
  eth->smac[2] = 0xFF;
  eth->smac[3] = 0x00;
  eth->smac[4] = 0x00;
  eth->smac[5] = 0x04;

  eth->etype   = htons(0x0800);
}

void
hntap_host_ether_header_add(char *pkt)
{
  struct vlan_header_t *vlan = (struct vlan_header_t *)pkt;

  vlan->dmac[0] = 0x00;
  vlan->dmac[1] = 0xEE;
  vlan->dmac[2] = 0xFF;
  vlan->dmac[3] = 0x00;
  vlan->dmac[4] = 0x00;
  vlan->dmac[5] = 0x04;

  vlan->smac[0] = 0x00;
  vlan->smac[1] = 0xEE;
  vlan->smac[2] = 0x00;
  vlan->smac[3] = 0x00;
  vlan->smac[4] = 0x00;
  vlan->smac[5] = 0x04;

  vlan->tpid    = htons(0x8100);
  vlan->vlan_tag= htons(3003);
  vlan->etype   = htons(0x0800);
}

int
host_pkt_pre_process(char *pktbuf, uint32_t len)
{
    hntap_host_ether_header_add(pktbuf);
    return 0;
}

int
net_pkt_pre_process(char *pktbuf, uint32_t len)
{
    hntap_net_ether_header_add(pktbuf);
    return 0;
}

int
host_process_nat_cb(char *pktbuf, uint32_t len, pkt_direction_t direction)
{
    if (direction == PKT_DIRECTION_FROM_DEV) {
        hntap_host_client_to_server_nat(pktbuf, len);
    } else if (direction == PKT_DIRECTION_TO_DEV) {
        hntap_host_server_to_client_nat(pktbuf, len);
    } else {
        abort();
    }
    return 0;
}

int
net_process_nat_cb(char *pktbuf, uint32_t len, pkt_direction_t direction)
{
    if (direction == PKT_DIRECTION_FROM_DEV) {
        hntap_net_server_to_client_nat(pktbuf, len);
    } else if (direction == PKT_DIRECTION_TO_DEV) {
        hntap_net_client_to_server_nat(pktbuf, len);
    } else {
        abort();
    }
    return 0;
}

int main(int argv, char *argc[])
{
#define MAX_DEV_HANDLES 2
  dev_handle_t* host_tap_hdl;
  dev_handle_t* net_tap_hdl;
  setlinebuf(stdout);
  setlinebuf(stderr);
  dev_handle_t *dev_handles[MAX_DEV_HANDLES];

  int opt = 0;
  while ((opt = getopt(argv, argc, "p:n:x")) != -1) {
    switch (opt) {
    case 'p':
        hntap_port = atoi(optarg);
    TLOG( "Port numer=%d\n", hntap_port);
        break;
    case 'n':
        nw_retries = atoi(optarg);
    TLOG( "NW Retries=%d\n", nw_retries);
    break;
    case 'x':
    hntap_drop_rexmit = true;
        break;
    case '?':
    default:
        TLOG( "usage: ./hntap [-n <NW Retries>] [-x] \n");
    exit(-1);
        break;
    }

  }
  TLOG("Starting Host/network Tapper..\n");

  /* Create tap interface for Host-tap */
  host_tap_hdl = hntap_create_tunnel_device(TAP_ENDPOINT_HOST,
                                            HNTAP_HOST_TAPIF,
                                            HNTAP_HOST_TAPIF_IP,
                                            HNTAP_HOST_TAPIF_IPMASK,
                                            HNTAP_HOST_ROUTE_DESTIP,
                                            HNTAP_HOST_ROUTE_GWIP);
  if (host_tap_hdl == nullptr ) {
    TLOG("Error creating tap interface %s!\n", HNTAP_HOST_TAPIF);
    abort();
  }
  host_tap_hdl->pre_process = host_pkt_pre_process;
  host_tap_hdl->nat_cb = host_process_nat_cb;
  host_tap_hdl->tap_ports[0] = hntap_port;

  dev_handles[0] = host_tap_hdl;
  net_tap_hdl = hntap_create_tunnel_device(TAP_ENDPOINT_NET,
                                           HNTAP_NET_TAPIF,
                                           HNTAP_NET_TAPIF_IP,
                                           HNTAP_NET_TAPIF_IPMASK,
                                           HNTAP_NET_ROUTE_DESTIP,
                                           HNTAP_NET_ROUTE_GWIP);

  /* Create tap interface for Network-tap */
  if (net_tap_hdl == nullptr) {
      TLOG("Error creating tap interface %s!\n", HNTAP_NET_TAPIF);
      abort();
  }
  net_tap_hdl->pre_process = net_pkt_pre_process;
  net_tap_hdl->nat_cb = net_process_nat_cb;
  net_tap_hdl->tap_ports[0] = hntap_port;
  dev_handles[1] = net_tap_hdl;

  TLOG("  Setup done, listening on tap devices..\n");
  add_dev_handle_tap_pair(host_tap_hdl, net_tap_hdl);
  hntap_do_select_loop(dev_handles, MAX_DEV_HANDLES);

  return(0);
}
