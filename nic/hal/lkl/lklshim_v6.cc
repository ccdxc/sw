#include <unistd.h>
#include <string>
#include <sstream>
#include <ostream>
#include <iomanip>
#include "nic/hal/hal.hpp"
#include "nic/hal/lkl/lklshim.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/tls/tls_api.hpp"
#include "nic/hal/src/proxyrcb.hpp"
#include "nic/hal/src/proxyccb.hpp"

extern "C" {
#include "lkl.h"
#include "lkl_host.h"
}

namespace hal {

// byte array to hex string for logging
std::string hex_dump6(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}

#define IF_NAME 10

static bool lklshim_v6_setsockopt_and_bind(int fd,
                                        char *dst_mac,
                                        char *src_mac,
                                        char *vlan,
                                        lklshim_flow_t *flow,
                                        char* src_ip,
                                        char* ifname,
                                        uint16_t bind_portnum)
{
    struct sockaddr_in6       local;
    int ret=0;
    int optval = 1;
    int proto = 0x86dd;
    if (lkl_sys_setsockopt(fd, LKL_SOL_SOCKET, LKL_SO_REUSEADDR,
              (char*)&optval, sizeof(optval)) < 0) {
        perror("setsockopt()");
        lkl_sys_close(fd);
        return false;
    }
    if (lkl_sys_setsockopt(fd, SOL_SOCKET, LKL_SO_PAGEHDR_SMAC,
                           src_mac, MAC_SIZE) < 0)  {
        perror("SO_PAGEHDR_SMAC");
        lkl_sys_close(fd);
        return false;
    }
    if (lkl_sys_setsockopt(fd, SOL_SOCKET, LKL_SO_PAGEHDR_DMAC,
                           dst_mac, MAC_SIZE) < 0)  {
        perror("SO_PAGEHDR_DMAC");
        lkl_sys_close(fd);
        return false;
    }

    HAL_TRACE_DEBUG("lklshim: setsockopt vlan={}", hex_dump6((uint8_t *)vlan, 2));
    if (lkl_sys_setsockopt(fd, SOL_SOCKET, LKL_SO_PAGEHDR_VLAN,
                           vlan, VLAN_SIZE) < 0)  {
        perror("SO_PAGEHDR_VLAN");
        lkl_sys_close(fd);
        return false;
    }

    if (lkl_sys_setsockopt(fd, SOL_SOCKET, LKL_SO_PAGEHDR_PROTO,
                           (char*)&proto, MAC_SIZE) < 0)  {
        perror("SO_PAGEHDR_PROTO");
        lkl_sys_close(fd);
        return false;
    }

    if (lkl_sys_setsockopt(fd, SOL_SOCKET, LKL_SO_PAGEHDR_FLOW,
                           (char*)flow, sizeof(char*)) < 0)  {
        perror("SO_PAGEHDR_FLOW");
        lkl_sys_close(fd);
        return false;
    }

    if (lkl_sys_setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
                           ifname, sizeof(char)*IF_NAME) < 0)  {
        perror("SO_BINDTODEVICE");
        lkl_sys_close(fd);
        return false;
    }

    if (src_ip) {
        if (lkl_sys_setsockopt(fd, SOL_SOCKET, LKL_SO_PAGEHDR_SIP,
                               src_ip, sizeof(ipv6_addr_t)) < 0)  {
            perror("SO_PAGEHDR_DMAC");
            lkl_sys_close(fd);
            return false;
        }
    }

    memset(&local, 0, sizeof(local));
    local.sin6_family = AF_INET6;
    if (src_ip) {
        memcpy(&local.sin6_addr, &flow->key.src_ip.v6_addr.addr8, sizeof(flow->key.src_ip.v6_addr.addr8));
    } else {
        local.sin6_addr = in6addr_any;
    }
    local.sin6_port = htons(bind_portnum);
    ret = lkl_sys_bind(fd, reinterpret_cast<lkl_sockaddr*>(&local), sizeof(local));
    if (ret != 0) {
        perror("bind");
        lkl_sys_close(fd);
        return false;
    }
    return true;
}

bool
lklshim_create_v6_listen_sockets (hal::flow_direction_t dir, lklshim_flow_t *flow)
{
    char *src_mac = NULL, *dst_mac = NULL, *vlan = NULL;
    int fd;
    char if_to_bind[IF_NAME];

    fd = lkl_sys_socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0); 

    memset(if_to_bind, 0, sizeof(char)*IF_NAME);
    strncpy(if_to_bind, flow->hostns.dev, sizeof(char)*IF_NAME);

    if (dir == hal::FLOW_DIR_FROM_ENIC) {
        src_mac = (char*)flow->hostns.src_mac;
        dst_mac = (char*)flow->hostns.dst_mac;
        vlan = (char*)flow->hostns.vlan;
    } else {
        src_mac = (char*)flow->netns.src_mac;
        dst_mac = (char*)flow->netns.dst_mac;
        vlan = (char*)flow->hostns.vlan;
    }

    HAL_TRACE_DEBUG("flow dp={} flow sp={}",
                    flow->key.dst_port,
                    flow->key.src_port);
    if (!lklshim_v6_setsockopt_and_bind(fd,
                                     src_mac,
                                     dst_mac,
                                     vlan,
                                     flow,
                                     NULL,
                                     if_to_bind,
                                     flow->key.dst_port)) {
        return false;
    }

    if (lkl_sys_listen(fd, 5) < 0) {
        perror("listen()");
        exit(1);
    }
    
    return true;
}

bool
lklshim_trigger_v6_flow_connection (lklshim_flow_t *flow, hal::flow_direction_t dir)
{
    struct sockaddr_in6 con_sa;
    char if_to_bind[IF_NAME];
    int fd;
    char *src_mac, *dst_mac, *src_ip, *vlan;

    memset(if_to_bind, 0, sizeof(char)*IF_NAME);
    src_ip = (char*)&flow->key.src_ip.v6_addr;
    fd = lkl_sys_socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
    HAL_ASSERT_RETURN((fd >= 0), false);
    strncpy(if_to_bind, flow->netns.dev, sizeof(char)*IF_NAME);
    if (dir != hal::FLOW_DIR_FROM_ENIC) {
        flow->netns.sockfd = fd;
        src_mac = (char*)flow->netns.src_mac;
        dst_mac = (char*)flow->netns.dst_mac;
        vlan = (char*)flow->netns.vlan;
    } else {
        flow->hostns.sockfd = fd;
        src_mac = (char*)flow->hostns.src_mac;
        dst_mac = (char*)flow->hostns.dst_mac;
        vlan = (char*)flow->hostns.vlan;
    }

    if (!lklshim_v6_setsockopt_and_bind(fd,
                                     dst_mac,
                                     src_mac,
                                     vlan,
                                     flow,
                                     src_ip,
                                     if_to_bind,
                                     flow->key.src_port)) {
        return false;
    }
    flow->netns.state = FLOW_STATE_CONNECT_PENDING;
    memset(&con_sa, 0, sizeof(con_sa));
    con_sa.sin6_family = AF_INET6;
    memcpy(&con_sa.sin6_addr, &flow->key.dst_ip.v6_addr.addr8, sizeof(flow->key.dst_ip.v6_addr.addr8));
    con_sa.sin6_port = htons(flow->key.dst_port);
    lkl_sys_connect(fd, reinterpret_cast<lkl_sockaddr*>(&con_sa), sizeof(con_sa));

    return true;
}

bool 
lklshim_release_client_syn6(uint16_t qid) 
{
    lklshim_flow_t *flow = lklshim_flow_by_qid[qid];
    
    if (flow == NULL) {
      HAL_TRACE_ERR("lklshim: flow does't exist to release client syn for qid = {}", qid);
      return false;
    }
    
    HAL_TRACE_DEBUG("lklshim: trying to release client syn6 for qid = {}", qid);

    if (flow->hostns.skbuff != NULL) {
      void *pkt_skb = flow->hostns.skbuff;
      ether_header_t *eth = (ether_header_t*)lkl_get_mac_start(pkt_skb);
      ipv6_header_t *ip = (ipv6_header_t*)lkl_get_network_start(pkt_skb);
      tcp_header_t *tcp = (tcp_header_t*)lkl_get_transport_start(pkt_skb);

      HAL_TRACE_DEBUG("lklshim: skb={}", pkt_skb);
      HAL_TRACE_DEBUG("lklshim: flow miss rx eth={}", hex_dump6((uint8_t*)eth, 18));
      HAL_TRACE_DEBUG("lklshim: flow miss rx ip={}", hex_dump6((uint8_t*)ip, sizeof(ipv6_header_t)));
      HAL_TRACE_DEBUG("lklshim: flow miss rx tcp={}", hex_dump6((uint8_t*)tcp, sizeof(tcp_header_t)));

      lkl_tcp_v6_rcv(pkt_skb);
    }

    return true;
}

bool
lklshim_process_v6_flow_hit_rx_packet (void *pkt_skb,
                                    hal::flow_direction_t dir,
                                    const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr)
{
    lklshim_flow_t     *flow;

    ipv6_header_t *ip = (ipv6_header_t*)lkl_get_network_start(pkt_skb);
    tcp_header_t *tcp = (tcp_header_t*)lkl_get_transport_start(pkt_skb);
    HAL_TRACE_DEBUG("lklshim: daddr={}, saddr={}, dport={}, sport={}, seqno={}, ackseqno={} ", 
                    ip->daddr, ip->saddr, ntohs(tcp->dport), ntohs(tcp->sport), ntohl(tcp->seq), ntohl(tcp->ack_seq));
    HAL_TRACE_DEBUG("lklshim: flow hit pkt rxhdr->qid={}", rxhdr->qid);
    flow = lklshim_flow_by_qid[rxhdr->qid];
    if (!flow) return false;
    if (flow->itor_dir != dir) {
        flow->dst_lif = rxhdr->src_lif;
        HAL_TRACE_DEBUG("lklshim: updating flow dst lif = {}", flow->dst_lif);
    }
    if (lkl_tcp_v6_rcv(pkt_skb)) {
        //return false;
    }
    HAL_TRACE_DEBUG("lklshim: flow hit pkt iqid={} rqid={}", flow->iqid, flow->rqid);

    /*
     * If the TLS bypass mode is set, we're doing TCP proxy only, so we can
     * release the client syn and establish session for the original flow.
     */
    if (!hal::tls::proxy_tls_bypass_mode) {
        if(flow->itor_dir == hal::FLOW_DIR_FROM_ENIC){
	    hal::tls::tls_api_start_handshake(flow->iqid, flow->rqid, false);
	} else {
            hal::tls::tls_api_start_handshake(flow->rqid, flow->iqid, false);
	}
    } else {

        HAL_TRACE_DEBUG("lklshim: TLS proxy bypass mode: release client syn for daddr={}, saddr={}, "
			"dport={}, sport={}, seqno={}, ackseqno={} ", 
			ip->daddr, ip->saddr, ntohs(tcp->dport), ntohs(tcp->sport), ntohl(tcp->seq),
			ntohl(tcp->ack_seq));
        // Inform LKL  
        lklshim_release_client_syn6(flow->iqid);
    }
 
    return true;
}

bool
lklshim_process_v6_flow_hit_rx_header (void *pkt_skb,
                                    hal::flow_direction_t dir,
                                    const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr)
{
    lklshim_flow_t     *flow;
    union tcp_word_hdr {                                            
      tcp_header_t  hdr;                                          
      __be32        words[5];                                     
    } *tp = (union tcp_word_hdr*)lkl_get_transport_start(pkt_skb);                                                              

    ipv6_header_t *ip = (ipv6_header_t*)lkl_get_network_start(pkt_skb);


    HAL_TRACE_DEBUG("lklshim: dir={} lif={} qid ={} lkp_vrf={} "
                    "tcp_flags={} tcp_seq_num={} tcp_ack_num={} tcp_window={} tcp_mss={} tcp_ws={}",
                    rxhdr->lkp_dir,rxhdr->lif, rxhdr->qid, rxhdr->lkp_vrf, 
                    rxhdr->tcp_flags, rxhdr->tcp_seq_num, rxhdr->tcp_ack_num, 
                    rxhdr->tcp_window, rxhdr->tcp_mss, rxhdr->tcp_ws);

    flow = lklshim_flow_by_qid[rxhdr->qid];
    if (flow == NULL) {
        HAL_TRACE_ERR("No lklshim flow found for qid={}", rxhdr->qid);
        return false;
    }
    lkl_skb_set_qid(pkt_skb, rxhdr->qid);
    lkl_skb_set_src_lif(pkt_skb, 0);
    HAL_TRACE_DEBUG("Key : [dir={}, sa={}, da={}, sp={}, dp={}]", 
                    flow->itor_dir,
                    flow->key.src_ip.v6_addr, flow->key.dst_ip.v6_addr, flow->key.src_port, flow->key.dst_port);

    HAL_TRACE_DEBUG("pkt={} iph={} tcp={}", pkt_skb, (void*)ip, (void *)tp);
    if (rxhdr->lkp_dir == 0) {
      memcpy(ip->saddr, &flow->key.src_ip.v6_addr.addr8, sizeof(flow->key.src_ip.v6_addr.addr8));
      memcpy(ip->daddr, &flow->key.dst_ip.v6_addr.addr8, sizeof(flow->key.dst_ip.v6_addr.addr8));
      ip->version = 6;
      ip->tc_high = 0;
      ip->tc_low = 0;
      ip->flow_lbl_high = 0;
      ip->flow_lbl[0] = ip->flow_lbl[1] = 0;
      ip->payload_len = htons(20);
      ip->nexthdr = 6;
      ip->hop_limit = 64;
      tp->words[3] = 0;
      tp->hdr.sport = htons(flow->key.src_port);
      tp->hdr.dport = htons(flow->key.dst_port);
      tp->hdr.doff = 5;
      tp->hdr.ack = 1;
      tp->hdr.window = htons(rxhdr->tcp_window);
      tp->hdr.seq = htonl(rxhdr->tcp_seq_num);
      tp->hdr.ack_seq = htonl(rxhdr->tcp_ack_num);
    } else {
      memcpy(ip->daddr, &flow->key.src_ip.v6_addr.addr8, sizeof(flow->key.src_ip.v6_addr.addr8));
      memcpy(ip->saddr, &flow->key.dst_ip.v6_addr.addr8, sizeof(flow->key.dst_ip.v6_addr.addr8));
      ip->version = 6;
      ip->tc_high = 0;
      ip->tc_low = 0;
      ip->flow_lbl_high = 0;
      ip->flow_lbl[0] = ip->flow_lbl[1] = 0;
      ip->payload_len = htons(20);
      ip->nexthdr = 6;
      ip->hop_limit = 64;
      tp->words[3] = 0;
      tp->hdr.sport = htons(flow->key.dst_port);
      tp->hdr.dport = htons(flow->key.src_port);
      tp->hdr.doff = 5;
      tp->hdr.ack = 1;
      tp->hdr.window = htons(rxhdr->tcp_window);
      tp->hdr.seq = htonl(rxhdr->tcp_seq_num);
      tp->hdr.ack_seq = htonl(rxhdr->tcp_ack_num);
    }

    HAL_TRACE_DEBUG("lklshim: ip={} tcp={} daddr={}, saddr={}, dport={}, sport={}, seqno={}, ackseqno={} ", 
                    (void*)ip, (void*)tp,
                    ip->daddr, ip->saddr, 
                    ntohs(tp->hdr.dport), ntohs(tp->hdr.sport), ntohl(tp->hdr.seq), ntohl(tp->hdr.ack_seq));

    if (flow->itor_dir != dir) {
        flow->dst_lif = rxhdr->src_lif;
        HAL_TRACE_DEBUG("lklshim: updating flow dst lif = {}", flow->dst_lif);
    }
    HAL_TRACE_DEBUG("lklshim: calling lkl_tcp_v6_rcv");
    if (!lkl_tcp_v6_rcv(pkt_skb)) {
        return false;
    }
    return true;
}

bool
lklshim_process_v6_flow_miss_rx_packet (void *pkt_skb,
                                     hal::flow_direction_t dir,
                                     uint32_t iqid, uint32_t rqid, 
				     uint16_t src_lif, uint16_t hw_vlan_id)
{
    lklshim_flow_t      *flow;
    lklshim_flow_key_t  flow_key;

    ether_header_t *eth = (ether_header_t*)lkl_get_mac_start(pkt_skb);
    ipv6_header_t *ip6 = (ipv6_header_t*)lkl_get_network_start(pkt_skb);
    tcp_header_t *tcp = (tcp_header_t*)lkl_get_transport_start(pkt_skb);
    vlan_header_t *vlan = (vlan_header_t *)lkl_get_mac_start(pkt_skb);
    HAL_TRACE_DEBUG("lklshim: flow miss pkt iqid={} rqid={}", iqid, rqid);
    HAL_TRACE_DEBUG("lklshim: flow miss rx eth={}", hex_dump6((uint8_t*)eth, 18));
    HAL_TRACE_DEBUG("lklshim: flow miss rx ip6={}", hex_dump6((uint8_t*)ip6, sizeof(ipv6_header_t)));
    HAL_TRACE_DEBUG("lklshim: flow miss rx tcp={}", hex_dump6((uint8_t*)tcp, sizeof(tcp_header_t)));

    lklshim_make_flow_v6key(&flow_key, ip6->saddr, ip6->daddr, ntohs(tcp->sport), ntohs(tcp->dport));
    flow = lklshim_flow_entry_alloc(&flow_key);
    if (!flow) return false;
    /*
     * Cache the pointer to the packet as we'll process it later.
     */
    flow->itor_dir = dir;
    flow->hw_vlan_id = hw_vlan_id;
    flow->iqid = iqid;
    lklshim_flow_by_qid[iqid] = flow;
    flow->rqid = rqid;
    lklshim_flow_by_qid[rqid] = flow;
    flow->src_lif = src_lif;

    proxy::tcp_create_cb_v6(flow->iqid, flow->src_lif, eth, vlan, ip6, tcp, true, hw_vlan_id,
                            proxyccb_tcpcb_l7_proxy_type_eval(flow->iqid));
    proxy::tcp_create_cb_v6(flow->rqid, flow->src_lif, eth, vlan, ip6, tcp, false, hw_vlan_id,
                            proxyccb_tcpcb_l7_proxy_type_eval(flow->rqid));

    // create tlscb
    hal::tls::tls_api_init_flow(flow->iqid, false);
    hal::tls::tls_api_init_flow(flow->rqid, true);

    if (dir == hal::FLOW_DIR_FROM_ENIC) {
        hal::tls::tls_api_init_flow(flow->iqid, flow->rqid);
        flow->hostns.skbuff = pkt_skb;
        flow->netns.skbuff = NULL;
        HAL_TRACE_DEBUG("lklshim: flow->hostns.skbuff={}", flow->hostns.skbuff);
        HAL_TRACE_DEBUG("lklshim: flow->netns.skbuff={}", flow->netns.skbuff);
        flow->hostns.state = FLOW_STATE_SYN_RCVD;
        strncpy(flow->hostns.dev, "eth0", 16);
        strncpy(flow->netns.dev, "eth1", 16);
        memcpy(flow->hostns.src_mac, eth->smac, ETH_ADDR_LEN);
        memcpy(flow->hostns.dst_mac, eth->dmac, ETH_ADDR_LEN);

        HAL_TRACE_DEBUG("lklshim: hostns eth etype {}", eth->etype);

        if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
          vlan_header_t *vlan = (vlan_header_t*)lkl_get_mac_start(pkt_skb);
          memcpy(flow->hostns.vlan, &(vlan->vlan_tag), sizeof(vlan->vlan_tag));
          HAL_TRACE_DEBUG("lklshim: flow hostns vlan={}",
                          hex_dump6(flow->hostns.vlan, sizeof(vlan->vlan_tag)));
        } else {
          HAL_TRACE_DEBUG("lklshim: flow hostns vlan not set");
        }
        lklshim_create_v6_listen_sockets(dir, flow);
        lklshim_trigger_v6_flow_connection(flow, dir);
    } else {
        hal::tls::tls_api_init_flow(flow->rqid, flow->iqid);
        flow->netns.skbuff = pkt_skb;
        flow->hostns.skbuff = NULL;
        flow->netns.state = FLOW_STATE_SYN_RCVD;
        strncpy(flow->hostns.dev, "eth1", 16);
        strncpy(flow->netns.dev, "eth0", 16);
        memcpy(flow->netns.src_mac, eth->smac, ETH_ADDR_LEN);
        memcpy(flow->netns.dst_mac, eth->dmac, ETH_ADDR_LEN);
        HAL_TRACE_DEBUG("lklshim: netns eth etype {}", eth->etype);

        if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
          vlan_header_t *vlan = (vlan_header_t*)lkl_get_mac_start(pkt_skb);
          memcpy(flow->netns.vlan, &(vlan->vlan_tag), sizeof(vlan->vlan_tag));
          HAL_TRACE_DEBUG("lklshim: flow netns vlan={}",
                          hex_dump6(flow->netns.vlan, sizeof(vlan->vlan_tag)));
        } else {
          HAL_TRACE_DEBUG("lklshim: flow netns vlan not set");
        }
        lklshim_create_v6_listen_sockets(dir, flow);
        lklshim_trigger_v6_flow_connection(flow, dir);
    }
    return true;
}

} // namespace hal


