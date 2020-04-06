//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// DPDK wrapper library for SDK
//------------------------------------------------------------------------------

#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_log.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include "lib/logger/logger.hpp"
#include "include/sdk/mem.hpp"
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include "dpdk.hpp"

using namespace std;

static dpdk_global_config_t dpdk_config;

#define SDK_DPDK_LOG(...) \
    if (dpdk_config.log_cb) {                                           \
        (*dpdk_config.log_cb) (sdk_mod_id_t::SDK_MOD_ID_SDK,            \
                               ##__VA_ARGS__);                          \
    }

#define SDK_DPDK_LOG_INFO(fmt, _args...)                                \
        SDK_DPDK_LOG(SDK_TRACE_LEVEL_INFO, fmt, ##_args);               \

#define SDK_DPDK_LOG_ERR(fmt, _args...)                                 \
        SDK_DPDK_LOG(SDK_TRACE_LEVEL_ERR, fmt, ##_args);                \

#define SDK_DPDK_LOG_WARN(fmt, _args...)                                \
        SDK_DPDK_LOG(SDK_TRACE_LEVEL_WARN, fmt, ##_args);               \

#define SDK_DPDK_LOG_DEBUG(fmt, _args...)                               \
        SDK_DPDK_LOG(SDK_TRACE_LEVEL_DEBUG, fmt, ##_args);              \

#define SDK_DPDK_LOG_VERBOSE(fmt, _args...)                             \
        SDK_DPDK_LOG(SDK_TRACE_LEVEL_VERBOSE, fmt, ##_args);            \

// if we support per device config, then move it to dpdk_device class in future.
struct rte_eth_conf ethdev_conf = {0};

// if we support per device config, then move it to dpdk_device class in future.
// rx ring configuration
static struct rte_eth_rxconf rx_queue_conf = {0};

// if we support per device config, then move it to dpdk_device class in future.
// tx ring configuration
static struct rte_eth_txconf tx_queue_conf = {0};

static void
init_dev_conf (struct rte_eth_conf *dev)
{
    dev->link_speeds = ETH_LINK_SPEED_AUTONEG;
    dev->rxmode.mq_mode = ETH_MQ_RX_RSS;
    dev->rxmode.offloads = DEV_RX_OFFLOAD_CHECKSUM;
    dev->rx_adv_conf.rss_conf.rss_key= NULL;
    dev->rx_adv_conf.rss_conf.rss_key_len = 0;
    dev->rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IPV4 |
                                       ETH_RSS_NONFRAG_IPV4_TCP |
                                       ETH_RSS_NONFRAG_IPV4_UDP |
                                       ETH_RSS_IPV6 |
                                       ETH_RSS_NONFRAG_IPV6_TCP |
                                       ETH_RSS_NONFRAG_IPV6_UDP;

    dev->txmode.mq_mode            = ETH_MQ_TX_NONE;
    dev->txmode.offloads = DEV_TX_OFFLOAD_UDP_CKSUM | DEV_TX_OFFLOAD_TCP_CKSUM | DEV_TX_OFFLOAD_IPV4_CKSUM;
    dev->txmode.hw_vlan_reject_tagged      = 0;
    dev->txmode.hw_vlan_insert_pvid        = 0;
    dev->fdir_conf.mode = RTE_FDIR_MODE_NONE;
    dev->fdir_conf.status = RTE_FDIR_NO_REPORT_STATUS;

    dev->intr_conf.lsc = 0; //Our driver doesn't support this
}

static void
init_rx_queue_conf (struct rte_eth_rxconf *conf)
{
    conf->rx_thresh.pthresh = 8;   // Ring prefetch threshold
    conf->rx_thresh.hthresh = 8;   // Ring host threshold
    conf->rx_thresh.wthresh = 4;   // Ring writeback threshold

    conf->rx_free_thresh = 0;
    conf->offloads = 0;
}

static void
init_tx_queue_conf (struct rte_eth_txconf *conf)
{
    conf->tx_thresh .pthresh = 32;  // Ring prefetch threshold
    conf->tx_thresh .hthresh = 0;   // Ring host threshold
    conf->tx_thresh .wthresh = 0;   // Ring writeback threshold
    conf->offloads  = DEV_TX_OFFLOAD_UDP_CKSUM | DEV_TX_OFFLOAD_TCP_CKSUM | DEV_TX_OFFLOAD_IPV4_CKSUM;
    conf->tx_free_thresh = 32;
    conf->tx_rs_thresh = 32;      // Use PMD default values
}

sdk_ret_t
sdk::lib::dpdk::dpdk_init (sdk_dpdk_params_t *args)
{
    int ret;
    char **dpdk_argv = NULL;
    int args_count = 0;
    vector <string> tokens;
    string intermediate;
    sdk_ret_t ret_val = SDK_RET_OK;
    int port_id;
    int ionicpmd_logtype;
    stringstream eal_list;

    if (!args || args->log_name.empty() || !args->log_cb ||
        args->mbuf_pool_name.empty()) {
        ret_val = SDK_RET_INVALID_ARG;
        goto end;
    }

    dpdk_config.log_cb = args->log_cb;
    eal_list << args->eal_init_list;
    while(getline(eal_list, intermediate, ' ')) {
        tokens.push_back(intermediate);
    }

    for(uint16_t i = 0; i < args->vdev_list.size(); i++) {
        tokens.push_back(string("--vdev"));
        tokens.push_back(args->vdev_list[i]);
    }
    args_count = tokens.size();
    dpdk_argv = (char **)calloc(sizeof(char *), args_count);
    for (int i = 0; i < args_count; i++) {
        dpdk_argv[i] = (char *) tokens[i].c_str();
    }

    ret = rte_eal_init(args_count, dpdk_argv);
    if (ret < 0) {
        ret_val = SDK_RET_ERR;
        goto end;
    }

    ionicpmd_logtype = rte_log_register(args->log_name.c_str());
    if (ionicpmd_logtype < 0) {
        ret_val = SDK_RET_ERR;
        goto end;
    }
    rte_log_set_level(ionicpmd_logtype, RTE_LOG_DEBUG);

    // TODO : getting DPDK logs to our callback.

    // create pools for mbuf
    dpdk_config.rte_mp = rte_pktmbuf_pool_create(args->mbuf_pool_name.c_str(),
                                                 args->num_mbuf,
                                                 250,0,
                                                 args->mbuf_size + sizeof(struct rte_mbuf),
                                                 0);
    if (dpdk_config.rte_mp == NULL) {
        ret_val = SDK_RET_ERR;
        goto end;
    }

    RTE_ETH_FOREACH_DEV(port_id) {
        struct rte_eth_dev_info dev_info;

        if (!rte_eth_dev_is_valid_port(port_id)) {
            continue;
        }

        rte_eth_dev_info_get (port_id, &dev_info);
        dpdk_config.name_to_port_map[string(dev_info.device->name)] = port_id;
    }

    init_dev_conf(&ethdev_conf);
    init_tx_queue_conf(&tx_queue_conf);
    init_rx_queue_conf(&rx_queue_conf);

end:
    if (dpdk_argv) {
        free(dpdk_argv);
        dpdk_argv = NULL;
    }
    return ret_val;
}

dpdk_device::dpdk_device() {
    rx_queues = DEFAULT_RX_QUEUES;
    tx_queues = DEFAULT_TX_QUEUES;
    tx_desc = DEFAULT_TX_DESC;
    rx_desc = DEFAULT_RX_DESC;
    portid = 0;
    return;
}

sdk_ret_t
dpdk_device::init_(sdk_dpdk_device_params_t *args) {
    if (args->dev_name.empty() ||
        (dpdk_config.name_to_port_map.find(args->dev_name) ==
         dpdk_config.name_to_port_map.end())) {
        return SDK_RET_INVALID_ARG;
    }
    if (args->num_tx_queue) {
        tx_queues = args->num_tx_queue;
    }
    if (args->num_rx_queue) {
        rx_queues = args->num_rx_queue;
    }
    if (args->num_tx_desc) {
        tx_desc = args->num_tx_desc;
    }
    if (args->num_rx_desc) {
        rx_desc = args->num_rx_desc;
    }
    portid = dpdk_config.name_to_port_map[args->dev_name];

    int ret = rte_eth_dev_configure(portid, rx_queues, tx_queues, &ethdev_conf);
    SDK_ASSERT(ret == 0);

    // Queue setup (Rx/Tx) and start port
    for (int i = 0; i < tx_queues; i++) {
        ret = rte_eth_tx_queue_setup(portid, i, tx_desc, 0, &tx_queue_conf);
        SDK_ASSERT(ret == 0);
    }

    for (int i = 0; i < rx_queues; i++) {
        ret = rte_eth_rx_queue_setup(portid, i, rx_desc, 0, &rx_queue_conf,
                                     dpdk_config.rte_mp);
        SDK_ASSERT(ret == 0);
    }
    ret = rte_eth_dev_start(portid);
    SDK_ASSERT(ret == 0);

    char *buf_ptr = (char *) SDK_CALLOC(SDK_MEM_ALLOC_LIB_DPDK_BUFFER,
                                        (sizeof(dpdk_mbuf *) *
                                        DPDK_MAX_BURST_SIZE * rx_queues));
    for (int i = 0; i < rx_queues; i++) {
        dpdk_mbuf **buf = (dpdk_mbuf **) (buf_ptr + (sizeof(dpdk_mbuf *) *
                          DPDK_MAX_BURST_SIZE * i));
        mbufs.push_back(buf);
    }
    return SDK_RET_OK;
}

dpdk_device::~dpdk_device(void) {
    SDK_FREE(SDK_MEM_ALLOC_LIB_DPDK_BUFFER, (void *)mbufs[0]);
}

dpdk_device *
dpdk_device::factory(sdk_dpdk_device_params_t *args) {
    dpdk_device *obj = NULL;
    void *dev = NULL;
    sdk_ret_t ret;

    dev = (dpdk_device *) SDK_CALLOC(SDK_MEM_ALLOC_LIB_DPDK_DEVICE,
                                     sizeof(dpdk_device));
    if (dev) {
        obj = new (dev) dpdk_device();
        ret = obj->init_(args);
        if (ret != SDK_RET_OK) {
            obj->~dpdk_device();
            SDK_FREE(SDK_MEM_ALLOC_LIB_DPDK_DEVICE, obj);
            obj = NULL;
        }
    } else {
        ret = SDK_RET_OOM;
    }
    SDK_DPDK_LOG_INFO("Factory ret[%d]", ret);
    return obj;
}

void
dpdk_device::destroy(dpdk_device *dev) {
    dev->~dpdk_device();
    SDK_FREE(SDK_MEM_ALLOC_LIB_DPDK_DEVICE, dev);
}

char *
dpdk_device::remove_header(dpdk_mbuf *packet, uint16_t len) {
    return rte_pktmbuf_adj((struct rte_mbuf *)packet, len);
}

char *
dpdk_device::add_header(dpdk_mbuf *packet, uint16_t len) {
    return rte_pktmbuf_prepend((struct rte_mbuf *)packet, len);
}

char *
dpdk_device::get_data_ptr(dpdk_mbuf *packet) {
    return rte_pktmbuf_mtod((struct rte_mbuf *)packet, char *);
}

char *
dpdk_device::append_data(dpdk_mbuf *packet, uint16_t len) {
    return rte_pktmbuf_append((struct rte_mbuf *)packet, len);
}

dpdk_mbuf **
dpdk_device::receive_packets(uint16_t rx_queue_id, uint16_t max_packets,
                             uint16_t *recv_count) {
    int n_rx_packets = 0, n;
    struct rte_mbuf **rx_pkts = (struct rte_mbuf**)mbufs[rx_queue_id];
    max_packets = (max_packets > DPDK_MAX_BURST_SIZE) ?
                  DPDK_MAX_BURST_SIZE : max_packets;

    while (n_rx_packets < max_packets) {
        n = rte_eth_rx_burst(portid, rx_queue_id,
                             rx_pkts + n_rx_packets,
                             max_packets - n_rx_packets);
        n_rx_packets += n;

        if (n < 32) {
            // this means no more packets
            break;
        }
    }

    *recv_count = n_rx_packets;
    return ((*recv_count) ? (dpdk_mbuf **)rx_pkts : NULL);
}

uint16_t
dpdk_device::transmit_packets(uint16_t tx_queue_id, dpdk_mbuf **packets,
                              uint16_t n_left) {
    int n_sent = 0;
    int n_retry = 16;
    struct rte_mbuf **tx_pkts = (struct rte_mbuf**)packets;

    do {
        n_sent = rte_eth_tx_burst(portid, tx_queue_id, tx_pkts, n_left);
        n_left -= n_sent;
        tx_pkts += n_sent;
    } while(n_left && (--n_retry > 0));

    return n_left;
}

void
dpdk_device::drop_packets(dpdk_mbuf **packets, uint16_t num_packets) {
    struct rte_mbuf **pkts = (struct rte_mbuf**)packets;

    for (int i = 0; i < num_packets; i++) {
        rte_pktmbuf_free((struct rte_mbuf *)pkts[i]);
    }
}

dpdk_mbuf *
dpdk_device::alloc_mbuf(void) {
    return (dpdk_mbuf *) rte_pktmbuf_alloc(dpdk_config.rte_mp);
}
