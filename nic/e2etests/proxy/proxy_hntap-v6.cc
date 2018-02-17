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

#include <linux/if_tun.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <zmq.h>
#include <arpa/inet.h>

#include "nic/e2etests/lib/helpers.hpp"
#include "nic/e2etests/hntap/dev.hpp"

#define HNTAP_HOST_TAPIF        "hntap_host0"
#define HNTAP_HOST_TAPIF_IP     "2000::1:0:4"
#define HNTAP_HOST_ROUTE_DESTIP "2000::1"
#define HNTAP_HOST_ROUTE_GWIP   "2000::1:0:4"

#define HNTAP_NET_TAPIF         "hntap_net0"
#define HNTAP_NET_TAPIF_IP      "2000::2"
#define HNTAP_NET_ROUTE_DESTIP  "2000::1:0:3"
#define HNTAP_NET_ROUTE_GWIP    "2000::2"

extern uint32_t nw_retries;
extern uint16_t hntap_port;
extern bool hntap_drop_rexmit;

static char buf[64];
static char* print_v6_addr(void* addr)
{
    inet_ntop(AF_INET6, addr, buf, sizeof(buf));
    return buf;
}

void
hntap_nat_worker(char *pkt, int len, bool source_ip, const char *orig_addr, const char *to_addr)
{
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
    TLOG(" IP BEFORE 1: tot_len=%d nexthdr=%d",
            ntohs(ip->payload_len), ip->nexthdr);
    TLOG(" saddr=%s", print_v6_addr(ip->saddr.s6_addr));
    TLOG(" daddr=%s\n", print_v6_addr(ip->daddr.s6_addr));

    if (ip->nexthdr == IPPROTO_TCP) {
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

    if (ip->nexthdr == IPPROTO_TCP) {
      tcp = (struct tcp_header_t*)(ip+1);
      tcp->check = 0;
      tcp->check = get_tcp_checksumv6(ip, tcp);

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

    uint8_t orig_addr_buf[16], to_addr_buf[16];
    inet_pton(AF_INET6, orig_addr, orig_addr_buf);
    inet_pton(AF_INET6, to_addr, to_addr_buf);

    if (source_ip) {
      TLOG("saddr cmp orig_addr_buf %s\n", print_v6_addr(orig_addr_buf));
      if (!memcmp(ip->saddr.s6_addr, orig_addr_buf, 16)) {
        TLOG("NAT IPSA: %s -> ", print_v6_addr(orig_addr_buf));
        TLOG(" %s\n", print_v6_addr(to_addr_buf));

        memcpy(ip->saddr.s6_addr, to_addr_buf, 16);
      } else {
        TLOG("No NAT\n");
      }
    } else {
      TLOG("daddr cmp orig_addr_buf %s\n", print_v6_addr(orig_addr_buf));
      if (!memcmp(ip->daddr.s6_addr, orig_addr_buf, 16)) {
        TLOG("NAT IPDA: %s -> ", print_v6_addr(orig_addr_buf));
        TLOG(" %s\n", print_v6_addr(to_addr_buf));

        memcpy(ip->daddr.s6_addr, to_addr_buf, 16);

      } else {
        TLOG("No NAT\n");
      }
    }

    TLOG(" IP AFTER: tot_len=%d nexthdr=%d\n",
            ntohs(ip->payload_len), ip->nexthdr);
    TLOG(" saddr=%s", print_v6_addr(ip->saddr.s6_addr));
    TLOG(" daddr=%s\n", print_v6_addr(ip->daddr.s6_addr));

    if (ip->nexthdr == IPPROTO_TCP) {
      tcp = (struct tcp_header_t*)(ip+1);
      tcp->check = 0;
      tcp->check = get_tcp_checksumv6(ip, tcp);

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
  hntap_nat_worker(pkt, len, true, "2000::1:0:4", "2000::1:0:3");
}
void
hntap_net_client_to_server_nat(char *pkt, int len)
{
  hntap_nat_worker(pkt, len, false, "2000::1", "2000::2");
}
void
hntap_net_server_to_client_nat(char *pkt, int len)

{
  hntap_nat_worker(pkt, len, true, "2000::2", "2000::1");
}
void
hntap_host_server_to_client_nat(char *pkt, int len)
{
  hntap_nat_worker(pkt, len, false, "2000::1:0:3", "2000::1:0:4");
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

  eth->etype   = htons(0x86dd);
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
  vlan->etype   = htons(0x86dd);
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
  while ((opt = getopt(argv, argc, "n:x")) != -1) {
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
  host_tap_hdl = hntap_create_tunnel_devicev6(TAP_ENDPOINT_HOST,
                                            HNTAP_HOST_TAPIF,
                                            HNTAP_HOST_TAPIF_IP,
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
  net_tap_hdl = hntap_create_tunnel_devicev6(TAP_ENDPOINT_NET,
                                           HNTAP_NET_TAPIF,
                                           HNTAP_NET_TAPIF_IP,
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
