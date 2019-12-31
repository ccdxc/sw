/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>
#include <string>
#include <sys/time.h>

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"
#include "cap_ms_c_hdr.h"

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"

#ifdef __aarch64__
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#endif
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"

#include "logger.hpp"
#include "nicmgr_utils.hpp"
#include "virtio_if.h"
#include "virtio_dev.hpp"
#include "virtio_lif.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"

using namespace std;

extern class pciemgr *pciemgr;

VirtIODev::VirtIODev(devapi *dapi,
                 void *dev_spec,
                 PdClient *pd_client,
                 EV_P) :
    spec((virtio_devspec_t *)dev_spec),
    pd(pd_client),
    dev_api(dapi)
{
    virtio_lif_res_t      lif_res;
    sdk_ret_t           ret = SDK_RET_OK;

    this->loop = loop;

    // Allocate lifs
    ret = pd->lm_->alloc_id(&lif_base, spec->lif_count);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to allocate lifs. ret: {}", DevNameGet(), ret);
        throw;
    }
    NIC_LOG_DEBUG("{}: lif_base {} lif_count {}",
                  DevNameGet(), lif_base, spec->lif_count);

    // Allocate interrupts
    intr_base = pd->intr_alloc(spec->intr_count);
    if (intr_base < 0) {
        NIC_LOG_ERR("{}: Failed to allocate interrupts", DevNameGet());
        throw;
    }
    NIC_LOG_DEBUG("{}: intr_base {} intr_count {}",
                  DevNameGet(), intr_base, spec->intr_count);

    // Allocate & Init Devcmd Region
    devcmd_mem_addr = pd->devcmd_mem_alloc(VIRTIO_DEV_PAGE_SIZE);
    MEM_SET(devcmd_mem_addr, 0, VIRTIO_DEV_PAGE_SIZE, 0);
    devcmddb_mem_addr = pd->devcmd_mem_alloc(VIRTIO_DEV_PAGE_SIZE);
    MEM_SET(devcmddb_mem_addr, 0, VIRTIO_DEV_PAGE_SIZE, 0);

    VirtIORegsInit();

    WRITE_MEM(devcmd_mem_addr, (uint8_t *)&devcmd, sizeof(devcmd), 0);

    NIC_LOG_DEBUG("{}: devcmd_addr {:#x} devcmddb_addr {:#x}",
                  DevNameGet(), devcmd_mem_addr, devcmddb_mem_addr);

    //Init cmb
    cmb_mem_addr = 0;
    cmb_mem_size = 0;

    // Create the device
    if (spec->pcie_port == 0xff) {
        NIC_LOG_DEBUG("{}: Skipped creating PCI device", DevNameGet());
    } else {
        if (!_CreateHostDevice()) {
            NIC_LOG_ERR("{}: Failed to create device", DevNameGet());
            throw;
        }
    }

    // Create LIF
    lif_res.lif_id = lif_base;
    lif_res.intr_base = intr_base;
    lif_res.cmb_mem_addr = cmb_mem_addr;
    lif_res.cmb_mem_size = cmb_mem_size;

    lif = new VirtIOLif(*this, lif_res, loop);
    if (!lif) {
        NIC_LOG_ERR("{}: failed to create VirtIOLif {}",
                    DevNameGet(), lif_res.lif_id);
        throw;
    }
    NIC_LOG_DEBUG("{}: LIF {}", DevNameGet(), lif->LifNameGet());

    evutil_timer_start(EV_A_ &devcmd_timer, &VirtIODev::_DevcmdPoll, this, 0.0, 0.01);
}

VirtIODev::~VirtIODev()
{
    /*
     * Most HBM related allocs don't have corresponding free API so
     * we'll leave them alone. Same with intr_alloc().
     */
    evutil_timer_stop(EV_A_ &devcmd_timer);

    delete lif;

    _DestroyHostDevice();
    pd->lm_->free_id(lif_base, spec->lif_count);
}

void
VirtIODev::VirtIORegsInit()
{
    uint64_t features;
    NIC_HEADER_TRACE("Preparing VirtIO Register Set");

    memset((void *)&devcmd, 0, sizeof(devcmd));
    memset((void *)&devcmd_old, 0, sizeof(devcmd_old));

    features = VIRTIO_F_VERSION_1;
    devcmd.cfg[0].device_feature = features;
    devcmd.cfg[1].device_feature = features >> 32;
    devcmd.cfg[0].num_queues = 2;
    devcmd.cfg[VIRTIO_RXQ(0)].queue_size = 64;
    devcmd.cfg[VIRTIO_RXQ(0)].queue_notify_off = VIRTIO_RXQ(0);
    devcmd.cfg[VIRTIO_TXQ(0)].queue_size = 64;
    devcmd.cfg[VIRTIO_TXQ(0)].queue_notify_off = VIRTIO_TXQ(0);

    uint16_t *notify_regs = reinterpret_cast<uint16_t *>(&reinterpret_cast<uint8_t *>(&devcmd)[0x200]);
    uint16_t *notify_regs_old = reinterpret_cast<uint16_t *>(&reinterpret_cast<uint8_t *>(&devcmd_old)[0x200]);
    memset((void *)notify_regs, -1, 0x200);
    memset((void *)notify_regs_old, -1, 0x200);

    WRITE_MEM(devcmd_mem_addr, (uint8_t *)&devcmd, sizeof(devcmd), 0);
}

void
VirtIODev::HalEventHandler(bool status)
{
    assert(lif != nullptr);
    if (lif) {
        lif->HalEventHandler(status);
    }
    return;
}

void
VirtIODev::SetHalClient(devapi *dapi)
{
    dev_api = dapi;

    assert(lif != nullptr);
    if (lif) {
        lif->SetHalClient(dapi);
    }
    return;
}

void
VirtIODev::DelphiMountEventHandler(bool mounted)
{
    return;
}

static sdk::platform::lif_type_t
virtio_dev_type_str_to_lif_type(std::string const& s)
{
    if (s == "host")
        return sdk::platform::LIF_TYPE_HOST;

    NIC_LOG_ERR("Unknown VirtIO dev type: {}", s);

    return sdk::platform::LIF_TYPE_NONE;
}

struct virtio_devspec *
VirtIODev::ParseConfig(boost::property_tree::ptree::value_type node)
{
    virtio_devspec* virtio_spec;
    auto val = node.second;

    NIC_HEADER_TRACE("Parsing VIRTIO Config");

    virtio_spec = new struct virtio_devspec;
    memset(virtio_spec, 0, sizeof(*virtio_spec));

    virtio_spec->enable = val.get<uint8_t>("enable");
    virtio_spec->name = val.get<string>("name");
    virtio_spec->lif_type = virtio_dev_type_str_to_lif_type(val.get<string>("lif_type"));
    virtio_spec->lif_count = val.get<uint64_t>("lif_count");
    virtio_spec->intr_count = val.get<uint32_t>("intr_count");
    virtio_spec->txrx_count = val.get<uint32_t>("txrx_count");

    virtio_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
    virtio_spec->uplink_port_num = val.get<uint64_t>("network.uplink");

    NIC_LOG_DEBUG("enable: {} name: {} lif_count: {} txrx_count: {} intr_count: {}",
                  virtio_spec->enable, virtio_spec->name, virtio_spec->lif_count,
                  virtio_spec->txrx_count, virtio_spec->intr_count);

    return virtio_spec;
}

bool
VirtIODev::_CreateHostDevice(void)
{
    pciehdevice_resources_t pres;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_VIRTIO;
    strncpy0(pres.pfres.name, DevNameGet().c_str(), sizeof(pres.pfres.name));
    pres.pfres.port = spec->pcie_port;
    pres.pfres.lifb = lif_base;
    pres.pfres.lifc = spec->lif_count;
    pres.pfres.intrb = intr_base;
    pres.pfres.intrc = spec->intr_count;
    pres.pfres.intrdmask = 0;
    pres.pfres.cmbpa = cmb_mem_addr;
    pres.pfres.cmbsz = cmb_mem_size;
    pres.pfres.virtio.regspa = devcmd_mem_addr;
    pres.pfres.virtio.regssz = VIRTIO_DEV_PAGE_SIZE;

    // Add device to PCI topology
    if (pciemgr) {
        int ret = pciemgr->add_devres(&pres);
        if (ret != 0) {
            NIC_LOG_ERR("{}: Failed to add PCI device to topology",
                        DevNameGet());
            return false;
        }
    }

    return true;
}

void
VirtIODev::_DestroyHostDevice(void)
{
}

void
VirtIODev::_DevcmdPoll(void *obj)
{
    VirtIODev        *dev = (VirtIODev *)obj;

    dev->DevcmdHandler();
}

void
VirtIODev::DevcmdHandler()
{
    READ_MEM(devcmd_mem_addr, (uint8_t *)&devcmd, sizeof(devcmd), 0);

    // This operates by timer-driven polling,
    // - read current register set and update devcmd.
    // - compare devcmd to devcmd_old, and react to changes.
    // - devcmd_old = devcmd, in preparation for next round of polling.

    // device status
    if (devcmd_old.cfg[0].device_status != devcmd.cfg[0].device_status) {
        NIC_LOG_DEBUG("{}: device_status {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[0].device_status, devcmd.cfg[0].device_status);
        if (!devcmd.cfg[0].device_status) {
            NIC_LOG_DEBUG("{}: driver initiated reset", DevNameGet());
            /* XXX this could be LIF INIT */
            lif->ResetRxQstate(0);
            lif->ResetTxQstate(0);
            VirtIORegsInit();
        }
    }

    // XXX: Workaround for Select Registers
    //
    // The virtio pci common config struct is duplicated, and presented to the
    // host as an array for common config structs.
    //
    // Normal operation of a select register would be:
    // cfg.select <- id
    // cfg.selected <- value
    //
    // Workaround operation (with modified driver):
    // cfg[0].select <- id (debug logged but ignored)
    // cfg[id].selected <- value

    // features
    if (devcmd_old.cfg[0].device_feature_select != devcmd.cfg[0].device_feature_select)
        NIC_LOG_DEBUG("{}: device_feature_select {} -> {}", DevNameGet(), devcmd_old.cfg[0].device_feature_select, devcmd.cfg[0].device_feature_select);
    if (devcmd_old.cfg[0].device_feature != devcmd.cfg[0].device_feature)
        NIC_LOG_DEBUG("{}: device_feature0 {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[0].device_feature, devcmd.cfg[0].device_feature);
    if (devcmd_old.cfg[1].device_feature != devcmd.cfg[1].device_feature)
        NIC_LOG_DEBUG("{}: device_feature1 {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[1].device_feature, devcmd.cfg[1].device_feature);

    if (devcmd_old.cfg[0].driver_feature_select != devcmd.cfg[0].driver_feature_select)
        NIC_LOG_DEBUG("{}: driver_feature_select {} -> {}", DevNameGet(), devcmd_old.cfg[0].driver_feature_select, devcmd.cfg[0].driver_feature_select);
    if (devcmd_old.cfg[0].driver_feature != devcmd.cfg[0].driver_feature)
        NIC_LOG_DEBUG("{}: driver_feature0 {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[0].driver_feature, devcmd.cfg[0].driver_feature);
    if (devcmd_old.cfg[1].driver_feature != devcmd.cfg[1].driver_feature)
        NIC_LOG_DEBUG("{}: driver_feature1 {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[1].driver_feature, devcmd.cfg[1].driver_feature);

    // general config
    if (devcmd_old.cfg[0].config_msix_vector != devcmd.cfg[0].config_msix_vector)
        NIC_LOG_DEBUG("{}: config_msix_vector {} -> {}", DevNameGet(), devcmd_old.cfg[0].config_msix_vector, devcmd.cfg[0].config_msix_vector);
    if (devcmd_old.cfg[0].num_queues != devcmd.cfg[0].num_queues)
        NIC_LOG_DEBUG("{}: num_queues {} -> {}", DevNameGet(), devcmd_old.cfg[0].num_queues, devcmd.cfg[0].num_queues);
    if (devcmd_old.cfg[0].config_generation != devcmd.cfg[0].config_generation)
        NIC_LOG_DEBUG("{}: config_generation {} -> {}", DevNameGet(), devcmd_old.cfg[0].config_generation, devcmd.cfg[0].config_generation);
    if (devcmd_old.cfg[0].queue_select != devcmd.cfg[0].queue_select)
        NIC_LOG_DEBUG("{}: queue_select {} -> {}", DevNameGet(), devcmd_old.cfg[0].queue_select, devcmd.cfg[0].queue_select);

    // rx queue config
    if (devcmd_old.cfg[VIRTIO_RXQ(0)].queue_size != devcmd.cfg[VIRTIO_RXQ(0)].queue_size)
        NIC_LOG_DEBUG("{}: rxq0_size {} -> {}", DevNameGet(), devcmd_old.cfg[VIRTIO_RXQ(0)].queue_size, devcmd.cfg[VIRTIO_RXQ(0)].queue_size);
    if (devcmd_old.cfg[VIRTIO_RXQ(0)].queue_msix_vector != devcmd.cfg[VIRTIO_RXQ(0)].queue_msix_vector)
        NIC_LOG_DEBUG("{}: rxq0_msix {} -> {}", DevNameGet(), devcmd_old.cfg[VIRTIO_RXQ(0)].queue_msix_vector, devcmd.cfg[VIRTIO_RXQ(0)].queue_msix_vector);
    if (devcmd_old.cfg[VIRTIO_RXQ(0)].queue_notify_off != devcmd.cfg[VIRTIO_RXQ(0)].queue_notify_off)
        NIC_LOG_DEBUG("{}: rxq0_notify_off {} -> {}", DevNameGet(), devcmd_old.cfg[VIRTIO_RXQ(0)].queue_notify_off, devcmd.cfg[VIRTIO_RXQ(0)].queue_notify_off);
    if (devcmd_old.cfg[VIRTIO_RXQ(0)].queue_desc != devcmd.cfg[VIRTIO_RXQ(0)].queue_desc)
        NIC_LOG_DEBUG("{}: rxq0_desc {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[VIRTIO_RXQ(0)].queue_desc, devcmd.cfg[VIRTIO_RXQ(0)].queue_desc);
    if (devcmd_old.cfg[VIRTIO_RXQ(0)].queue_driver != devcmd.cfg[VIRTIO_RXQ(0)].queue_driver)
        NIC_LOG_DEBUG("{}: rxq0_driver {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[VIRTIO_RXQ(0)].queue_driver, devcmd.cfg[VIRTIO_RXQ(0)].queue_driver);
    if (devcmd_old.cfg[VIRTIO_RXQ(0)].queue_device != devcmd.cfg[VIRTIO_RXQ(0)].queue_device)
        NIC_LOG_DEBUG("{}: rxq0_device {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[VIRTIO_RXQ(0)].queue_device, devcmd.cfg[VIRTIO_RXQ(0)].queue_device);

    // rx enable
    if (devcmd_old.cfg[VIRTIO_RXQ(0)].queue_enable != devcmd.cfg[VIRTIO_RXQ(0)].queue_enable) {
        NIC_LOG_DEBUG("{}: rxq0_enable {} -> {}", DevNameGet(), devcmd_old.cfg[VIRTIO_RXQ(0)].queue_enable, devcmd.cfg[VIRTIO_RXQ(0)].queue_enable);
        if (!devcmd.cfg[VIRTIO_RXQ(0)].queue_enable) {
            lif->ResetRxQstate(0);
        } else {
            lif->SetupRxQstate(0,
                               devcmd.cfg[0].driver_feature | ((uint64_t)devcmd.cfg[1].driver_feature << 32),
                               devcmd.cfg[VIRTIO_RXQ(0)].queue_desc,
                               devcmd.cfg[VIRTIO_RXQ(0)].queue_driver,
                               devcmd.cfg[VIRTIO_RXQ(0)].queue_device,
                               devcmd.cfg[VIRTIO_RXQ(0)].queue_msix_vector,
                               devcmd.cfg[VIRTIO_RXQ(0)].queue_size);
        }
    }

    // tx queue config
    if (devcmd_old.cfg[VIRTIO_TXQ(0)].queue_size != devcmd.cfg[VIRTIO_TXQ(0)].queue_size)
        NIC_LOG_DEBUG("{}: txq0_size {} -> {}", DevNameGet(), devcmd_old.cfg[VIRTIO_TXQ(0)].queue_size, devcmd.cfg[VIRTIO_TXQ(0)].queue_size);
    if (devcmd_old.cfg[VIRTIO_TXQ(0)].queue_msix_vector != devcmd.cfg[VIRTIO_TXQ(0)].queue_msix_vector)
        NIC_LOG_DEBUG("{}: txq0_msix {} -> {}", DevNameGet(), devcmd_old.cfg[VIRTIO_TXQ(0)].queue_msix_vector, devcmd.cfg[VIRTIO_TXQ(0)].queue_msix_vector);
    if (devcmd_old.cfg[VIRTIO_TXQ(0)].queue_notify_off != devcmd.cfg[VIRTIO_TXQ(0)].queue_notify_off)
        NIC_LOG_DEBUG("{}: txq0_notify_off {} -> {}", DevNameGet(), devcmd_old.cfg[VIRTIO_TXQ(0)].queue_notify_off, devcmd.cfg[VIRTIO_TXQ(0)].queue_notify_off);
    if (devcmd_old.cfg[VIRTIO_TXQ(0)].queue_desc != devcmd.cfg[VIRTIO_TXQ(0)].queue_desc)
        NIC_LOG_DEBUG("{}: txq0_desc {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[VIRTIO_TXQ(0)].queue_desc, devcmd.cfg[VIRTIO_TXQ(0)].queue_desc);
    if (devcmd_old.cfg[VIRTIO_TXQ(0)].queue_driver != devcmd.cfg[VIRTIO_TXQ(0)].queue_driver)
        NIC_LOG_DEBUG("{}: txq0_driver {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[VIRTIO_TXQ(0)].queue_driver, devcmd.cfg[VIRTIO_TXQ(0)].queue_driver);
    if (devcmd_old.cfg[VIRTIO_TXQ(0)].queue_device != devcmd.cfg[VIRTIO_TXQ(0)].queue_device)
        NIC_LOG_DEBUG("{}: txq0_device {:#x} -> {:#x}", DevNameGet(), devcmd_old.cfg[VIRTIO_TXQ(0)].queue_device, devcmd.cfg[VIRTIO_TXQ(0)].queue_device);

    // tx enable
    if (devcmd_old.cfg[VIRTIO_TXQ(0)].queue_enable != devcmd.cfg[VIRTIO_TXQ(0)].queue_enable) {
        NIC_LOG_DEBUG("{}: txq0_enable {} -> {}", DevNameGet(), devcmd_old.cfg[VIRTIO_TXQ(0)].queue_enable, devcmd.cfg[VIRTIO_TXQ(0)].queue_enable);
        if (!devcmd.cfg[VIRTIO_TXQ(0)].queue_enable) {
            lif->ResetTxQstate(0);
        } else {
            lif->SetupTxQstate(0,
                               devcmd.cfg[0].driver_feature | ((uint64_t)devcmd.cfg[1].driver_feature << 32),
                               devcmd.cfg[VIRTIO_TXQ(0)].queue_desc,
                               devcmd.cfg[VIRTIO_TXQ(0)].queue_driver,
                               devcmd.cfg[VIRTIO_TXQ(0)].queue_device,
                               devcmd.cfg[VIRTIO_TXQ(0)].queue_msix_vector,
                               devcmd.cfg[VIRTIO_TXQ(0)].queue_size);
        }
    }

    // XXX for now this other stuff is jammed in the devcmd region too
    // XXX ISR status registers not implemented, only needed for legacy INTX

    // legacy intx status registers
    // section 4.1.5.4
    uint16_t *isr_regs = reinterpret_cast<uint16_t *>(&reinterpret_cast<uint8_t *>(&devcmd)[0x100]);
    uint16_t *isr_regs_old = reinterpret_cast<uint16_t *>(&reinterpret_cast<uint8_t *>(&devcmd_old)[0x100]);

    if (isr_regs_old[VIRTIO_RXQ(0)] != isr_regs[VIRTIO_RXQ(0)])
        NIC_LOG_DEBUG("{}: isr_rxq0 {} -> {}", DevNameGet(), isr_regs_old[VIRTIO_RXQ(0)], isr_regs[VIRTIO_RXQ(0)]);
    if (isr_regs_old[VIRTIO_TXQ(0)] != isr_regs[VIRTIO_TXQ(0)])
        NIC_LOG_DEBUG("{}: isr_txq0 {} -> {}", DevNameGet(), isr_regs_old[VIRTIO_TXQ(0)], isr_regs[VIRTIO_TXQ(0)]);

    // notify registers aka doorbell
    // section 4.1.5.2
    // XXX notify should be done with real doorbell registers
    uint16_t *notify_regs = reinterpret_cast<uint16_t *>(&reinterpret_cast<uint8_t *>(&devcmd)[0x200]);
    uint16_t *notify_regs_old = reinterpret_cast<uint16_t *>(&reinterpret_cast<uint8_t *>(&devcmd_old)[0x200]);

    if (notify_regs_old[VIRTIO_RXQ(0)] != notify_regs[VIRTIO_RXQ(0)]) {
        NIC_LOG_DEBUG("{}: notify_rxq0 -> {}", DevNameGet(), notify_regs[VIRTIO_RXQ(0)]);
        // reset rx notify register, and ignore
        notify_regs[VIRTIO_RXQ(0)] = notify_regs_old[VIRTIO_RXQ(0)];
        WRITE_MEM(devcmd_mem_addr + 0x200 + 2 * VIRTIO_RXQ(0), (uint8_t *)&notify_regs[VIRTIO_RXQ(0)], 2, 0);
    }
    if (notify_regs_old[VIRTIO_TXQ(0)] != notify_regs[VIRTIO_TXQ(0)]) {
        NIC_LOG_DEBUG("{}: notify_txq0 -> {}", DevNameGet(), notify_regs[VIRTIO_TXQ(0)]);
        // reset tx notify register, and ring the real doorbell
        notify_regs[VIRTIO_TXQ(0)] = notify_regs_old[VIRTIO_TXQ(0)];
        WRITE_MEM(devcmd_mem_addr + 0x200 + 2 * VIRTIO_TXQ(0), (uint8_t *)&notify_regs[VIRTIO_TXQ(0)], 2, 0);
        lif->NotifyTxQueue(0);
    }

    // network device-specific registers eg. mac, link status, multiqueue, mtu
    // section 5.1.4
    uint32_t *device_regs = reinterpret_cast<uint32_t *>(&reinterpret_cast<uint8_t *>(&devcmd)[0x400]);
    uint32_t *device_regs_old = reinterpret_cast<uint32_t *>(&reinterpret_cast<uint8_t *>(&devcmd_old)[0x400]);

    if (device_regs_old[0] != device_regs[0])
        NIC_LOG_DEBUG("{}: device0 {} -> {}", DevNameGet(), device_regs_old[0], device_regs[0]);
    if (device_regs_old[1] != device_regs[1])
        NIC_LOG_DEBUG("{}: device1 {} -> {}", DevNameGet(), device_regs_old[1], device_regs[1]);

    // prime the next polling to detect changed values
    devcmd_old = devcmd;
}

VirtIOLif *
VirtIODev::_LifFind(uint64_t lif_id)
{
    if (lif_base == lif_id) {
        return lif;
    }
    return nullptr;
}
