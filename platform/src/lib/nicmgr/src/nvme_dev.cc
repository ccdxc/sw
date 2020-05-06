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
#include "nic/sdk/platform/pciemgr/include/pciehdev_event.h"

#include "logger.hpp"
#include "nicmgr_utils.hpp"
#include "nvme_if.h"
#include "nvme_dev.hpp"
#include "nvme_lif.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"

using namespace std;

extern class pciemgr *pciemgr;

NvmeDev::NvmeDev(devapi *dapi,
                 void *dev_spec,
                 PdClient *pd_client,
                 EV_P) :
    spec((nvme_devspec_t *)dev_spec),
    pd(pd_client),
    dev_api(dapi)
{
    nvme_lif_res_t      lif_res;
    sdk_ret_t           ret = SDK_RET_OK;

    this->loop = loop;

    // Allocate lifs
    // lif_base = pd->lm_->LIFRangeAlloc(-1, spec->lif_count);
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
    devcmd_mem_addr = pd->devcmd_mem_alloc(NVME_DEV_PAGE_SIZE);
    MEM_SET(devcmd_mem_addr, 0, NVME_DEV_PAGE_SIZE, 0);
    // TODO: mmap instead of calloc after porting to real pal
    devcmd = (nvme_dev_cmd_regs_t *)calloc(1, sizeof(nvme_dev_cmd_regs_t));
    if (devcmd == NULL) {
        NIC_LOG_ERR("{}: Failed to map devcmd region", DevNameGet());
        throw;
    }

    devcmddb_mem_addr = pd->devcmd_mem_alloc(NVME_DEV_PAGE_SIZE);
    MEM_SET(devcmddb_mem_addr, 0, NVME_DEV_PAGE_SIZE, 0);

    NvmeRegsInit();

    WRITE_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(*devcmd), 0);

    NIC_LOG_DEBUG("{}: devcmd_addr {:#x} devcmddb_addr {:#x}",
                  DevNameGet(), devcmd_mem_addr, devcmddb_mem_addr);

    //Init cmb
    cmb_mem_addr = 0;
    cmb_mem_size = 0;

    cc_en = false;

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

    lif = new NvmeLif(*this, lif_res, loop);
    if (!lif) {
        NIC_LOG_ERR("{}: failed to create NvmeLif {}",
                    DevNameGet(), lif_res.lif_id);
        throw;
    }

    evutil_timer_start(EV_A_ &devcmd_timer, &NvmeDev::_DevcmdPoll, this, 0.0, 0.01);
}

NvmeDev::~NvmeDev()
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
NvmeDev::NvmeRegsInit()
{
    nvme_reg_cap_t  cap;
    nvme_reg_vs_t   vs;

    NIC_HEADER_TRACE("Preparing Nvme Register Set");

    // cap
    cap.mpsmax = cap.mpsmin = 0;    //4K page
    cap.css = 0x01;                 //NVMe command set
    cap.nssrs = 0;
    cap.dstrd = 0;
    cap.to = 10;                    //5sec
    cap.ams = 0;                    //only RR
    cap.cqr = 1;                    //need physically contiguous queues
    cap.mqes = 4095;                //0-based 4096

    //vs = 1.2
    vs.mjr = 1;
    vs.mnr = 2;

    devcmd->cap.num64 = (cap.num64);
    devcmd->vs.num32 = (vs.num32);

    NIC_LOG_DEBUG("NvmeRegsInit: cap {:#x} vs {:#x}",
                  devcmd->cap.num64, devcmd->vs.num32);
    return;
}

void
NvmeDev::HalEventHandler(bool status)
{
    assert(lif != nullptr);
    if (lif) {
        lif->HalEventHandler(status);
    }
    return;
}

void
NvmeDev::SetHalClient(devapi *dapi)
{
    dev_api = dapi;

    assert(lif != nullptr);
    if (lif) {
        lif->SetHalClient(dapi);
    }
    return;
}

void
NvmeDev::DelphiMountEventHandler(bool mounted)
{
    return;
}

struct nvme_devspec *
NvmeDev::ParseConfig(boost::property_tree::ptree::value_type node)
{
    nvme_devspec* nvme_spec;
    auto val = node.second;

    NIC_HEADER_TRACE("Parsing NVME Config");

    nvme_spec = new struct nvme_devspec;
    memset(nvme_spec, 0, sizeof(*nvme_spec));

    nvme_spec->enable = val.get<uint8_t>("enable");
    nvme_spec->name = val.get<string>("name");
    nvme_spec->lif_count = val.get<uint64_t>("lif_count");
    nvme_spec->adminq_count = val.get<uint32_t>("adminq_count");
    nvme_spec->sq_count = val.get<uint32_t>("sq_count");
    nvme_spec->cq_count = val.get<uint32_t>("cq_count");
    nvme_spec->intr_count = val.get<uint32_t>("intr_count");

    nvme_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
    nvme_spec->pcie_total_vfs = val.get<uint8_t>("pcie.total_vfs", 0);

    NIC_LOG_DEBUG("enable: {} name: {} lif_count: {} adminq_count: {} sq_count: {} cq count: {} intr_count: {}",
                  nvme_spec->enable, nvme_spec->name, nvme_spec->lif_count, nvme_spec->adminq_count,
                  nvme_spec->sq_count, nvme_spec->cq_count, nvme_spec->intr_count);

    return nvme_spec;
}

bool
NvmeDev::_CreateHostDevice(void)
{
    pciehdevice_resources_t pres;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_NVME;
    strncpy0(pres.pfres.name, DevNameGet().c_str(), sizeof(pres.pfres.name));
    pres.pfres.port = spec->pcie_port;
    pres.pfres.lifb = lif_base;
    pres.pfres.lifc = spec->lif_count;
    pres.pfres.intrb = intr_base;
    pres.pfres.intrc = spec->intr_count;
    pres.pfres.intrdmask = 0;
    pres.pfres.cmbpa = cmb_mem_addr;
    pres.pfres.cmbsz = cmb_mem_size;
    pres.pfres.totalvfs = spec->pcie_total_vfs;
    pres.pfres.nvme.regspa = devcmd_mem_addr;
    pres.pfres.nvme.regssz = 0x1000;

    if (pres.pfres.totalvfs > 0) {
        pciehdev_res_t *vfres = &pres.vfres;
        pciehdev_res_t *pfres = &pres.pfres;
        vfres->is_vf = 1;
        if (pfres->lifc) {
            vfres->lifb = pfres->lifb + pfres->lifc;
            vfres->lifc = pfres->lifc;
        }
        if (pfres->intrc) {
            vfres->intrb = pfres->intrb + pfres->intrc;
            vfres->intrc = pfres->intrc;
            vfres->intrdmask = pfres->intrdmask;
        }
        vfres->nvme.regspa = pfres->nvme.regspa + 0x1000;
        vfres->nvme.regssz = pfres->nvme.regssz;
        vfres->nvme.regs_stride = vfres->nvme.regssz;
    }

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
NvmeDev::_DestroyHostDevice(void)
{
}

void
NvmeDev::_DevcmdPoll(void *obj)
{
    NvmeDev        *dev = (NvmeDev *)obj;

    dev->DevcmdHandler();
}

void
NvmeDev::DevcmdHandler()
{
    nvme_status_code_t  status = NVME_RC_ERROR;

    READ_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(*devcmd), 0);

    if ((cc_en == false) && (devcmd->cc.en == 1)) {
        // controller getting enabled

        NIC_LOG_DEBUG("!!! controller is getting enabled.. !!!");
        NIC_LOG_DEBUG("cc: {:#x}  iocqes: {} iosqes: {} ams: {} "
                      "mps: {} css: {} en: {}",
                      devcmd->cc.num32, devcmd->cc.iocqes,
                      devcmd->cc.iosqes, devcmd->cc.ams,
                      devcmd->cc.mps, devcmd->cc.css,
                      devcmd->cc.en);

        if (lif != nullptr) {
            status = lif->Enable(devcmd);
        }

        if (status == NVME_RC_SUCCESS) {
            cc_en = true;
            devcmd->csts.rdy = true;
            WRITE_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(*devcmd), 0);
        }
        IntrReset();

    } else if ((cc_en == true) && (devcmd->cc.en == 0)) {
        NIC_LOG_DEBUG("!!! controller is getting disabled.. !!!");

        // cleanup
        if (lif != nullptr) {
            status = lif->Disable(devcmd);
        }

        if (status == NVME_RC_SUCCESS) {
            cc_en = false;
            devcmd->csts.rdy = false;
            WRITE_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(*devcmd), 0);
        }

        IntrReset();
    }

    return;
}


void
NvmeDev::IntrReset(void)
{
    intr_reset_dev(intr_base, spec->intr_count, 0);
}

NvmeLif *
NvmeDev::_LifFind(uint64_t lif_id)
{
    if (lif_base == lif_id) {
        return lif;
    }
    return nullptr;
}
