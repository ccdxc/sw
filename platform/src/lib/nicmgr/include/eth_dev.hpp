/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __ETH_DEV_HPP__
#define __ETH_DEV_HPP__

#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"

#include "dev.hpp"
#include "nic/include/notify.hpp"
#include "nic/include/edma.hpp"
#include "nic/include/eth_common.h"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "platform/src/lib/hal_api/include/hal_types.hpp"
#include "platform/src/lib/mnet/include/mnet.h"
#include "platform/src/lib/evutils/include/evutils.h"

/* Supply these for ionic_if.h */
#define BIT(n)                  (1 << n)
#define TEST_BIT(x, n)          ((x) & (1 << n))
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define dma_addr_t uint64_t

#include "platform/drivers/common/ionic_if.h"



namespace pt = boost::property_tree;

// Doorbell address
#define UPD_BITS_POSITION   (17)
#define LIF_BITS_POSITION   (6)

#define DOORBELL_ADDR(lif_num) \
    ((0x8400000) | (0xb << UPD_BITS_POSITION) |  (lif_num << LIF_BITS_POSITION))

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

#pragma pack(push, 1)

/**
 * ETH PF Devcmd Region
 */

union dev_cmd {
    u32 words[16];
    struct admin_cmd cmd;
    struct nop_cmd nop;
    struct reset_cmd reset;
    struct identify_cmd identify;
    struct lif_init_cmd lif_init;
    struct adminq_init_cmd adminq_init;
    struct txq_init_cmd txq_init;
    struct rxq_init_cmd rxq_init;
    struct notifyq_init_cmd notifyq_init;
    struct features_cmd features;
    struct q_enable_cmd q_enable;
    struct q_disable_cmd q_disable;
    struct station_mac_addr_get_cmd station_mac_addr_get;
    struct mtu_set_cmd mtu_set;
    struct rx_mode_set_cmd rx_mode_set;
    struct rx_filter_add_cmd rx_filter_add;
    struct rx_filter_del_cmd rx_filter_del;
    struct rss_hash_set_cmd rss_hash_set;
    struct rss_indir_set_cmd rss_indir_set;
};

union dev_cmd_comp {
    u32 words[4];
    u8 status;
    struct admin_comp comp;
    struct nop_comp nop;
    struct reset_comp reset;
    struct identify_comp identify;
    struct lif_init_comp lif_init;
    struct adminq_init_comp adminq_init;
    struct txq_init_comp txq_init;
    struct rxq_init_comp rxq_init;
    struct notifyq_init_comp notifyq_init;
    struct features_comp features;
    q_enable_comp q_enable;
    q_disable_comp q_disable;
    struct station_mac_addr_get_comp station_mac_addr_get;
    mtu_set_comp mtu_set;
    rx_mode_set_comp rx_mode_set;
    struct rx_filter_add_comp rx_filter_add;
    rx_filter_del_comp rx_filter_del;
    rss_hash_set_comp rss_hash_set;
    rss_indir_set_comp rss_indir_set;
};

#pragma pack(pop)

struct dev_cmd_regs {
    u32 signature;
    u32 done;
    union dev_cmd cmd;
    union dev_cmd_comp comp;
    uint8_t data[2048] __attribute__((aligned (2048)));
};

static_assert(sizeof(struct dev_cmd_regs) == 4096);
static_assert((offsetof(struct dev_cmd_regs, cmd)  % 4) == 0);
static_assert(sizeof(((struct dev_cmd_regs*)0)->cmd) == 64);
static_assert((offsetof(struct dev_cmd_regs, comp) % 4) == 0);
static_assert(sizeof(((struct dev_cmd_regs*)0)->comp) == 16);
static_assert((offsetof(struct dev_cmd_regs, data) % 4) == 0);

#define LG2_LIF_STATS_SIZE              10
#define LIF_STATS_SIZE                  (1 << LG2_LIF_STATS_SIZE)

#define ETH_NOTIFYQ_ID                  0
#define LG2_ETH_NOTIFYQ_RING_SIZE       4
#define ETH_NOTIFYQ_RING_SIZE           (1 << LG2_ETH_NOTIFYQ_RING_SIZE)

#define ETH_EDMAQ_ID                    1
#define LG2_ETH_EDMAQ_RING_SIZE         4
#define ETH_EDMAQ_RING_SIZE             (1 << LG2_ETH_EDMAQ_RING_SIZE)

enum lif_state {
    LIF_STATE_RESET,
    LIF_STATE_CREATING,
    LIF_STATE_CREATED,
    LIF_STATE_INITING,
    LIF_STATE_INITED,
    LIF_STATE_UP,
    LIF_STATE_DOWN,
};

/**
 * ETH PF Device
 */
class Eth : public Device {
public:
    Eth(HalClient *hal_client,
        HalCommonClient *hal_common_client,
        void *dev_spec,
        hal_lif_info_t *nicmgr_lif_info,
        PdClient *pd_client);

    struct dev_cmd_regs *devcmd;
    struct notify_block *notify_block;

    void DevcmdPoll();

    void DevcmdHandler();
    void DevObjSave();
    static void StatsUpdateHandler(void *obj);
    void LinkEventHandler(link_eventdata_t *evd);
    enum DevcmdStatus CmdHandler(void *req, void *req_data, void *resp, void *resp_data);

    int GenerateQstateInfoJson(pt::ptree &lifs);
    bool isMnic();
    bool isHostManagement();
    bool isHost();
    void CreateMnet();
    void Update();
    hal_lif_info_t *GetHalLifInfo(void) { return &hal_lif_info_; }
    void SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client);

private:
    /* Static members */
    static sdk::lib::indexer *fltr_allocator;
    static sdk::lib::indexer *mnic_allocator;
    /* Members */
    struct eth_devspec *spec;
    hal_lif_info_t hal_lif_info_;
    /* Mnet Info */
    struct mnet_dev_create_req_t mnet_req;
    // Hardware Info
    struct queue_info qinfo[NUM_QUEUE_TYPES];
    // PD Info
    PdClient *pd;
    // HAL Info
    HalClient *hal;
    HalCommonClient *hal_common_client;
    hal_lif_info_t *nicmgr_lif_info;
    enum lif_state lif_state;
    // Coses
    uint8_t  coses; // {uint8_t CosA:4; uint8_t CosB:4;}
    // Notify state
    uint64_t eid;
    uint16_t link_flap_count;
    // Stats
    uint64_t stats_mem_addr;
    uint64_t host_stats_mem_addr;
    // NotifyQ
    uint16_t notify_ring_head;
    uint64_t notify_ring_base;
    uint64_t notify_block_addr;
    uint64_t host_notify_block_addr;
    // EdmaQ
    uint16_t edma_ring_head;
    uint64_t edma_ring_base;
    uint64_t edma_comp_base;
    // Rss config
    uint16_t rss_type;
    uint8_t  rss_key[RSS_HASH_KEY_SIZE]; // 40B
    uint8_t  rss_indir[RSS_IND_TBL_SIZE]; // 128B
    // PCIe info
    pciehdev_t *pdev;
    pciehdevice_resources_t pci_resources;
    // Network info
    map<uint64_t, uint64_t> mac_addrs;
    map<uint64_t, uint16_t> vlans;
    map<uint64_t, tuple<uint64_t, uint16_t>> mac_vlans;
    // Tasks
    evutil_timer stats_timer;

    /* Command Handlers */
    enum DevcmdStatus _CmdReset(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdHangNotify(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdIdentify(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdLifInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdAdminQInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdTxQInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRxQInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdNotifyQInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdFeatures(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdQEnable(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdQDisable(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdSetMode(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRxFilterAdd(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRxFilterDel(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdMacAddrGet(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdStatsDumpStart(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdStatsDumpStop(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRssHashSet(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRssIndirSet(void *req, void *req_data, void *resp, void *resp_data);

    /* RDMA Command handlers */
    enum DevcmdStatus _CmdRDMACreateEQ(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRDMACreateCQ(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRDMACreateAdminQ(void *req, void *req_data, void *resp, void *resp_data);

    uint64_t GetQstateAddr(uint8_t qtype, uint32_t qid);

    friend ostream &operator<<(ostream&, const Eth&);

    const char *opcode_to_str(enum cmd_opcode opcode);
    const char *lif_state_to_str(enum lif_state state);

    types::LifType ConvertDevTypeToLifType(EthDevType dev_type);
};

#endif
