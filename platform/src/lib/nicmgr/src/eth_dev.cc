/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"
#include "cap_ms_c_hdr.h"

#include "nic/include/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/p4/common/defines.h"

#ifndef APOLLO
#include "gen/proto/nicmgr/nicmgr.pb.h"
#include "gen/proto/nicmgr/metrics.delphi.hpp"
#include "gen/proto/common/nicmgr_status_msgs.pb.h"
#include "gen/proto/common/nicmgr_status_msgs.delphi.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"
#endif

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "nic/sdk/platform/fru/fru.hpp"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "nicmgr_utils.hpp"

#include "logger.hpp"
#include "eth_if.h"
#include "eth_dev.hpp"
#include "rdma_dev.hpp"
#include "pd_client.hpp"

extern class pciemgr *pciemgr;

EthDevType
Eth::eth_dev_type_str_to_type(std::string const& s)
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
    } else {
        NIC_LOG_ERR("Unknown ETH dev type: {}", s);
        return ETH_UNKNOWN;
    }
}

Eth::Eth(devapi *dev_api,
         void *dev_spec,
         PdClient *pd_client)
{
    sdk_ret_t ret = SDK_RET_OK;
    Eth::dev_api = dev_api;
    Eth::spec = (struct eth_devspec *)dev_spec;
    Eth::pd = pd_client;

    // Allocate lifs
    ret = pd->lm_->alloc_id(&lif_base, spec->lif_count);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to allocate lifs. ret: {}", spec->name, ret);
        throw;
    }
    NIC_LOG_DEBUG("{}: lif_base {} lif_count {}", spec->name, lif_base, spec->lif_count);

    // Allocate interrupts
    intr_base = pd->intr_alloc(spec->intr_count);
    if (intr_base < 0) {
        NIC_LOG_ERR("{}: Failed to allocate interrupts", spec->name);
        throw;
    }
    NIC_LOG_DEBUG("{}: intr_base {} intr_count {}", spec->name, intr_base, spec->intr_count);

    // Allocate & Init Devcmd Region
    devcmd_mem_addr = pd->devcmd_mem_alloc(4096);
    if (devcmd_mem_addr == 0) {
        NIC_LOG_ERR("{}: Failed to allocate devcmd region", spec->name);
        throw;
    }
    MEM_SET(devcmd_mem_addr, 0, 4096, 0);
    // TODO: mmap instead of calloc after porting to real pal
    devcmd = (struct dev_cmd_regs *)calloc(1, sizeof(struct dev_cmd_regs));
    if (devcmd == NULL) {
        NIC_LOG_ERR("{}: Failed to map devcmd region", spec->name);
        throw;
    }
    devcmd->signature = DEV_CMD_SIGNATURE;
    WRITE_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(*devcmd), 0);

    devcmddb_mem_addr = pd->devcmd_mem_alloc(4096);
    if (devcmddb_mem_addr == 0) {
        NIC_LOG_ERR("{}: Failed to allocate devcmddb region", spec->name);
        throw;
    }
    MEM_SET(devcmddb_mem_addr, 0, 4096, 0);

    NIC_LOG_DEBUG("{}: devcmd_addr {:#x} devcmddb_addr {:#x}",
        spec->name, devcmd_mem_addr, devcmddb_mem_addr);

    // Allocate CMB region
    if (spec->enable_rdma && spec->barmap_size) {
        cmb_mem_size = (spec->barmap_size << MEM_BARMAP_SIZE_SHIFT);
        assert (cmb_mem_size <= (8 * 1024 * 1024));

        cmb_mem_addr = pd->rdma_mem_bar_alloc(cmb_mem_size);
        assert (cmb_mem_addr != 0);
        // bar address must be aligned to bar size
        assert ((cmb_mem_size % cmb_mem_size) == 0);

        NIC_LOG_DEBUG("{}: cmb_mem_addr {:#x}, cmb_mem_size: {}",
                      spec->name, cmb_mem_addr, cmb_mem_size);
    } else {
        cmb_mem_addr = 0;
        cmb_mem_size = 0;
    }

    // Create the device
    if (spec->eth_type == ETH_HOST_MGMT || spec->eth_type == ETH_HOST) {
        if (!CreateHostDevice()) {
            NIC_LOG_ERR("{}: Failed to create device", spec->name);
            throw;
        }
    } else {
        NIC_LOG_DEBUG("{}: Skipped creating device", spec->name);
    }

    // Create all LIFs
    for (uint32_t lif_index = 0; lif_index < spec->lif_count; lif_index++) {
        eth_lif_res_t *lif_res = new eth_lif_res_t();
        uint64_t lif_id = lif_base + lif_index;

        lif_res->lif_id = lif_id;
        lif_res->intr_base = intr_base;
        lif_res->cmb_mem_addr = cmb_mem_addr;
        lif_res->cmb_mem_size = cmb_mem_size;

        EthLif *eth_lif = new EthLif(dev_api,
            dev_spec, pd_client, lif_res);
        lif_map[lif_id] = eth_lif;
    }

    // Enable Devcmd Handling
    evutil_timer_start(&devcmd_timer, Eth::DevcmdPoll, this, 0.0, 0.001);
    evutil_add_check(&devcmd_check, Eth::DevcmdPoll, this);
    evutil_add_prepare(&devcmd_prepare, Eth::DevcmdPoll, this);
}

static void *
create_mnet(void *obj)
{
    struct mnet_dev_create_req_t *req = (struct mnet_dev_create_req_t *)obj;

    NIC_LOG_INFO("{}: started mnet thread", req->iface_name);

    int ret = create_mnet(req);
    if (ret) {
        NIC_LOG_ERR("{}: Failed to create MNIC device. ret: {}",
            req->iface_name, ret);
        free(obj);
        return NULL;
    }

    free(obj);
    return NULL;
}

struct eth_devspec *
Eth::ParseConfig(boost::property_tree::ptree::value_type node)
{
    eth_devspec* eth_spec;
    auto val = node.second;

    eth_spec = new struct eth_devspec;
    memset(eth_spec, 0, sizeof(*eth_spec));

    eth_spec->name = val.get<string>("name");
    eth_spec->dev_uuid = val.get<uint64_t>("dev_uuid");
    eth_spec->lif_count = val.get<uint64_t>("lif_count");
    eth_spec->rxq_count = val.get<uint64_t>("rxq_count");
    eth_spec->txq_count = val.get<uint64_t>("txq_count");
    eth_spec->eq_count = val.get<uint64_t>("eq_count");
    eth_spec->adminq_count = val.get<uint64_t>("adminq_count");
    eth_spec->intr_count = val.get<uint64_t>("intr_count");

    if (val.get_optional<string>("rdma")) {
        eth_spec->enable_rdma = true;
        eth_spec->rdma_sq_count = val.get<uint64_t>("rdma.sq_count");
        eth_spec->rdma_rq_count = val.get<uint64_t>("rdma.rq_count");
        eth_spec->rdma_cq_count = val.get<uint64_t>("rdma.cq_count");
        eth_spec->rdma_eq_count = val.get<uint64_t>("rdma.eq_count");
        eth_spec->rdma_adminq_count = val.get<uint64_t>("rdma.adminq_count");
        eth_spec->rdma_pid_count = val.get<uint64_t>("rdma.pid_count");
        eth_spec->key_count = val.get<uint64_t>("rdma.key_count");
        eth_spec->pte_count = val.get<uint64_t>("rdma.pte_count");
        eth_spec->ah_count = val.get<uint64_t>("rdma.ah_count");
        //eth_spec->barmap_size = val.get<uint64_t>("rdma.barmap_size");
        eth_spec->barmap_size = 1;
    }

    if (val.get_optional<string>("network")) {
        eth_spec->uplink_port_num = val.get<uint64_t>("network.uplink");
    }

    eth_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
    if (val.get_optional<string>("pcie.oprom")) {
        eth_spec->oprom = Device::oprom_type_str_to_type(val.get<string>("pcie.oprom"));
    }

    if (val.get_optional<string>("type")) {
        eth_spec->eth_type = eth_dev_type_str_to_type(val.get<string>("type"));
    } else {
        eth_spec->eth_type = ETH_UNKNOWN;
    }

    eth_spec->qos_group = val.get<string>("qos_group", "DEFAULT");

    NIC_LOG_DEBUG("Creating eth device with name: {}, type: {}, "
            "pinned_uplink: {}, qos_group {}",
            eth_spec->name,
            eth_dev_type_to_str(eth_spec->eth_type),
            eth_spec->uplink_port_num,
            eth_spec->qos_group);

    return eth_spec;
}

bool
Eth::CreateLocalDevice()
{
    NIC_LOG_DEBUG("{}: Creating MNIC device", spec->name);

#ifndef __aarch64__
    return true;
#endif

    struct mnet_dev_create_req_t *mnet_req = NULL;

    mnet_req = (struct mnet_dev_create_req_t *)calloc(1, sizeof(*mnet_req));
    if (mnet_req == NULL) {
        NIC_LOG_ERR("{}: Failed to allocate mnet request", spec->name);
        return false;
    }

    mnet_req->devcmd_pa = devcmd_mem_addr;
    mnet_req->devcmd_db_pa = devcmddb_mem_addr;
    mnet_req->doorbell_pa = DOORBELL_ADDR(lif_base);
    mnet_req->drvcfg_pa = intr_drvcfg_addr(intr_base);
    mnet_req->msixcfg_pa = intr_msixcfg_addr(intr_base);
    strcpy(mnet_req->iface_name, spec->name.c_str());

    NIC_LOG_DEBUG("{}: devcmd_pa: {:#x}, devcmddb_pa: {:#x}, doorbell_pa: {:#x},"
                    " drvcfg_pa: {:#x}, msixcfg_pa: {:#x}",
                    mnet_req->iface_name,
                    mnet_req->devcmd_pa,
                    mnet_req->devcmd_db_pa,
                    mnet_req->doorbell_pa,
                    mnet_req->drvcfg_pa,
                    mnet_req->msixcfg_pa);

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_fwcfg(intr_base + intr, lif_base, 0, 0, 0, 0);
        intr_fwcfg_local(intr_base + intr, 1);
    }

#define NICMGRD_THREAD_ID_MNET 0
    sdk::lib::thread *mnet_thread = NULL;

    sdk::lib::thread::control_cores_mask_set(0x8);
    mnet_thread = sdk::lib::thread::factory(spec->name.c_str(),
                                            NICMGRD_THREAD_ID_MNET,
                                            sdk::lib::THREAD_ROLE_CONTROL,
                                            0x8,
                                            create_mnet,
                                            sched_get_priority_max(SCHED_OTHER),
                                            SCHED_OTHER,
                                            false); // yield
    if (mnet_thread == NULL) {
        NIC_LOG_ERR("{}: Unable to start mnet thread. Exiting!!", spec->name);
        return false;
    }

    mnet_thread->start(mnet_req);

    return true;
}

bool
Eth::LoadOprom()
{
    std::string rom_file_path;
    uint64_t rom_file_size, rom_bar_size;

    rom_mem_addr = 0;
    rom_mem_size = 0;

    // FIXME: Get the filepaths from catalog
#ifdef __aarch64__
    rom_file_path = "/platform/oprom/";
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
    rom_mem_size = roundup_power2(rom_file_size);
    rom_bar_size = roundup_power2(rom_file_size);
    rom_mem_addr = roundup(pd->nicmgr_mem_alloc(2*rom_bar_size), rom_bar_size);
    NIC_LOG_INFO("{}: rom_mem_addr {:#x} rom_mem_size {}"
                " rom_file_size {} rom_bar_size {}",
                spec->name, rom_mem_addr, rom_mem_size,
                rom_file_size, rom_bar_size);
    // Must be naturally aligned
    if ((rom_mem_addr % rom_bar_size) != 0) {
        NIC_LOG_ERR("{}: rom_mem_addr is not naturally aligned", spec->name);
        fclose(rom_file);
        rom_mem_addr = 0;
        rom_mem_size = 0;
        return false;
    }

    NIC_LOG_INFO("{}: Writing oprom", spec->name);
    uint64_t rom_addr = rom_mem_addr;
    uint8_t buf[4096] = {0};
    uint32_t bytes_read = 0;
    while (!feof(rom_file)) {
        bytes_read = fread(buf, sizeof(buf[0]), sizeof(buf), rom_file);
        WRITE_MEM(rom_addr, buf, bytes_read, 0);
        rom_addr += bytes_read;
    }
    // zero-out rest of the bar
    MEM_SET(rom_addr, 0, rom_mem_size - rom_file_size, 0);
    NIC_LOG_INFO("{}: Finished writing oprom", spec->name);

    fclose(rom_file);

    return true;
}

bool
Eth::CreateHostDevice()
{
    pciehdevice_resources_t pci_resources = {0};

    if (!LoadOprom()) {
        NIC_LOG_ERR("{}: Failed to load oprom", spec->name);
        // FIXME: error out after oproms are pacakged in the image
        // return false;
    }

    pci_resources.port = spec->pcie_port;
    pci_resources.lifb = lif_base;
    pci_resources.lifc = spec->lif_count;
    pci_resources.intrb = intr_base;
    pci_resources.intrc = spec->intr_count;
    pci_resources.npids = spec->rdma_pid_count;
    pci_resources.devcmdpa = devcmd_mem_addr;
    pci_resources.devcmddbpa = devcmddb_mem_addr;
    pci_resources.cmbpa = cmb_mem_addr;
    pci_resources.cmbsz = cmb_mem_size;
    pci_resources.rompa = rom_mem_addr;
    pci_resources.romsz = rom_mem_size;

    // Create PCI device
    if (spec->eth_type == ETH_HOST) {
        NIC_LOG_DEBUG("{}: Creating Host device", spec->name);
        pdev = pciehdev_eth_new(spec->name.c_str(), &pci_resources);
    } else if (spec->eth_type == ETH_HOST_MGMT) {
        NIC_LOG_DEBUG("{}: Creating Host Management device", spec->name);
        pdev = pciehdev_mgmteth_new(spec->name.c_str(), &pci_resources);
    } else {
        assert(0); // NOT REACHABLE
    }

    if (pdev == NULL) {
        NIC_LOG_ERR("{}: Failed to create PCI device", spec->name);
        return false;
    }

    // Add device to PCI topology
    if (pciemgr) {
        int ret = pciemgr->add_device(pdev);
        if (ret != 0) {
            NIC_LOG_ERR("{}: Failed to add PCI device to topology", spec->name);
            return false;
        }
    }

    return true;
}

void
Eth::DevcmdPoll(void *obj)
{
    Eth             *dev = (Eth *)obj;
    dev_cmd_db_t    db = {0};
    dev_cmd_db_t    db_clear = {0};

    READ_MEM(dev->devcmddb_mem_addr, (uint8_t *)&db, sizeof(db), 0);
    if (db.v) {
        NIC_LOG_INFO("{}: Devcmd doorbell", dev->spec->name);
        WRITE_MEM(dev->devcmddb_mem_addr, (uint8_t *)&db_clear, sizeof(db_clear), 0);
        dev->DevcmdHandler();
    }
}

void
Eth::DevcmdHandler()
{
    status_code_t status;

    NIC_HEADER_TRACE("Devcmd");

    // read devcmd region
    READ_MEM(devcmd_mem_addr, (uint8_t *)devcmd,
             sizeof(struct dev_cmd_regs), 0);

    if (devcmd->done != 0) {
        NIC_LOG_ERR("{}: Devcmd done is set before processing command, opcode {}",
            spec->name,
            opcode_to_str((cmd_opcode_t)devcmd->cmd.cmd.opcode));
        status = IONIC_RC_ERROR;
        goto devcmd_done;
    }

    if (devcmd->signature != DEV_CMD_SIGNATURE) {
        NIC_LOG_ERR("{}: Devcmd signature mismatch, opcode {}",
            spec->name,
            opcode_to_str((cmd_opcode_t)devcmd->cmd.cmd.opcode));
        status = IONIC_RC_ERROR;
        goto devcmd_done;
    }

    status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->comp, &devcmd->data);

    // write data
    if (status == IONIC_RC_SUCCESS) {
        WRITE_MEM(devcmd_mem_addr + offsetof(struct dev_cmd_regs, data),
                  (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, data),
                  sizeof(devcmd->data), 0);
    }

devcmd_done:
    devcmd->comp.comp.status = status;
    devcmd->done = 1;

    // write completion
    WRITE_MEM(devcmd_mem_addr + offsetof(struct dev_cmd_regs, comp),
              (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, comp),
              sizeof(devcmd->comp), 0);

    // write done
    WRITE_MEM(devcmd_mem_addr + offsetof(struct dev_cmd_regs, done),
              (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, done),
              sizeof(devcmd->done), 0);

    NIC_HEADER_TRACE("Devcmd End");
}

#define CASE(opcode) case opcode: return #opcode

const char*
Eth::opcode_to_str(cmd_opcode_t opcode)
{
    switch(opcode) {
        CASE(CMD_OPCODE_NOP);
        CASE(CMD_OPCODE_RESET);
        CASE(CMD_OPCODE_IDENTIFY);
        CASE(CMD_OPCODE_LIF_INIT);
        CASE(CMD_OPCODE_LIF_RESET);
        CASE(CMD_OPCODE_ADMINQ_INIT);
        CASE(CMD_OPCODE_PORT_CONFIG_SET);
        default: return "DEVCMD_UNKNOWN";
    }
}

status_code_t
Eth::CmdHandler(void *req, void *req_data,
    void *resp, void *resp_data)
{
    union dev_cmd *cmd = (union dev_cmd *)req;
    union dev_cmd_comp *comp = (union dev_cmd_comp *)resp;
    status_code_t status = IONIC_RC_SUCCESS;

    NIC_LOG_DEBUG("{}: Handling cmd: {}", spec->name,
        opcode_to_str((cmd_opcode_t)cmd->cmd.opcode));

    switch (cmd->cmd.opcode) {

    case CMD_OPCODE_NOP:
        status = IONIC_RC_SUCCESS;
        break;

    case CMD_OPCODE_RESET:
        status = _CmdReset(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_IDENTIFY:
        status = _CmdIdentify(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_LIF_INIT:
        status = _CmdLifInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_LIF_RESET:
        status = _CmdLifReset(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_ADMINQ_INIT:
        status = _CmdAdminQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_PORT_CONFIG_SET:
        status = _CmdPortConfigSet(req, req_data, resp, resp_data);
        break;

    default:
        // FIXME: Remove Backwards compatibility
        status = AdminCmdHandler(lif_base, req, req_data, resp, resp_data);
        // NIC_LOG_ERR("{}: Unknown Opcode {}", spec->name, cmd->cmd.opcode);
        // status = IONIC_RC_EOPCODE;
        break;
    }

    comp->comp.status = status;
    comp->comp.rsvd = 0xff;
    NIC_LOG_DEBUG("{}: Done cmd: {}, status: {}", spec->name,
        opcode_to_str((cmd_opcode_t)cmd->cmd.opcode), status);

    return (status);
}

status_code_t
Eth::_CmdIdentify(void *req, void *req_data, void *resp, void *resp_data)
{
    union identity *rsp = (union identity *)resp_data;
    struct identify_comp *comp = (struct identify_comp *)resp;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_IDENTIFY", spec->name);

    const uint32_t sta_ver = READ_REG32(CAP_ADDR_BASE_MS_MS_OFFSET +
                                        CAP_MS_CSR_STA_VER_BYTE_ADDRESS);
    rsp->dev.asic_type = sta_ver & 0xf;
    rsp->dev.asic_rev  = (sta_ver >> 4) & 0xfff;

    std::string sn;
    readKey(SERIALNUMBER_KEY, sn);
    strncpy0(rsp->dev.serial_num, sn.c_str(), sizeof(rsp->dev.serial_num));

    boost::property_tree::ptree ver;
    boost::property_tree::read_json(VERSION_FILE, ver);
    strncpy0(rsp->dev.fw_version, ver.get<std::string>("sw.version").c_str(),
        sizeof(rsp->dev.fw_version));

    rsp->dev.nlifs = spec->lif_count;
    rsp->dev.nintrs = spec->intr_count;
    rsp->dev.ndbpgs_per_lif = MAX(spec->rdma_pid_count, 1);
    rsp->dev.nucasts_per_lif = 32;
    rsp->dev.nmcasts_per_lif = 32;
    rsp->dev.intr_coal_mult = 1;
    rsp->dev.intr_coal_div = 10;

    rsp->dev.rdma_version = 1;
    rsp->dev.rdma_qp_opcodes = 27;
    rsp->dev.rdma_admin_opcodes = 50;
    rsp->dev.nrdma_pts_per_lif = spec->pte_count;
    rsp->dev.nrdma_mrs_per_lif = spec->key_count;
    rsp->dev.nrdma_ahs_per_lif = spec->ah_count;
    rsp->dev.rdma_max_stride = 11;
    rsp->dev.rdma_cl_stride = 6;
    rsp->dev.rdma_pte_stride = 3;
    rsp->dev.rdma_rrq_stride = 6;
    rsp->dev.rdma_rsq_stride = 5;

    rsp->dev.tx_qtype.qtype = 0;
    rsp->dev.tx_qtype.qid_count = spec->txq_count;
    rsp->dev.tx_qtype.qid_base = 0;

    rsp->dev.rx_qtype.qtype = 0;
    rsp->dev.rx_qtype.qid_count = spec->rxq_count;
    rsp->dev.rx_qtype.qid_base = 0;

    rsp->dev.admin_qtype.qtype = 2;
    rsp->dev.admin_qtype.qid_count = spec->adminq_count;
    rsp->dev.admin_qtype.qid_base = 0;

    rsp->dev.notify_qtype.qtype = 6;
    rsp->dev.notify_qtype.qid_count = spec->eq_count;
    rsp->dev.notify_qtype.qid_base = 0;

    rsp->dev.rdma_aq_qtype.qtype = 2;
    rsp->dev.rdma_aq_qtype.qid_count = spec->rdma_adminq_count;
    rsp->dev.rdma_aq_qtype.qid_base = spec->adminq_count;

    rsp->dev.rdma_sq_qtype.qtype = 3;
    rsp->dev.rdma_sq_qtype.qid_count = spec->rdma_sq_count;
    rsp->dev.rdma_sq_qtype.qid_base = 0;

    rsp->dev.rdma_rq_qtype.qtype = 4;
    rsp->dev.rdma_rq_qtype.qid_count = spec->rdma_rq_count;
    rsp->dev.rdma_rq_qtype.qid_base = 0;

    rsp->dev.rdma_cq_qtype.qtype = 5;
    rsp->dev.rdma_cq_qtype.qid_count = spec->rdma_cq_count;
    rsp->dev.rdma_cq_qtype.qid_base = 0;

    rsp->dev.rdma_eq_qtype.qtype = 6;
    rsp->dev.rdma_eq_qtype.qid_count = spec->rdma_eq_count;
    rsp->dev.rdma_eq_qtype.qid_base = spec->eq_count;

    // XXX RDMA data path requires that RDMA AdminQ is QID 1 for now.
    rsp->dev.admin_qtype.qid_base = 0;
    rsp->dev.admin_qtype.qid_count = 1;
    rsp->dev.rdma_aq_qtype.qid_base = 1;
    rsp->dev.rdma_aq_qtype.qid_count = 1;
    // XXX Remove hardcode when flexibility is added.

    comp->ver = IDENTITY_VERSION_1;

    NIC_LOG_DEBUG("{}: asic_type {} asic_rev {} serial_num {} fw_version {} "
                 "ndbpgs_per_lif {} nintrs {} nucasts_per_lif {} nmcasts_per_lif {} "
                 "intr_coal_mult {} intr_coal_div {} "
                 "rdma_version {} rdma_qp_opcodes {} rdma_admin_opcodes {} "
                 "nrdma_pts_per_lif {} nrdma_mrs_per_lif {} "
                 "nrdma_ahs_per_lif {} rdma_max_stride {} "
                 "rdma_cl_stride {} rdma_pte_stride {} "
                 "rdma_rrq_stride {} rdma_rsq_stride {} "
                 "tx_qtype {} tx_qid_count {} tx_qid_base {} "
                 "rx_qtype {} rx_qid_count {} rx_qid_base {} "
                 "admin_qtype {} admin_qid_count {} admin_qid_base {} "
                 "notify_qtype {} notify_qid_count {} notify_qid_base {} "
                 "rdma_aq_qtype {} rdma_aq_qid_count {} rdma_aq_qid_base {} "
                 "rdma_sq_qtype {} rdma_sq_qid_count {} rdma_sq_qid_base {} "
                 "rdma_rq_qtype {} rdma_rq_qid_count {} rdma_rq_qid_base {} "
                 "rdma_cq_qtype {} rdma_cq_qid_count {} rdma_cq_qid_base {} "
                 "rdma_eq_qtype {} rdma_eq_qid_count {} rdma_eq_qid_base {}",
                 spec->name,
                 rsp->dev.asic_type, rsp->dev.asic_rev, rsp->dev.serial_num,
                 rsp->dev.fw_version, rsp->dev.ndbpgs_per_lif, rsp->dev.nintrs,
                 rsp->dev.nucasts_per_lif, rsp->dev.nmcasts_per_lif,
                 rsp->dev.intr_coal_mult, rsp->dev.intr_coal_div,
                 rsp->dev.rdma_version, rsp->dev.rdma_qp_opcodes, rsp->dev.rdma_admin_opcodes,
                 rsp->dev.nrdma_pts_per_lif, rsp->dev.nrdma_mrs_per_lif, rsp->dev.nrdma_ahs_per_lif,
                 rsp->dev.rdma_max_stride, rsp->dev.rdma_cl_stride, rsp->dev.rdma_pte_stride,
                 rsp->dev.rdma_rrq_stride, rsp->dev.rdma_rsq_stride,
                 rsp->dev.tx_qtype.qtype, rsp->dev.tx_qtype.qid_count, rsp->dev.tx_qtype.qid_base,
                 rsp->dev.rx_qtype.qtype, rsp->dev.rx_qtype.qid_count, rsp->dev.rx_qtype.qid_base,
                 rsp->dev.admin_qtype.qtype, rsp->dev.admin_qtype.qid_count, rsp->dev.admin_qtype.qid_base,
                 rsp->dev.notify_qtype.qtype, rsp->dev.notify_qtype.qid_count, rsp->dev.notify_qtype.qid_base,
                 rsp->dev.rdma_aq_qtype.qtype, rsp->dev.rdma_aq_qtype.qid_count, rsp->dev.rdma_aq_qtype.qid_base,
                 rsp->dev.rdma_sq_qtype.qtype, rsp->dev.rdma_sq_qtype.qid_count, rsp->dev.rdma_sq_qtype.qid_base,
                 rsp->dev.rdma_rq_qtype.qtype, rsp->dev.rdma_rq_qtype.qid_count, rsp->dev.rdma_rq_qtype.qid_base,
                 rsp->dev.rdma_cq_qtype.qtype, rsp->dev.rdma_cq_qtype.qid_count, rsp->dev.rdma_cq_qtype.qid_base,
                 rsp->dev.rdma_eq_qtype.qtype, rsp->dev.rdma_eq_qtype.qid_count, rsp->dev.rdma_eq_qtype.qid_base);

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdReset(void *req, void *req_data, void *resp, void *resp_data)
{
    status_code_t status;
    EthLif *eth_lif = NULL;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_RESET", spec->name);

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_pba_clear(intr_base + intr);
        intr_drvcfg(intr_base + intr);
    }

    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        eth_lif = it->second;
        status = eth_lif->Reset(req, req_data, resp, resp_data);
        if (status != IONIC_RC_SUCCESS) {
            NIC_LOG_ERR("{}: Failed to reset lif", spec->name);
            return (status);
        }
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::_CmdLifInit(void *req, void *req_data, void *resp, void *resp_data)
{
    struct lif_init_cmd *cmd = (struct lif_init_cmd *)req;
    uint64_t lif_id = lif_base + cmd->index;
    EthLif *eth_lif = NULL;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_LIF_INIT: index {}", spec->name, cmd->index);

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    if (cmd->index >= spec->lif_count) {
        NIC_LOG_ERR("{}: bad lif index {}", spec->name, cmd->index);
        return (IONIC_RC_ERROR);
    }

    auto it = lif_map.find(lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, lif_id);
        return (IONIC_RC_ERROR);
    }
    eth_lif = it->second;

    return eth_lif->Init(req, req_data, resp, resp_data);
}

status_code_t
Eth::_CmdLifReset(void *req, void *req_data, void *resp, void *resp_data)
{
    struct lif_reset_cmd *cmd = (struct lif_reset_cmd *)req;
    uint64_t lif_id = lif_base + cmd->index;
    EthLif *eth_lif = NULL;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_LIF_RESET: index {}", spec->name, cmd->index);

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    if (cmd->index >= spec->lif_count) {
        NIC_LOG_ERR("{}: bad lif index {}", spec->name, cmd->index);
        return (IONIC_RC_ERROR);
    }

    auto it = lif_map.find(lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, lif_id);
        return (IONIC_RC_ERROR);
    }
    eth_lif = it->second;

    return eth_lif->Reset(req, req_data, resp, resp_data);
}

status_code_t
Eth::_CmdAdminQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    struct adminq_init_cmd *cmd = (struct adminq_init_cmd *)req;
    uint64_t lif_id = lif_base + cmd->lif_index;
    EthLif *eth_lif = NULL;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_ADMINQ_INIT: lif_index {}", spec->name, cmd->lif_index);

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

    return eth_lif->AdminQInit(req, req_data, resp, resp_data);
}

status_code_t
Eth::_CmdPortConfigSet(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t ret = SDK_RET_OK;
    struct port_config_cmd *cmd = (struct port_config_cmd *)req;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_PORT_CONFIG_SET", spec->name);

    if (!dev_api) {
        return (IONIC_RC_ERROR);
    }
    ret = dev_api->port_set_config(spec->uplink_port_num,
                                   (port_config_t *)&cmd->config);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: failed to set port config", spec->name);
        return (IONIC_RC_ERROR);
    }
#if 0
    int ret = hal->PortConfigSet(spec->uplink_port_num,
                (hal_port_config_t &)cmd->config);
    if (ret < 0) {
        NIC_LOG_ERR("{}: failed to set port config", spec->name);
        return (IONIC_RC_ERROR);
    }
#endif

    return (IONIC_RC_SUCCESS);
}

status_code_t
Eth::AdminCmdHandler(uint64_t lif_id,
    void *req, void *req_data,
    void *resp, void *resp_data)
{
    EthLif *eth_lif = NULL;

    auto it = lif_map.find(lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, lif_id);
        return (IONIC_RC_ERROR);
    }
    eth_lif = it->second;

    return eth_lif->CmdHandler(req, req_data, resp, resp_data);
}

/*
 * Event Handlers
 */
void
Eth::HalEventHandler(bool status)
{
    hal_status = status;

    if (!status) {
        return;
    }

    // Create the MNIC devices
    if (spec->eth_type == ETH_MNIC_OOB_MGMT ||
        spec->eth_type == ETH_MNIC_INTERNAL_MGMT ||
        spec->eth_type == ETH_MNIC_INBAND_MGMT) {
        if (!CreateLocalDevice()) {
            NIC_LOG_ERR("{}: Failed to create device", spec->name);
        }
    }

    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->HalEventHandler(status);
    }
}

void
Eth::LinkEventHandler(port_status_t *evd)
{
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->LinkEventHandler(evd);
    }
}

void
Eth::XcvrEventHandler(port_status_t *evd)
{
    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        EthLif *eth_lif = it->second;
        eth_lif->XcvrEventHandler(evd);
    }
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

lif_type_t
Eth::ConvertDevTypeToLifType(EthDevType dev_type)
{
    switch(dev_type) {
        case ETH_HOST: return sdk::platform::LIF_TYPE_HOST;
        case ETH_HOST_MGMT: return sdk::platform::LIF_TYPE_HOST_MANAGEMENT;
        case ETH_MNIC_OOB_MGMT: return sdk::platform::LIF_TYPE_MNIC_OOB_MANAGEMENT;
        case ETH_MNIC_INTERNAL_MGMT: return sdk::platform::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT;
        case ETH_MNIC_INBAND_MGMT: return sdk::platform::LIF_TYPE_MNIC_INBAND_MANAGEMENT;
        default: return sdk::platform::LIF_TYPE_NONE;
    }
}
