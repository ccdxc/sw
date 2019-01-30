/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __ETH_DEV_HPP__
#define __ETH_DEV_HPP__

#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "nic/include/notify.hpp"
#include "nic/include/edma.hpp"
#include "nic/include/eth_common.h"

#include "nic/sdk/lib/indexer/indexer.hpp"

#include "platform/src/lib/hal_api/include/hal_types.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "platform/src/lib/mnet/include/mnet.h"

#include "dev.hpp"
#include "eth_if.h"
#include "eth_lif.hpp"

namespace pt = boost::property_tree;

// Doorbell address
#define UPD_BITS_POSITION   (17)
#define LIF_BITS_POSITION   (6)

#define DOORBELL_ADDR(lif_num) \
    ((0x8400000) | (0xb << UPD_BITS_POSITION) | (lif_num << LIF_BITS_POSITION))

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

    void DevcmdHandler();
    enum status_code CmdHandler(void *req, void *req_data,
                                 void *resp, void *resp_data);
    enum status_code AdminCmdHandler(uint64_t lif_id,
                                      void *req, void *req_data,
                                      void *resp, void *resp_data);

    void LinkEventHandler(port_status_t *evd);
    void HalEventHandler(bool status);

    void SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client);
    hal_lif_info_t *GetHalLifInfo(void) { return lif_map[lif_base]->GetHalLifInfo(); }
    uint32_t GetHalLifCount() { return spec->lif_count; }

    void DevObjSave();
    int GenerateQstateInfoJson(pt::ptree &lifs);

private:
    // Device Spec
    const struct eth_devspec *spec;
    // PD Info
    PdClient *pd;
    // HAL Info
    HalClient *hal;
    HalCommonClient *hal_common_client;
    bool hal_status;
    // Resources
    std::map<uint64_t, EthLif *> lif_map;
    int32_t lif_base;
    uint32_t intr_base;
    // Devcmd
    struct dev_cmd_regs *devcmd;
    uint64_t devcmd_mem_addr;
    uint64_t devcmddb_mem_addr;
    // CMB
    uint64_t cmb_mem_addr;
    uint32_t cmb_mem_size;
    // ROM
    uint64_t rom_mem_addr;
    uint32_t rom_mem_size;
    // PCIe info
    pciehdev_t *pdev;
    // Tasks
    evutil_timer devcmd_timer;
    evutil_check devcmd_check;
    evutil_prepare devcmd_prepare;
    // Device Constructors
    bool CreateHostDevice();
    bool CreateLocalDevice();
    //
    bool LoadOprom();

    /* Command Handlers */
    static void DevcmdPoll(void *obj);
    enum status_code _CmdReset(void *req, void *req_data, void *resp, void *resp_data);
    enum status_code _CmdIdentify(void *req, void *req_data, void *resp, void *resp_data);
    enum status_code _CmdLifInit(void *req, void *req_data, void *resp, void *resp_data);
    enum status_code _CmdLifReset(void *req, void *req_data, void *resp, void *resp_data);
    enum status_code _CmdAdminQInit(void *req, void *req_data, void *resp, void *resp_data);

    const char *opcode_to_str(enum cmd_opcode opcode);
};

#endif
