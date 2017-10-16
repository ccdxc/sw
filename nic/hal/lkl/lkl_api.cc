#include <string>
#include <unistd.h>
#include <getopt.h>
#include "nic/hal/hal.hpp"
#include "nic/hal/lkl/lkl_api.hpp"
#include "nic/hal/lkl/lklshim.hpp"

extern "C" {
#include "nic/third-party/lkl/export/include/lkl.h"
#include "nic/third-party/lkl/export/include/lkl_host.h"
struct lkl_netdev *lkl_register_netdev_fd(int fd);
}

namespace hal {
namespace pd {

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
    ret = lkl_start_kernel(&lkl_host_ops, "mem=16M loglevel=8");
    if (ret) {
        HAL_TRACE_DEBUG("LKL could not be started: %s\n", lkl_strerror(ret));
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
    return HAL_RET_OK;
}

void* lkl_alloc_skbuff(const p4_to_p4plus_cpu_pkt_t* rxhdr, const uint8_t* pkt, size_t pkt_len, hal::flow_direction_t direction) {
    if (rxhdr == NULL) {HAL_TRACE_DEBUG("LKL call!"); return NULL;}
    HAL_TRACE_DEBUG("Allocating SKBUFF\n");
    void *dev = NULL;
    if (direction == hal::FLOW_DIR_FROM_ENIC) {
        dev = host_dev;
    } else {
        dev = net_dev;
    }
    void* skb = lkl_alloc_skb(pkt_len, dev);
    if (skb) {
        lkl_skb_reserve(skb);
        lkl_skb_copy_to_linear_data(skb, (char*)pkt, (unsigned int)pkt_len); 
        lkl_skb_set_mac_header(skb, rxhdr->l2_offset);
        lkl_skb_set_transport_header(skb, rxhdr->l4_offset);
        lkl_skb_set_network_header(skb, rxhdr->l3_offset);
    }
    return skb;
}

  bool lkl_handle_flow_miss_pkt(void* skb, hal::flow_direction_t dir, uint32_t iqid, uint32_t rqid, const p4_to_p4plus_cpu_pkt_t *rxhdr) {
    if (!skb) return false;
    return hal::lklshim_process_flow_miss_rx_packet(skb, dir, iqid, rqid, rxhdr->src_lif);
}

bool lkl_handle_flow_hit_pkt(void* skb, hal::flow_direction_t dir, const p4_to_p4plus_cpu_pkt_t* rxhdr) {
    if (!skb) return false;
    return hal::lklshim_process_flow_hit_rx_packet(skb, dir, rxhdr);
}

uint32_t lkl_get_tcpcb_rcv_nxt(void *tcpcb)
{
    return lkl_tcpcb_rcv_nxt(tcpcb);
}

uint32_t lkl_get_tcpcb_snd_nxt(void *tcpcb)
{
    return lkl_tcpcb_snd_nxt(tcpcb);
}

uint32_t lkl_get_tcpcb_snd_una(void *tcpcb)
{
    return lkl_tcpcb_snd_una(tcpcb);
}

uint32_t lkl_get_tcpcb_rcv_tsval(void *tcpcb) 
{
    return lkl_tcpcb_rcv_tsval(tcpcb);
}

uint32_t lkl_get_tcpcb_ts_recent(void *tcpcb)
{
    return lkl_tcpcb_ts_recent(tcpcb);
}

uint32_t lkl_get_tcpcb_state(void *tcpcb)
{
#if 0
    return lkl_tcpcb_state(tcpcb);
#else
    return 0;
#endif
}

}
}
