//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains FTE core functionality
///
//----------------------------------------------------------------------------
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <vector>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/athena/ftl_pollers_client.hpp"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "app_test_utils.hpp"
#include "fte_athena.hpp"
#include "athena_app_server.hpp"

namespace fte_ath {

uint8_t g_athena_app_mode = ATHENA_APP_MODE_CPP;

char const * g_eal_args[] = {"fte",
                             "-l", "1,2,3",
                             "--vdev=net_ionic0",
                             "--vdev=net_ionic1"};
#define NELEMS(_a) (sizeof(_a)/sizeof(_a[0]))

#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT 16
#define MAX_RX_QUEUE_PER_PORT 128
#define MEMPOOL_CACHE_SIZE 256

#define MAX_PKT_BURST 32
#define BURST_TX_DRAIN_US 100 // TX drain every ~100us
#define MAX_ETHPORTS 16
#define NB_SOCKETS 8

#define FTE_MAX_CORES 4 // Max Cores
#define FTE_MAX_TXDSCR 256 // Max TX Descriptors
#define FTE_MAX_RXDSCR 256 // Max RX Descriptors
#define FTE_PREFETCH_NLINES 7

const static enum rte_rmt_call_master_t fte_call_master_type = SKIP_MASTER;
static uint16_t nb_rxd = FTE_MAX_RXDSCR;
static uint16_t nb_txd = FTE_MAX_TXDSCR;

typedef std::vector<uint32_t> pollers_qid_vec_t;
static pollers_qid_vec_t pollers_qid_conf[FTE_MAX_CORES];
static rte_atomic32_t pollers_lcore_idx = RTE_ATOMIC32_INIT(-1);

// Ports set in promiscuous mode off by default.
static int promiscuous_on;

// NUMA is enabled by default.
static int numa_on = 1;

struct mbuf_table {
        uint16_t len;
        struct rte_mbuf *m_table[MAX_PKT_BURST];
};

struct lcore_rx_queue {
        uint16_t port_id;
        uint8_t queue_id;
} __rte_cache_aligned;

struct lcore_conf {
        uint16_t n_rx_queue;
        struct lcore_rx_queue rx_queue_list[MAX_RX_QUEUE_PER_LCORE];
        uint16_t n_tx_port;
        uint16_t tx_port_id[MAX_ETHPORTS];
        uint16_t tx_queue_id[MAX_ETHPORTS];
        struct mbuf_table tx_mbufs[MAX_ETHPORTS];
} __rte_cache_aligned;

struct lcore_conf lcore_conf[FTE_MAX_CORES];

struct lcore_params {
    uint16_t port_id;
    uint8_t queue_id;
    uint8_t lcore_id;
} __rte_cache_aligned;

static struct lcore_params lcore_params_arr_default[] = {
    {0, 0, 2},
    {1, 0, 3},
};

static struct lcore_params * lcore_params = lcore_params_arr_default;
static uint16_t nb_lcore_params = sizeof(lcore_params_arr_default) /
                                sizeof(lcore_params_arr_default[0]);

static struct rte_mempool * pktmbuf_pool[NB_SOCKETS];

pds_flow_stats_t flow_stats[FTE_MAX_CORES];
pds_flow_stats_t g_flow_stats;
static bool fte_threads_started;
static bool fte_threads_done;

// Send burst of packets on an output interface 
static inline void
send_burst (struct lcore_conf *qconf, uint16_t n, uint16_t port)
{
    struct rte_mbuf **m_table;
    int ret;
    uint16_t queueid;

    queueid = qconf->tx_queue_id[port];
    m_table = (struct rte_mbuf **)qconf->tx_mbufs[port].m_table;

    ret = rte_eth_tx_burst(port, queueid, m_table, n);
    if (unlikely(ret < n)) {
        do {
            rte_pktmbuf_free(m_table[ret]);
        } while (++ret < n);
    }

    return;
}

// Enqueue a single packet, and send burst if queue is filled 
static inline void
send_single_packet (struct lcore_conf *qconf,
                    struct rte_mbuf *m, uint16_t port)
{
    uint16_t len;

    len = qconf->tx_mbufs[port].len;
    qconf->tx_mbufs[port].m_table[len] = m;
    len++;

    if (unlikely(len == MAX_PKT_BURST)) {
        send_burst(qconf, MAX_PKT_BURST, port);
        len = 0;
    }

    qconf->tx_mbufs[port].len = len;
    return;
}

static void
_process (struct rte_mbuf *m, struct lcore_conf *qconf,
          uint16_t portid)
{
    uint16_t dst_port;

    if (!skip_fte_flow_prog() &&
        (g_athena_app_mode == ATHENA_APP_MODE_CPP)) {
        if (fte_flow_prog(m) != SDK_RET_OK) {
            PDS_TRACE_DEBUG("fte_flow_prog failed..\n");
            // TODO: Unsupported traffic should be dropped?
            rte_pktmbuf_free(m);
            return;
        }
    }

    dst_port = (portid ? 0 : 1);
    send_single_packet(qconf, m, dst_port);
    return;
}

// main processing loop
static void
fte_rx_loop (pollers_qid_vec_t *qid_vec)
{
    struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
    unsigned lcore_id;
    uint64_t prev_tsc, diff_tsc, cur_tsc;
    int i, nb_rx;
    uint8_t queueid;
    uint16_t portid;
    struct lcore_conf *qconf;
    const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) /
                               US_PER_S * BURST_TX_DRAIN_US;

    prev_tsc = 0;

    lcore_id = rte_lcore_id();
    qconf = &lcore_conf[lcore_id];

    if (qconf->n_rx_queue == 0) {
        PDS_TRACE_DEBUG("lcore %u has nothing to do\n", lcore_id);
        return;
    }

    PDS_TRACE_DEBUG("Entering main loop on lcore %u\n", lcore_id);

    for (i = 0; i < qconf->n_rx_queue; i++) {
        portid = qconf->rx_queue_list[i].port_id;
        queueid = qconf->rx_queue_list[i].queue_id;
        PDS_TRACE_DEBUG(" -- lcoreid=%u portid=%u rxqueueid=%hhu\n",
                        lcore_id, portid, queueid);
    }

    while (!fte_threads_done) {
        if (qid_vec && !ftl_pollers_client::user_will_poll()) {
            for (size_t q = 0; q < qid_vec->size(); q++) {
                pds_flow_age_sw_pollers_poll(qid_vec->at(q), nullptr);
            }
        }
        cur_tsc = rte_rdtsc();

        /*
         * TX burst queue drain
         */
        diff_tsc = cur_tsc - prev_tsc;
        if (unlikely(diff_tsc > drain_tsc)) {
            for (i = 0; i < qconf->n_tx_port; ++i) {
                portid = qconf->tx_port_id[i];
                if (qconf->tx_mbufs[portid].len == 0) {
                    continue;
                }
                send_burst(qconf, qconf->tx_mbufs[portid].len,
                           portid);
                qconf->tx_mbufs[portid].len = 0;
            }
            prev_tsc = cur_tsc;
        }

        /*
         * Read packet from RX queues
         */
        for (i = 0; !fte_threads_done && (i < qconf->n_rx_queue); ++i) {
            portid = qconf->rx_queue_list[i].port_id;
            queueid = qconf->rx_queue_list[i].queue_id;
            nb_rx = rte_eth_rx_burst(portid, queueid, pkts_burst,
                                     MAX_PKT_BURST);
            if (nb_rx == 0) {
                continue;
            }

            PDS_TRACE_DEBUG("Core #%u received %d pkt(s) from "
                            "port:%u queue:%u \n",
                            lcore_id, nb_rx, portid, queueid);
            for (i = 0; !fte_threads_done && (i < nb_rx); i++) {
                auto m = pkts_burst[i];
#ifdef DEBUG
                pkt_hex_dump_trace("PKT:",
                        rte_pktmbuf_mtod(m, char*), 
                        rte_pktmbuf_pkt_len(m));
#endif
                if ((i+1) < nb_rx) {
                    auto m2 = pkts_burst[i+1];
                    uint8_t *d2 = rte_pktmbuf_mtod(m2, uint8_t*);
                    for (int i = 0; i < FTE_PREFETCH_NLINES; i++) {
                        rte_prefetch0(d2+i*64);
                    }    
                }
                _process(m, qconf, portid);
            }
        }
    }
    return;
}

static int
fte_launch_one_lcore (__attribute__((unused)) void *dummy)
{
    pollers_qid_vec_t *qid_vec;
    uint32_t lcore_id;
    int lcore_idx;

    lcore_id = rte_lcore_id();
    if (g_athena_app_mode == ATHENA_APP_MODE_CPP) {
        fte_thread_init(lcore_id);
    }

    lcore_idx = rte_atomic32_add_return(&pollers_lcore_idx, 1);
    qid_vec = lcore_idx < FTE_MAX_CORES ? 
              &pollers_qid_conf[lcore_idx] : nullptr;
    PDS_TRACE_DEBUG("Launching fte_rx_loop on core_id %u(#%d) with %d "
                    "poller queues", lcore_id, lcore_idx,
                    qid_vec ? (int)qid_vec->size() : 0);
    fte_rx_loop(qid_vec);
    return 0;
}

static void
_init_pollers_client (void)
{
    uint32_t lcore_count;
    uint32_t total_qcount;
    uint32_t per_lcore_qcount;
    uint32_t qid;

    lcore_count = std::min(rte_lcore_count(), (uint32_t)FTE_MAX_CORES);
    if (lcore_count && (fte_call_master_type == SKIP_MASTER)) {
        lcore_count--;
    }
    total_qcount = ftl_pollers_client::qcount_get();
    PDS_TRACE_DEBUG("lcore_count: %u pollers_qcount: %u",
                    lcore_count, total_qcount);
    if (lcore_count) {
        per_lcore_qcount = std::max(total_qcount / lcore_count, (uint32_t)1);
        qid = 0;
        for (uint32_t lc = 0; lc < lcore_count; lc++) {

            /*
             * Last lcore gets all the remaining queues.
             */
            if (lc == (lcore_count - 1)) {
                per_lcore_qcount = total_qcount - qid;
            }
            for (uint32_t q = 0; q < per_lcore_qcount; q++) {
                if (qid >= total_qcount) {
                    break;
                }
                pollers_qid_conf[lc].push_back(qid);
                qid++;
            }
        }
    }
}

void
dump_single_flow(pds_flow_iter_cb_arg_t *iter_cb_arg)
{
    pds_flow_key_t *key = &iter_cb_arg->flow_key;
    pds_flow_data_t *data = &iter_cb_arg->flow_appdata;

    if (key->key_type == KEY_TYPE_IPV6) {
        PDS_TRACE_DEBUG("SrcIP:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x "
                        "DstIP:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x "
                        "Dport:%u Sport:%u Proto:%u "
                        "Ktype:%u VNICID:%u "
                        "index:%u index_type:%u\n\n",
                        key->ip_saddr[0], key->ip_saddr[1], key->ip_saddr[2], key->ip_saddr[3],
                        key->ip_saddr[4], key->ip_saddr[5], key->ip_saddr[6], key->ip_saddr[7],
                        key->ip_saddr[8], key->ip_saddr[9], key->ip_saddr[10], key->ip_saddr[11],
                        key->ip_saddr[12], key->ip_saddr[13], key->ip_saddr[14], key->ip_saddr[15],
                        key->ip_daddr[0], key->ip_daddr[1], key->ip_daddr[2], key->ip_daddr[3],
                        key->ip_daddr[4], key->ip_daddr[5], key->ip_daddr[6], key->ip_daddr[7],
                        key->ip_daddr[8], key->ip_daddr[9], key->ip_daddr[10], key->ip_daddr[11],
                        key->ip_daddr[12], key->ip_daddr[13], key->ip_daddr[14], key->ip_daddr[15],
                        key->l4.tcp_udp.dport, key->l4.tcp_udp.sport,
                        key->ip_proto, (uint8_t)key->key_type, key->vnic_id,
                        data->index, (uint8_t)data->index_type);
    } else {
        PDS_TRACE_DEBUG("SrcIP:%02x:%02x:%02x:%02x "
                        "DstIP:%02x:%02x:%02x:%02x "
                        "Dport:%u Sport:%u Proto:%u "
                        "Ktype:%u VNICID:%u "
                        "index:%u index_type:%u\n\n",
                        key->ip_saddr[0], key->ip_saddr[1], key->ip_saddr[2], key->ip_saddr[3],
                        key->ip_daddr[0], key->ip_daddr[1], key->ip_daddr[2], key->ip_daddr[3],
                        key->l4.tcp_udp.dport, key->l4.tcp_udp.sport,
                        key->ip_proto, (uint8_t)key->key_type, key->vnic_id,
                        data->index, (uint8_t)data->index_type);
    }
    return;
}

void
dump_flows(void)
{
    pds_flow_iter_cb_arg_t iter_cb_arg = { 0 };

    PDS_TRACE_DEBUG("\nPrinting Flow cache flows\n");
    pds_flow_cache_entry_iterate(dump_single_flow, &iter_cb_arg);
}

static void
dump_stats (pds_flow_stats_t *stats)
{
    PDS_TRACE_DEBUG("\nPrinting Flow cache statistics\n");
    PDS_TRACE_DEBUG("Insert %lu, Insert_fail_dupl %lu, Insert_fail %lu, "
                    "Insert_fail_recirc %lu\n"
                    "Remove %lu, Remove_not_found %lu, Remove_fail %lu\n"
                    "Update %lu, Update_fail %lu\n"
                    "Get %lu, Get_fail %lu\n"
                    "Reserve %lu, reserve_fail %lu\n"
                    "Release %lu, Release_fail %lu\n"
                    "Tbl_entries %lu, Tbl_collision %lu\n"
                    "Tbl_insert %lu, Tbl_remove %lu, Tbl_read %lu, Tbl_write %lu\n",
                    stats->api_insert,
                    stats->api_insert_duplicate,
                    stats->api_insert_fail,
                    stats->api_insert_recirc_fail,
                    stats->api_remove,
                    stats->api_remove_not_found,
                    stats->api_remove_fail,
                    stats->api_update,
                    stats->api_update_fail,
                    stats->api_get,
                    stats->api_get_fail,
                    stats->api_reserve,
                    stats->api_reserve_fail,
                    stats->api_release,
                    stats->api_release_fail,
                    stats->table_entries,
                    stats->table_collisions,
                    stats->table_insert,
                    stats->table_remove,
                    stats->table_read,
                    stats->table_write);
    for (int i= 0; i < PDS_FLOW_TABLE_MAX_RECIRC; i++) {
         PDS_TRACE_DEBUG("Tbl_lvl %u, Tbl_insert %lu, Tbl_remove %lu\n",
                         i, stats->table_insert_lvl[i],
                         stats->table_remove_lvl[i]);
    }
    return;
}

void
accumulate_stats (pds_flow_stats_t *stats)
{
    g_flow_stats.api_insert += stats->api_insert;
    g_flow_stats.api_insert_duplicate += stats->api_insert_duplicate;
    g_flow_stats.api_insert_fail += stats->api_insert_fail;
    g_flow_stats.api_insert_recirc_fail += stats->api_insert_recirc_fail;
    g_flow_stats.api_remove += stats->api_remove;
    g_flow_stats.api_remove_not_found += stats->api_remove_not_found;
    g_flow_stats.api_remove_fail += stats->api_remove_fail;
    g_flow_stats.api_update += stats->api_update;
    g_flow_stats.api_update_fail += stats->api_update_fail;
    g_flow_stats.api_get += stats->api_get;
    g_flow_stats.api_get_fail += stats->api_get_fail;
    g_flow_stats.api_reserve += stats->api_reserve;
    g_flow_stats.api_reserve_fail += stats->api_reserve_fail;
    g_flow_stats.api_release += stats->api_release;
    g_flow_stats.api_release_fail += stats->api_release_fail;

    g_flow_stats.table_entries += stats->table_entries;
    g_flow_stats.table_collisions += stats->table_collisions;
    g_flow_stats.table_insert += stats->table_insert;
    g_flow_stats.table_remove += stats->table_remove;
    g_flow_stats.table_read += stats->table_read;
    g_flow_stats.table_write += stats->table_write;
    for (int i = 0; i < PDS_FLOW_TABLE_MAX_RECIRC; i++) {
         g_flow_stats.table_insert_lvl[i] += stats->table_insert_lvl[i];
         g_flow_stats.table_remove_lvl[i] += stats->table_remove_lvl[i];
    }
    return;
}

pds_ret_t
fte_dump_flows(zmq_msg_t *rx_msg,
               zmq_msg_t *tx_msg)
{
    dump_flows();

    if (tx_msg) {
        SERVER_RSP_INIT(tx_msg, rsp, test::athena_app::server_rsp_t);
    }
    return PDS_RET_OK;
}

pds_ret_t
fte_dump_flow_stats(zmq_msg_t *rx_msg,
                    zmq_msg_t *tx_msg)
{
    memset(&g_flow_stats, 0, sizeof(pds_flow_stats_t));
    for (int i = 0; i < FTE_MAX_CORES; i++) {
         memset(&flow_stats[i], 0, sizeof(pds_flow_stats_t));
         if (pds_flow_cache_stats_get(i, &flow_stats[i])
             == PDS_RET_OK) {
             accumulate_stats(&flow_stats[i]);
         } else {
             PDS_TRACE_ERR("Stats get failed for core#%u\n", i);
         }
    }
    dump_stats(&g_flow_stats);

    if (tx_msg) {
        SERVER_RSP_INIT(tx_msg, rsp, test::athena_app::server_rsp_t);
    }
    return PDS_RET_OK;
}

static void
signal_handler (int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        PDS_TRACE_DEBUG("\nSIGNAL %d received..core#:%u\n",
                        signum, rte_lcore_id());
        program_prepare_exit();
    }
    if (signum == SIGUSR1) {
        PDS_TRACE_DEBUG("\nSIGNAL %d received..\n",
                        signum);
        fte_dump_flows();
    }
    if (signum == SIGUSR2) {
        PDS_TRACE_DEBUG("\nSIGNAL %d received..\n",
                        signum);
        fte_dump_flow_stats();
    }
}

static int
check_lcore_params (void)
{
    uint8_t queue, lcore;
    uint16_t i;
    int socketid;

    for (i = 0; i < nb_lcore_params; ++i) {
        queue = lcore_params[i].queue_id;
        if (queue >= MAX_RX_QUEUE_PER_PORT) {
            printf("invalid queue number: %hhu\n", queue);
            return -1;
        }
        lcore = lcore_params[i].lcore_id;
        if (!rte_lcore_is_enabled(lcore)) {
            printf("error: lcore %hhu is not enabled in lcore mask\n",
                   lcore);
            return -1;
        }
        if ((socketid = rte_lcore_to_socket_id(lcore) != 0) &&
            (numa_on == 0)) {
            printf("warning: lcore %hhu is on socket %d with "
                   "numa off \n", lcore, socketid);
        }
    }
    return 0;
}

static int
init_lcore_conf_rx_queues (void)
{
    uint16_t i, nb_rx_queue;
    uint8_t lcore;

    for (i = 0; i < nb_lcore_params; ++i) {
        lcore = lcore_params[i].lcore_id;
        nb_rx_queue = lcore_conf[lcore].n_rx_queue;
        if (nb_rx_queue >= MAX_RX_QUEUE_PER_LCORE) {
            printf("error: too many queues (%u) for lcore: %u\n",
                   (unsigned)nb_rx_queue + 1, (unsigned)lcore);
            return -1;
        } else {
            lcore_conf[lcore].rx_queue_list[nb_rx_queue].port_id =
                    lcore_params[i].port_id;
            lcore_conf[lcore].rx_queue_list[nb_rx_queue].queue_id =
                    lcore_params[i].queue_id;
            lcore_conf[lcore].n_rx_queue++;
        }
    }
    return 0;
}

static int
check_port_config (void)
{
    uint16_t portid;
    uint16_t i;

    for (i = 0; i < nb_lcore_params; ++i) {
        portid = lcore_params[i].port_id;
        if (!rte_eth_dev_is_valid_port(portid)) {
            printf("port %u is not present on the board\n", portid);
            return -1;
        }
    }
    return 0;
}

static uint8_t
get_port_n_rx_queues (const uint16_t port)
{
    int queue = -1;
    uint16_t i;

    for (i = 0; i < nb_lcore_params; ++i) {
        if (lcore_params[i].port_id == port) {
            if (lcore_params[i].queue_id == queue+1) {
                queue = lcore_params[i].queue_id;
            } else {
                rte_exit(EXIT_FAILURE, "queue ids of the port %d must"
                         " be in sequence and must start with 0\n",
                         lcore_params[i].port_id);
            }
        }
    }
    return (uint8_t)(++queue);
}

// Number of mbufs needed.
// Taking into account memory for rx and tx hardware rings, 
// cache per lcore and mtable per port per lcore.
// RTE_MAX is used to ensure that NB_MBUF never goes below a minimum
// value of 8192
#define NB_MBUF RTE_MAX(                        \
        (nb_ports*nb_rx_queue*nb_rxd +          \
        nb_ports*nb_lcores*MAX_PKT_BURST +      \
        nb_ports*n_tx_queue*nb_txd +            \
        nb_lcores*MEMPOOL_CACHE_SIZE),          \
        (unsigned)8192)

static int
init_mem (unsigned nb_mbuf)
{
    int socketid;
    unsigned lcore_id;
    char s[64];

    for (lcore_id = 0; lcore_id < FTE_MAX_CORES; lcore_id++) {
        if (rte_lcore_is_enabled(lcore_id) == 0) {
            continue;
        }

        if (numa_on) {
            socketid = rte_lcore_to_socket_id(lcore_id);
        } else {
            socketid = 0;
        }

        if (socketid >= NB_SOCKETS) {
            rte_exit(EXIT_FAILURE, 
                     "Socket %d of lcore %u is out of range %d\n",
                     socketid, lcore_id, NB_SOCKETS);
        }

        if (pktmbuf_pool[socketid] == NULL) {
            snprintf(s, sizeof(s), "mbuf_pool_%d", socketid);
            pktmbuf_pool[socketid] =
                    rte_pktmbuf_pool_create(s, nb_mbuf,
                            MEMPOOL_CACHE_SIZE, 0,
                            RTE_MBUF_DEFAULT_BUF_SIZE, socketid);
            if (pktmbuf_pool[socketid] == NULL) {
                rte_exit(EXIT_FAILURE,
                         "Cannot init mbuf pool on socket %d\n",
                         socketid);
            } else {
                PDS_TRACE_DEBUG("Allocated mbuf pool on socket %d\n",
                                socketid);
            }
        }
    }
    return 0;
}

static void
init_rx_queues (void)
{
    struct rte_eth_dev_info dev_info;
    struct lcore_conf *qconf;
    unsigned lcore_id;
    uint16_t queueid, portid;
    uint8_t queue;
    int socketid;
    int ret;

    for (lcore_id = 0; lcore_id < FTE_MAX_CORES; lcore_id++) {
        if (rte_lcore_is_enabled(lcore_id) == 0) {
            continue;
        }
        qconf = &lcore_conf[lcore_id];
        PDS_TRACE_DEBUG("\nInitializing rx queues on lcore %u ...",
                        lcore_id);

        for(queue = 0; queue < qconf->n_rx_queue; ++queue) {
            struct rte_eth_dev *dev;
            struct rte_eth_conf *conf;
            struct rte_eth_rxconf rxq_conf;

            portid = qconf->rx_queue_list[queue].port_id;
            queueid = qconf->rx_queue_list[queue].queue_id;
            dev = &rte_eth_devices[portid];
            conf = &dev->data->dev_conf;

            if (numa_on) {
                socketid = (uint8_t)rte_lcore_to_socket_id(lcore_id);
            } else {
                socketid = 0;
            }

            PDS_TRACE_DEBUG("rxq=%d,%d,%d ", portid, queueid,
                            socketid);

            rte_eth_dev_info_get(portid, &dev_info);
            rxq_conf = dev_info.default_rxconf;
            rxq_conf.offloads = conf->rxmode.offloads;
            ret = rte_eth_rx_queue_setup(portid, queueid, nb_rxd,
                                         socketid, &rxq_conf,
                                         pktmbuf_pool[socketid]);
            if (ret < 0) {
                rte_exit(EXIT_FAILURE,
                         "rte_eth_rx_queue_setup: err=%d, port=%d\n",
                         ret, portid);
            }
        }
    }

    return;
}

// Check the link status of all ports in up to 9s, and print
static void
check_all_ports_link_status (void)
{
#define CHECK_INTERVAL 100 // 100ms 
#define MAX_CHECK_TIME 90 // 9s (90 * 100ms) in total
    uint16_t portid;
    uint8_t count, all_ports_up, print_flag = 0;
    struct rte_eth_link link;

    printf("\nChecking link status ");
    fflush(stdout);
    for (count = 0; count <= MAX_CHECK_TIME; count++) {
        all_ports_up = 1;
        RTE_ETH_FOREACH_DEV(portid) {
            memset(&link, 0, sizeof(link));
            rte_eth_link_get_nowait(portid, &link);
            if (print_flag == 1) {
                if (link.link_status) {
                    printf("Port%d Link Up. Speed %u Mbps -%s\n",
                           portid, link.link_speed,
                           (link.link_duplex == ETH_LINK_FULL_DUPLEX)
                           ? ("full-duplex") : ("half-duplex\n"));
                } else {
                    printf("Port %d Link Down\n", portid);
                }
                continue;
            }
            if (link.link_status == ETH_LINK_DOWN) {
                all_ports_up = 0;
                break;
            }
        }
        if (print_flag == 1) {
            break;
        }

        if (all_ports_up == 0) {
            printf(".");
            fflush(stdout);
            rte_delay_ms(CHECK_INTERVAL);
        }

        // set the print_flag if all ports up or timeout
        if (all_ports_up == 1 || count == (MAX_CHECK_TIME - 1)) {
            print_flag = 1;
            printf("done\n");
        }
    }
}

static void
_init_port (void)
{
    int ret = 0;
    struct lcore_conf *qconf;
    struct rte_eth_txconf *txconf;
    struct rte_eth_conf local_port_conf = {0};
    struct rte_eth_dev_info dev_info;
    unsigned nb_ports, lcore_id;
    uint32_t n_tx_queue, nb_lcores;
    int8_t nb_rx_queue, socketid;
    uint16_t queueid, portid;

    nb_ports = rte_eth_dev_count_avail();
    nb_lcores = rte_lcore_count();

    RTE_ETH_FOREACH_DEV(portid) {
        PDS_TRACE_DEBUG("Initializing port %d... ", portid);

        nb_rx_queue = get_port_n_rx_queues(portid);
        n_tx_queue = nb_lcores;
        if (n_tx_queue > MAX_TX_QUEUE_PER_PORT) {
            n_tx_queue = MAX_TX_QUEUE_PER_PORT;
        }
        PDS_TRACE_DEBUG("Creating queues: nb_rxq=%d nb_txq=%u... ",
                        nb_rx_queue, (unsigned)n_tx_queue );

        rte_eth_dev_info_get(portid, &dev_info);
        if (dev_info.tx_offload_capa &
            DEV_TX_OFFLOAD_MBUF_FAST_FREE) {
            local_port_conf.txmode.offloads |=
                        DEV_TX_OFFLOAD_MBUF_FAST_FREE;
        }

        local_port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
        local_port_conf.rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
        local_port_conf.rxmode.split_hdr_size = 0;
        local_port_conf.rxmode.offloads = DEV_RX_OFFLOAD_CHECKSUM;

        local_port_conf.rx_adv_conf.rss_conf.rss_hf |=
            dev_info.flow_type_rss_offloads;

        ret = rte_eth_dev_configure(portid, nb_rx_queue,
                                    (uint16_t)n_tx_queue,
                                    &local_port_conf);
        if (ret < 0) {
            rte_exit(EXIT_FAILURE,
                     "Cannot configure device: err=%d, port=%d\n",
                     ret, portid);
        }

        ret = rte_eth_dev_adjust_nb_rx_tx_desc(portid, &nb_rxd,
                                               &nb_txd);
        if (ret < 0) {
            rte_exit(EXIT_FAILURE,
                     "Cannot adjust number of descriptors: err=%d, "
                     "port=%d\n", ret, portid);
        }

        ret = init_mem(NB_MBUF);
        if (ret < 0) {
            rte_exit(EXIT_FAILURE, "_init_mem failed\n");
        }

        // init one TX queue per couple (lcore,port)
        queueid = 0;
        for (lcore_id = 0; lcore_id < FTE_MAX_CORES; lcore_id++) {
            if (rte_lcore_is_enabled(lcore_id) == 0) {
                continue;
            }

            if (numa_on) {
                socketid = (uint8_t)rte_lcore_to_socket_id(lcore_id);
            } else {
                socketid = 0;
            }

            PDS_TRACE_DEBUG("txq=%u,%d,%d ", lcore_id, queueid,
                            socketid);

            txconf = &dev_info.default_txconf;
            txconf->offloads = local_port_conf.txmode.offloads;
            ret = rte_eth_tx_queue_setup(portid, queueid, nb_txd,
                                         socketid, txconf);
            if (ret < 0) {
                rte_exit(EXIT_FAILURE,
                         "rte_eth_tx_queue_setup: err=%d, "
                         "port=%d\n", ret, portid);
            }
            qconf = &lcore_conf[lcore_id];
            qconf->tx_queue_id[portid] = queueid;
            queueid++;

            qconf->tx_port_id[qconf->n_tx_port] = portid;
            qconf->n_tx_port++;
        }
        PDS_TRACE_DEBUG("\n");
    }

    init_rx_queues();

    // start ports
    RTE_ETH_FOREACH_DEV(portid) {
        ret = rte_eth_dev_start(portid);
        if (ret < 0) {
            rte_exit(EXIT_FAILURE,
                     "rte_eth_dev_start: err=%d, port=%d\n",
                     ret, portid);
        }

        if (promiscuous_on) {
            rte_eth_promiscuous_enable(portid);
        }
    }

    check_all_ports_link_status();

    return;
}

static int
fte_main (void)
{
    int ret;
    sdk_ret_t sdk_ret;

    // init EAL
    ret = rte_eal_init(NELEMS(g_eal_args), (char**)g_eal_args);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    if (check_lcore_params() < 0) {
        rte_exit(EXIT_FAILURE, "check_lcore_params failed\n");
    }

    ret = init_lcore_conf_rx_queues();
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "init_lcore_conf_rx_queues failed\n");
    }

    if (check_port_config() < 0) {
        rte_exit(EXIT_FAILURE, "check_port_config failed\n");
    }

    // Configure Port
    _init_port();

    // init FTL
    if (g_athena_app_mode == ATHENA_APP_MODE_CPP) {
        if ((sdk_ret = fte_flows_init()) != SDK_RET_OK) {
            PDS_TRACE_DEBUG("FTE flow init failed: ret=%d ", sdk_ret);
        }
    }

    _init_pollers_client();

    ret = 0;
    // launch per-lcore init on every slave lcore
    fte_threads_started = true;
    rte_eal_mp_remote_launch(fte_launch_one_lcore, NULL, fte_call_master_type);

    return ret;
}

void
fte_init (void)
{
    PDS_TRACE_DEBUG("FTE entering forever loop ...");

    fte_main();

    return;
}

void
fte_fini (void)
{
    if (fte_threads_started) {
        fte_threads_done = true;
        rte_eal_mp_wait_lcore();
    }

    /*
     * Anything else in fte_athena that needs explicit cleanup goes here.
     */
}

} // namespace fte
