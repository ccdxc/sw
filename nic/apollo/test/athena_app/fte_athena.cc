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
#include "fte_athena.hpp"

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

static uint16_t nb_rxd = FTE_MAX_RXDSCR;
static uint16_t nb_txd = FTE_MAX_TXDSCR;

static uint32_t pollers_client_qcount;

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

    if (g_athena_app_mode == ATHENA_APP_MODE_CPP) {
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
fte_rx_loop (int poller_qid)
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

    while (1) {
        if ((poller_qid != -1) &&
            !ftl_pollers_client::user_will_poll()) {
            pds_flow_age_sw_pollers_poll(poller_qid, nullptr);
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
        for (i = 0; i < qconf->n_rx_queue; ++i) {
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
            for (i = 0; i < nb_rx; i++) {
                auto m = pkts_burst[i];
                pkt_hex_dump_trace("PKT:",
                        rte_pktmbuf_mtod(m, char*), 
                        rte_pktmbuf_pkt_len(m));
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
    int poller_qid;

    if (g_athena_app_mode == ATHENA_APP_MODE_CPP) {
        fte_ftl_set_core_id(rte_lcore_id());
    }

    poller_qid = rte_lcore_index(rte_lcore_id());
    if (poller_qid >= (int)pollers_client_qcount) {
        poller_qid = -1;
    }

    fte_rx_loop(poller_qid);
    return 0;
}

static void
_init_pollers_client (void)
{
    if (pds_flow_age_init() != SDK_RET_OK) {
        rte_exit(EXIT_FAILURE, "failed pds_flow_age_init");
    }

    pollers_client_qcount = ftl_pollers_client::qcount_get();
    if (rte_lcore_count() < pollers_client_qcount) {
        PDS_TRACE_DEBUG("Number of lcores (%u) is less than number of"
                        " poller queues (%u)", rte_lcore_count(),
                        pollers_client_qcount);
    }
}

static void
signal_handler (int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        PDS_TRACE_DEBUG("\nSIGNAL %d received..core#:%u\n",
                        signum, rte_lcore_id());
        rte_exit(0, "received signal: %d, exiting\n", signum);
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

        local_port_conf.rx_adv_conf.rss_conf.rss_hf &=
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
        if ((sdk_ret = fte_ftl_init()) != SDK_RET_OK) {
            rte_exit(EXIT_FAILURE, "fte_ftl_init:err=%d", sdk_ret);
        }
    }

    _init_pollers_client();

    ret = 0;
    // launch per-lcore init on every slave lcore
    rte_eal_mp_remote_launch(fte_launch_one_lcore, NULL, SKIP_MASTER);

    return ret;
}

void
fte_init (void)
{
    PDS_TRACE_DEBUG("FTE entering forever loop ...");

    fte_main();

    return;
}

} // namespace fte
