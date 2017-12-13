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

extern "C" {
#include "nic/third-party/lkl/export/include/lkl.h"
#include "nic/third-party/lkl/export/include/lkl_host.h"
}


namespace hal {

ht                       *lklshim_flow_db;
ht                       *lklshim_host_lsock_db;
ht                       *lklshim_net_lsock_db;
slab                     *lklshim_flowdb_slab;
slab                     *lklshim_lsockdb_slab;
int                      lklshim_max_lfd = 0;
lklshim_flow_t           *lklshim_flow_by_qid[MAX_PROXY_FLOWS];

void *
lklshim_flow_get_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((lklshim_flow_t *)entry)->key);
}

uint32_t
lklshim_flow_compute_key_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(lklshim_flow_key_t)) % ht_size;
}

bool
lklshim_flow_compare_key_func(void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(lklshim_flow_key_t))) {
        return true;
    }
    return false;
}

void *
lklshim_lsock_get_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((lklshim_listen_sockets_t *)entry)->tcp_portnum);
}

uint32_t
lklshim_lsock_compute_key_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(int)) % ht_size;
}

bool
lklshim_lsock_compare_key_func(void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if ((((lklshim_flow_key_t*)key1)->src_ip == ((lklshim_flow_key_t*)key2)->src_ip) && 
        (((lklshim_flow_key_t*)key1)->dst_ip == ((lklshim_flow_key_t*)key2)->dst_ip) &&
        (((lklshim_flow_key_t*)key1)->src_port == ((lklshim_flow_key_t*)key2)->src_port) &&
        (((lklshim_flow_key_t*)key1)->dst_port == ((lklshim_flow_key_t*)key2)->dst_port))
       {
        return true;
    }
    return false;
}

// byte array to hex string for logging
std::string hex_dump(const uint8_t *buf, size_t sz)
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

bool
lklshim_mem_init(void)
{
    // initialize SLAB for flow entry allocations
    lklshim_flowdb_slab = slab::factory("lklshim_flowdb", HAL_SLAB_LKLSHIM_FLOWDB,
                                sizeof(lklshim_flow_t), 16,
                                false, true, true);
    HAL_ASSERT_RETURN((lklshim_flowdb_slab != NULL), false);

    // initialize SLAB for listen-socket entry allocations
    lklshim_lsockdb_slab = slab::factory("lklshim_lsockdb", HAL_SLAB_LKLSHIM_LSOCKDB,
                                sizeof(lklshim_listen_sockets_t), 16,
                                false, true, true);
    HAL_ASSERT_RETURN((lklshim_lsockdb_slab != NULL), false);

    // initialize hash-table for lklshim flow entries
    lklshim_flow_db = ht::factory(HAL_MAX_LKLSHIM_FLOWS,
                           lklshim_flow_get_key_func,
                           lklshim_flow_compute_key_hash_func,
                           lklshim_flow_compare_key_func);
    HAL_ASSERT_RETURN((lklshim_flow_db != NULL), false);

    // initialize hash-table for lklshim listen socket entries in host NS
    lklshim_host_lsock_db = ht::factory(HAL_SLAB_LKLSHIM_LSOCKS,
                           lklshim_lsock_get_key_func,
                           lklshim_lsock_compute_key_hash_func,
                           lklshim_lsock_compare_key_func);
    HAL_ASSERT_RETURN((lklshim_host_lsock_db != NULL), false);

    // initialize hash-table for lklshim listen socket entries in Network NS
    lklshim_net_lsock_db = ht::factory(HAL_SLAB_LKLSHIM_LSOCKS,
                           lklshim_lsock_get_key_func,
                           lklshim_lsock_compute_key_hash_func,
                           lklshim_lsock_compare_key_func);
    HAL_ASSERT_RETURN((lklshim_net_lsock_db != NULL), false);
    return true;
}

lklshim_flow_t *
lklshim_flow_entry_create (lklshim_flow_key_t *flow_key)
{
    lklshim_flow_t *flow;

    flow = (lklshim_flow_t *) lklshim_flowdb_slab->alloc();
    HAL_ASSERT_RETURN((flow != NULL), NULL);

    memcpy(&flow->key, flow_key, sizeof(flow->key));
    flow->ht_ctxt.reset();
    lklshim_flow_db->insert(flow, &flow->ht_ctxt);
    return(flow);
}

lklshim_flow_t *
lklshim_flow_entry_get_or_create (lklshim_flow_key_t *flow_key)
{
    lklshim_flow_t *entry;

    entry = lklshim_flow_entry_lookup(flow_key);
    if (!entry) {
        entry = lklshim_flow_entry_create(flow_key);
    }
    return entry;
}

void
lklshim_flow_entry_delete (lklshim_flow_t *flow)
{
    if (!flow) return;

    lklshim_flow_db->remove(&flow->key);
    return lklshim_flowdb_slab->free(flow);
}

#define IF_NAME 10

static bool lklshim_setsockopt_and_bind(int fd,
                                        char *dst_mac,
                                        char *src_mac,
                                        char *vlan,
                                        lklshim_flow_t *flow,
                                        char* src_ip,
                                        char* ifname,
                                        uint16_t bind_portnum)
{
    struct sockaddr_in       local;
    int optval = 1;
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

    HAL_TRACE_DEBUG("lklshim: setsockopt vlan={}", hex_dump((uint8_t *)vlan, 2));
    if (lkl_sys_setsockopt(fd, SOL_SOCKET, LKL_SO_PAGEHDR_VLAN,
                           vlan, VLAN_SIZE) < 0)  {
        perror("SO_PAGEHDR_VLAN");
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
                               src_ip, sizeof(in_addr_t)) < 0)  {
            perror("SO_PAGEHDR_DMAC");
            lkl_sys_close(fd);
            return false;
        }
    }

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    if (src_ip) {
        local.sin_addr.s_addr = htonl(flow->key.src_ip);
    } else {
        local.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    local.sin_port = htons(bind_portnum);
    if (lkl_sys_bind(fd, (struct lkl_sockaddr*) &local, sizeof(local)) != 0) {
        perror("bind");
        lkl_sys_close(fd);
        return false;
    }
    return true;
}

bool
lklshim_create_listen_sockets (hal::flow_direction_t dir, lklshim_flow_t *flow)
{
    lklshim_listen_sockets_t *lsock;
    char *src_mac = NULL, *dst_mac = NULL, *vlan = NULL;
    int fd;
    char if_to_bind[IF_NAME];

    /*
     * Get config from TCP/TLS Agents, and for each of the layer-4
     * Ports that the service needs to be enabled, we'll create
     * an IPv4 and IPv6 listen socket with {INADDR(6)_ANY, port-num}
     * to capture all the H-flow and N-flow connection requests.
     *
     */
    lsock = (lklshim_listen_sockets_t *) lklshim_lsockdb_slab->alloc();
    HAL_ASSERT_RETURN((lsock != NULL), false);
    lsock->ht_ctxt.reset();
    lsock->tcp_portnum = flow->key.dst_port;
    fd = lsock->ipv4_sockfd = lkl_sys_socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); 

    memset(if_to_bind, 0, sizeof(char)*IF_NAME);
    strncpy(if_to_bind, flow->hostns.dev, sizeof(char)*IF_NAME);

    if (dir == hal::FLOW_DIR_FROM_ENIC) {
        lklshim_host_lsock_db->insert(lsock, &lsock->ht_ctxt);
        src_mac = (char*)flow->hostns.src_mac;
        dst_mac = (char*)flow->hostns.dst_mac;
        vlan = (char*)flow->hostns.vlan;
    } else {
        lklshim_net_lsock_db->insert(lsock, &lsock->ht_ctxt);
        src_mac = (char*)flow->netns.src_mac;
        dst_mac = (char*)flow->netns.dst_mac;
        vlan = (char*)flow->hostns.vlan;
    }

    HAL_TRACE_DEBUG("tcp_portnum={} flow dp={} flow sp={}",
                    lsock->tcp_portnum,
                    flow->key.dst_port,
                    flow->key.src_port);
    if (!lklshim_setsockopt_and_bind(fd,
                                     src_mac,
                                     dst_mac,
                                     vlan,
                                     flow,
                                     NULL,
                                     if_to_bind,
                                     lsock->tcp_portnum)) {
        return false;
    }

    if (lkl_sys_listen(fd, 5) < 0) {
        perror("listen()");
        exit(1);
    }
    
    return true;
}

bool
lklshim_trigger_flow_connection (lklshim_flow_t *flow, hal::flow_direction_t dir)
{
    struct sockaddr_in con_sa;
    char if_to_bind[IF_NAME];
    int fd;
    char *src_mac, *dst_mac, *src_ip, *vlan;

    memset(if_to_bind, 0, sizeof(char)*IF_NAME);
    src_ip = (char*)&flow->key.src_ip;
    fd = lkl_sys_socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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


    if (!lklshim_setsockopt_and_bind(fd,
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
    con_sa.sin_family = AF_INET;
    con_sa.sin_addr.s_addr = htonl(flow->key.dst_ip);
    con_sa.sin_port = htons(flow->key.dst_port);
    lkl_sys_connect(fd, (struct lkl_sockaddr *) &con_sa, sizeof(con_sa));

    //flow->netns.state = FLOW_STATE_ESTABLISHED;
    return true;
}

void
lklshim_flowdb_init ()
{
    lklshim_mem_init();
}

bool 
lklshim_release_client_syn(uint16_t qid) 
{
    lklshim_flow_t *flow = lklshim_flow_by_qid[qid];

    
    if (flow == NULL) {
      HAL_TRACE_ERR("lklshim: flow does't exist to release client syn for qid = {}", qid);
      return false;
    }
    
    HAL_TRACE_DEBUG("lklshim: trying to release client syn for qid = {}", qid);

    if (flow->hostns.skbuff != NULL) {
      void *pkt_skb = flow->hostns.skbuff;
      ether_header_t *eth = (ether_header_t*)lkl_get_mac_start(pkt_skb);
      ipv4_header_t *ip = (ipv4_header_t*)lkl_get_network_start(pkt_skb);
      tcp_header_t *tcp = (tcp_header_t*)lkl_get_transport_start(pkt_skb);

      HAL_TRACE_DEBUG("lklshim: flow miss rx eth={}", hex_dump((uint8_t*)eth, 18));
      HAL_TRACE_DEBUG("lklshim: flow miss rx ip={}", hex_dump((uint8_t*)ip, sizeof(ipv4_header_t)));
      HAL_TRACE_DEBUG("lklshim: flow miss rx tcp={}", hex_dump((uint8_t*)tcp, sizeof(tcp_header_t)));

      lkl_tcp_v4_rcv(pkt_skb);
    }

    return true;
}

bool
lklshim_process_flow_hit_rx_packet (void *pkt_skb,
                                    hal::flow_direction_t dir,
                                    const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr)
{
    lklshim_flow_t     *flow;
    lklshim_flow_key_t flow_key;

    ipv4_header_t *ip = (ipv4_header_t*)lkl_get_network_start(pkt_skb);
    tcp_header_t *tcp = (tcp_header_t*)lkl_get_transport_start(pkt_skb);
    HAL_TRACE_DEBUG("lklshim: daddr={}, saddr={}, dport={}, sport={}, seqno={}, ackseqno={} ", 
                    ip->daddr, ip->saddr, ntohs(tcp->dport), ntohs(tcp->sport), ntohl(tcp->seq), ntohl(tcp->ack_seq));

    lklshim_make_flow_v4key(&flow_key, ip->daddr, ip->saddr, ntohs(tcp->dport), ntohs(tcp->sport));
    flow = lklshim_flow_entry_get_or_create(&flow_key);
    if (!flow) return false;
    if (flow->itor_dir != dir) {
        flow->dst_lif = rxhdr->src_lif;
        HAL_TRACE_DEBUG("lklshim: updating flow dst lif = {}", flow->dst_lif);
    }
    if (lkl_tcp_v4_rcv(pkt_skb)) {
        return false;
    }

    /*
     * If the TLS bypass mode is set, we're doing TCP proxy only, so we can
     * release the client syn and establish session for the original flow.
     */
    if (!hal::tls::proxy_tls_bypass_mode) {
        if(flow->itor_dir == hal::FLOW_DIR_FROM_ENIC){
	    hal::tls::tls_api_start_handshake(flow->iqid, flow->rqid);
	} else {
            hal::tls::tls_api_start_handshake(flow->rqid, flow->iqid);
	}
    } else {

        HAL_TRACE_DEBUG("lklshim: TLS proxy bypass mode: release client syn for daddr={}, saddr={}, "
			"dport={}, sport={}, seqno={}, ackseqno={} ", 
			ip->daddr, ip->saddr, ntohs(tcp->dport), ntohs(tcp->sport), ntohl(tcp->seq),
			ntohl(tcp->ack_seq));
        // Inform LKL  
        lklshim_release_client_syn(flow->iqid);
    }
 
    return true;
}
bool
lklshim_process_flow_hit_rx_header (void *pkt_skb,
                                    hal::flow_direction_t dir,
                                    const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr)
{
    lklshim_flow_t     *flow;
    union tcp_word_hdr {                                            
      tcp_header_t  hdr;                                          
      __be32        words[5];                                     
    } *tp = (union tcp_word_hdr*)lkl_get_transport_start(pkt_skb);                                                              

    ipv4_header_t *ip = (ipv4_header_t*)lkl_get_network_start(pkt_skb);


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
                    flow->key.src_ip, flow->key.dst_ip, flow->key.src_port, flow->key.dst_port);

    HAL_TRACE_DEBUG("pkt={} iph={} tcp={}", pkt_skb, (void*)ip, (void *)tp);
    if (rxhdr->lkp_dir == 0) {
      ip->saddr = flow->key.src_ip;
      ip->daddr = flow->key.dst_ip;
      ip->version = 4;
      ip->ihl = 5;
      ip->protocol = 6;
      ip->tos = 0;
      ip->tot_len = htons(40);
      ip->id = 0;
      ip->frag_off = 0;
      ip->check = 0;
      tp->words[3] = 0;
      tp->hdr.sport = htons(flow->key.src_port);
      tp->hdr.dport = htons(flow->key.dst_port);
      tp->hdr.doff = 5;
      tp->hdr.ack = 1;
      tp->hdr.window = htons(rxhdr->tcp_window);
      tp->hdr.seq = htonl(rxhdr->tcp_seq_num);
      tp->hdr.ack_seq = htonl(rxhdr->tcp_ack_num);
    } else {
      ip->saddr = flow->key.dst_ip;
      ip->daddr = flow->key.src_ip;
      ip->version = 4;
      ip->ihl = 5;
      ip->protocol = 6;
      ip->tos = 0;
      ip->tot_len = htons(40);
      ip->id = 0;
      ip->frag_off = 0;
      ip->check = 0;
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

    if (lkl_tcp_v4_rcv(pkt_skb)) {
        return false;
    }

    return true;
}

bool
lklshim_process_flow_miss_rx_packet (void *pkt_skb,
                                     hal::flow_direction_t dir,
                                     uint32_t iqid, uint32_t rqid, 
				     uint16_t src_lif, uint16_t hw_vlan_id)
{
    lklshim_flow_t     *flow;
    lklshim_flow_key_t flow_key;


    ether_header_t *eth = (ether_header_t*)lkl_get_mac_start(pkt_skb);
    ipv4_header_t *ip = (ipv4_header_t*)lkl_get_network_start(pkt_skb);
    tcp_header_t *tcp = (tcp_header_t*)lkl_get_transport_start(pkt_skb);
    vlan_header_t *vlan = (vlan_header_t *)lkl_get_mac_start(pkt_skb);
    HAL_TRACE_DEBUG("lklshim: flow miss rx eth={}", hex_dump((uint8_t*)eth, 18));
    HAL_TRACE_DEBUG("lklshim: flow miss rx ip={}", hex_dump((uint8_t*)ip, sizeof(ipv4_header_t)));
    HAL_TRACE_DEBUG("lklshim: flow miss rx tcp={}", hex_dump((uint8_t*)tcp, sizeof(tcp_header_t)));

    lklshim_make_flow_v4key(&flow_key, ip->saddr, ip->daddr, ntohs(tcp->sport), ntohs(tcp->dport));
    flow = lklshim_flow_entry_get_or_create(&flow_key);
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
    proxy::tcp_create_cb(flow->iqid, flow->src_lif, eth, vlan, ip, tcp, true, hw_vlan_id);
    proxy::tcp_create_cb(flow->rqid, flow->src_lif, eth, vlan, ip, tcp, false, hw_vlan_id);
    // create tlscb
    hal::tls::tls_api_init_flow(flow->iqid, false);
    hal::tls::tls_api_init_flow(flow->rqid, true);

    if (dir == hal::FLOW_DIR_FROM_ENIC) {
        hal::tls::tls_api_init_flow(flow->iqid, flow->rqid);
        flow->hostns.skbuff = pkt_skb;
        flow->netns.skbuff = NULL;
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
                          hex_dump(flow->hostns.vlan, sizeof(vlan->vlan_tag)));
        } else {
          HAL_TRACE_DEBUG("lklshim: flow hostns vlan not set");
        }
        lklshim_create_listen_sockets(dir, flow);
        lklshim_trigger_flow_connection(flow, dir);
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
                          hex_dump(flow->netns.vlan, sizeof(vlan->vlan_tag)));
        } else {
          HAL_TRACE_DEBUG("lklshim: flow netns vlan not set");
        }
        lklshim_create_listen_sockets(dir, flow);
        lklshim_trigger_flow_connection(flow, dir);
    }
    return true;
}

void lklshim_update_tcpcb(void *tcpcb, uint32_t qid, uint32_t src_lif)
{
    if (tcpcb == NULL) 
        return;
    proxy::tcp_update_cb(tcpcb, qid, src_lif);
}

void lklshim_process_listen(lklshim_flow_t *flow)
{
    void *skb = (flow->hostns.skbuff?flow->hostns.skbuff:flow->netns.skbuff);
    flow->netns.skbuff = flow->hostns.skbuff = NULL;
    if (skb) {
      //lkl_tcp_v4_rcv(skb);
        lkl_kfree_skb(skb);
    }
}


void lklshim_process_tx_packet(unsigned char* pkt,
                               unsigned int len,
                               void* flowp,
                               bool is_connect_req,
                               void *tcpcb,
                               bool tx_pkt) 
{
    if (pkt) {
        HAL_TRACE_DEBUG("lklshim_process_tx_packet len={} pkt={}", len, hex_dump((const uint8_t *)pkt, len));
    } else {
       HAL_TRACE_DEBUG("lklshim_process_tx_packet sending ack");
    }
    if (tcpcb != NULL)  {
        HAL_TRACE_DEBUG("flowp={} is_connect_req={} tcpcb={} tx_pkt={}", flowp, is_connect_req, tcpcb, tx_pkt);
    } else {
        HAL_TRACE_DEBUG("flowp={} is_connect_req={} tcpcb=NULL tx_pkt={}", flowp, is_connect_req, tx_pkt);
    }
    lklshim_flow_t* flow = (lklshim_flow_t*)flowp;
    if (flow) {
        uint32_t qid = (is_connect_req?flow->rqid:flow->iqid);
        HAL_TRACE_DEBUG("flow dst lif={} src lif={}", flow->dst_lif, flow->src_lif);
        lklshim_update_tcpcb(tcpcb, qid, flow->src_lif);
        if (tx_pkt) {
            HAL_TRACE_DEBUG("Calling tcp_transmit_pkt");
            proxy::tcp_transmit_pkt(pkt, len, is_connect_req, flow->dst_lif, flow->src_lif, flow->itor_dir, flow->hw_vlan_id);
        } else {
            HAL_TRACE_DEBUG("Calling tcp_ring_doorbell");
            proxy::tcp_ring_doorbell(qid);
        }
    }
}
 
hal::flow_direction_t
lklshim_get_flow_hit_pkt_direction(uint16_t qid) {
    lklshim_flow_t *flow = lklshim_flow_by_qid[qid];
    if (qid == flow->iqid) {
        HAL_TRACE_DEBUG("itor returning direction={}", flow->itor_dir);
        return flow->itor_dir;
    } else {
        HAL_TRACE_DEBUG("rtor returning direction={}", ((flow->itor_dir==hal::FLOW_DIR_FROM_ENIC)?"hal::FLOW_DIR_FROM_UPLINK":"hal::FLOW_DIR_FROM_ENIC"));
        return ((flow->itor_dir==hal::FLOW_DIR_FROM_ENIC)?hal::FLOW_DIR_FROM_UPLINK:hal::FLOW_DIR_FROM_ENIC);
    }
}

} // namespace hal


