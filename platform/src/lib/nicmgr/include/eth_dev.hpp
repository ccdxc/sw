/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __ETH_DEV_HPP__
#define __ETH_DEV_HPP__

#include <map>
#include <set>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "nic/include/eth_common.h"
#include "nic/include/notify.hpp"

#include "nic/sdk/lib/indexer/indexer.hpp"
#include "nic/sdk/platform/devapi/devapi.hpp"

#include "nic/sdk/platform/pciemgr/include/pciehdev_event.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"

#include "device.hpp"
#include "eth_lif.hpp"
#include "pd_client.hpp"

namespace pt = boost::property_tree;

// Doorbell address
#define UPD_BITS_POSITION (17)
#define LIF_BITS_POSITION (6)

#define DOORBELL_ADDR(lif_num)                                                                    \
    ((0x8400000) | (0xb << UPD_BITS_POSITION) | (lif_num << LIF_BITS_POSITION))

#define DEVCMD_TIMEOUT 5
#define PORT_MAC_STATS_REPORT_SIZE (1024)
#define PORT_PB_STATS_REPORT_SIZE  (1024)

/* We can transmit frames that are MTU+FCS, but only receive frames that are
 * MTU, including the fcs.  We will the OS about the smaller value without FCS,
 * and account for the additional FCS when programming the port.
 *
 * The MTU constants below do not include the FCS.
 */
#define ETH_FCS 4
#define MTU_DEFAULT 1500
#define MTU_MIN 64
#define MTU_MAX (9216 - ETH_FCS)

/**
 * ETH Device type
 * TODO: keep in sync with enum port_type in ionic_if.h
 *       until we can rework headers to have a single definition
 */
typedef enum EthDevType_s {
    ETH_UNKNOWN,
    ETH_HOST,
    ETH_HOST_MGMT,
    ETH_MNIC_OOB_MGMT,
    ETH_MNIC_INTERNAL_MGMT,
    ETH_MNIC_INBAND_MGMT,
    ETH_MNIC_CPU,
    ETH_MNIC_LEARN,
    ETH_MNIC_CONTROL,
} EthDevType;

struct eth_dev_res {
    uint32_t lif_base;
    uint32_t intr_base;
    uint64_t rx_eq_base;
    uint64_t tx_eq_base;
    // DEVCMD
    uint64_t regs_mem_addr;
    uint64_t port_info_addr;
    // CMB
    uint64_t cmb_mem_addr;
    uint32_t cmb_mem_size;
    // ROM
    uint64_t rom_mem_addr;
    uint32_t rom_mem_size;
};

/**
 * Eth Device Spec
 */
struct eth_devspec {
    // Delphi
    uint64_t dev_uuid;
    // Device
    EthDevType eth_type;
    std::string name;
    OpromType oprom;
    uint8_t pcie_port;
    uint32_t pcie_total_vfs;
    bool host_dev;
    bool vf_dev;
    // Network
    uint32_t uplink_port_num;
    std::string qos_group;
    // RES
    uint32_t lif_count;
    uint32_t rxq_count;
    uint32_t txq_count;
    uint32_t eq_count;
    uint32_t adminq_count;
    uint32_t intr_count;
    uint64_t mac_addr;
    // RDMA
    bool enable_rdma;
    uint32_t pte_count;
    uint32_t prefetch_count;
    uint32_t key_count;
    uint32_t ah_count;
    uint32_t rdma_sq_count;
    uint32_t rdma_rq_count;
    uint32_t rdma_cq_count;
    uint32_t rdma_eq_count;
    uint32_t rdma_aq_count;
    uint32_t rdma_num_dcqcn_profiles;
    uint32_t rdma_pid_count;
    uint32_t barmap_size; // in 8MB units
};

struct EthDevInfo {
    eth_dev_res *eth_res;
    eth_devspec *eth_spec;
};

/**
 * ETH PF Device
 */
class Eth : public Device
{
public:
    Eth(devapi *dev_api, void *dev_spec, PdClient *pd_client, EV_P);
    Eth(devapi *dev_api, struct EthDevInfo *dev_info, PdClient *pd_client, EV_P);
    ~Eth();

    static std::vector<Eth *> factory(devapi *dev_api, void *dev_spec, PdClient *pd_client, EV_P);
    void Init(struct eth_devspec *spec);
    void UpgradeGracefulInit(struct eth_devspec *spec);
    void UpgradeHitlessInit(struct eth_devspec *spec);

    std::string GetName() { return spec->name; }
    EthDevType GetEthType() { return spec->eth_type; }

    void DevcmdHandler();
    status_code_t CmdProxyHandler(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t CmdHandler(void *req, void *req_data, void *resp, void *resp_data);
    static struct eth_devspec *ParseConfig(boost::property_tree::ptree::value_type node);

    static lif_type_t ConvertDevTypeToLifType(EthDevType dev_type);

    void LinkEventHandler(port_status_t *evd);
    void XcvrEventHandler(port_status_t *evd);
    void QuiesceEventHandler(bool quiesce);
    void UpdateQStatus (bool enable);
    void HalEventHandler(bool status);
    void DelphiMountEventHandler(bool mounted);
    status_code_t Reset();
    void PcieResetEventHandler(uint32_t rsttype);

    void SetHalClient(devapi *dapi);

    int GenerateQstateInfoJson(pt::ptree &lifs);
    bool CreateHostDevice();
    void LocalDeviceInit();
    void LocalDeviceInitSkip();
    struct mnet_dev_create_req_t *GetDeviceCreateReq();
    void SetFwStatus(uint8_t fw_status);
    void HeartbeatEventHandler();

    bool IsDevQuiesced();
    bool IsDevReset();
    bool IsDevLif(uint32_t lif_id);
    bool IsPlatformDev();

    int SendDeviceReset(void);
    void GetEthDevInfo(struct EthDevInfo *dev_info);
    static std::string eth_type_to_str(EthDevType type);
    static EthDevType str_to_eth_type(std::string const &s);
    sdk_ret_t RemoveDevice(void);

private:
    // Device Spec
    const struct eth_devspec *spec;
    // Info
    char name[IONIC_IFNAMSIZ];
    // PD Info
    PdClient *pd;
    // HAL Info
    devapi *dev_api;
    // Lif map
    std::map<uint64_t, EthLif *> lif_map;
    // Active lif set
    std::set<uint16_t> active_lif_set;
    // Resources
    struct eth_dev_res dev_resources;
    // Devcmd
    uint64_t devcmd_mem_addr;
    union ionic_dev_regs *regs;
    union ionic_dev_cmd_regs *devcmd;
    // PCIe info
    pciehdev_t *pdev;
    // Port Info
    uint64_t host_port_info_addr;
    // Port Config
    union ionic_port_config *port_config;
    uint64_t port_config_addr;
    uint64_t host_port_config_addr;
    // Port Status
    struct ionic_port_status *port_status;
    uint64_t port_status_addr;
    uint64_t host_port_status_addr;
    // Port MAC Stats
    uint64_t port_mac_stats_addr;
    uint64_t host_port_mac_stats_addr;
    uint32_t port_mac_stats_size;
    // Port PacketBuffer (PB) Stats
    uint64_t port_pb_stats_addr;
    uint64_t host_port_pb_stats_addr;
    uint32_t port_pb_stats_size;
    // Tasks
    EV_P;
    ev_prepare devcmd_prepare = {0};
    ev_check devcmd_check = {0};
    ev_timer devcmd_timer = {0};
    ev_timer stats_timer = {0};
    // upgrade status
    bool is_device_upgrade;

    bool LoadOprom();

    // Devcmd Memory/control opertions
    void DevcmdRegInit(void);
    void DevcmdRegMemReserve(void);
    void DevcmdRegMemAlloc(void);
    void DevcmdRegsReset();
    void DevcmdPoll(void);
    void DevcmdInit(void);
    void DevcmdStart(void);
    void DevcmdStop(void);

    // init helper Funcitons
    void PortStatusMem(bool mem_clr);
    void PortConfigMem(bool mem_clr);
    void CMBMemReserve(void);
    void CMBMemAlloc(void);
    void EQstateMemAlloc(void);
    void EQstateMemReserve(void);
    void LifIDAlloc(void);
    void LifIDReserve(void);
    void IntrMemAlloc(void);
    void IntrMemReserve(void);

    /* Command Handlers */
    static void DevcmdPreparePoll(EV_P_ ev_prepare *w, int events);
    static void DevcmdCheckPoll(EV_P_ ev_check *w, int events);
    static void DevcmdTimerPoll(EV_P_ ev_timer *w, int events);

    status_code_t _CmdIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdReset(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdGetAttr(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdSetAttr(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdPortIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdPortInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdPortReset(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdPortGetAttr(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdPortSetAttr(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdQosIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdQosInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdQosUpdate(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdQosReset(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdQosClearStats(void *req, void *req_data, void *resp, void *resp_data);

    status_code_t _CmdLifIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdLifInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdLifReset(void *req, void *req_data, void *resp, void *resp_data);

    // Tasks
    static void StatsUpdate(EV_P_ ev_timer *w, int events);
    static void PortConfigUpdate(void *obj);
    static void PortStatusUpdate(void *obj);

    static std::string os_type_to_str(unsigned int os_type);

    const char *opcode_to_str(cmd_opcode_t opcode);
    const char *qos_class_to_str(uint8_t qos_class);

    // stats
    void StatsInit(void);
    uint64_t PortStatsGetOffset(uint32_t ifindex, sdk::types::mem_addr_t stats_hbm_base_addr);
    void PortMacStatsUpdateSize(uint32_t ifindex);
    void PortMacStatsMappingInit(const struct eth_devspec *spec, PdClient *pd);
    void PortPbStatsMappingInit(const struct eth_devspec *spec, PdClient *pd);
};

#endif
