#ifndef _LKLSHIM_HPP_
#define _LKLSHIM_HPP_

#include "nic/include/base.h"
#include "sdk/slab.hpp"
#include "sdk/list.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/hal/src/session.hpp"

#include <netinet/in.h>
#include <memory>
#include <map>

extern "C" {
#include <netinet/in.h>
}
using sdk::lib::ht_ctxt_t;
using sdk::lib::slab;

namespace hal {

#define HAL_MAX_LKLSHIM_FLOWS  (1024 * 8)
#define PACKED __attribute__((__packed__))

typedef enum {
    FLOW_STATE_INVALID = 0,
    FLOW_STATE_SYN_RCVD = 1,
    FLOW_STATE_CONNECT_PENDING = 2,
    FLOW_STATE_ESTABLISHED = 3
} lklshim_flow_state_e;

typedef struct lklshim_flow_key_t_ {
    in_addr_t src_ip;
    in_addr_t dst_ip;
    uint16_t    src_port;
    uint16_t    dst_port;
    //uint16_t    protocol;
} PACKED lklshim_flow_key_t;

#define MAC_SIZE 6
#define VLAN_SIZE 2

typedef struct lklshim_flow_ns_t_ {
    int                  sockfd;
    lklshim_flow_state_e state;
    void                 *skbuff;
    //tcpcb_t              *tcpcb;
    //tlscb_t              *tlscb;
    char                 dev[16];
    uint8_t   src_mac[MAC_SIZE];
    uint8_t   dst_mac[MAC_SIZE];
    uint8_t   vlan[VLAN_SIZE];
} lklshim_flow_ns_t;

typedef struct lklshim_flow_t_ {
    lklshim_flow_key_t      key;
    lklshim_flow_ns_t       hostns;
    lklshim_flow_ns_t       netns;
    /*
     * TLS related parameters for the N-flow.
     */
//    SSL_CTX             *ssl_ctx;
//    SSL                 *ssl;
//    SSL_METHOD          *ssl_meth;

    ht_ctxt_t               ht_ctxt;
    hal::flow_direction_t   itor_dir;
    uint16_t                src_lif;
    uint16_t                dst_lif;
    uint32_t                iqid;
    uint32_t                rqid;
    uint16_t                hw_vlan_id;
} lklshim_flow_t;

typedef struct lklshim_listen_sockets_t_ {
    int       tcp_portnum; // Key
    int       ipv4_sockfd;
    int       ipv6_sockfd;
    ht_ctxt_t ht_ctxt;
} PACKED lklshim_listen_sockets_t;

#define MAX_PROXY_FLOWS 32768

/*
 * Extern definitions.
 */
extern slab *lklshim_flowdb_slab;
extern slab *lklshim_lsockdb_slab;
extern ht *lklshim_flow_db;
extern lklshim_flow_t           *lklshim_flow_by_qid[MAX_PROXY_FLOWS];

static inline void
lklshim_make_flow_v4key (lklshim_flow_key_t *key, in_addr_t src, in_addr_t dst,
			 uint16_t sport, uint16_t dport)
{
    key->src_ip = src;
    key->dst_ip = dst;
    key->src_port = sport;
    key->dst_port = dport;
}

static inline lklshim_flow_t *
lklshim_flow_entry_lookup (lklshim_flow_key_t *key)
{
    lklshim_flow_t *lklshim_flow;

    HAL_ASSERT(key != NULL);
    lklshim_flow =
        (lklshim_flow_t *)lklshim_flow_db->lookup(key);
    return(lklshim_flow);
}

bool lklshim_process_flow_miss_rx_packet (void *pkt_skb, hal::flow_direction_t dir, uint32_t iqid, uint32_t rqid, uint16_t src_lif, uint16_t hw_vlan_id);
bool lklshim_process_flow_hit_rx_packet (void *pkt_skb, hal::flow_direction_t dir, const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr);
bool lklshim_process_flow_hit_rx_header (void *pkt_skb, hal::flow_direction_t dir, const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr);
void lklshim_process_tx_packet(unsigned char* pkt, unsigned int len, void* flow, bool is_connect_req, void *tcpcb, bool tx_pkt);

void lklshim_flowdb_init(void);
void lklshim_update_tcpcb(void *tcpcb, uint32_t qid, uint32_t src_lif);
hal::flow_direction_t lklshim_get_flow_hit_pkt_direction(uint16_t qid);

} //namespace hal

#endif // _LKLSHIM_HPP_
