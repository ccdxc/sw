/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __ETH_DEV_HPP__
#define __ETH_DEV_HPP__

#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "nic/include/notify.hpp"
#include "nic/include/eth_common.h"

#include "nic/sdk/lib/indexer/indexer.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/devapi/devapi_types.hpp"

#ifdef __aarch64__
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#endif
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"

#include "device.hpp"
#include "pd_client.hpp"
#include "eth_lif.hpp"

namespace pt = boost::property_tree;

// Doorbell address
#define UPD_BITS_POSITION   (17)
#define LIF_BITS_POSITION   (6)

#define DOORBELL_ADDR(lif_num) \
    ((0x8400000) | (0xb << UPD_BITS_POSITION) | (lif_num << LIF_BITS_POSITION))

/**
 * ETH Device type
 */
typedef enum EthDevType_s {
    ETH_UNKNOWN,
    ETH_HOST,
    ETH_HOST_MGMT,
    ETH_MNIC_OOB_MGMT,
    ETH_MNIC_INTERNAL_MGMT,
    ETH_MNIC_INBAND_MGMT,
    ETH_MNIC_CPU,
} EthDevType;

const char *eth_dev_type_to_str(EthDevType type);

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
    uint32_t key_count;
    uint32_t ah_count;
    uint32_t rdma_sq_count;
    uint32_t rdma_rq_count;
    uint32_t rdma_cq_count;
    uint32_t rdma_eq_count;
    uint32_t rdma_aq_count;
    uint32_t rdma_pid_count;
    uint32_t barmap_size;    // in 8MB units
};

/**
 * ETH PF Device
 */
class Eth : public Device {
public:
    Eth(devapi *dev_api,
        void *dev_spec,
        PdClient *pd_client,
        bool upg_mode = false);
    static std::vector<Eth*> factory(enum DeviceType type, devapi *dev_api,
         void *dev_spec,
         PdClient *pd_client,
         bool upg_mode);

    std::string GetName() { return spec->name; }
    EthDevType GetType() { return spec->eth_type; }

    void DevcmdHandler();
    status_code_t CmdHandler(void *req, void *req_data,
                                 void *resp, void *resp_data);
    static struct eth_devspec *ParseConfig(boost::property_tree::ptree::value_type node);

    static lif_type_t ConvertDevTypeToLifType(EthDevType dev_type);

    void LinkEventHandler(port_status_t *evd);
    void XcvrEventHandler(port_status_t *evd);
    void HalEventHandler(bool status);

    void SetHalClient(devapi *dapi);

    int GenerateQstateInfoJson(pt::ptree &lifs);
    bool CreateHostDevice();
    void SetFwStatus(uint8_t fw_status);

    bool IsDevQuiesced();
    bool IsDevReset();
    int SendFWDownEvent();

private:
    // Device Spec
    const struct eth_devspec *spec;
    // Info
    char name[IONIC_IFNAMSIZ];
    // PD Info
    PdClient *pd;
    // HAL Info
    devapi *dev_api;
    bool hal_status;
    // Resources
    std::map<uint64_t, EthLif *> lif_map;
    uint32_t lif_base;
    uint32_t intr_base;
    // Devcmd
    uint64_t regs_mem_addr;
    uint64_t devcmd_mem_addr;
    union dev_regs *regs;
    union dev_cmd_regs *devcmd;
    // CMB
    uint64_t cmb_mem_addr;
    uint32_t cmb_mem_size;
    // ROM
    uint64_t rom_mem_addr;
    uint32_t rom_mem_size;
    // Port Status
    struct port_info *port_info;
    uint64_t port_info_addr;
    uint64_t host_port_info_addr;
    uint64_t port_stats_addr;
    uint64_t host_port_stats_addr;
    // Tasks
    evutil_prepare devcmd_prepare = {0};
    evutil_check devcmd_check = {0};
    evutil_timer devcmd_timer = {0};
    evutil_timer stats_timer = {0};

    // Device Constructors
    bool CreateLocalDevice();
    //
    bool LoadOprom();

    //Lif ref cnt
    uint32_t active_lif_ref_cnt;

    static EthDevType eth_dev_type_str_to_type(std::string const& s);

    /* Command Handlers */
    static void DevcmdPoll(void *obj);

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

    status_code_t _CmdLifIdentify(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdLifInit(void *req, void *req_data, void *resp, void *resp_data);
    status_code_t _CmdLifReset(void *req, void *req_data, void *resp, void *resp_data);

    /* Proxy Command Handlers */
    status_code_t _CmdProxyHandler(void *req, void *req_data, void *resp, void *resp_data);

    // Tasks
    static void StatsUpdate(void *obj);
    static void StatsUpdateComplete(void *obj);

    const char *opcode_to_str(cmd_opcode_t opcode);

};

#endif
