#include <string>
#include <unistd.h>
#include <getopt.h>
#include "nic/hal/hal.hpp"
#include "nic/hal/lkl/lklshim.hpp"
#include "nic/hal/lkl/lkl_api.hpp"

extern "C" {
#include "lkl.h"
#include "lkl_host.h"
struct lkl_netdev *lkl_register_netdev_fd(int fd);
}

namespace hal {
namespace pd {

static void lkl_printk(const char *str, int len)
{
    char* mystr = (char*) str;
    mystr[len-1]='\0';
    HAL_TRACE_DEBUG("{}", str);
}

void *host_ns, *net_ns, *host_dev, *net_dev;
int lkl_init(void) {
    int ret;
    //struct lkl_netdev *nd = NULL;
    HAL_TRACE_DEBUG("Starting LKL\n");
    //nd = lkl_netdev_raw_create("lkleth");
    ret = lkl_netdev_add(lkl_register_netdev_fd(999), NULL);
    ret = lkl_netdev_add(lkl_register_netdev_fd(998), NULL);
    if (ret < 0) {
	printf("failed to add netdev: %s\n",lkl_strerror(ret));
    }
    lkl_host_ops.print = lkl_printk;
    ret = lkl_start_kernel(&lkl_host_ops, "mem=16M loglevel=8");
    if (ret) {
        HAL_TRACE_DEBUG("LKL could not be started: {}", lkl_strerror(ret));
        return HAL_RET_ERR;
    }
    host_dev = lkl_dev_get_by_name("eth0");
    net_dev = lkl_dev_get_by_name("eth1");
    host_ns = lkl_create_net_ns();
    net_ns = lkl_create_net_ns();
    lkl_dev_net_set(host_dev, host_ns);
    lkl_dev_net_set(net_dev, net_ns);

    lkl_skb_init();
    hal::lklshim_flowdb_init();
    lkl_register_tx_func((void*)hal::lklshim_process_tx_packet);
    lkl_register_tcpcb_update_func((void *)hal::lklshim_update_tcpcb);
    return HAL_RET_OK;
}

void* lkl_alloc_skbuff(const p4_to_p4plus_cpu_pkt_t* rxhdr, const uint8_t* pkt, size_t pkt_len, hal::flow_direction_t direction) {
    if (rxhdr == NULL) {
        HAL_TRACE_DEBUG("LKL call!");
        return NULL;
    }
    HAL_TRACE_DEBUG("Allocating SKBUFF direction {} host_dev {} net_dev {}\n",
                           (direction==hal::FLOW_DIR_FROM_DMA)?"from host":"from net", host_dev, net_dev);
    void *dev = NULL;
    bool is_pkt_src_uplink = FALSE;
    uint32_t pktlen, tpt_offset, nw_offset;
    if (direction == hal::FLOW_DIR_FROM_DMA) {
        dev = host_dev;
    } else {
        dev = net_dev;
        is_pkt_src_uplink = TRUE;
    }
    void* skb;
    bool is_v4_pkt = ((rxhdr->lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4) ||
                       (lklshim_flow_by_qid[rxhdr->qid] && lklshim_flow_by_qid[rxhdr->qid]->key.type == hal::FLOW_TYPE_V4));
    if (pkt_len == 0) {
        if (is_v4_pkt) {
            HAL_TRACE_DEBUG("lklshim: v4 pkt");
            pktlen = 40;
            tpt_offset = 20;
            nw_offset = 0;
        } else {
            HAL_TRACE_DEBUG("lklshim: v6 pkt");
            pktlen = 60;
            tpt_offset = 40;
            nw_offset = 0;
        }
    }

    if (pkt_len == 0) {
        skb = lkl_alloc_skb((void*)pkt, pktlen, dev, is_pkt_src_uplink);
        HAL_TRACE_DEBUG("lkl_alloc_skbuff: Setting skb len to {} dev={} skb={}", pktlen, dev, skb);
    } else {
        skb = lkl_alloc_skb((void*)pkt, pkt_len, dev, is_pkt_src_uplink);
        HAL_TRACE_DEBUG("lkl_alloc_skbuff: Setting skb len={} dev={} skb={}", pkt_len, dev, skb);
    }
    if (skb) {
        lkl_skb_reserve(skb);
        lkl_skb_set_mac_header(skb, rxhdr->l2_offset);
        HAL_TRACE_DEBUG("lkl_alloc_skbuff : l3 offset = {} l4 offset = {}",
                        rxhdr->l3_offset, rxhdr->l4_offset);
        if (rxhdr->l4_offset == -1) {
            lkl_skb_set_transport_header(skb, tpt_offset);
            HAL_TRACE_DEBUG("lkl_alloc_skbuff: setting transport header offset {}", tpt_offset);
        } else {
            lkl_skb_set_transport_header(skb, rxhdr->l4_offset);
            HAL_TRACE_DEBUG("lkl_alloc_skbuff: setting transport header offset={}", rxhdr->l4_offset);
        }
        if (rxhdr->l3_offset == -1) {
            lkl_skb_set_network_header(skb, nw_offset);
            HAL_TRACE_DEBUG("lkl_alloc_skbuff: setting network header offset 0");
        } else {
            lkl_skb_set_network_header(skb, rxhdr->l3_offset);
            HAL_TRACE_DEBUG("lkl_alloc_skbuff: setting network header offset={}", rxhdr->l3_offset);
        }
    }
    return skb;
}

bool lkl_handle_flow_miss_pkt(void* skb, hal::flow_direction_t dir,
                              uint32_t iqid, uint32_t rqid,
                              const p4_to_p4plus_cpu_pkt_t* rxhdr,
                              proxy_flow_info_t* pfi,
                              hal::lklshim_flow_encap_t *flow_encap) {
    if (!skb) return false;
    if (rxhdr->lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4)
        return hal::lklshim_process_flow_miss_rx_packet(skb, dir, iqid, rqid, pfi, flow_encap);
    return hal::lklshim_process_v6_flow_miss_rx_packet(skb, dir, iqid, rqid, pfi, flow_encap);
}

bool lkl_handle_flow_hit_pkt(void* skb, hal::flow_direction_t dir, const p4_to_p4plus_cpu_pkt_t* rxhdr) {
    if (!skb) return false;
    if (rxhdr->lkp_type == FLOW_KEY_LOOKUP_TYPE_IPV4)
        return hal::lklshim_process_flow_hit_rx_packet(skb, dir, rxhdr);
    return hal::lklshim_process_v6_flow_hit_rx_packet(skb, dir, rxhdr);
}

bool lkl_handle_flow_hit_hdr(void* skb, hal::flow_direction_t dir, const p4_to_p4plus_cpu_pkt_t* rxhdr) {
    if (!skb) return false;
    if (lklshim_flow_by_qid[rxhdr->qid]->key.type == hal::FLOW_TYPE_V4)
        return hal::lklshim_process_flow_hit_rx_header(skb, dir, rxhdr);
    return hal::lklshim_process_v6_flow_hit_rx_header(skb, dir, rxhdr);
}

uint32_t lkl_get_tcpcb_rcv_nxt(void *tcpcb)
{
    HAL_TRACE_DEBUG("lkl_get_tcpcb_rcv_nxt : tcpcb = {}", tcpcb);
    return lkl_tcpcb_rcv_nxt(tcpcb);
}

uint32_t lkl_get_tcpcb_snd_nxt(void *tcpcb)
{
    HAL_TRACE_DEBUG("lkl_get_tcpcb_snd_nxt : tcpcb = {}", tcpcb);
    return lkl_tcpcb_snd_nxt(tcpcb);
}

uint32_t lkl_get_tcpcb_snd_una(void *tcpcb)
{
    HAL_TRACE_DEBUG("lkl_get_tcpcb_snd_una : tcpcb = {}", tcpcb);
    return lkl_tcpcb_snd_una(tcpcb);
}

uint32_t lkl_get_tcpcb_rcv_tsval(void *tcpcb)
{
    HAL_TRACE_DEBUG("lkl_get_tcpcb_rcv_tsval : tcpcb = {}", tcpcb);
    return lkl_tcpcb_rcv_tsval(tcpcb);
}

uint32_t lkl_get_tcpcb_ts_recent(void *tcpcb)
{
    HAL_TRACE_DEBUG("lkl_get_tcpcb_ts_recent : tcpcb = {}", tcpcb);
    return lkl_tcpcb_ts_recent(tcpcb);
}

uint32_t lkl_get_tcpcb_state(void *tcpcb)
{
    return lkl_tcpcb_state(tcpcb);
}

}
}
