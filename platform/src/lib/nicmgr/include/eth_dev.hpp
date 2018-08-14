/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __ETH_DEV_HPP__
#define __ETH_DEV_HPP__

#include <map>

#include "dev.hpp"
#include "pci_ids.h"
#include "misc.h"
#include "bdf.h"
#include "cfgspace.h"
#include "pciehost.h"
#include "pciehdevices.h"
#include "pciehw.h"
#include "pcieport.h"
#include "eth_common.h"
#include "sdk/indexer.hpp"


/* Supply these for ionic_if.h */
#define BIT(n)                  (1 << n)
#define TEST_BIT(x, n)          ((x) & (1 << n))
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define dma_addr_t uint64_t

#include "ionic_if.h"

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
    struct create_mr_cmd create_mr;
    struct create_cq_cmd create_cq;
    struct create_qp_cmd create_qp;
    struct modify_qp_cmd modify_qp;
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
    struct create_mr_comp create_mr;
    struct create_cq_comp create_cq;
    struct create_qp_comp create_qp;
    struct modify_qp_comp modify_qp;
};
#pragma pack(pop)

struct dev_cmd_regs {
    u32 signature;
    u32 done;
    union dev_cmd cmd;
    union dev_cmd_comp comp;
    uint8_t data[2048] __attribute__((aligned (2048)));
};

/**
 * ETH PF Device
 */
class Eth_PF : public Device {
public:
    Eth_PF(HalClient *hal_client, void *dev_spec);
    struct lif_info info;
    struct dev_cmd_regs *devcmd;
    void DevcmdPoll();
    void DevcmdHandler();
    enum DevcmdStatus CmdHandler(void *req, void *req_data,
        void *resp, void *resp_data);
private:
    /* Static members */
    static sdk::lib::indexer *fltr_allocator;

    /* Members */
    string name;
    struct eth_devspec *spec;
    // Hardware Info
    static struct queue_info qinfo[NUM_QUEUE_TYPES];
    // HAL Info
    HalClient *hal;
    uint64_t lif_handle;    // TODO: Support multiple LIFs per ETH device
    // Rss config
    uint16_t rss_type;
    string rss_key;
    string rss_indir;
    // PCIe info
    pciehdev_t *pdev;
    pciehdevice_resources_t pci_resources;
    // Network info
    map<uint64_t, uint64_t> mac_addrs;
    map<uint64_t, uint16_t> vlans;
    map<uint64_t, tuple<uint64_t, uint16_t>> mac_vlan;

    map<tuple<uint64_t, uint16_t>, uint64_t> endpoints;  /* (mac, vlan) > endpoint_handle */

    /* Command Handlers */
    enum DevcmdStatus _CmdReset(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdIdentify(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdLifInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdAdminQInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdTxQInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRxQInit(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdFeatures(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdQEnable(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdQDisable(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdSetMode(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRxFilterAdd(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRxFilterDel(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdMacAddrGet(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRssHashSet(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRssIndirSet(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRDMACreateQP(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRDMAModifyQP(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRDMACreateCQ(void *req, void *req_data, void *resp, void *resp_data);
    enum DevcmdStatus _CmdRDMACreateMR(void *req, void *req_data, void *resp, void *resp_data);    

    uint64_t GetQstateAddr(uint8_t qtype, uint32_t qid);

    friend ostream &operator<<(ostream&, const Eth_PF&);
};

#endif
