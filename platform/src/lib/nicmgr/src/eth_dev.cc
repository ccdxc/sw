/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <endian.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/param.h>

#include "cap_ms_c_hdr.h"
#include "cap_pics_c_hdr.h"
#include "cap_top_csr_defines.h"
#include "cap_wa_c_hdr.h"

#include "nic/include/edmaq.h"
#include "nic/p4/common/defines.h"

#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/misc/include/maclib.h"
#include "nic/sdk/platform/mnet/include/mnet.h"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/include/sdk/port_utils.hpp"

#include "dev.hpp"
#include "edmaq.hpp"
#include "eth_dev.hpp"
#include "eth_if.h"
#include "logger.hpp"
#include "nicmgr_utils.hpp"
#include "pd_client.hpp"
#include "rdma_dev.hpp"

extern class pciemgr *pciemgr;

void
Eth::GetEthDevInfo(struct EthDevInfo *dev_info)
{
    dev_info->eth_res = &dev_resources;
    dev_info->eth_spec = const_cast<struct eth_devspec *>(spec);
}

std::string
Eth::eth_type_to_str(EthDevType type)
{
    if (type == ETH_HOST) {
        return "host";
    } else if (type == ETH_HOST_MGMT) {
        return "host_mgmt";
    } else if (type == ETH_MNIC_OOB_MGMT) {
        return "oob_mgmt";
    } else if (type == ETH_MNIC_INTERNAL_MGMT) {
        return "internal_mgmt";
    } else if (type == ETH_MNIC_INBAND_MGMT) {
        return "inband_mgmt";
    } else if (type == ETH_MNIC_CPU) {
        return "cpu";
    } else if (type == ETH_MNIC_LEARN) {
        return "learn";
    } else if (type == ETH_MNIC_CONTROL) {
        return "control";
    } else {
        NIC_LOG_ERR("Unknown ETH dev type: {}", type);
        return "Unknown";
    }
}

EthDevType
Eth::str_to_eth_type(std::string const &s)
{
    if (s == "host") {
        return ETH_HOST;
    } else if (s == "host_mgmt") {
        return ETH_HOST_MGMT;
    } else if (s == "oob_mgmt") {
        return ETH_MNIC_OOB_MGMT;
    } else if (s == "internal_mgmt") {
        return ETH_MNIC_INTERNAL_MGMT;
    } else if (s == "inband_mgmt") {
        return ETH_MNIC_INBAND_MGMT;
    } else if (s == "cpu") {
        return ETH_MNIC_CPU;
    } else if (s == "learn") {
        return ETH_MNIC_LEARN;
    } else if (s == "control") {
        return ETH_MNIC_CONTROL;
    } else {
        NIC_LOG_ERR("Unknown ETH dev type: {}", s);
        return ETH_UNKNOWN;
    }
}

uint64_t
Eth::PortStatsGetOffset(uint32_t ifindex, sdk::types::mem_addr_t stats_hbm_base_addr)
{

    // ifindex=0 for internal mnic
    if (ifindex == 0) {
        return 0;
    }
    return (stats_hbm_base_addr + port_stats_addr_offset(ifindex));
}

void
Eth::PortMacStatsUpdateSize(uint32_t ifindex)
{
    uint32_t parent_port;

    // ifindex=0 for internal mnic
    if (ifindex == 0) {
        port_mac_stats_size  = 0;
        return;
    }

    parent_port = ETH_IFINDEX_TO_PARENT_PORT(ifindex);
    // TODO store the type from catalog and set port_stats_size accordingly
    switch (parent_port) {
    case 1:
    case 2:
        // mx0, mx1
        port_mac_stats_size = sizeof(struct ionic_port_stats);
        break;
    case 3:
        // bx
        port_mac_stats_size = sizeof(struct ionic_mgmt_port_stats);
        break;
    default:
        port_mac_stats_size = 0;
        break;
    }
}

void
Eth::PortMacStatsMappingInit(const struct eth_devspec *spec, PdClient *pd)
{
    bool use_hbm = true;
    sdk::types::mem_addr_t mac_stats_hbm_base = INVALID_MEM_ADDRESS;

    if (pd == NULL) {
        use_hbm = false;
        NIC_LOG_ERR("{}: No PD allocation!", spec->name);
    } else if (pd->mp_ == NULL) {
        use_hbm = false;
        NIC_LOG_ERR("{}: No HBM region!", spec->name);
    }

    if (use_hbm == true) {
        mac_stats_hbm_base = pd->mp_->start_addr("port_stats");
        if ((mac_stats_hbm_base == 0) || (mac_stats_hbm_base == INVALID_MEM_ADDRESS)) {
            use_hbm = false;
            NIC_LOG_ERR("{}: Failed to get HBM mac stats hbm base addr!", spec->name);
        }
    }
    if (use_hbm == false) {
        port_mac_stats_addr = 0;
        port_mac_stats_size = 0;
        return;
    }

    port_mac_stats_addr = PortStatsGetOffset(spec->uplink_port_num, mac_stats_hbm_base);
    PortMacStatsUpdateSize(spec->uplink_port_num);

    NIC_LOG_INFO("{}: {:#d} port_mac_stats_addr {:#x} size {:#d}", spec->name, spec->uplink_port_num,
                 port_mac_stats_addr, port_mac_stats_size);
}

void
Eth::PortPbStatsMappingInit(const struct eth_devspec *spec, PdClient *pd)
{
    bool use_hbm = true;
    sdk::types::mem_addr_t pb_stats_hbm_base = INVALID_MEM_ADDRESS;

    if (pd == NULL) {
        use_hbm = false;
        NIC_LOG_ERR("{}: No PD allocation!", spec->name);
    } else if (pd->mp_ == NULL) {
        use_hbm = false;
        NIC_LOG_ERR("{}: No HBM region!", spec->name);
    }

    if (use_hbm == true) {
        pb_stats_hbm_base = pd->mp_->start_addr("uplink_stats");
        if ((pb_stats_hbm_base == 0) || (pb_stats_hbm_base == INVALID_MEM_ADDRESS)) {
            use_hbm = false;
            NIC_LOG_ERR("{}: Failed to get HBM pb stats base addr!", spec->name);
        }
    }
    if (use_hbm == false) {
        port_pb_stats_addr = 0;
        port_pb_stats_size = 0;
        return;
    }

    port_pb_stats_addr = PortStatsGetOffset(spec->uplink_port_num, pb_stats_hbm_base);
    port_pb_stats_size = sizeof(struct ionic_port_pb_stats);

    NIC_LOG_INFO("{}: {:#d} port_pb_stats_addr {:#x} size {:#d}", spec->name, spec->uplink_port_num,
                 port_pb_stats_addr, port_pb_stats_size);
}

// constructor used for normal boot
Eth::Eth(devapi *dev_api, void *dev_spec, PdClient *pd_client, EV_P) {
    this->dev_api = dev_api;
    this->spec = (struct eth_devspec *)dev_spec;
    this->pd = pd_client;
    this->loop = loop;
    this->active_lif_set.clear();
    this->host_port_info_addr = 0;
    this->host_port_mac_stats_addr = 0;
    this->host_port_pb_stats_addr = 0;
    this->host_port_status_addr = 0;
    this->host_port_config_addr = 0;
    NIC_LOG_DEBUG("{}: eth device initializing", spec->name);
}

// constructor used in upgrade boot
Eth::Eth(devapi *dev_api, struct EthDevInfo *dev_info,
         PdClient *pd_client, EV_P) {
    this->dev_api = dev_api;
    this->spec = dev_info->eth_spec;
    this->pd = pd_client;
    this->dev_resources = *(dev_info->eth_res);
    this->loop = loop;
    this->active_lif_set.clear();
    this->host_port_info_addr = 0;
    this->host_port_mac_stats_addr = 0;
    this->host_port_pb_stats_addr = 0;
    this->host_port_status_addr = 0;
    this->host_port_config_addr = 0;
    NIC_LOG_DEBUG("{}: Restoring eth device in Upgrade mode", spec->name);
}

void
Eth::Init(struct eth_devspec *spec) {
    uint64_t        lif_id;
    eth_lif_res_t   *lif_res;
    EthLif          *eth_lif;

    // alloc lif memory
    LifIDAlloc();
    IntrMemAlloc();
    EQstateMemAlloc();
    DevcmdRegMemAlloc();
    DevcmdRegInit();
    CMBMemAlloc();

    // Create all LIFs
    for (uint32_t lif_index = 0; lif_index < spec->lif_count; lif_index++) {
        lif_res = new eth_lif_res_t();
        lif_id = dev_resources.lif_base + lif_index;

        lif_res->lif_index = lif_index;
        lif_res->lif_id = lif_id;
        lif_res->intr_base = dev_resources.intr_base;
        lif_res->rx_eq_base = dev_resources.rx_eq_base;
        lif_res->tx_eq_base = dev_resources.tx_eq_base;
        lif_res->cmb_mem_addr = dev_resources.cmb_mem_addr;
        lif_res->cmb_mem_size = dev_resources.cmb_mem_size;

        eth_lif = new EthLif(this, dev_api, spec, pd, lif_res, loop);
        eth_lif->Init();

        lif_map[lif_id] = eth_lif;
    }

    // config stats and start services
    PortConfigMem(true);
    PortStatusMem(true);
    DevcmdInit();
    DevcmdStart();
    StatsInit();
}

void
Eth::UpgradeGracefulInit(struct eth_devspec *spec) {
    uint64_t        lif_id;
    eth_lif_res_t   *lif_res;
    EthLif          *eth_lif;

    // reserve lif memory
    LifIDReserve();
    IntrMemReserve();
    DevcmdRegMemReserve();
    CMBMemReserve();

    // Create all LIFs
    for (uint32_t lif_index = 0; lif_index < spec->lif_count; lif_index++) {
        lif_res = new eth_lif_res_t();
        lif_id = dev_resources.lif_base + lif_index;

        lif_res->lif_index = lif_index;
        lif_res->lif_id = lif_id;
        lif_res->intr_base = dev_resources.intr_base;
        lif_res->rx_eq_base = dev_resources.rx_eq_base;
        lif_res->tx_eq_base = dev_resources.tx_eq_base;
        lif_res->cmb_mem_addr = dev_resources.cmb_mem_addr;
        lif_res->cmb_mem_size = dev_resources.cmb_mem_size;

        eth_lif = new EthLif(this, dev_api, spec, pd, lif_res, loop);
        eth_lif->UpgradeGracefulInit();

        lif_map[lif_id] = eth_lif;
    }

    // config stats and start services
    PortConfigMem(false);
    PortStatusMem(false);
    DevcmdInit();
    DevcmdStart();
    StatsInit();
}

void
Eth::UpgradeHitlessInit(struct eth_devspec *spec) {
    uint64_t        lif_id;
    eth_lif_res_t   *lif_res;
    EthLif          *eth_lif;

    // reserve lif memory
    LifIDReserve();
    IntrMemReserve();
    DevcmdRegMemReserve();
    CMBMemReserve();

    // Create all LIFs
    for (uint32_t lif_index = 0; lif_index < spec->lif_count; lif_index++) {
        lif_res = new eth_lif_res_t();
        lif_id = dev_resources.lif_base + lif_index;

        lif_res->lif_index = lif_index;
        lif_res->lif_id = lif_id;
        lif_res->intr_base = dev_resources.intr_base;
        lif_res->rx_eq_base = dev_resources.rx_eq_base;
        lif_res->tx_eq_base = dev_resources.tx_eq_base;
        lif_res->cmb_mem_addr = dev_resources.cmb_mem_addr;
        lif_res->cmb_mem_size = dev_resources.cmb_mem_size;

        eth_lif = new EthLif(this, dev_api, spec, pd, lif_res, loop);
        eth_lif->UpgradeHitlessInit();

        lif_map[lif_id] = eth_lif;
    }

    // config stats and start services
    PortConfigMem(false);
    PortStatusMem(false);
    DevcmdInit();
    DevcmdStart();
    StatsInit();
}

void
Eth::LifIDAlloc(void) {
    sdk_ret_t ret = SDK_RET_OK;

    // Allocate lifs
    ret = pd->lm_->alloc_id(&dev_resources.lif_base, spec->lif_count);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to allocate lifs. ret: {}", spec->name, ret);
        throw;
    }
    NIC_LOG_DEBUG("{}: lif_base {} lif_count {}", spec->name, dev_resources.lif_base,
                  spec->lif_count);
}

void
Eth::LifIDReserve(void) {
    sdk_ret_t ret = SDK_RET_OK;

    // Reserve lifs
    ret = pd->lm_->reserve_id(dev_resources.lif_base, spec->lif_count);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to reserve lifs. ret: {}", spec->name, ret);
        throw;
    }
    NIC_LOG_DEBUG("{}: lif_base {} lif_count {}", spec->name, dev_resources.lif_base,
                  spec->lif_count);
}

void
Eth::IntrMemAlloc(void) {
    // Allocate interrupts
    dev_resources.intr_base = pd->intr_alloc(spec->intr_count);
    if (dev_resources.intr_base < 0) {
        NIC_LOG_ERR("{}: Failed to allocate interrupts", spec->name);
        throw;
    }
    NIC_LOG_DEBUG("{}: intr_base {} intr_count {}", spec->name, dev_resources.intr_base,
                  spec->intr_count);
}

void
Eth::IntrMemReserve(void) {
    int err = 0;

    // Reserve interrupts
    err = pd->intr_reserve(dev_resources.intr_base, spec->intr_count);
    if (err) {
        NIC_LOG_ERR("{}: Failed to reserve interrupt with error: {}", spec->name, err);
        throw;
    }
    NIC_LOG_DEBUG("{}: intr_base {} intr_count {}", spec->name, dev_resources.intr_base,
                  spec->intr_count);
}

void
Eth::EQstateMemAlloc(void) {
    // Allocate EQ states
    if (spec->eq_count) {
        dev_resources.rx_eq_base = pd->nicmgr_mem_alloc(spec->eq_count * sizeof(eth_eq_qstate_t));
        dev_resources.tx_eq_base = pd->nicmgr_mem_alloc(spec->eq_count * sizeof(eth_eq_qstate_t));
        if (dev_resources.rx_eq_base == 0 || dev_resources.tx_eq_base == 0) {
            NIC_LOG_ERR("{}: Failed to allocate eq states", spec->name);
            throw;
        }
    }
}

void
Eth::EQstateMemReserve(void) {
    int err = 0;

    if (spec->eq_count) {
        if (dev_resources.rx_eq_base) {
            err = pd->nicmgr_mem_reserve(dev_resources.rx_eq_base,
                                         spec->eq_count * sizeof(eth_eq_qstate_t));
            if (err) {
                NIC_LOG_ERR("{}: Failed to reserve rx eq states with error: {}", spec->name, err);
                throw;
            }
        } else {
            dev_resources.rx_eq_base =
                pd->nicmgr_mem_alloc(spec->eq_count * sizeof(eth_eq_qstate_t));
            if (dev_resources.rx_eq_base == 0) {
                NIC_LOG_ERR("{}: Failed to allocate rx eq states", spec->name);
                throw;
            }
        }

        if (dev_resources.tx_eq_base) {
            err = pd->nicmgr_mem_reserve(dev_resources.tx_eq_base,
                                         spec->eq_count * sizeof(eth_eq_qstate_t));
            if (err) {
                NIC_LOG_ERR("{}: Failed to reserve tx eq states with error: {}", spec->name, err);
                throw;
            }
        } else {
            dev_resources.tx_eq_base =
                pd->nicmgr_mem_alloc(spec->eq_count * sizeof(eth_eq_qstate_t));
            if (dev_resources.tx_eq_base == 0) {
                NIC_LOG_ERR("{}: Failed to allocate tx eq states", spec->name);
                throw;
            }
        }
    }
}

void
Eth::DevcmdRegMemAlloc(void) {
    // Allocate & Init Device registers
    dev_resources.regs_mem_addr = pd->devcmd_mem_alloc(sizeof(union ionic_dev_regs));
    if (dev_resources.regs_mem_addr == 0) {
        NIC_LOG_ERR("{}: Failed to allocate registers", spec->name);
        throw;
    }
    devcmd_mem_addr = dev_resources.regs_mem_addr + offsetof(union ionic_dev_regs, devcmd);

    NIC_LOG_DEBUG("{}: regs_mem_addr {:#x} devcmd_mem_addr {:#x}", spec->name,
                  dev_resources.regs_mem_addr, devcmd_mem_addr);

    regs = (union ionic_dev_regs *)MEM_MAP(dev_resources.regs_mem_addr,
                                           sizeof(union ionic_dev_regs), 0);
    if (regs == NULL) {
        NIC_LOG_ERR("{}: Failed to map register region", spec->name);
        throw;
    }

    devcmd = &regs->devcmd;
}

void
Eth::DevcmdRegMemReserve() {
    int err = 0;

    // Reserve Device registers
    err = pd->devcmd_mem_reserve(dev_resources.regs_mem_addr, sizeof(union ionic_dev_regs));
    if (err) {
        NIC_LOG_ERR("{}: Failed to reserve registers mem with error {}", spec->name, err);
        throw;
    }
    devcmd_mem_addr = dev_resources.regs_mem_addr + offsetof(union ionic_dev_regs, devcmd);

    NIC_LOG_DEBUG("{}: regs_mem_addr {:#x} devcmd_mem_addr {:#x}", spec->name,
                  dev_resources.regs_mem_addr, devcmd_mem_addr);

    regs = (union ionic_dev_regs *)MEM_MAP(dev_resources.regs_mem_addr, sizeof(union ionic_dev_regs), 0);
    if (regs == NULL) {
        NIC_LOG_ERR("{}: Failed to map register region", spec->name);
        throw;
    }

    devcmd = &regs->devcmd;
}

void
Eth::DevcmdRegInit(void) {
    const uint32_t sta_ver =
        READ_REG32(CAP_ADDR_BASE_MS_MS_OFFSET + CAP_MS_CSR_STA_VER_BYTE_ADDRESS);

    // Init Device registers
    regs->info.signature = IONIC_DEV_INFO_SIGNATURE;
    regs->info.version = 0x1;

    regs->info.asic_type = sta_ver & 0xf;
    regs->info.asic_rev = (sta_ver >> 4) & 0xfff;

#ifdef __aarch64__
    std::string sn;
    sdk::platform::readfrukey(BOARD_SERIALNUMBER_KEY, sn);
    strncpy0(regs->info.serial_num, sn.c_str(), sizeof(regs->info.serial_num));

    boost::property_tree::ptree ver;
    boost::property_tree::read_json(VERSION_FILE, ver);
    strncpy0(regs->info.fw_version, ver.get<std::string>("sw.version").c_str(),
             sizeof(regs->info.fw_version));
#endif

#ifndef __aarch64__
    WRITE_MEM(dev_resources.regs_mem_addr, (uint8_t *)regs, sizeof(*regs), 0);
#endif
}

void
Eth::CMBMemAlloc(void) {
    // Allocate CMB region
    if (spec->barmap_size) {
        dev_resources.cmb_mem_size = (spec->barmap_size << MEM_BARMAP_SIZE_SHIFT);
        if (dev_resources.cmb_mem_size < (8 * 1024 * 1024)) {
            NIC_LOG_ERR("{}: Invalid cmb bar size {}", spec->name, dev_resources.cmb_mem_size);
            throw;
        }

        dev_resources.cmb_mem_addr = pd->cmb_mem_alloc(dev_resources.cmb_mem_size);
        assert(dev_resources.cmb_mem_addr != 0);
        // bar address must be aligned to bar size
        assert((dev_resources.cmb_mem_size % dev_resources.cmb_mem_size) == 0);

        NIC_LOG_DEBUG("{}: cmb_mem_addr {:#x}, cmb_mem_size: {}", spec->name,
                      dev_resources.cmb_mem_addr, dev_resources.cmb_mem_size);
    } else {
        dev_resources.cmb_mem_addr = 0;
        dev_resources.cmb_mem_size = 0;
    }
}


void
Eth::CMBMemReserve(void) {
    int err = 0;

    // Allocate CMB region
    if (spec->barmap_size) {
        dev_resources.cmb_mem_size = (spec->barmap_size << MEM_BARMAP_SIZE_SHIFT);
        if (dev_resources.cmb_mem_size < (8 * 1024 * 1024)) {
            NIC_LOG_ERR("{}: Invalid cmb bar size {}", spec->name, dev_resources.cmb_mem_size);
            throw;
        }

        err = pd->cmb_mem_reserve(dev_resources.cmb_mem_addr, dev_resources.cmb_mem_size);
        if (err) {
            NIC_LOG_ERR("{}: Failed to reserve cmb mem with error: {}", spec->name, err);
            throw;
        }
        NIC_LOG_DEBUG("{}: cmb_mem_addr {:#x}, cmb_mem_size: {}", spec->name,
                      dev_resources.cmb_mem_addr, dev_resources.cmb_mem_size);
    } else {
        dev_resources.cmb_mem_addr = 0;
        dev_resources.cmb_mem_size = 0;
    }

}

void
Eth::PortConfigMem(bool mem_clr) {
    // Port Config
    port_config_addr = pd->nicmgr_mem_alloc(sizeof(union ionic_port_config));
    port_config = (union ionic_port_config *)MEM_MAP(port_config_addr,
                                             sizeof(union ionic_port_config), 0);
    if (port_config == NULL) {
        NIC_LOG_ERR("{}: Failed to map lif config!", spec->name);
        throw;
    }

    if (mem_clr) {
        MEM_CLR(port_config_addr, port_config, sizeof(union ionic_port_config));
    }
    NIC_LOG_INFO("{}: port_config_addr {:#x}", spec->name, port_config_addr);
}

void
Eth::PortStatusMem(bool mem_clr) {
    // Port Status
    port_status_addr = pd->nicmgr_mem_alloc(sizeof(struct ionic_port_status));
    port_status = (struct ionic_port_status *)MEM_MAP(port_status_addr,
                                              sizeof(struct ionic_port_status), 0);

    if (port_status == NULL) {
        NIC_LOG_ERR("{}: Failed to map lif status!", spec->name);
        throw;
    }

    if (mem_clr) {
        MEM_CLR(port_status_addr, port_status, sizeof(struct ionic_port_status));
    }
    NIC_LOG_INFO("{}: port_status_addr {:#x}", spec->name, port_status_addr);
}

void
Eth::StatsInit(void) {
    // init stats interface to host
    PortMacStatsMappingInit(spec, pd);
    PortPbStatsMappingInit(spec, pd);

    // Init stats timer
    ev_timer_init(&stats_timer, &Eth::StatsUpdate, 0.0, 0.1);
    stats_timer.data = this;

    // initialize heartbeat as 0 when device got created.
    regs->info.fw_heartbeat = 0;
}

void
Eth::DevcmdInit(void) {
    // Init Devcmd Handling
    ev_prepare_init(&devcmd_prepare, Eth::DevcmdPreparePoll);
    devcmd_prepare.data = this;
    ev_check_init(&devcmd_check, Eth::DevcmdCheckPoll);
    devcmd_check.data = this;
    ev_timer_init(&devcmd_timer, Eth::DevcmdTimerPoll, 0.0, 0.001);
    devcmd_timer.data = this;
}

void
Eth::DevcmdStart(void) {
    // Start Devcmd Handling
    ev_prepare_start(EV_A_ & devcmd_prepare);
    ev_check_start(EV_A_ & devcmd_check);
    ev_timer_start(EV_A_ & devcmd_timer);
}

void
Eth::DevcmdStop(void) {
    // Disable devcmd polling
    ev_prepare_stop(EV_A_ & devcmd_prepare);
    ev_check_stop(EV_A_ & devcmd_check);
    ev_timer_stop(EV_A_ & devcmd_timer);
}

Eth::~Eth()
{
    NIC_LOG_INFO("{}: Destroying device", spec->name);

    if (!IsDevReset()) {
        NIC_LOG_ERR("{}: Device not in RESET state during destroy", spec->name);
    }

    DevcmdStop();
}

std::vector<Eth *>
Eth::factory(devapi *dev_api, void *dev_spec, PdClient *pd_client, EV_P)
{
    std::vector<Eth *> eth_devs;
    Eth *dev_obj;
    struct eth_devspec *spec = (struct eth_devspec *)dev_spec;

    // Create object for PF device
    dev_obj = new Eth(dev_api, spec, pd_client, EV_A);
    dev_obj->Init(spec);
    eth_devs.push_back(dev_obj);

    NIC_LOG_DEBUG("{}: num_vfs: {}", spec->name, spec->pcie_total_vfs);

    // Create objects for all VFs within this PF
    for (uint32_t dev = 0; dev < spec->pcie_total_vfs; dev++) {
        // Copy the PF's spec and modify it
        struct eth_devspec *vf_spec = new struct eth_devspec(*spec);
        vf_spec->vf_dev = true;
        vf_spec->name = vf_spec->name + "_vf_" + std::to_string(dev);
        // Keeping the mac for VF as 0
        vf_spec->mac_addr = 0;
        // limit 1 lif per VF
        vf_spec->lif_count = 1;
        // TODO: no rdma on VFs for now
        vf_spec->enable_rdma = 0;
        // TODO: no cmb bar for now
        vf_spec->barmap_size = 0;
        // Create object for VF device
        dev_obj = new Eth(dev_api, vf_spec, pd_client, EV_A);
        dev_obj->Init(vf_spec);
        eth_devs.push_back(dev_obj);
    }

    return eth_devs;
}

struct eth_devspec *
Eth::ParseConfig(boost::property_tree::ptree::value_type node)
{
    eth_devspec *eth_spec;
    auto val = node.second;

    eth_spec = new struct eth_devspec;
    memset(eth_spec, 0, sizeof(*eth_spec));

    eth_spec->name = val.get<string>("name");
    eth_spec->dev_uuid = val.get<uint64_t>("dev_uuid");
    eth_spec->lif_count = val.get<uint64_t>("lif_count");
    eth_spec->rxq_count = val.get<uint64_t>("rxq_count");
    eth_spec->txq_count = val.get<uint64_t>("txq_count");
    eth_spec->eq_count = val.get<uint64_t>("eq_count", 0);
    eth_spec->adminq_count = val.get<uint64_t>("adminq_count");
    eth_spec->intr_count = val.get<uint64_t>("intr_count");
    eth_spec->barmap_size = val.get<uint16_t>("cmb_size", 0);

    auto mac_addr = val.get_optional<string>("mac_addr");
    if (mac_addr) {
        mac_from_str(&eth_spec->mac_addr, mac_addr->c_str());
    }

    if (val.get_optional<string>("rdma")) {
        eth_spec->enable_rdma = true;
        eth_spec->rdma_sq_count = val.get<uint64_t>("rdma.sq_count");
        eth_spec->rdma_rq_count = val.get<uint64_t>("rdma.rq_count");
        eth_spec->rdma_cq_count = val.get<uint64_t>("rdma.cq_count");
        eth_spec->rdma_eq_count = val.get<uint64_t>("rdma.eq_count");
        eth_spec->rdma_aq_count = val.get<uint64_t>("rdma.adminq_count");
        eth_spec->rdma_num_dcqcn_profiles = val.get<uint64_t>("rdma.num_dcqcn_profiles");
        eth_spec->rdma_pid_count = val.get<uint64_t>("rdma.pid_count");
        eth_spec->key_count = val.get<uint64_t>("rdma.key_count");
        eth_spec->pte_count = val.get<uint64_t>("rdma.pte_count");
        eth_spec->prefetch_count = val.get<uint64_t>("rdma.prefetch_count");
        eth_spec->ah_count = val.get<uint64_t>("rdma.ah_count");
    }

    if (val.get_optional<string>("network")) {
        eth_spec->uplink_port_num = val.get<uint64_t>("network.uplink");
    }

    eth_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
    eth_spec->pcie_total_vfs = val.get<uint8_t>("pcie.total_vfs", 0);

    if (val.get_optional<string>("pcie.oprom")) {
        eth_spec->oprom = str_to_oprom_type(val.get<string>("pcie.oprom"));
    }

    if (val.get_optional<string>("type")) {
        eth_spec->eth_type = str_to_eth_type(val.get<string>("type"));
    } else {
        eth_spec->eth_type = ETH_UNKNOWN;
    }

    if (eth_spec->eth_type == ETH_HOST_MGMT ||
        eth_spec->eth_type == ETH_MNIC_OOB_MGMT ||
        eth_spec->eth_type == ETH_MNIC_INTERNAL_MGMT ||
        eth_spec->eth_type == ETH_MNIC_INBAND_MGMT ||
        eth_spec->eth_type == ETH_MNIC_CONTROL) {
        eth_spec->qos_group = val.get<string>("qos_group", "CONTROL");
    } else {
        eth_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
    }

    NIC_LOG_DEBUG("Creating eth device with name: {}, type: {}, "
                  "pinned_uplink: {}, qos_group {}",
                  eth_spec->name, eth_type_to_str(eth_spec->eth_type),
                  eth_spec->uplink_port_num, eth_spec->qos_group);

    return eth_spec;
}

struct mnet_dev_create_req_t *
Eth::GetDeviceCreateReq()
{
    struct mnet_dev_create_req_t *mnet_req = NULL;

    mnet_req = (struct mnet_dev_create_req_t *)calloc(1, sizeof(*mnet_req));
    if (mnet_req == NULL) {
        NIC_LOG_ERR("{}: Failed to allocate mnet request", spec->name);
        return NULL;
    }
    mnet_req->regs_pa = dev_resources.regs_mem_addr;
    mnet_req->drvcfg_pa = intr_drvcfg_addr(dev_resources.intr_base);
    mnet_req->msixcfg_pa = intr_msixcfg_addr(dev_resources.intr_base);
    mnet_req->doorbell_pa = DOORBELL_ADDR(dev_resources.lif_base);
    strcpy(mnet_req->iface_name, spec->name.c_str());

    NIC_LOG_DEBUG("{}: regs_pa: {:#x}, "
                  "drvcfg_pa: {:#x}, msixcfg_pa: {:#x}, doorbell_pa: {:#x}",
                  mnet_req->iface_name, mnet_req->regs_pa, mnet_req->drvcfg_pa,
                  mnet_req->msixcfg_pa, mnet_req->doorbell_pa);
    return mnet_req;
}

void
Eth::LocalDeviceInitSkip(void)
{
    NIC_LOG_DEBUG("{}: Skipping MNIC device creation", spec->name);
    if (spec->eth_type != ETH_HOST && spec->eth_type != ETH_HOST_MGMT) {
        WRITE_DEVINFO(DeviceManager::GetInstance()->CfgPath().c_str(),
                      spec->name.c_str(), dev_resources.regs_mem_addr,
                      dev_resources.lif_base);
    }
}

void
Eth::LocalDeviceInit()
{
    NIC_LOG_DEBUG("{}: Initializing MNIC device", spec->name);

    // pba is unused by mnic, but config anyway
    intr_pba_cfg(dev_resources.lif_base, dev_resources.intr_base, spec->intr_count);
    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        // lif,port params will be unusued in local mode
        intr_fwcfg_msi(dev_resources.intr_base + intr, 0, 0);
        intr_fwcfg_local(dev_resources.intr_base + intr, 1);
    }
    // reset device registers to defaults
    intr_reset_dev(dev_resources.intr_base, spec->intr_count, 1);

}

sdk_ret_t
Eth::RemoveDevice(void)
{
#ifdef __aarch64__
    if (remove_mnet(spec->name.c_str()) != 0) {
        NIC_LOG_ERR("{}: Unable to remove the device", spec->name);
        return SDK_RET_ERR;
    }
#else
    NIC_LOG_DEBUG("{}: Skipping device removal", spec->name);
#endif
    return SDK_RET_OK;
}

bool
Eth::LoadOprom()
{
    std::string rom_file_path;
    uint64_t rom_file_size, rom_bar_size;

    dev_resources.rom_mem_addr = 0;
    dev_resources.rom_mem_size = 0;

    // FIXME: Get the filepaths from catalog
#ifdef __aarch64__
    rom_file_path = "/nic/oprom/";
#else
    rom_file_path = "/sw/platform/gen/";
#endif
    switch (spec->oprom) {
    case OPROM_LEGACY:
        rom_file_path += "ionic.rom";
        break;
    case OPROM_UEFI:
        rom_file_path += "ionic.efirom";
        break;
    case OPROM_UNIFIED:
        NIC_LOG_ERR("Not implemented!");
        return true;
    case OPROM_UNKNOWN:
        NIC_LOG_DEBUG("{}: No oprom configured", spec->name);
        return true;
    };

    // FIXME: The same ROM bar can be shared by all ethernet devices. We
    // can make this a static method and call it once.
    NIC_LOG_DEBUG("{}: Opening oprom {}", spec->name, rom_file_path);
    FILE *rom_file = fopen(rom_file_path.c_str(), "rb");
    if (rom_file == NULL) {
        NIC_LOG_ERR("{}: Failed to open oprom : {}", spec->name, strerror(errno));
        return false;
    }

    // Get file size
    fseek(rom_file, 0L, SEEK_END);
    rom_file_size = ftell(rom_file);
    rewind(rom_file);

    // FIXME: Create a new memory region and allocator for OPROMs.
    dev_resources.rom_mem_size = roundup_power2(rom_file_size);
    rom_bar_size = roundup_power2(rom_file_size);
    dev_resources.rom_mem_addr = roundup(pd->nicmgr_mem_alloc(2 * rom_bar_size), rom_bar_size);
    NIC_LOG_INFO("{}: rom_mem_addr {:#x} rom_mem_size {}"
                 " rom_file_size {} rom_bar_size {}",
                 spec->name, dev_resources.rom_mem_addr, dev_resources.rom_mem_size, rom_file_size,
                 rom_bar_size);
    // Must be naturally aligned
    if ((dev_resources.rom_mem_addr % rom_bar_size) != 0) {
        NIC_LOG_ERR("{}: rom_mem_addr is not naturally aligned", spec->name);
        fclose(rom_file);
        dev_resources.rom_mem_addr = 0;
        dev_resources.rom_mem_size = 0;
        return false;
    }

    NIC_LOG_INFO("{}: Writing oprom", spec->name);
    uint64_t rom_addr = dev_resources.rom_mem_addr;
    uint8_t buf[4096] = {0};
    uint32_t bytes_read = 0;
    while (!feof(rom_file)) {
        bytes_read = fread(buf, sizeof(buf[0]), sizeof(buf), rom_file);
        WRITE_MEM(rom_addr, buf, bytes_read, 0);
        rom_addr += bytes_read;
    }
    // zero-out rest of the bar
    MEM_SET(rom_addr, 0, dev_resources.rom_mem_size - rom_file_size, 0);
    NIC_LOG_INFO("{}: Finished writing oprom", spec->name);

    fclose(rom_file);

    return true;
}

bool
Eth::CreateHostDevice()
{
    pciehdevice_resources_t pres;

    if (!LoadOprom()) {
        NIC_LOG_ERR("{}: Failed to load oprom", spec->name);
        // FIXME: error out after oproms are packaged in the image
        // return false;
    }

    memset(&pres, 0, sizeof(pres));
    strncpy0(pres.pfres.name, spec->name.c_str(), sizeof(pres.pfres.name));
    pres.pfres.port = spec->pcie_port;
    pres.pfres.lifb = dev_resources.lif_base;
    pres.pfres.lifc = spec->lif_count;
    pres.pfres.intrb = dev_resources.intr_base;
    pres.pfres.intrc = spec->intr_count;
    pres.pfres.intrdmask = 1;
    pres.pfres.npids = spec->rdma_pid_count;
    pres.pfres.cmbpa = dev_resources.cmb_mem_addr;
    pres.pfres.cmbsz = dev_resources.cmb_mem_size;
    pres.pfres.cmbprefetch = 1;
    pres.pfres.rompa = dev_resources.rom_mem_addr;
    pres.pfres.romsz = dev_resources.rom_mem_size;
    pres.pfres.totalvfs = spec->pcie_total_vfs;
    pres.pfres.eth.devregspa = dev_resources.regs_mem_addr;
    pres.pfres.eth.devregssz = sizeof(union ionic_dev_regs);
    pres.pfres.eth.macaddr = spec->mac_addr;

    if (pres.pfres.totalvfs > 0) {
        pciehdev_res_t *vfres = &pres.vfres;
        pciehdev_res_t *pfres = &pres.pfres;
        vfres->is_vf = 1;
        if (pfres->lifc) {
            vfres->lifb = pfres->lifb + pfres->lifc;
            // XXX this matches what the resource allocator does for VFs,
            // but we should get this information from vf_spec.
            vfres->lifc = 1;
        }
        if (pfres->intrc) {
            vfres->intrb = pfres->intrb + pfres->intrc;
            vfres->intrc = pfres->intrc;
            vfres->intrdmask = pfres->intrdmask;
        }
        vfres->eth.devregspa = pfres->eth.devregspa + 0x1000;
        vfres->eth.devregssz = pfres->eth.devregssz;
        vfres->eth.devregs_stride = vfres->eth.devregssz;
    }

    // Create PCI device
    if (spec->eth_type == ETH_HOST) {
        NIC_LOG_DEBUG("{}: Creating Host device with total_vfs: {}", spec->name,
                      pres.pfres.totalvfs);
        pres.type = PCIEHDEVICE_ETH;
    } else if (spec->eth_type == ETH_HOST_MGMT) {
        NIC_LOG_DEBUG("{}: Creating Host Management device", spec->name);
        pres.type = PCIEHDEVICE_MGMTETH;
    } else {
        return false;
    }

    // Add device to PCI topology
    if (pciemgr) {
        int ret = pciemgr->add_devres(&pres);
        if (ret != 0) {
            NIC_LOG_ERR("{}: Failed to add PCI device to topology", spec->name);
            return false;
        }
    }

    return true;
}

/**
 * DevcmdRegsReset - Clears the dev_cmd_regs region
 */
void
Eth::DevcmdRegsReset()
{
    memset((uint8_t *)devcmd, 0, sizeof(union ionic_dev_cmd_regs));
#ifndef __aarch64__
    WRITE_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(union ionic_dev_cmd_regs), 0);
#endif
}

void
Eth::DevcmdPreparePoll(EV_P_ ev_prepare *w, int events)
{
    ((Eth *)w->data)->DevcmdPoll();
}

void
Eth::DevcmdCheckPoll(EV_P_ ev_check *w, int events)
{
    ((Eth *)w->data)->DevcmdPoll();
}

void
Eth::DevcmdTimerPoll(EV_P_ ev_timer *w, int events)
{
    ((Eth *)w->data)->DevcmdPoll();
}

void
Eth::DevcmdPoll()
{
#ifndef __aarch64__
    READ_MEM(devcmd_mem_addr, (uint8_t *)&devcmd->doorbell,
             sizeof(devcmd->doorbell), 0);
#endif
    if (devcmd->doorbell & 0x1) {
        NIC_LOG_INFO("{}: Devcmd doorbell", spec->name);
        devcmd->doorbell = 0x0;
#ifndef __aarch64__
        WRITE_MEM(devcmd_mem_addr, (uint8_t *)&devcmd->doorbell,
                  sizeof(devcmd->doorbell), 0);
#endif
        DevcmdHandler();
    }
}

void
Eth::DevcmdHandler()
{
    status_code_t status;
    timespec_t start_ts, end_ts, ts_diff;

    NIC_HEADER_TRACE("Devcmd");
    clock_gettime(CLOCK_MONOTONIC, &start_ts);
#ifndef __aarch64__
    // read devcmd region
    READ_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(union ionic_dev_cmd_regs), 0);
#endif
    if (devcmd->done != 0) {
        NIC_LOG_ERR("{}: Devcmd done is set before processing command, opcode {}", spec->name,
                    opcode_to_str((cmd_opcode_t)devcmd->cmd.cmd.opcode));
        status = IONIC_RC_ERROR;
        goto devcmd_done;
    }

    status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->comp, &devcmd->data);

    // write data
    if (status == IONIC_RC_SUCCESS) {
#ifndef __aarch64__
        WRITE_MEM(devcmd_mem_addr + offsetof(union ionic_dev_cmd_regs, data),
                  (uint8_t *)devcmd + offsetof(union ionic_dev_cmd_regs, data), sizeof(devcmd->data), 0);
#endif
    }

devcmd_done:
    devcmd->comp.comp.status = status;
    PAL_barrier();
    devcmd->done = 1;
#ifndef __aarch64__
    // write completion
    WRITE_MEM(devcmd_mem_addr + offsetof(union ionic_dev_cmd_regs, comp),
              (uint8_t *)devcmd + offsetof(union ionic_dev_cmd_regs, comp), sizeof(devcmd->comp), 0);

    // write done
    WRITE_MEM(devcmd_mem_addr + offsetof(union ionic_dev_cmd_regs, done),
              (uint8_t *)devcmd + offsetof(union ionic_dev_cmd_regs, done), sizeof(devcmd->done), 0);
#endif
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    ts_diff = sdk::timestamp_diff(&end_ts, &start_ts);
    NIC_LOG_DEBUG("DevCmd Time taken: {}s.{}ns", ts_diff.tv_sec, ts_diff.tv_nsec);
    if (ts_diff.tv_sec >= DEVCMD_TIMEOUT) {
        NIC_LOG_ERR("Fatal Error: Devmd took more than 2 secs");
    }
    NIC_HEADER_TRACE("Devcmd End");
}

const char *
Eth::opcode_to_str(cmd_opcode_t opcode)
{
    switch (opcode) {
        CASE(IONIC_CMD_NOP);
        CASE(IONIC_CMD_IDENTIFY);
        CASE(IONIC_CMD_INIT);
        CASE(IONIC_CMD_RESET);
        CASE(IONIC_CMD_GETATTR);
        CASE(IONIC_CMD_SETATTR);
        CASE(IONIC_CMD_PORT_IDENTIFY);
        CASE(IONIC_CMD_PORT_INIT);
        CASE(IONIC_CMD_PORT_RESET);
        CASE(IONIC_CMD_PORT_GETATTR);
        CASE(IONIC_CMD_PORT_SETATTR);
        CASE(IONIC_CMD_LIF_IDENTIFY);
        CASE(IONIC_CMD_LIF_INIT);
        CASE(IONIC_CMD_LIF_RESET);
        CASE(IONIC_CMD_QOS_CLASS_IDENTIFY);
        CASE(IONIC_CMD_QOS_CLASS_INIT);
        CASE(IONIC_CMD_QOS_CLASS_UPDATE);
        CASE(IONIC_CMD_QOS_CLASS_RESET);
        CASE(IONIC_CMD_FW_DOWNLOAD);
        CASE(IONIC_CMD_FW_CONTROL);
        CASE(IONIC_CMD_VF_GETATTR);
        CASE(IONIC_CMD_VF_SETATTR);
        default:
            return EthLif::opcode_to_str(opcode);
    }
}

std::string
Eth::os_type_to_str(unsigned int os_type)
{
    static char buf[16];
    const char *s;

    switch (os_type) {
    case IONIC_OS_TYPE_LINUX:   s = "Linux";   break;
    case IONIC_OS_TYPE_WIN:     s = "Windows"; break;
    case IONIC_OS_TYPE_DPDK:    s = "DPDK";    break;
    case IONIC_OS_TYPE_FREEBSD: s = "FreeBSD"; break;
    case IONIC_OS_TYPE_IPXE:    s = "iPXE";    break;
    case IONIC_OS_TYPE_ESXI:    s = "ESXi";    break;
    default:
        snprintf(buf, sizeof(buf), "unknown(%d)", os_type);
        s = buf;
        break;
    }
    return s;
}

status_code_t
Eth::CmdHandler(void *req, void *req_data, void *resp, void *resp_data)
{
    union ionic_dev_cmd *cmd = (union ionic_dev_cmd *)req;
    union ionic_dev_cmd_comp *comp = (union ionic_dev_cmd_comp *)resp;
    status_code_t status = IONIC_RC_SUCCESS;

    if ((cmd_opcode_t)cmd->cmd.opcode != IONIC_CMD_NOP) {
        NIC_LOG_DEBUG("{}: Handling cmd: {}", spec->name,
                      opcode_to_str((cmd_opcode_t)cmd->cmd.opcode));
    }

    switch ((cmd_opcode_t)cmd->cmd.opcode) {

    case IONIC_CMD_NOP:
        status = IONIC_RC_SUCCESS;
        break;

    /* Device Commands */
    case IONIC_CMD_IDENTIFY:
        status = _CmdIdentify(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_INIT:
        status = _CmdInit(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_RESET:
        status = _CmdReset(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_GETATTR:
        status = _CmdGetAttr(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_SETATTR:
        status = _CmdSetAttr(req, req_data, resp, resp_data);
        break;

    /* Port Commands */
    case IONIC_CMD_PORT_IDENTIFY:
        status = _CmdPortIdentify(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_PORT_INIT:
        status = _CmdPortInit(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_PORT_RESET:
        status = _CmdPortReset(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_PORT_GETATTR:
        status = _CmdPortGetAttr(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_PORT_SETATTR:
        status = _CmdPortSetAttr(req, req_data, resp, resp_data);
        break;

    /* LIF Commands */
    case IONIC_CMD_LIF_IDENTIFY:
        status = _CmdLifIdentify(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_LIF_INIT:
        status = _CmdLifInit(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_LIF_RESET:
        status = _CmdLifReset(req, req_data, resp, resp_data);
        break;

    /* QoS commands */
    case IONIC_CMD_QOS_CLASS_IDENTIFY:
        status = _CmdQosIdentify(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_QOS_CLASS_INIT:
        status = _CmdQosInit(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_QOS_CLASS_UPDATE:
        status = _CmdQosUpdate(req, req_data, resp, resp_data);
        break;

    case IONIC_CMD_QOS_CLASS_RESET:
        status = _CmdQosReset(req, req_data, resp, resp_data);
        break;

    default:
        // FIXME: Check if this is a valid opcode
        status = CmdProxyHandler(req, req_data, resp, resp_data);
        break;
    }

    comp->comp.status = status;

    if ((cmd_opcode_t)cmd->cmd.opcode != IONIC_CMD_NOP) {
        NIC_LOG_DEBUG("{}: Done cmd: {}, status: {}", spec->name,
                      opcode_to_str((cmd_opcode_t)cmd->cmd.opcode), status);
    }

    return (status);
}

status_code_t
Eth::_CmdIdentify(void *req, void *req_data, void *resp, void *resp_data)
{
    union ionic_drv_identity *drvid = (union ionic_drv_identity *)req_data;
    union ionic_dev_identity *ident = (union ionic_dev_identity *)resp_data;
    struct ionic_dev_identify_cmd *cmd = (struct ionic_dev_identify_cmd *)req;
    struct ionic_dev_identify_comp *comp = (struct ionic_dev_identify_comp *)resp;
    int mul, div;

    NIC_LOG_DEBUG("{}: {}", spec->name, opcode_to_str(cmd->opcode));

    // Note drvid (req_data) and ident (resp_data) are aliases to
    // the same memory region.  Process all "drvid" input parameters
    // first, then we'll clear "ident" and fill output info.

    // sanitize these strings
    drvid->os_dist_str[sizeof(drvid->os_dist_str) - 1] = '\0';
    drvid->kernel_ver_str[sizeof(drvid->kernel_ver_str) - 1] = '\0';
    drvid->driver_ver_str[sizeof(drvid->driver_ver_str) - 1] = '\0';
    // log driver identify info
    NIC_LOG_INFO("{}: driver identify info", spec->name);
    NIC_LOG_INFO("    os: {} {} {}",
                 os_type_to_str(drvid->os_type),
                 drvid->os_dist, drvid->os_dist_str);
    NIC_LOG_INFO("    kernel: {} {}",
                 drvid->kernel_ver, drvid->kernel_ver_str);
    NIC_LOG_INFO("    driver: {}",
                 drvid->driver_ver_str);

    memset(ident, 0, sizeof(union ionic_dev_identity));

    intr_coal_get_params(&mul, &div);

    ident->nports = 1;
    ident->nlifs = spec->lif_count;
    ident->nintrs = spec->intr_count;
    ident->ndbpgs_per_lif = MAX(spec->rdma_pid_count, 1);
    ident->intr_coal_mult = mul;
    ident->intr_coal_div = div;
    ident->eq_count = spec->eq_count;

    comp->ver = IONIC_IDENTITY_VERSION_1;

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdInit(void *req, void *req_data, void *resp, void *resp_data)
{
    struct ionic_dev_init_cmd *cmd = (struct ionic_dev_init_cmd *)req;
    status_code_t status;
    EthLif *eth_lif = NULL;

    NIC_LOG_DEBUG("{}: {}", spec->name, opcode_to_str(cmd->opcode));

    // for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
    //     intr_pba_clear(intr_base + intr);
    //     intr_drvcfg(intr_base + intr);
    // }

    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        eth_lif = it->second;
        status = eth_lif->Reset();
        if (status != IONIC_RC_SUCCESS) {
            NIC_LOG_ERR("{}: Failed to reset lif", spec->name);
            return (status);
        }
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdReset(void *req, void *req_data, void *resp, void *resp_data)
{
    struct ionic_dev_reset_cmd *cmd = (struct ionic_dev_reset_cmd *)req;
    status_code_t status;

    NIC_LOG_DEBUG("{}: {}", spec->name, opcode_to_str(cmd->opcode));
    status = Reset();

    return (status);
}

status_code_t
Eth::_CmdGetAttr(void *req, void *req_data, void *resp, void *resp_data)
{
    struct ionic_dev_getattr_cmd *cmd = (struct ionic_dev_getattr_cmd *)req;

    NIC_LOG_DEBUG("{}: {} attr {}", spec->name, opcode_to_str(cmd->opcode), cmd->attr);

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdSetAttr(void *req, void *req_data, void *resp, void *resp_data)
{
    struct ionic_dev_setattr_cmd *cmd = (struct ionic_dev_setattr_cmd *)req;

    NIC_LOG_DEBUG("{}: {} attr {}", spec->name, opcode_to_str(cmd->opcode), cmd->attr);

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdPortIdentify(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t ret = SDK_RET_OK;
    union ionic_port_identity *info = (union ionic_port_identity *)resp_data;
    union ionic_port_config *cfg = (union ionic_port_config *)&info->config;
    struct ionic_port_identify_cmd *cmd = (struct ionic_port_identify_cmd *)req;

    NIC_LOG_DEBUG("{}: {}", spec->name, opcode_to_str(cmd->opcode));

    DEVAPI_CHECK

    memset(info, 0, sizeof(union ionic_port_identity));

    info->type = spec->eth_type;

    if (spec->uplink_port_num == 0) {
        port_config->speed = IONIC_SPEED_1G;
        port_config->mtu = MTU_DEFAULT + ETH_FCS;
        port_config->state = IONIC_PORT_ADMIN_STATE_UP;
        return (IONIC_RC_SUCCESS);
    }

    ret = dev_api->port_get_config(spec->uplink_port_num, (port_config_t *)cfg);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: failed to get port config", spec->name);
        return (IONIC_RC_ERROR);
    }

    // The port mtu would incude the FCS.
    port_config->mtu -= ETH_FCS;

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdPortInit(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t ret = SDK_RET_OK;
    struct ionic_port_init_cmd *cmd = (struct ionic_port_init_cmd *)req;
    union ionic_port_config *cfg = (union ionic_port_config *)req_data;

    NIC_LOG_DEBUG("{}: {}", spec->name, opcode_to_str(cmd->opcode));

    if (spec->uplink_port_num == 0) {
        return (IONIC_RC_SUCCESS);
    }

    DEVAPI_CHECK

    // The host mtu would not incude the FCS.
    port_config->mtu += ETH_FCS;

    ret = dev_api->port_set_config(spec->uplink_port_num, (port_config_t *)cfg);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: failed to set port config", spec->name);
        return (IONIC_RC_ERROR);
    }

    if (cmd->info_pa) {
        host_port_info_addr = cmd->info_pa;
        NIC_LOG_INFO("{}: host_port_info_addr {:#x}", spec->name, cmd->info_pa);

        host_port_config_addr = cmd->info_pa + offsetof(struct ionic_port_info, config);
        NIC_LOG_INFO("{}: host_port_config_addr {:#x}", spec->name, host_port_config_addr);

        host_port_status_addr = cmd->info_pa + offsetof(struct ionic_port_info, status);
        NIC_LOG_INFO("{}: host_port_status_addr {:#x}", spec->name, host_port_status_addr);

        host_port_mac_stats_addr = cmd->info_pa + offsetof(struct ionic_port_info, stats);
        NIC_LOG_INFO("{}: host_port_mac_stats_addr {:#x}", spec->name, host_port_mac_stats_addr);

        host_port_pb_stats_addr = cmd->info_pa + offsetof(struct ionic_port_info, pb_stats);
        NIC_LOG_INFO("{}: host_port_pb_stats_addr {:#x}", spec->name, host_port_pb_stats_addr);
    } else {
        host_port_info_addr = 0;
        host_port_config_addr = 0;
        host_port_status_addr = 0;
        host_port_mac_stats_addr = 0;
        host_port_pb_stats_addr = 0;
    }

    memcpy(port_config, cfg, sizeof(*port_config));
    PortConfigUpdate(this);

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdPortReset(void *req, void *req_data, void *resp, void *resp_data)
{
    struct ionic_port_reset_cmd *cmd = (struct ionic_port_reset_cmd *)req;

    NIC_LOG_DEBUG("{}: {}", spec->name, opcode_to_str(cmd->opcode));

    if (spec->uplink_port_num == 0) {
        return (IONIC_RC_SUCCESS);
    }

    host_port_info_addr = 0;
    host_port_config_addr = 0;
    host_port_status_addr = 0;
    host_port_mac_stats_addr = 0;
    host_port_pb_stats_addr = 0;

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdPortSetAttr(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t ret = SDK_RET_OK;
    port_config_t cfg = {0};
    struct ionic_port_setattr_cmd *cmd = (struct ionic_port_setattr_cmd *)req;
    bool pause_config = FALSE;

    NIC_LOG_DEBUG("{}: {} attr {}", spec->name, opcode_to_str(cmd->opcode), cmd->attr);

    if (spec->uplink_port_num == 0) {
        return (IONIC_RC_SUCCESS);
    }

    DEVAPI_CHECK

    ret = dev_api->port_get_config(spec->uplink_port_num, &cfg);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: failed to get port config", spec->name);
        return (IONIC_RC_ERROR);
    }

    switch (cmd->attr) {
    case IONIC_PORT_ATTR_STATE:
        cfg.state = cmd->state;
        break;
    case IONIC_PORT_ATTR_SPEED:
        cfg.speed = cmd->speed;
        break;
    case IONIC_PORT_ATTR_MTU:
        // The host mtu would not incude the FCS.
        cfg.mtu = cmd->mtu + ETH_FCS;
        break;
    case IONIC_PORT_ATTR_AUTONEG:
        cfg.an_enable = cmd->an_enable;
        break;
    case IONIC_PORT_ATTR_FEC:
        cfg.fec_type = cmd->fec_type;
        break;
    case IONIC_PORT_ATTR_PAUSE:
        cfg.pause_type = cmd->pause_type;
        pause_config = TRUE;
        break;
    case IONIC_PORT_ATTR_LOOPBACK:
        cfg.loopback_mode = cmd->loopback_mode;
        break;
    case IONIC_PORT_ATTR_STATS_CTRL:
        switch (cmd->stats_ctl) {
        case IONIC_STATS_CTL_RESET:
            cfg.reset_mac_stats = 1;
            break;
        default:
            NIC_LOG_ERR("{}: UNKNOWN COMMAND {} FOR IONIC_PORT_ATTR_STATS_CTRL", spec->name,
                        cmd->stats_ctl);
            return (IONIC_RC_ENOSUPP);
        }

        break;
    default:
        NIC_LOG_ERR("{}: Unknown attr {}", spec->name, cmd->attr);
        return (IONIC_RC_ENOSUPP);
    }

    ret = dev_api->port_set_config(spec->uplink_port_num, &cfg);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: failed to set port config", spec->name);
        return (IONIC_RC_ERROR);
    }

    // if there is a change in the pause config between LINK-LEVEL and PFC,
    // update the global_pause_type
    if (pause_config) {
        NIC_LOG_DEBUG("Invoking dev_api call to set global pause_type {}", cmd->pause_type);
        ret = dev_api->qos_class_set_global_pause_type(cmd->pause_type);
        if (ret != SDK_RET_OK) {
            NIC_LOG_ERR("{}: failed to set global_pause_type to {} ret {}", spec->name,
                        cmd->pause_type, ret);
        } else {
            NIC_LOG_DEBUG("set global pause_type to {}", cmd->pause_type);
        }
    }

    memcpy(port_config, &cfg, sizeof(*port_config));
    PortConfigUpdate(this);

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdPortGetAttr(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t ret = SDK_RET_OK;
    port_config_t cfg = {0};
    struct ionic_port_getattr_cmd *cmd = (struct ionic_port_getattr_cmd *)req;
    struct ionic_port_getattr_comp *comp = (struct ionic_port_getattr_comp *)resp;

    NIC_LOG_DEBUG("{}: {} attr {}", spec->name, opcode_to_str(cmd->opcode), cmd->attr);

    if (spec->uplink_port_num == 0) {
        return (IONIC_RC_SUCCESS);
    }

    DEVAPI_CHECK

    ret = dev_api->port_get_config(spec->uplink_port_num, &cfg);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: failed to get port config", spec->name);
        return (IONIC_RC_ERROR);
    }

    switch (cmd->attr) {
    case IONIC_PORT_ATTR_STATE:
        comp->state = cfg.state;
        break;
    case IONIC_PORT_ATTR_SPEED:
        comp->speed = cfg.speed;
        break;
    case IONIC_PORT_ATTR_MTU:
        // The port mtu would incude the FCS.
        comp->mtu = cfg.mtu - ETH_FCS;
        break;
    case IONIC_PORT_ATTR_AUTONEG:
        comp->an_enable = cfg.an_enable;
        break;
    case IONIC_PORT_ATTR_FEC:
        comp->fec_type = cfg.fec_type;
        break;
    case IONIC_PORT_ATTR_PAUSE:
        comp->pause_type = cfg.pause_type;
        break;
    case IONIC_PORT_ATTR_LOOPBACK:
        break;
    default:
        NIC_LOG_ERR("{}: Unknown attr {}", spec->name, cmd->attr);
        return (IONIC_RC_ERROR);
    }

    memcpy(port_config, &cfg, sizeof(*port_config));
    PortConfigUpdate(this);

    return (IONIC_RC_SUCCESS);
}

const char *
Eth::qos_class_to_str(u8 qos_class)
{
    switch (qos_class) {
        CASE(IONIC_QOS_CLASS_DEFAULT);
        CASE(IONIC_QOS_CLASS_USER_DEFINED_1);
        CASE(IONIC_QOS_CLASS_USER_DEFINED_2);
        CASE(IONIC_QOS_CLASS_USER_DEFINED_3);
        CASE(IONIC_QOS_CLASS_USER_DEFINED_4);
        CASE(IONIC_QOS_CLASS_USER_DEFINED_5);
        CASE(IONIC_QOS_CLASS_USER_DEFINED_6);
    default:
        return "IONIC_QOS_CLASS_UNKNOWN";
    }
}

status_code_t
Eth::_CmdQosIdentify(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t rs = SDK_RET_OK;
    struct ionic_qos_identify_cmd *cmd = (struct ionic_qos_identify_cmd *)req;
    union ionic_qos_identity *ident = (union ionic_qos_identity *)resp_data;
    union ionic_qos_config *cfg;
    qos_class_info_t info = {0};

    NIC_LOG_DEBUG("{}: {}", spec->name, opcode_to_str(cmd->opcode));

    DEVAPI_CHECK

    memset(ident, 0, sizeof(union ionic_qos_identity));

    ident->version = 1;
    ident->type = 0;

    /* convert qos class info to qos config */
    auto to_config = [](qos_class_info_t *info, union ionic_qos_config *cfg) {
        cfg->flags = IONIC_QOS_CONFIG_F_ENABLE;

        cfg->mtu = info->mtu;

        cfg->class_type = info->class_type;
        if (info->class_type == sdk::platform::QOS_CLASS_TYPE_PCP) {
            cfg->dot1q_pcp = info->class_dot1q_pcp;
        } else if (info->class_type == sdk::platform::QOS_CLASS_TYPE_DSCP) {
            cfg->ndscp = info->class_ndscp;
            for (uint8_t i = 0; i < info->class_ndscp; i++)
                cfg->ip_dscp[i] = info->class_ip_dscp[i];
        }

        if ( (info->group == IONIC_QOS_CLASS_DEFAULT) &&
             ((info->class_dot1q_pcp == 0) ||
              (info->class_type == sdk::platform::QOS_CLASS_TYPE_NONE)) ) {
            NIC_LOG_DEBUG("Returning do1q_pcp as match-all value (0xFF) for class-default");
            /*cfg->dot1q_pcp = info->class_dot1q_pcp;*/
            cfg->dot1q_pcp = 0xFF;
        }

        if(info->no_drop)
            cfg->flags |= IONIC_QOS_CONFIG_F_NO_DROP;

        cfg->pfc_cos = info->pause_dot1q_pcp;

        cfg->sched_type = info->sched_type;
        if (info->sched_type == sdk::platform::QOS_SCHED_TYPE_STRICT) {
            cfg->strict_rlmt = info->sched_strict_rlmt;
        } else if (info->sched_type == sdk::platform::QOS_SCHED_TYPE_DWRR) {
            cfg->dwrr_weight = info->sched_dwrr_weight;
        }

        if (info->rewrite_dot1q_pcp_en) {
            cfg->flags |= IONIC_QOS_CONFIG_F_RW_DOT1Q_PCP;
            cfg->rw_dot1q_pcp = info->rewrite_dot1q_pcp;
        }

        if (info->rewrite_ip_dscp_en) {
            cfg->flags |= IONIC_QOS_CONFIG_F_RW_IP_DSCP;
            cfg->rw_ip_dscp = info->rewrite_ip_dscp;
        }
        NIC_LOG_DEBUG("pause_type: {} no_drop {} flags: {} dot1q_pcp {} class_type {}",
                      info->pause_type, info->no_drop, cfg->flags, cfg->dot1q_pcp, info->class_type);
    };

    for (unsigned int i = 0; i < IONIC_QOS_CLASS_MAX; i++) {
        rs = dev_api->qos_class_exist(i);
        if (rs != SDK_RET_OK) {
            NIC_LOG_DEBUG("{}: qos group {} doesn't exist", spec->name, i);
            continue;
        }

        rs = dev_api->qos_class_get(i, &info);
        if (rs != SDK_RET_OK) {
            NIC_LOG_WARN("{}: failed to get qos group {}", spec->name, i);
            continue;
        }

        cfg = &ident->config[i];
        strncpy0(cfg->name, qos_class_to_str(i), sizeof(cfg->name));
        to_config(&info, cfg);
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdQosInit(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t rs = SDK_RET_OK;
    union ionic_qos_config *cfg = (union ionic_qos_config *)req_data;
    struct ionic_qos_init_cmd *cmd = (struct ionic_qos_init_cmd *)req;
    qos_class_info_t info = {0};

    NIC_LOG_DEBUG("{}: {} qos group {}", spec->name, opcode_to_str(cmd->opcode),
                  qos_class_to_str(cmd->group));

    DEVAPI_CHECK

    info.group = cmd->group;

    info.mtu = cfg->mtu;

    info.class_type = cfg->class_type;
    if (info.class_type == sdk::platform::QOS_CLASS_TYPE_PCP) {
        info.class_dot1q_pcp = cfg->dot1q_pcp;
    } else if (info.class_type == sdk::platform::QOS_CLASS_TYPE_DSCP) {
        info.class_ndscp = cfg->ndscp;
        for (uint8_t i = 0; i < info.class_ndscp; i++)
            info.class_ip_dscp[i] = cfg->ip_dscp[i];
    }

    if (cfg->pause_type == IONIC_PORT_PAUSE_TYPE_LINK) {
        info.pause_type = sdk::platform::PAUSE_TYPE_LINK_LEVEL;
    } else if (cfg->pause_type == IONIC_PORT_PAUSE_TYPE_PFC) {
        info.pause_type = sdk::platform::PAUSE_TYPE_PFC;
    } else {
        info.pause_type = sdk::platform::PAUSE_TYPE_NONE;
    }
    info.pause_dot1q_pcp = cfg->pfc_cos;

    info.sched_type = cfg->sched_type;
    if (info.sched_type == sdk::platform::QOS_SCHED_TYPE_STRICT) {
        info.sched_strict_rlmt = cfg->strict_rlmt;
    } else if (info.sched_type == sdk::platform::QOS_SCHED_TYPE_DWRR) {
        info.sched_dwrr_weight = cfg->dwrr_weight;
    }

    if (cfg->flags & IONIC_QOS_CONFIG_F_RW_DOT1Q_PCP) {
        info.rewrite_dot1q_pcp = cfg->rw_dot1q_pcp;
    }

    if (cfg->flags & IONIC_QOS_CONFIG_F_RW_IP_DSCP) {
        info.rewrite_ip_dscp = cfg->rw_ip_dscp;
    }

    if (cfg->flags & IONIC_QOS_CONFIG_F_NO_DROP) {
        info.no_drop = true;
    } else {
        info.no_drop = false;
    }

    NIC_LOG_DEBUG("group: {} mtu: {} class_type: {} pcp: {} pause_type: {} "
                  "pfc_cos: {} no_drop {} flags: {}",
                  info.group, info.mtu, info.class_type, info.class_dot1q_pcp,
                  info.pause_type, info.pause_dot1q_pcp, info.no_drop, cfg->flags);

    rs = dev_api->qos_class_create(&info);
    if (rs != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to create qos group {}", spec->name, cmd->group);
        return (IONIC_RC_ERROR);
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdQosUpdate(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t rs = SDK_RET_OK;
    union ionic_qos_config *cfg = (union ionic_qos_config *)req_data;
    struct ionic_qos_init_cmd *cmd = (struct ionic_qos_init_cmd *)req;
    qos_class_info_t info = {0};

    NIC_LOG_DEBUG("{}: {} qos group {}", spec->name, opcode_to_str(cmd->opcode),
        qos_class_to_str(cmd->group));

    DEVAPI_CHECK

    rs = dev_api->qos_class_exist(cmd->group);
    if (rs != SDK_RET_OK) {
        NIC_LOG_ERR("{}: qos group doesn't exist for update {}", spec->name, cmd->group);
        return (IONIC_RC_ERROR);
    }

    rs = dev_api->qos_class_delete(cmd->group);
    if (rs != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to delete qos group for update {}", spec->name, cmd->group);
        return (IONIC_RC_ERROR);
    }

    info.group = cmd->group;
    info.mtu = cfg->mtu;
    info.class_type = cfg->class_type;
    if (info.class_type == sdk::platform::QOS_CLASS_TYPE_PCP) {
        info.class_dot1q_pcp = cfg->dot1q_pcp;
    } else if (info.class_type == sdk::platform::QOS_CLASS_TYPE_DSCP) {
        info.class_ndscp = cfg->ndscp;
        for (uint8_t i = 0; i < info.class_ndscp; i++)
            info.class_ip_dscp[i] = cfg->ip_dscp[i];
    }

    if (cfg->pause_type == IONIC_PORT_PAUSE_TYPE_LINK) {
        info.pause_type = sdk::platform::PAUSE_TYPE_LINK_LEVEL;
    } else if (cfg->pause_type == IONIC_PORT_PAUSE_TYPE_PFC) {
        info.pause_type = sdk::platform::PAUSE_TYPE_PFC;
    }
    info.pause_dot1q_pcp = cfg->pfc_cos;

    info.sched_type = cfg->sched_type;
    if (info.sched_type == sdk::platform::QOS_SCHED_TYPE_STRICT) {
        info.sched_strict_rlmt = cfg->strict_rlmt;
    } else if (info.sched_type == sdk::platform::QOS_SCHED_TYPE_DWRR) {
        info.sched_dwrr_weight = cfg->dwrr_weight;
    }

    if (cfg->flags & IONIC_QOS_CONFIG_F_RW_DOT1Q_PCP) {
        info.rewrite_dot1q_pcp = cfg->rw_dot1q_pcp;
    }

    if (cfg->flags & IONIC_QOS_CONFIG_F_RW_IP_DSCP) {
        info.rewrite_ip_dscp = cfg->rw_ip_dscp;
    }

    if (cfg->flags & IONIC_QOS_CONFIG_F_NO_DROP) {
        info.no_drop = true;
    } else {
        info.no_drop = false;
    }

    NIC_LOG_DEBUG("group: {} mtu: {} class_type: {} pcp: {} pause_type: {} "
                  "pfc_cos: {} no_drop {} flags: {}",
                  info.group, info.mtu, info.class_type, info.class_dot1q_pcp,
                  info.pause_type, info.pause_dot1q_pcp, info.no_drop, cfg->flags);

    rs = dev_api->qos_class_create(&info);
    if (rs != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to update qos group {}", spec->name, cmd->group);
        return (IONIC_RC_ERROR);
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdQosReset(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t rs = SDK_RET_OK;
    struct ionic_qos_reset_cmd *cmd = (struct ionic_qos_reset_cmd *)req;

    NIC_LOG_DEBUG("{}: {} {}", spec->name, opcode_to_str(cmd->opcode),
                  qos_class_to_str(cmd->group));

    DEVAPI_CHECK

    rs = dev_api->qos_class_delete(cmd->group);
    if (rs != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to delete qos group {}", spec->name, cmd->group);
        return (IONIC_RC_ERROR);
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdLifIdentify(void *req, void *req_data, void *resp, void *resp_data)
{
    union ionic_lif_identity *ident = (union ionic_lif_identity *)resp_data;
    struct ionic_lif_identify_cmd *cmd = (struct ionic_lif_identify_cmd *)req;
    struct ionic_lif_identify_comp *comp = (struct ionic_lif_identify_comp *)resp;

    NIC_LOG_DEBUG("{}: {}", spec->name, opcode_to_str(cmd->opcode));

    memset(ident, 0, sizeof(union ionic_lif_identity));

    ident->capabilities = IONIC_LIF_CAP_ETH;

    ident->eth.version = 1;
    dev_api->lif_get_max_filters(&ident->eth.max_ucast_filters, &ident->eth.max_mcast_filters);
    ident->eth.rss_ind_tbl_sz = RSS_IND_TBL_SIZE;

    if (spec->eth_type == ETH_HOST_MGMT || spec->eth_type == ETH_MNIC_INTERNAL_MGMT) {
        ident->eth.min_frame_size = MTU_DEFAULT;
        ident->eth.max_frame_size = MTU_DEFAULT;
    } else {
        ident->eth.min_frame_size = MTU_MIN;
        ident->eth.max_frame_size = MTU_MAX;
    }

    ident->eth.config.features = 0;
    ident->eth.config.queue_count[IONIC_QTYPE_ADMINQ] = 1; // spec->adminq_count;
    ident->eth.config.queue_count[IONIC_QTYPE_NOTIFYQ] = 1;
    ident->eth.config.queue_count[IONIC_QTYPE_TXQ] = spec->txq_count;
    ident->eth.config.queue_count[IONIC_QTYPE_RXQ] = spec->rxq_count;
    ident->eth.config.queue_count[IONIC_QTYPE_EQ] = 0; // spec->eq_count in device identify

    if (spec->enable_rdma) {
        ident->capabilities |= IONIC_LIF_CAP_RDMA;

        ident->rdma.version = 1;
        ident->rdma.qp_opcodes = 11;
        ident->rdma.admin_opcodes = 17;
        ident->rdma.npts_per_lif = spec->pte_count;
        ident->rdma.nmrs_per_lif = spec->key_count;
        ident->rdma.nahs_per_lif = spec->ah_count;
        ident->rdma.max_stride = 11;
        ident->rdma.cl_stride = 6;
        ident->rdma.pte_stride = 3;
        ident->rdma.rrq_stride = 6;
        ident->rdma.rsq_stride = 5;

        ident->rdma.aq_qtype.qtype = ETH_HW_QTYPE_ADMIN;
        ident->rdma.aq_qtype.qid_count = spec->rdma_aq_count;
        ident->rdma.aq_qtype.qid_base = spec->adminq_count;

        ident->rdma.sq_qtype.qtype = ETH_HW_QTYPE_SQ;
        ident->rdma.sq_qtype.qid_count = spec->rdma_sq_count;
        ident->rdma.sq_qtype.qid_base = 0;

        ident->rdma.rq_qtype.qtype = ETH_HW_QTYPE_RQ;
        ident->rdma.rq_qtype.qid_count = spec->rdma_rq_count;
        ident->rdma.rq_qtype.qid_base = 0;

        ident->rdma.cq_qtype.qtype = ETH_HW_QTYPE_CQ;
        ident->rdma.cq_qtype.qid_count = spec->rdma_cq_count;
        ident->rdma.cq_qtype.qid_base = 0;

        ident->rdma.eq_qtype.qtype = ETH_HW_QTYPE_EQ;
        ident->rdma.eq_qtype.qid_count = spec->rdma_eq_count;
        ident->rdma.eq_qtype.qid_base = 0;

        ident->rdma.dcqcn_profiles = spec->rdma_num_dcqcn_profiles;
    }

    comp->ver = IONIC_IDENTITY_VERSION_1;

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdLifInit(void *req, void *req_data, void *resp, void *resp_data)
{
    struct ionic_lif_init_cmd *cmd = (struct ionic_lif_init_cmd *)req;
    struct ionic_lif_init_comp *comp = (struct ionic_lif_init_comp *)resp;
    uint64_t lif_id = 0;
    EthLif *eth_lif = NULL;
    status_code_t ret = IONIC_RC_SUCCESS;
    sdk_ret_t rs = SDK_RET_OK;

    NIC_LOG_DEBUG("{}: {}: index {}", spec->name, opcode_to_str(cmd->opcode), cmd->index);

    DEVAPI_CHECK

    if (spec->enable_rdma && cmd->index > 0) {
        NIC_LOG_INFO("{}: Multi-lif not supported on RDMA enabled device", spec->name);
        lif_id = dev_resources.lif_base;
        comp->hw_index = 0;
    } else {
        if (cmd->index >= spec->lif_count) {
            NIC_LOG_ERR("{}: bad lif index {}", spec->name, cmd->index);
            return (IONIC_RC_ERROR);
        }
        lif_id = dev_resources.lif_base + cmd->index;
        comp->hw_index = cmd->index;
    }

    auto it = lif_map.find(lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, lif_id);
        return (IONIC_RC_ERROR);
    }
    eth_lif = it->second;

    ret = eth_lif->CmdInit(req, req_data, resp, resp_data);
    if (ret != IONIC_RC_SUCCESS) {
        NIC_LOG_ERR("{}: Failed to initialize lif, status {}", spec->name, ret);
        return (ret);
    }

    active_lif_set.insert(lif_id);
    NIC_LOG_DEBUG("{}: active_lif_cnt: {}", spec->name, active_lif_set.size());

    if (spec->uplink_port_num == 0) {
        port_status->id = 0;
        port_status->speed = IONIC_SPEED_1G;
        port_status->status = IONIC_PORT_OPER_STATUS_UP;
        port_status->fec_type = IONIC_PORT_FEC_TYPE_NONE;
    } else {
        // Update port config
        rs = dev_api->port_get_config(spec->uplink_port_num, (port_config_t *)port_config);
        if (rs != SDK_RET_OK) {
            NIC_LOG_ERR("{}: Unable to get port config {}", spec->name, lif_id);
            return (IONIC_RC_ERROR);
        }

        // Update port status
        rs = dev_api->port_get_status(spec->uplink_port_num, (port_status_t *)port_status);
        if (rs != SDK_RET_OK) {
            NIC_LOG_ERR("{}: Unable to get port status {}", spec->name, lif_id);
            return (IONIC_RC_ERROR);
        }
    };

    if (spec->eth_type == ETH_HOST && cmd->index == 0 && host_port_mac_stats_addr != 0) {
        NIC_LOG_INFO("{}: port{}: Starting stats update to "
                     "host_port_mac_stats_addr {:#x} host_port_pb_stats_addr {:#x}",
                     spec->name, spec->uplink_port_num,
                     host_port_mac_stats_addr, host_port_pb_stats_addr);

        // Start stats timer
        ev_timer_start(EV_A_ &stats_timer);
    }

    // TODO: Workaround for linkmgr not setting port id
    port_status->id = spec->uplink_port_num;

    PortConfigUpdate(this);
    PortStatusUpdate(this);

    // Update link status
    eth_lif->LinkEventHandler((port_status_t *)port_status);

    return (ret);
}

status_code_t
Eth::_CmdLifReset(void *req, void *req_data, void *resp, void *resp_data)
{
    status_code_t ret;
    struct ionic_lif_reset_cmd *cmd = (struct ionic_lif_reset_cmd *)req;
    uint64_t lif_id = 0;
    EthLif *eth_lif = NULL;

    NIC_LOG_DEBUG("{}: {}: index {}", spec->name, opcode_to_str(cmd->opcode), cmd->index);

    DEVAPI_CHECK

    if (spec->enable_rdma && cmd->index > 0) {
        NIC_LOG_INFO("{}: Multi-lif not supported on RDMA enabled device", spec->name);
        lif_id = dev_resources.lif_base;
    } else {
        if (cmd->index >= spec->lif_count) {
            NIC_LOG_ERR("{}: bad lif index {}", spec->name, cmd->index);
            return (IONIC_RC_ERROR);
        }
        lif_id = dev_resources.lif_base + cmd->index;
    }

    auto it = lif_map.find(lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, lif_id);
        return (IONIC_RC_ERROR);
    }
    eth_lif = it->second;

    if (spec->eth_type == ETH_HOST && cmd->index == 0 && host_port_mac_stats_addr != 0) {
        NIC_LOG_INFO("{}: port{}: Stopping stats update", spec->name, spec->uplink_port_num);
        ev_timer_stop(EV_A_ & stats_timer);
    }

    ret = eth_lif->Reset();
    if (ret != IONIC_RC_SUCCESS) {
        NIC_LOG_DEBUG("{}: LIF reset failed !", spec->name);
    }

    active_lif_set.erase(lif_id);
    NIC_LOG_DEBUG("{}: active_lif_cnt: {}", spec->name, active_lif_set.size());

    return ret;
}

status_code_t
Eth::CmdProxyHandler(void *req, void *req_data, void *resp, void *resp_data)
{
    struct ionic_admin_cmd *cmd = (struct ionic_admin_cmd *)req;
    uint64_t lif_id = dev_resources.lif_base + cmd->lif_index;
    EthLif *eth_lif = NULL;

    NIC_LOG_DEBUG("{}: PROXY_CMD: lif_index {}", spec->name, cmd->lif_index);

    if (cmd->lif_index >= spec->lif_count) {
        NIC_LOG_ERR("{}: bad lif index {}", spec->name, cmd->lif_index);
        return (IONIC_RC_ERROR);
    }

    auto it = lif_map.find(lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, lif_id);
        return (IONIC_RC_ERROR);
    }
    eth_lif = it->second;

    return eth_lif->CmdProxyHandler(req, req_data, resp, resp_data);
}

/*
 * Callbacks
 */

void
Eth::StatsUpdate(EV_P_ ev_timer *w, int events)
{
    Eth *eth = (Eth *)w->data;
    EthLif *eth_lif = NULL;
    struct edmaq_ctx ctx = { .cb = NULL, .obj = NULL };

    auto it = eth->lif_map.find(eth->dev_resources.lif_base);
    if (it == eth->lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", eth->spec->name, eth->dev_resources.lif_base);
        return;
    }
    eth_lif = it->second;

    if (eth->port_mac_stats_addr != 0 && eth->host_port_mac_stats_addr != 0) {
        eth_lif->EdmaAsyncProxy(eth->spec->host_dev ? EDMA_OPCODE_LOCAL_TO_HOST : EDMA_OPCODE_LOCAL_TO_LOCAL,
                                eth->port_mac_stats_addr, eth->host_port_mac_stats_addr,
                                sizeof(struct ionic_port_stats), &ctx);

        if (eth->port_pb_stats_addr != 0 && eth->host_port_pb_stats_addr != 0) {
            eth_lif->EdmaAsyncProxy(eth->spec->host_dev ? EDMA_OPCODE_LOCAL_TO_HOST : EDMA_OPCODE_LOCAL_TO_LOCAL,
                                    eth->port_pb_stats_addr, eth->host_port_pb_stats_addr,
                                    sizeof(struct ionic_port_pb_stats), &ctx);
        }
    }
}


void
Eth::PortConfigUpdate(void *obj)
{
    Eth *eth = (Eth *)obj;
    EthLif *eth_lif = NULL;

    auto it = eth->lif_map.find(eth->dev_resources.lif_base);
    if (it == eth->lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", eth->spec->name, eth->dev_resources.lif_base);
        return;
    }
    eth_lif = it->second;

    if (eth->host_port_config_addr) {
        eth_lif->EdmaProxy(
            eth->spec->host_dev ? EDMA_OPCODE_LOCAL_TO_HOST : EDMA_OPCODE_LOCAL_TO_LOCAL,
            eth->port_config_addr, eth->host_port_config_addr, sizeof(union ionic_port_config), NULL);
    }
}

void
Eth::PortStatusUpdate(void *obj)
{
    Eth *eth = (Eth *)obj;
    EthLif *eth_lif = NULL;

    auto it = eth->lif_map.find(eth->dev_resources.lif_base);
    if (it == eth->lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", eth->spec->name, eth->dev_resources.lif_base);
        return;
    }
    eth_lif = it->second;

    if (eth->host_port_status_addr) {
        eth_lif->EdmaProxy(
            eth->spec->host_dev ? EDMA_OPCODE_LOCAL_TO_HOST : EDMA_OPCODE_LOCAL_TO_LOCAL,
            eth->port_status_addr, eth->host_port_status_addr, sizeof(struct ionic_port_status), NULL);
    }
}

/*
 * Event Handlers
 */
void
Eth::HalEventHandler(bool status)
{
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->HalEventHandler(status);
    }

    SetFwStatus(status);
}

void
Eth::DelphiMountEventHandler(bool mounted)
{
    if (!mounted) {
        return;
    }

    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->DelphiMountEventHandler(mounted);
    }
}

void
Eth::LinkEventHandler(port_status_t *evd)
{
    if (spec->uplink_port_num != evd->id) {
        return;
    }

    port_status->id = evd->id;
    port_status->speed = evd->speed;
    if (port_status->status == IONIC_PORT_OPER_STATUS_UP && evd->status == IONIC_PORT_OPER_STATUS_DOWN)
        ++port_status->link_down_count;
    port_status->status = evd->status;
    port_status->fec_type = evd->fec_type;
    NIC_LOG_DEBUG("Link Event id: {} speed: {} status: {} fec_type: {}",
                  port_status->id, port_status->speed,
                  port_status->status == IONIC_PORT_OPER_STATUS_UP ? "UP" : "DOWN",
                  port_status->fec_type);
    PortStatusUpdate(this);
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->LinkEventHandler(evd);
    }
}

void
Eth::XcvrEventHandler(port_status_t *evd)
{
    if (spec->uplink_port_num != evd->id) {
        return;
    }

    port_status->id = evd->id;
    port_status->speed = evd->speed;
    port_status->status = evd->status;
    memcpy(&port_status->xcvr, &evd->xcvr, sizeof(struct ionic_xcvr_status));

    PortStatusUpdate(this);
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->XcvrEventHandler(evd);
    }
}

void
Eth::UpdateQStatus(bool enable)
{
    NIC_LOG_DEBUG("Eth Device QControl Handler");
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->UpdateQStatus(enable);
    }
}

void
Eth::QuiesceEventHandler(bool quiesce)
{
    port_status_t port_status = {0};

    /* HAL will bringdown the real ports so devices bound to those ports will
       get a link event. Generate a link down event here for fake ports. */
    if (spec->uplink_port_num != 0) {
        return;
    }

    port_status.id = 0;
    port_status.speed = IONIC_SPEED_1G;
    port_status.status = quiesce ? IONIC_PORT_OPER_STATUS_DOWN : IONIC_PORT_OPER_STATUS_UP;

    LinkEventHandler(&port_status);
}

void
Eth::PcieResetEventHandler(uint32_t rsttype)
{
    const char *rsttype_str;
    status_code_t status;

    switch (rsttype) {
    case PCIEHDEV_RSTTYPE_BUS:
        rsttype_str = "BUS Reset";
        break;
    case PCIEHDEV_RSTTYPE_FLR:
        rsttype_str = "FLR Reset";
        break;
    case PCIEHDEV_RSTTYPE_VF:
        rsttype_str = "VF Reset";
        break;
    default:
        NIC_LOG_ERR("{}: received invalid reset type {}", spec->name, rsttype);
        return;
    }

    NIC_LOG_INFO("{}: received {}", spec->name, rsttype_str);

    DevcmdRegsReset();
    status = Reset();
    if (status != IONIC_RC_SUCCESS) {
        NIC_LOG_ERR("{}: {} failed", spec->name, rsttype_str);
    }
}

status_code_t
Eth::Reset()
{
    status_code_t status;
    EthLif *eth_lif = NULL;

    intr_reset_dev(dev_resources.intr_base, spec->intr_count, 1);

    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        eth_lif = it->second;
        status = eth_lif->Reset();
        if (status != IONIC_RC_SUCCESS) {
            NIC_LOG_ERR("{}: Failed to reset lif", spec->name);
            // TODO: Handle lif reset fail
        }
        active_lif_set.erase(it->first);
    }

    NIC_LOG_DEBUG("{}: active_lif_cnt: {}", spec->name, active_lif_set.size());

    return (IONIC_RC_SUCCESS);
}

bool
Eth::IsDevQuiesced()
{
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;

        if (!eth_lif->IsLifQuiesced()) {
            NIC_LOG_DEBUG("{}: Device is not Quiesced", spec->name);
            return false;
        }
    }

    NIC_LOG_DEBUG("{}: Device is Quiesced", spec->name);

    return true;
}

bool
Eth::IsDevReset()
{
    if (!active_lif_set.size()) {
        NIC_LOG_DEBUG("{}: Device is in reset state!", spec->name);
        return true;
    }

    NIC_LOG_DEBUG("{}: Device is not in reset state yet! active_lif_cnt: {}",
            spec->name, active_lif_set.size());
    return false;
}

bool
Eth::IsDevLif(uint32_t lif_id)
{
    if (lif_map.find(lif_id) == lif_map.end()) {
        return false;
    }

    return true;
}

bool
Eth::IsPlatformDev()
{
    switch(spec->eth_type) {
        case ETH_MNIC_OOB_MGMT:
        case ETH_MNIC_INTERNAL_MGMT:
        case ETH_MNIC_INBAND_MGMT:
        case ETH_MNIC_CPU:
        case ETH_MNIC_LEARN:
        case ETH_MNIC_CONTROL:
            return true;
        default:
            return false;
    }
}

int
Eth::SendFWDownEvent()
{
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        NIC_LOG_DEBUG("{}: DBG: setting fw_state to 0", spec->name);
        eth_lif->SendFWDownEvent();
    }

    // TODO: Wait for device reset before doing this!
    SetFwStatus(0);

    return 0;
}

int
Eth::GenerateQstateInfoJson(pt::ptree &lifs)
{
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->GenerateQstateInfoJson(lifs);
    }

    return 0;
}

void
Eth::SetHalClient(devapi *dapi)
{
    dev_api = dapi;

    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->SetHalClient(dapi);
    }
}

void
Eth::SetFwStatus(uint8_t fw_status)
{
    NIC_LOG_INFO("{}: fw_status {}", spec->name, fw_status);

    regs->info.fw_status = fw_status;
#ifndef __aarch64__
    WRITE_MEM(dev_resources.regs_mem_addr + offsetof(union ionic_dev_regs, info) +
                  offsetof(union ionic_dev_info_regs, fw_status),
              (uint8_t *)&regs->info.fw_status, sizeof(regs->info.fw_status), 0);
#endif
}

void
Eth::HeartbeatEventHandler()
{
    regs->info.fw_heartbeat = regs->info.fw_heartbeat + 1;
#ifndef __aarch64__
    WRITE_MEM(dev_resources.regs_mem_addr + offsetof(union ionic_dev_regs, info) +
                  offsetof(union ionic_dev_info_regs, fw_heartbeat),
              (uint8_t *)&regs->info.fw_heartbeat, sizeof(regs->info.fw_heartbeat), 0);
#endif
}

lif_type_t
Eth::ConvertDevTypeToLifType(EthDevType dev_type)
{
    switch (dev_type) {
    case ETH_HOST:
        return sdk::platform::LIF_TYPE_HOST;
    case ETH_HOST_MGMT:
        return sdk::platform::LIF_TYPE_HOST_MGMT;
    case ETH_MNIC_OOB_MGMT:
        return sdk::platform::LIF_TYPE_MNIC_OOB_MGMT;
    case ETH_MNIC_INTERNAL_MGMT:
        return sdk::platform::LIF_TYPE_MNIC_INTERNAL_MGMT;
    case ETH_MNIC_INBAND_MGMT:
        return sdk::platform::LIF_TYPE_MNIC_INBAND_MGMT;
    case ETH_MNIC_CPU:
        return sdk::platform::LIF_TYPE_MNIC_CPU;
    case ETH_MNIC_LEARN:
        return sdk::platform::LIF_TYPE_LEARN;
    case ETH_MNIC_CONTROL:
        return sdk::platform::LIF_TYPE_CONTROL;
    default:
        return sdk::platform::LIF_TYPE_NONE;
    }
}
