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
#include <rte_ip.h>
#include <rte_udp.h>

#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/fte/fte.hpp"
#include "gen/p4gen/p4/include/ftl.h"
#include "gen/p4gen/p4/include/ftl_table.hpp"
#include "nic/utils/ftl/ftl_base.hpp"
#include <impl.h>

namespace fte {

char const * g_eal_args[] = {"fte", "-l", "2,3", "--vdev=net_ionic0"};
#define RTE_LOGTYPE_FTE RTE_LOGTYPE_USER1
#define NELEMS(_a) (sizeof(_a)/sizeof(_a[0]))

#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT 16

#define FTE_PID 0 // Default PORTID
#define FTE_QID 0 // Default QueueID
#define FTE_MAX_CORES 1 // Max Cores
#define FTE_MAX_TXDSCR 1024 // Max TX Descriptors
#define FTE_MAX_RXDSCR 1024 // Max RX Descriptors
#define FTE_PKT_BATCH_SIZE 32 // Packet Batch Size
#define FTE_MEMPOOL_SIZE 256
#define FTE_PREFETCH_NLINES 7
/* Global State */
typedef struct gls_s {
    struct rte_mempool* mbfpool;
    uint16_t ntxdscr;
    uint16_t nrxdscr;
    uint32_t nmbfs;
    struct rte_eth_dev_tx_buffer* txbf;
    stats_t stats;
    uint64_t ftlstats[sdk::SDK_RET_MAX];
} __rte_cache_aligned gls_t;
gls_t gls = { NULL, FTE_MAX_TXDSCR, FTE_MAX_RXDSCR };

/* FTL */
#define FTE_MAX_NUM_CORES 4
#define FTE_MAX_BURST_SIZE 64
/* Session ID is 23 bits and ID 0 is reserved, so Max sessions are (8 * 1024 * 1024) - 2. */
#define FTE_MAX_SESSION_INDEX (8388606)
#define FTE_FLOW_SESSION_POOL_COUNT_MAX 256

typedef struct fte_v4flow_params_s {
    ipv4_flow_hash_entry_t entry;
    uint32_t hash;
} fte_v4flow_params_t;

typedef struct fte_v6flow_params_s {
    flow_hash_entry_t entry;
    uint32_t hash;
} fte_v6flow_params_t;

typedef struct fte_flow_hw_ctx_s {
    uint8_t dummy;
} fte_flow_hw_ctx_t;

typedef struct fte_flow_session_id_thr_local_pool_s {
    int16_t         pool_count;
    uint32_t        session_ids[FTE_FLOW_SESSION_POOL_COUNT_MAX];
} fte_flow_session_id_thr_local_pool_t;

typedef struct fte_flow_main_s {
    uint64_t no_cores;
    volatile uint32_t flow_prog_lock;
    ftl_base *table4[FTE_MAX_NUM_CORES];
    ftl_base *table6[FTE_MAX_NUM_CORES];
    fte_v4flow_params_t ip4_flow_params[FTE_MAX_NUM_CORES][FTE_MAX_BURST_SIZE];
    fte_v6flow_params_t ip6_flow_params[FTE_MAX_NUM_CORES][FTE_MAX_BURST_SIZE];
    fte_flow_hw_ctx_t session_index_pool[FTE_MAX_SESSION_INDEX];
    fte_flow_session_id_thr_local_pool_t session_id_thr_local_pool[FTE_MAX_NUM_CORES];
} fte_flow_main_t;

fte_flow_main_t g_fte_fm;
uint32_t g_session_id;

typedef char* (*key2str_t)(void *key);
typedef char* (*appdata2str_t)(void *data);

// Flow dump vars
uint32_t dump_src_ip, dump_dst_ip;
uint16_t dump_sport, dump_dport;
uint8_t dump_protocol;

static void
fte_ftlv4_set_key (ipv4_flow_hash_entry_t *entry, uint32_t sip, uint32_t dip,
                   uint8_t ip_proto, uint16_t src_port, uint16_t dst_port,
                   uint16_t lookup_id)
{
    PDS_TRACE_DEBUG("\nFTE KEY:sip:0x%x dip:0x%x prot:%d sport:%u dport:%u\n",
                    sip, dip, ip_proto, src_port, dst_port);
    entry->set_key_metadata_ipv4_src(sip);
    entry->set_key_metadata_ipv4_dst(dip);
    entry->set_key_metadata_proto(ip_proto);
    entry->set_key_metadata_sport(src_port);
    entry->set_key_metadata_dport(dst_port);
}

static void
fte_ftlv4_dump_hw_entry (ftl_base *obj, uint32_t src, uint32_t dst,
                         uint8_t ip_proto, uint16_t sport, uint16_t dport,
                         uint16_t lookup_id, char *buf, int max_len)
{
    sdk_ret_t ret;
    sdk_table_api_params_t params = {0};
    ipv4_flow_hash_entry_t entry;

    entry.clear();
    fte_ftlv4_set_key(&entry, src, dst, ip_proto, sport, dport, lookup_id);
    params.entry = &entry;

    ret = obj->get(&params);
    if (ret != SDK_RET_OK) {
	PDS_TRACE_DEBUG("\nFTE GET FAILED..\n");
	return;
    }
    entry.tostr(buf, max_len);
    return;
}

static ftl_base *
fte_ftlv4_create (void *key2str, void *appdata2str, uint32_t thread_id)
{
    sdk_table_factory_params_t factory_params = {0};

    // fte is enabled only for Athena
    // In case of Athena, its Unified Table for both v4 and v6
    factory_params.key2str = (key2str_t) (key2str);
    factory_params.appdata2str = (appdata2str_t) (appdata2str);
    factory_params.thread_id = thread_id;

    return ipv4_flow_hash::factory(&factory_params);
}


static ftl_base *
fte_ftlv6_create (void *key2str, void *appdata2str, uint32_t thread_id)
{
    sdk_table_factory_params_t factory_params = {0};

    factory_params.key2str = (key2str_t) (key2str);
    factory_params.appdata2str = (appdata2str_t) (appdata2str);
    factory_params.thread_id = thread_id;

    return flow_hash::factory(&factory_params);
}

static int
fte_ftlv4_insert (ftl_base *obj, ipv4_flow_hash_entry_t *entry,
                  uint32_t hash)
{
    sdk_table_api_params_t params = {0};

#if 0
    if (get_skip_ftl_program()) {
        return 0;
    }
#endif

    if (hash) {
        params.hash_32b = hash;
        params.hash_valid = 1;
    }
    params.entry = entry;
    if (SDK_RET_OK != obj->insert(&params)) {
        return -1;
    }
    return 0;
}

static void
fte_ftl_init (void)
{
    int i;

    memset(&g_fte_fm, 0, sizeof(fte_flow_main_t));

    for (i = 0; i < FTE_MAX_NUM_CORES; i++) {
        g_fte_fm.table4[i] = fte_ftlv4_create((void *) fte_flow4_key2str,
                                            (void *) fte_flow_appdata2str, i);

        g_fte_fm.table6[i] = fte_ftlv6_create((void *) fte_flow6_key2str,
                                            (void *) fte_flow_appdata2str, i);

        g_fte_fm.session_id_thr_local_pool[i].pool_count = -1;
    }

    return;
}

#define IP_PROTOCOL_TCP 0x06
#define IP_PROTOCOL_UDP 0x11

static void
fte_flow_extract_prog_args_x1 (struct rte_mbuf *m,
                               fte_v4flow_params_t *local_params0,
                               uint32_t session_id,
                               uint8_t is_ip4)
{
    struct udp_hdr *udp0;

    if (is_ip4) {
        struct ipv4_hdr *ip40;
        ipv4_flow_hash_entry_t *local_entry = &local_params0->entry;
        uint32_t src_ip, dst_ip;
        uint16_t sport, dport;
        uint8_t protocol;

        fte_ftlv4_set_session_index(local_entry, session_id);

        // APOLLO_P4_TO_ARM_HDR_SZ + L2_HDR_OFFSET = 35
        ip40 = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr *, 35);

        src_ip = rte_be_to_cpu_32(ip40->src_addr);
        dst_ip = rte_be_to_cpu_32(ip40->dst_addr);
        protocol = ip40->next_proto_id;

        if ((ip40->next_proto_id == IP_PROTOCOL_TCP)
                || (ip40->next_proto_id == IP_PROTOCOL_UDP)) {
            udp0 = (struct udp_hdr *) (((uint8_t *) ip40) +
                    ((ip40->version_ihl & IPV4_HDR_IHL_MASK) *
                    IPV4_IHL_MULTIPLIER));
            sport = rte_be_to_cpu_16(udp0->src_port);
            dport = rte_be_to_cpu_16(udp0->dst_port);
        } else {
            sport = dport =0;
        }
        fte_ftlv4_set_key(local_entry, src_ip, dst_ip,
                          protocol, sport, dport, 0 /* TODO: lkp_id */);

	// TODO: To be reomved. Debug purpose
        dump_src_ip = src_ip;
        dump_dst_ip = dst_ip;
        dump_protocol = protocol;
        dump_sport = sport;
        dump_dport = dport;
    }
#if 0
    else {
        ip6_header_t *ip60;
        flow_hash_entry_t *local_entry = &local_params0->entry6;
        flow_hash_entry_t *remote_entry = &remote_params0->entry6;
        u8 *src_ip, *dst_ip;
        u16 sport, dport;
        u8 protocol;
        u16 lkp_id;

        ftlv6_set_session_index(local_entry, session_id);
        ftlv6_set_session_index(remote_entry, session_id);
        ftlv6_set_epoch(local_entry, 0xff);
        ftlv6_set_epoch(remote_entry, 0xff);

        ip60 = vlib_buffer_get_current(p0);

        src_ip = ip60->src_address.as_u8;
        dst_ip = ip60->dst_address.as_u8;
        protocol = ip60->protocol;
        lkp_id = vnet_buffer (p0)->sw_if_index[VLIB_TX];

        if (PREDICT_TRUE(((ip60->protocol == IP_PROTOCOL_TCP)
                || (ip60->protocol == IP_PROTOCOL_UDP)))) {
            udp0 = (udp_header_t *) (((u8 *) ip60) +
                    (vnet_buffer (p0)->l4_hdr_offset -
                            vnet_buffer (p0)->l3_hdr_offset));
            sport = clib_net_to_host_u16(udp0->src_port);
            dport = clib_net_to_host_u16(udp0->dst_port);
        } else {
            sport = dport =0;
        }
        ftlv6_set_key(local_entry, src_ip, dst_ip,
                      protocol, sport, dport, lkp_id, 0);
        ftlv6_set_key(remote_entry, dst_ip, src_ip,
                      protocol, dport, sport, lkp_id, 0);
        pds_flow_extract_nexthop_info((void *)local_entry,
                                      (void *)remote_entry, p0, 0);
    }
#endif
    // TODO: vnet_buffer (p0)->pds_data.flow_hash;
    local_params0->hash = 0;
    return;
}

static void
fte_flow_program_hw_ipv4 (fte_v4flow_params_t *key, unsigned int lcore_id)
{
    int ret;
    ftl_base *table = g_fte_fm.table4[lcore_id];

    ret = fte_ftlv4_insert(table, &key->entry, key->hash);

    if (0 != ret) {
    	PDS_TRACE_DEBUG("\nFTE fte_ftlv4_insert failed.. \n\n");
    }

    PDS_TRACE_DEBUG("\nFTE fte_ftlv4_insert passed.. \n\n");
    return;
}

static void
fte_flow_dump (void)
{
    char buf[1024];

    fte_ftlv4_dump_hw_entry(g_fte_fm.table4[0], dump_src_ip, dump_dst_ip,
                            dump_protocol, dump_sport, dump_dport, 0,
                            buf, 1024);

    PDS_TRACE_DEBUG("\nFTE DUMP:%s \n", buf);

    return;
}

static void
fte_flow_prog_ipv4 (struct rte_mbuf *m)
{
    uint32_t session_id;
    unsigned int lcore_id;
    fte_v4flow_params_t *params;

    lcore_id = rte_lcore_id();
    params = g_fte_fm.ip4_flow_params[lcore_id];

    // TODO: Use sdk indexer or rte_indexer for session id mgmt
    session_id = ++g_session_id;

    fte_flow_extract_prog_args_x1(m, params, session_id, 1);
    fte_flow_program_hw_ipv4(params, lcore_id);

    fte_flow_dump();
    return;
}

static void
_process (struct rte_mbuf *m)
{
    //gls.ftlstats[ret]++;

    fte_flow_prog_ipv4(m);

    int numtx = rte_eth_tx_buffer(FTE_PID, FTE_QID, gls.txbf, m);
    if (numtx) {
        gls.stats.tx += numtx;
    }
    // TODO: Free is needed??
    //rte_pktmbuf_free(m);
}

// main processing loop
static void
fte_rx_loop (void)
{
    struct rte_mbuf *pkts_burst[FTE_PKT_BATCH_SIZE];
    int numrx, numtx;

    PDS_TRACE_DEBUG("\nFTE fte_rx_loop.. core:%u \n", rte_lcore_id());
    while (1) {
        numrx = rte_eth_rx_burst(0, 0, pkts_burst, FTE_PKT_BATCH_SIZE);
        if (!numrx) {
            continue;
        }

    	PDS_TRACE_DEBUG("\n\nFTE receives %d packets.. \n\n", numrx);
        gls.stats.rx += numrx;
        for (int i = 0; i < numrx; i++) {
            auto m = pkts_burst[i];
            if ((i+1) < numrx) {
                auto m2 = pkts_burst[i+1];
                uint8_t *d2 = rte_pktmbuf_mtod(m2, uint8_t*);
                for (int i = 0; i < FTE_PREFETCH_NLINES; i++) {
                    rte_prefetch0(d2+i*64);
                }
            }
            _process(m);
        }
//#ifndef FTE_REINJECT_DISABLE
        numtx = rte_eth_tx_buffer_flush(0, 0, gls.txbf);
        if (numtx) {
            // TODO: Duplicate increment? Check _process as well.
            gls.stats.tx += numtx;
        }
//#endif
    }
}

static int
fte_launch_one_lcore (__attribute__((unused)) void *dummy)
{
    fte_rx_loop();
    return 0;
}

static void
_init_gls (void)
{
    uint16_t nports = 0;
    uint32_t nmbfs = 0;

    nports = rte_eth_dev_count_avail();
    if (nports == 0) {
        rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");
    }

    nmbfs = nports * (FTE_MAX_RXDSCR + FTE_MAX_TXDSCR + FTE_PKT_BATCH_SIZE +
                      FTE_MAX_CORES * FTE_MEMPOOL_SIZE);
    gls.nmbfs = RTE_MAX(nmbfs, 8192U);

    // create the mbuf pool
    gls.mbfpool = rte_pktmbuf_pool_create("mbuf_pool", gls.nmbfs,
                                          FTE_MEMPOOL_SIZE,
                                          0, RTE_MBUF_DEFAULT_BUF_SIZE,
                                          rte_socket_id());
    if (gls.mbfpool == NULL) {
        rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");
    }

    return;
}

static void
_init_port (uint16_t portid)
{
    int ret = 0;
    struct rte_eth_rxconf rxq_conf;
    struct rte_eth_txconf txq_conf;
    struct rte_eth_conf local_port_conf = {0};
    struct rte_eth_dev_info dev_info;

    printf("Initializing port %u... \n", portid);
    rte_eth_dev_info_get(portid, &dev_info);
    if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE) {
        local_port_conf.txmode.offloads |= DEV_TX_OFFLOAD_MBUF_FAST_FREE;
    }
    ret = rte_eth_dev_configure(portid, 1, 1, &local_port_conf);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d, port=%u\n",
                 ret, portid);
    }

    ret = rte_eth_dev_adjust_nb_rx_tx_desc(portid, &gls.nrxdscr, &gls.ntxdscr);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Descriptors updated failed: err=%d, port=%u\n",
                 ret, FTE_PID);
    }

    // init one RX queue
    rxq_conf = dev_info.default_rxconf;
    rxq_conf.offloads = local_port_conf.rxmode.offloads;
    ret = rte_eth_rx_queue_setup(portid, 0, gls.nrxdscr,
                                 rte_eth_dev_socket_id(portid),
                                 &rxq_conf, gls.mbfpool);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n",
                 ret, portid);
    }

    txq_conf = dev_info.default_txconf;
    txq_conf.offloads = local_port_conf.txmode.offloads;
    ret = rte_eth_tx_queue_setup(portid, 0, gls.ntxdscr,
                                 rte_eth_dev_socket_id(portid),
                                 &txq_conf);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%u\n",
                 ret, portid);
    }

    return;
}

static void
_init_txbf (uint16_t portid)
{
    int ret = 0;

    // Initialize TX buffers
    gls.txbf = (rte_eth_dev_tx_buffer*)rte_zmalloc_socket("tx_buffer",
                                  RTE_ETH_TX_BUFFER_SIZE(FTE_PKT_BATCH_SIZE), 0,
                                  rte_eth_dev_socket_id(portid));
    if (gls.txbf == NULL) {
        rte_exit(EXIT_FAILURE, "Cannot allocate buffer for tx on port %u\n",
                portid);
    }

    rte_eth_tx_buffer_init(gls.txbf, FTE_PKT_BATCH_SIZE);

    ret = rte_eth_tx_buffer_set_err_callback(gls.txbf,
                                rte_eth_tx_buffer_count_callback,
                                &gls.stats.drop);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE,
                 "Cannot set error callback for tx buffer on port %u\n",
                 portid);
    }

    return;
}

static int
fte_main(void)
{
    int ret;
    unsigned lcore_id;
    // init EAL
    ret = rte_eal_init(NELEMS(g_eal_args), (char**)g_eal_args);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
    }

    // Initialize Global State
    _init_gls();

    // Configure Port
    _init_port(FTE_PID);

    // Initialize TX Buffer
    _init_txbf(FTE_PID);

    // Start device
    ret = rte_eth_dev_start(FTE_PID);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n",
                 ret, FTE_PID);
    }
    rte_eth_promiscuous_enable(FTE_PID);

    ret = 0;
    // launch per-lcore init on every lcore
    rte_eal_mp_remote_launch(fte_launch_one_lcore, NULL, CALL_MASTER);
    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
        if (rte_eal_wait_lcore(lcore_id) < 0) {
            ret = -1;
            break;
        }
    }

    printf("Closing port %d...\n", FTE_PID);
    rte_eth_dev_stop(FTE_PID);
    rte_eth_dev_close(FTE_PID);
    return ret;
}

void *
fte_thread_start (void *ctxt)
{
    SDK_THREAD_INIT(ctxt);

    sleep(10);

    fte_ftl_init();
    PDS_TRACE_DEBUG("FTE entering forever loop ...");

    fte_main();

    return NULL;
}

} // namespace fte
