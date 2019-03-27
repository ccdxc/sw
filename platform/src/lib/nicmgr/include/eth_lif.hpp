/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __ETH_LIF_HPP__
#define __ETH_LIF_HPP__

using namespace std;

#include "pd_client.hpp"
#include "nic/sdk/platform/devapi/devapi.hpp"

namespace pt = boost::property_tree;

class AdminQ;
typedef uint8_t status_code_t;
typedef uint16_t cmd_opcode_t;

/**
 * ETH Qtype Enum
 */
enum EthQtype {
    ETH_QTYPE_RX = 0,
    ETH_QTYPE_TX = 1,
    ETH_QTYPE_ADMIN = 2,
    ETH_QTYPE_SQ = 3,
    ETH_QTYPE_RQ = 4,
    ETH_QTYPE_CQ = 5,
    ETH_QTYPE_EQ = 6,
    ETH_QTYPE_SVC = 7,
};

#define BIT_MASK(n)                     ((1ULL << n) - 1)
#define HOST_ADDR(lif, addr)            ((1ULL << 63) | (lif << 52) | (addr))

#define LG2_LIF_STATS_SIZE              10
#define LIF_STATS_SIZE                  (1 << LG2_LIF_STATS_SIZE)

#define ETH_NOTIFYQ_QTYPE               7
#define ETH_NOTIFYQ_QID                 0
#define LG2_ETH_NOTIFYQ_RING_SIZE       4
#define ETH_NOTIFYQ_RING_SIZE           (1 << LG2_ETH_NOTIFYQ_RING_SIZE)

#define ETH_EDMAQ_QTYPE                 7
#define ETH_EDMAQ_QID                   1
#define LG2_ETH_EDMAQ_RING_SIZE         4
#define ETH_EDMAQ_RING_SIZE             (1 << LG2_ETH_EDMAQ_RING_SIZE)
#define ETH_EDMAQ_COMP_POLL_US          (1000)
#define ETH_EDMAQ_COMP_POLL_MAX         (10)

#define ETH_ADMINQ_REQ_QTYPE            7
#define ETH_ADMINQ_REQ_QID              2
#define LG2_ETH_ADMINQ_REQ_RING_SIZE    4
#define ETH_ADMINQ_REQ_RING_SIZE        (1 << LG2_ETH_ADMINQ_REQ_RING_SIZE)

#define ETH_ADMINQ_RESP_QTYPE           7
#define ETH_ADMINQ_RESP_QID             3
#define LG2_ETH_ADMINQ_RESP_RING_SIZE   4
#define ETH_ADMINQ_RESP_RING_SIZE       (1 << LG2_ETH_ADMINQ_RESP_RING_SIZE)

#define RSS_HASH_KEY_SIZE	40
#define RSS_IND_TBL_SIZE	128

/**
 * LIF Resource structure
 */
typedef struct eth_lif_res_s {
    uint64_t lif_id;
    uint64_t intr_base;
    uint64_t cmb_mem_addr;
    uint64_t cmb_mem_size;
} eth_lif_res_t;

/**
 * LIF State enum
 */
enum lif_state {
    LIF_STATE_RESETING,
    LIF_STATE_RESET,
    LIF_STATE_CREATING,
    LIF_STATE_CREATED,
    LIF_STATE_INITING,
    LIF_STATE_INIT,
    LIF_STATE_UP,
    LIF_STATE_DOWN,
};

class EthLif {
public:
    EthLif(devapi *dev_api,
           void *dev_spec,
           PdClient *pd_client,
           eth_lif_res_t *res);

    status_code_t Init(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t Reset(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t AdminQInit(void *req, void *req_data, void *resp, void *resp_data);

    // Event Handlers
    status_code_t CmdHandler(void *req, void *req_data,
                                 void *resp, void *resp_data);
    void LinkEventHandler(port_status_t *evd);
    void XcvrEventHandler(port_status_t *evd);
    void HalEventHandler(bool status);

    void SetHalClient(devapi *dev_api);

    int GenerateQstateInfoJson(pt::ptree &lifs);

private:
    static sdk::lib::indexer *fltr_allocator;
    // Info
    std::string nd_name, dev_name;
    enum lif_state state;
    // PD Info
    PdClient *pd;
    // HAL Info
    devapi *dev_api;
    lif_info_t hal_lif_info_;
    bool hal_status;
    // LIF Info
    eth_lif_res_t *res;
    uint8_t cosA, cosB, ctl_cosA, ctl_cosB;
    // Spec
    const struct eth_devspec *spec;
    struct queue_info qinfo[NUM_QUEUE_TYPES];
    // Stats
    uint64_t stats_mem_addr;
    uint64_t host_stats_mem_addr;
    // NotifyQ
    struct notify_block *notify_block;
    uint16_t notify_ring_head;
    uint64_t notify_ring_base;
    uint64_t notify_block_addr;
    uint64_t host_notify_block_addr;
    // EdmaQ
    uint16_t edma_ring_head;
    uint16_t edma_comp_tail;
    uint16_t edma_exp_color;
    uint64_t edma_ring_base;
    uint64_t edma_comp_base;
    uint64_t edma_buf_base;
    // RSS config
    uint16_t rss_type;
    uint8_t  rss_key[RSS_HASH_KEY_SIZE]; // 40B
    uint8_t  rss_indir[RSS_IND_TBL_SIZE]; // 128B
    // Network info
    map<uint64_t, uint64_t> mac_addrs;
    map<uint64_t, uint16_t> vlans;
    map<uint64_t, tuple<uint64_t, uint16_t>> mac_vlans;
    // Tasks
    evutil_timer stats_timer;
    evutil_check stats_check;

    /* AdminQ Commands */
    AdminQ *adminq;

    static void AdminCmdHandler(void *obj,
        void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdHangNotify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdNotifyQInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdTxQInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRxQInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdFeatures(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdSetNetdevInfo(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdQEnable(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdQDisable(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdSetMode(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRxFilterAdd(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRxFilterDel(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdMacAddrGet(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdStatsDumpStart(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdStatsDumpStop(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRssHashSet(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRssIndirSet(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdRDMACreateEQ(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRDMACreateCQ(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRDMACreateAdminQ(void *req, void *req_data, void *resp, void *resp_data);

    // Callbacks
    static void StatsUpdate(void *obj);
    static void StatsUpdateCheck(void *obj);
    static void NotifyBlockUpdate(void *arg);

    // Helper methods
    void FreeUpMacFilters();
    void FreeUpVlanFilters();
    void FreeUpMacVlanFilters();

    const char *lif_state_to_str(enum lif_state state);
    const char *opcode_to_str(cmd_opcode_t opcode);
};

#endif   /* __ETH_LIF_HPP__*/
