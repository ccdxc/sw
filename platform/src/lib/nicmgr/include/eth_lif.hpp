/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __ETH_LIF_HPP__
#define __ETH_LIF_HPP__

using namespace std;

#include "nic/include/edmaq.h"
#include "nic/sdk/platform/devapi/devapi.hpp"
#include "pd_client.hpp"
#include "ev.h"

namespace pt = boost::property_tree;

class Eth;
class AdminQ;
class EdmaQ;
typedef uint8_t status_code_t;
typedef uint16_t cmd_opcode_t;

/**
 * ETH Qtype Enum
 */
enum eth_hw_qtype {
    ETH_HW_QTYPE_RX = 0,
    ETH_HW_QTYPE_TX = 1,
    ETH_HW_QTYPE_ADMIN = 2,
    ETH_HW_QTYPE_SQ = 3,
    ETH_HW_QTYPE_RQ = 4,
    ETH_HW_QTYPE_CQ = 5,
    ETH_HW_QTYPE_EQ = 6,
    ETH_HW_QTYPE_SVC = 7,
    ETH_HW_QTYPE_NONE = 15,
};

#define IONIC_IFNAMSIZ 16

#define BIT_MASK(n) ((1ULL << n) - 1)

#define LG2_LIF_STATS_SIZE 10
#define LIF_STATS_SIZE (1 << LG2_LIF_STATS_SIZE)

#define ETH_NOTIFYQ_QTYPE 7
#define ETH_NOTIFYQ_QID 0
#define LG2_ETH_NOTIFYQ_RING_SIZE 4
#define ETH_NOTIFYQ_RING_SIZE (1 << LG2_ETH_NOTIFYQ_RING_SIZE)

#define ETH_EDMAQ_QTYPE 7
#define ETH_EDMAQ_QID 1
#define LG2_ETH_EDMAQ_RING_SIZE 6
#define ETH_EDMAQ_RING_SIZE (1 << LG2_ETH_EDMAQ_RING_SIZE)

#define ETH_ADMINQ_REQ_QTYPE 7
#define ETH_ADMINQ_REQ_QID 2
#define LG2_ETH_ADMINQ_REQ_RING_SIZE 4
#define ETH_ADMINQ_REQ_RING_SIZE (1 << LG2_ETH_ADMINQ_REQ_RING_SIZE)

#define ETH_ADMINQ_RESP_QTYPE 7
#define ETH_ADMINQ_RESP_QID 3
#define LG2_ETH_ADMINQ_RESP_RING_SIZE 4
#define ETH_ADMINQ_RESP_RING_SIZE (1 << LG2_ETH_ADMINQ_RESP_RING_SIZE)

#define ETH_EDMAQ_ASYNC_QTYPE 7
#define ETH_EDMAQ_ASYNC_QID 4
#define LG2_ETH_EDMAQ_ASYNC_RING_SIZE 6
#define ETH_EDMAQ_ASYNC_RING_SIZE (1 << LG2_ETH_EDMAQ_ASYNC_RING_SIZE)

#define RXDMA_Q_QUIESCE_WAIT_S 0.001  // 1 ms
#define RXDMA_LIF_QUIESCE_WAIT_S 0.01 // 10 ms

#define RSS_HASH_KEY_SIZE 40
#define RSS_IND_TBL_SIZE 128

#define FW_MAX_SZ ((900 << 20)) // 900 MiB
#define FW_FILEPATH "/update/firmware.tar"

/**
 * LIF Resource structure
 */
typedef struct eth_lif_res_s {
    uint64_t lif_index;
    uint64_t lif_id;
    uint64_t intr_base;
    uint64_t rx_eq_base;
    uint64_t tx_eq_base;
    uint64_t cmb_mem_addr;
    uint64_t cmb_mem_size;
} eth_lif_res_t;

/**
 * LIF State enum
 */
enum eth_lif_state {
    LIF_STATE_RESETTING,
    LIF_STATE_RESET,
    LIF_STATE_CREATING,
    LIF_STATE_CREATED,
    LIF_STATE_INITING,
    LIF_STATE_INIT,
    LIF_STATE_UP,
    LIF_STATE_DOWN,
};

class EthLif
{
  public:
    EthLif(Eth *dev, devapi *dev_api, void *dev_spec, PdClient *pd_client, eth_lif_res_t *res,
           EV_P);

    status_code_t Init(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t Reset();
    bool EdmaProxy(edma_opcode opcode, uint64_t from, uint64_t to, uint16_t size,
                   struct edmaq_ctx *ctx);
    bool EdmaAsyncProxy(edma_opcode opcode, uint64_t from, uint64_t to, uint16_t size,
                        struct edmaq_ctx *ctx);

    // Command Handlers
    status_code_t CmdProxyHandler(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t CmdHandler(void *req, void *req_data, void *resp, void *resp_data);

    static const char *opcode_to_str(cmd_opcode_t opcode);

    // Event Handlers
    void LinkEventHandler(port_status_t *evd);
    void XcvrEventHandler(port_status_t *evd);
    void DelphiMountEventHandler(bool mounted);
    void SendFWDownEvent();
    void HalEventHandler(bool status);

    void SetHalClient(devapi *dev_api);

    bool IsLifQuiesced();

    int GenerateQstateInfoJson(pt::ptree &lifs);

    void AddLifMetrics(void);

    EV_P;

  private:
    Eth *dev;
    static sdk::lib::indexer *fltr_allocator;
    // Info
    char name[IONIC_IFNAMSIZ];
    enum eth_lif_state state;
    // PD Info
    PdClient *pd;
    // HAL Info
    devapi *dev_api;
    lif_info_t hal_lif_info_;
    bool skip_hwinit;
    // LIF Info
    eth_lif_res_t *res;
    uint8_t cosA, cosB, admin_cosA, admin_cosB;
    // Spec
    const struct eth_devspec *spec;
    struct queue_info qinfo[NUM_QUEUE_TYPES];
    // Config
    union ionic_lif_config *lif_config;
    uint64_t lif_config_addr;
    uint64_t host_lif_config_addr;
    // Status
    struct ionic_lif_status *lif_status;
    uint64_t lif_status_addr;
    uint64_t host_lif_status_addr;
    // Stats
    uint64_t lif_stats_addr;
    uint64_t host_lif_stats_addr;
    // NotifyQ
    uint16_t notify_ring_head;
    uint64_t notify_ring_base;
    uint8_t notify_enabled;
    // EdmaQ
    uint64_t edma_buf_addr;
    uint8_t *edma_buf;
    // Firmware
    uint64_t fw_buf_addr;
    uint32_t fw_buf_size;
    uint8_t *fw_buf;
    // RSS config
    uint16_t rss_type;
    uint8_t rss_key[RSS_HASH_KEY_SIZE];  // 40B
    uint8_t rss_indir[RSS_IND_TBL_SIZE]; // 128B
    // Network info
    map<uint64_t, uint64_t> mac_addrs;
    map<uint64_t, uint16_t> vlans;
    map<uint64_t, tuple<uint64_t, uint16_t>> mac_vlans;
    // Tasks
    ev_timer stats_timer = {0};

    // ref_cnt for queues for this lif
    uint32_t active_q_ref_cnt;

    // Services
    AdminQ *adminq;
    EdmaQ *edmaq;
    EdmaQ *edmaq_async;

    /* AdminQ Commands */
    static void AdminCmdHandler(void *obj, void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdSetAttr(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t SetFeatures(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t RssConfig(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdGetAttr(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdRxSetMode(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRxFilterAdd(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRxFilterDel(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdQIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t AdminQIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t NotifyQIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t TxQIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t RxQIdentify(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdQInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t AdminQInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t NotifyQInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t EQInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t TxQInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t RxQInit(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdQControl(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdRDMAResetLIF(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRDMACreateEQ(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRDMACreateCQ(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdRDMACreateAdminQ(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdFwDownload(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdFwControl(void *req, void *req_data, void *resp, void *resp_data);

    // Callbacks
    static void StatsUpdate(EV_P_ ev_timer *w, int events);

    // Helper methods
    void FreeUpMacFilters();
    void FreeUpVlanFilters();
    void FreeUpMacVlanFilters();
    bool IsLifTypeCpu(void);
    bool IsLifInitialized();

    void Create();

    static const char *lif_state_to_str(enum eth_lif_state state);
    static lif_state_t ConvertEthLifStateToLifState(enum eth_lif_state lif_state);
};

#endif /* __ETH_LIF_HPP__ */
