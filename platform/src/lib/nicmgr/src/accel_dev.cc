
/**
 * Accelerator PF Device
 */

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>
#include <sys/time.h>

// Tell accel_dev.hpp to emumerate definitions of all devcmds
#define ACCEL_DEV_CMD_ENUMERATE  1

#include "base.hpp"
#include "logger.hpp"
#include "intrutils.h"
#include "accel_dev.hpp"
#include "hal_client.hpp"
#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"

// Amount of time to wait for sequencer queues to be quiesced
#define ACCEL_DEV_SEQ_QUEUES_QUIESCE_TIME_US    5000000
#define ACCEL_DEV_RING_OP_QUIESCE_TIME_US       1000000
#define ACCEL_DEV_ALL_RINGS_MAX_QUIESCE_TIME_US (10 * ACCEL_DEV_RING_OP_QUIESCE_TIME_US)

// Invoke a function for each and every HW ring
#define ACCEL_DEV_FOR_EACH_RING_INVOKE(csr, func, arg)      \
    for (csr = &accel_csr_tbl[0];                           \
         csr < &accel_csr_tbl[ACCEL_RING_ID_MAX];           \
         csr++) func(csr, arg)
         
static uint32_t log_2(uint32_t x);
static uint64_t timestamp(void);

template<typename ... Args>
string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1 /* for '\0' */;
    unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return string( buf.get(), buf.get() + size - 1 ); // ignore '\0'
}

/*
 * blank_qstate, used only during LIF creation and device reset.
 */
const uint8_t   blank_qstate[64] = { 0 };
const uint8_t   blank_page[ACCEL_DEV_PAGE_SIZE] = { 0 };

struct queue_info Accel_PF::qinfo [NUM_QUEUE_TYPES] = {
    [STORAGE_SEQ_QTYPE_SQ] = {
        .type_num = STORAGE_SEQ_QTYPE_SQ,
        .size = 1,      // qstate size (6 - 5 = 1, i.e., 64 bytes)
        .entries = 6,   // modified at runtime
        .purpose = ::intf::LIF_QUEUE_PURPOSE_STORAGE,
        .prog = "txdma_stage0.bin",
        .label = "storage_seq_stage0",
        .qstate = (const char *)blank_qstate,
    },
    [STORAGE_SEQ_QTYPE_UNUSED] = {
        .type_num = STORAGE_SEQ_QTYPE_UNUSED,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
        .prog = "txdma_stage0.bin",
        .label = "adminq_stage0",
        .qstate = (const char *)blank_qstate,
    },
    [STORAGE_SEQ_QTYPE_ADMIN] = {
        .type_num = STORAGE_SEQ_QTYPE_ADMIN,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
        .prog = "txdma_stage0.bin",
        .label = "adminq_stage0",
        .qstate = (const char *)blank_qstate,
    },
};

static const std::string accel_pf_rgroup_name("Accel_PF_RGroup");
static const std::vector<std::pair<const std::string,uint32_t>> accel_pf_ring_vec = {
    {"cp",      ACCEL_RING_CP},
    {"cp_hot",  ACCEL_RING_CP_HOT},
    {"dc",      ACCEL_RING_DC},
    {"dc_hot",  ACCEL_RING_DC_HOT},
    {"xts0",    ACCEL_RING_XTS0},
    {"xts1",    ACCEL_RING_XTS1},
    {"gcm0",    ACCEL_RING_GCM0},
    {"gcm1",    ACCEL_RING_GCM1},
};

static const std::unordered_map<uint32_t,const char*> accel_ring_id2name_map = {
    {ACCEL_RING_CP,      "cp"},
    {ACCEL_RING_CP_HOT,  "cp_hot"},
    {ACCEL_RING_DC,      "dc"},
    {ACCEL_RING_DC_HOT,  "dc_hot"},
    {ACCEL_RING_XTS0,    "xts0"},
    {ACCEL_RING_XTS1,    "xts1"},
    {ACCEL_RING_GCM0,    "gcm0"},
    {ACCEL_RING_GCM1,    "gcm1"},
};

static inline const char *
accel_ring_id2name_get(uint32_t ring_handle)
{
    std::unordered_map<uint32_t,const char*>::const_iterator iter;

    iter = accel_ring_id2name_map.find(ring_handle);
    if (iter != accel_ring_id2name_map.end()) {
        return iter->second;
    }
    return "unknown";
}

static accel_rgroup_map_t   accel_pf_rgroup_map;

Accel_PF::Accel_PF(HalClient *hal_client, void *dev_spec,
                   const struct lif_info *nicmgr_lif_info) :
    seq_qid_init_high(0),
    nicmgr_lif_info(nicmgr_lif_info)
{
    uint64_t    lif_handle;
    uint64_t    hbm_addr;
    uint32_t    hbm_size;

    hal = hal_client;
    spec = (accel_devspec_t *)dev_spec;

    // Locate HBM region dedicated to STORAGE_SEQ_HBM_HANDLE
    memset(&pci_resources, 0, sizeof(pci_resources));
    if (hal->AllocHbmAddress(STORAGE_SEQ_HBM_HANDLE, &hbm_addr, &hbm_size)) {
        NIC_LOG_ERR("Failed to get HBM base for {}", STORAGE_SEQ_HBM_HANDLE);
        return;
    }

    // hal/pd/capri/capri_hbm.cc stores size in KB;
    hbm_size *= 1024;

    // First, ensure size is a power of 2, then per PCIe BAR mapping
    // requirement, align the region on its natural boundary, i.e.,
    // if size is 64MB then the region must be aligned on a 64MB boundary!
    // This means we could potentially waste half of the space if
    // the region was not already aligned.
    assert(hbm_size && !(hbm_size & (hbm_size - 1)));
    if (hbm_addr & (hbm_size - 1)) {
        hbm_size /= 2;
        hbm_addr = ACCEL_DEV_ADDR_ALIGN(hbm_addr, hbm_size);
    }

    pci_resources.cmbpa = hbm_addr;
    pci_resources.cmbsz = hbm_size;
    NIC_LOG_INFO("HBM address {:#x} size {} bytes", pci_resources.cmbpa, pci_resources.cmbsz);

    // Find devcmd/devcmddb, etc., area
    if (hal->AllocHbmAddress(ACCEL_DEVCMD_HBM_HANDLE, &hbm_addr, &hbm_size)) {
        NIC_LOG_ERR("Failed to get HBM base for {}", ACCEL_DEVCMD_HBM_HANDLE);
        return;
    }

    // devcmd page needs to be page aligned, then all subsequent pages
    // would fall into the same alignment.
    hbm_size *= 1024;
    assert(hbm_size >= ((ACCEL_DEV_BAR0_NUM_PAGES_MAX + 1) *
                        ACCEL_DEV_PAGE_SIZE));
    pci_resources.devcmdpa = ACCEL_DEV_PAGE_ALIGN(hbm_addr);
    pci_resources.devcmddbpa = pci_resources.devcmdpa + ACCEL_DEV_PAGE_SIZE;
    WRITE_MEM(pci_resources.devcmddbpa, (uint8_t *)blank_page, ACCEL_DEV_PAGE_SIZE);

    static_assert(sizeof(dev_cmd_regs_t) == ACCEL_DEV_PAGE_SIZE);
    static_assert((offsetof(dev_cmd_regs_t, cmd) % 4) == 0);
    static_assert(sizeof(devcmd->cmd) == 64);
    static_assert((offsetof(dev_cmd_regs_t, cpl) % 4) == 0);
    static_assert(sizeof(devcmd->cpl) == 16);
    static_assert((offsetof(dev_cmd_regs_t, data) % 4) == 0);

    // Acquire rings info as initialized by HAL
    accel_ring_info_get_all();

    // Create LIF
    qinfo[STORAGE_SEQ_QTYPE_SQ].entries = log_2(spec->seq_queue_count);
    spec->seq_created_count = 1 << qinfo[STORAGE_SEQ_QTYPE_SQ].entries;
    if (hal->lif_map.find(spec->lif_id) != hal->lif_map.end()) {
        if (hal->LifDelete(spec->lif_id)) {
            NIC_LOG_ERR("Failed to delete LIF, id = {}", spec->lif_id);
            return;
        }
    }

    lif_handle = hal->LifCreate(spec->lif_id, qinfo, &info, 0, false, 0, 0, 0);
    if (lif_handle == 0) {
        NIC_LOG_ERR("Failed to create LIF");
        return;
    }

    NIC_LOG_INFO("lif{}: {} sequencer queues created",
           info.hw_lif_id, spec->seq_created_count);

    name = string_format("accel{}", spec->lif_id);

    // Configure PCI resources
    pci_resources.lif_valid = 1;
    pci_resources.lif = info.hw_lif_id;
    pci_resources.intrb = spec->intr_base;
    pci_resources.intrc = spec->intr_count;
    pci_resources.port = spec->pcie_port;
    pci_resources.npids = 0;

    // Init Devcmd Region
    devcmd = (dev_cmd_regs_t *)calloc(1, sizeof(dev_cmd_regs_t));
    devcmd->signature = DEV_CMD_SIGNATURE;
    WRITE_MEM(pci_resources.devcmdpa, (uint8_t *)devcmd, sizeof(*devcmd));

    NIC_LOG_INFO("lif{}: Devcmd PA {:#x} DevcmdDB PA {:#x}", info.hw_lif_id,
        pci_resources.devcmdpa, pci_resources.devcmddbpa);

    if (spec->pcie_port == 0xff) {
        NIC_LOG_INFO("lif{}: Skipped creating PCI device, pcie_port {}", info.hw_lif_id,
            spec->pcie_port);
        return;
    }

    // Create PCI device
    NIC_LOG_INFO("lif{}: creating Accel_PF PCI device", info.hw_lif_id);
    pdev = pciehdev_accel_new(name.c_str(), &pci_resources);
    if (pdev == NULL) {
        NIC_LOG_ERR("lif{}: Failed to create Accel_PF PCI device", info.hw_lif_id);
        return;
    }
    pciehdev_set_priv(pdev, (void *)this);

    // Add device to PCI topology
    int ret = pciehdev_add(pdev);
    if (ret != 0) {
        NIC_LOG_ERR("lif{}: Failed to add Accel_PF PCI device to topology", info.hw_lif_id);
        return;
    }
}

uint64_t
Accel_PF::GetQstateAddr(uint8_t qtype, uint32_t qid)
{
    uint32_t cnt, sz;

    assert(qtype < STORAGE_SEQ_QTYPE_MAX);

    cnt = 1 << this->qinfo[qtype].entries;
    sz = 1 << (5 + this->qinfo[qtype].size);

    assert(qid < cnt);

    return info.qstate_addr[qtype] + (qid * sz);
}

void
Accel_PF::DevcmdPoll()
{
#ifdef __x86_64__
    dev_cmd_db_t    db;
    dev_cmd_db_t    db_clear = {0};

    db.v = 0;
    READ_MEM(pci_resources.devcmddbpa, (uint8_t *)&db, sizeof(db));
    if (db.v) {
        NIC_LOG_INFO("lif{} active", info.hw_lif_id);
        DevcmdHandler();
        WRITE_MEM(pci_resources.devcmddbpa, (uint8_t *)&db_clear, sizeof(db_clear));
    }
#endif
}

void
Accel_PF::DevcmdHandler()
{
    enum DevcmdStatus   status = DEVCMD_SUCCESS;

    // read devcmd region
    READ_MEM(pci_resources.devcmdpa, (uint8_t *)devcmd, sizeof(dev_cmd_regs_t));

    if (devcmd->done) {
        NIC_LOG_ERR("lif{}: Devcmd done is set before processing command, opcode = {}",
            info.hw_lif_id, devcmd->cmd.cmd.opcode);
        status = DEVCMD_ERROR;
    }

    if (devcmd->signature != DEV_CMD_SIGNATURE) {
        NIC_LOG_ERR("lif{}: Devcmd signature mismatch, opcode = {}", info.hw_lif_id,
            devcmd->cmd.cmd.opcode);
        status = DEVCMD_ERROR;
    }

    if (status == DEVCMD_SUCCESS) {
        status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->cpl, &devcmd->data);
    }

    // write data
    if (status == DEVCMD_SUCCESS) {
        WRITE_MEM(pci_resources.devcmdpa + offsetof(dev_cmd_regs_t, data),
                  (uint8_t *)devcmd + offsetof(dev_cmd_regs_t, data),
                  sizeof(devcmd->data));
    }
    _PostDevcmdDone(status);
}

enum DevcmdStatus
Accel_PF::CmdHandler(void *req, void *req_data,
                     void *resp, void *resp_data)
{
    dev_cmd_t       *cmd = (dev_cmd_t *)req;
    dev_cmd_cpl_t   *cpl = (dev_cmd_cpl_t *)resp;
    enum DevcmdStatus status;

    switch (cmd->cmd.opcode) {

    case CMD_OPCODE_NOP:
        NIC_LOG_INFO("lif{}: CMD_OPCODE_NOP", info.hw_lif_id);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RESET:
        status = this->_DevcmdReset(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_IDENTIFY:
        status = this->_DevcmdIdentify(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_LIF_INIT:
        status = this->_DevcmdLifInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_ADMINQ_INIT:
        status = this->_DevcmdAdminQueueInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_SEQ_QUEUE_INIT:
        status = this->_DevcmdSeqQueueInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_HANG_NOTIFY:
        NIC_LOG_INFO("lif{}: CMD_OPCODE_HANG_NOTIFY", info.hw_lif_id);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_SEQ_QUEUE_ENABLE:
        status = this->_DevcmdSeqQueueControl(req, req_data, resp, resp_data, true);
        break;

    case CMD_OPCODE_SEQ_QUEUE_DISABLE:
        status = this->_DevcmdSeqQueueControl(req, req_data, resp, resp_data, false);
        break;

    case CMD_OPCODE_SEQ_QUEUE_DUMP:
        NIC_LOG_INFO("lif{}: CMD_OPCODE_SEQ_QUEUE_DUMP", info.hw_lif_id);
        status = DEVCMD_SUCCESS;
        break;

    default:
        NIC_LOG_ERR("lif{}: Unknown Opcode {}", info.hw_lif_id, cmd->cmd.opcode);
        status = DEVCMD_UNKNOWN;
        break;
    }

    cpl->cpl.status = status;
    cpl->cpl.rsvd = 0xff;

    return (status);
}

void
Accel_PF::_PostDevcmdDone(enum DevcmdStatus status)
{
    devcmd->cpl.cpl.status = status;
    devcmd->done = 1;

    // write completion
    WRITE_MEM(pci_resources.devcmdpa + offsetof(dev_cmd_regs_t, cpl),
              (uint8_t *)devcmd + offsetof(dev_cmd_regs_t, cpl),
              sizeof(devcmd->cpl));

    // write done
    WRITE_MEM(pci_resources.devcmdpa + offsetof(dev_cmd_regs_t, done),
              (uint8_t *)devcmd + offsetof(dev_cmd_regs_t, done),
              sizeof(devcmd->done));
}

enum DevcmdStatus
Accel_PF::_DevcmdIdentify(void *req, void *req_data,
                          void *resp, void *resp_data)
{
    identity_t      *rsp = (identity_t *)resp_data;
    identify_cpl_t  *cpl = (identify_cpl_t *)resp;

    NIC_LOG_INFO("lif{} identity_dev size {}", info.hw_lif_id, (int)sizeof(rsp->dev));
    memset(&devcmd->data[0], 0, sizeof(devcmd->data));

    // TODO: Get these from hw
    rsp->dev.asic_type = 0x00;
    rsp->dev.asic_rev = 0xA0;
    sprintf((char *)&rsp->dev.serial_num, "haps");
    // TODO: Get this from sw
    sprintf((char *)&rsp->dev.fw_version, "v0.0.1");
    rsp->dev.num_lifs = 1;
    rsp->dev.hw_lif_id_tbl[0] = info.hw_lif_id;
    rsp->dev.db_pages_per_lif = 1;
    rsp->dev.admin_queues_per_lif = spec->adminq_count;
    rsp->dev.seq_queues_per_lif = spec->seq_created_count;
    rsp->dev.num_intrs = spec->intr_count;
    rsp->dev.cm_base_pa = pci_resources.cmbpa;
    memcpy(rsp->dev.accel_ring_tbl, spec->accel_ring_tbl,
           sizeof(rsp->dev.accel_ring_tbl));
    cpl->ver = IDENTITY_VERSION_1;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdReset(void *req, void *req_data,
                       void *resp, void *resp_data)
{
    uint64_t                qstate_addr;
    uint32_t                qid;
    uint8_t                 enable = 0;
    uint8_t                 abort = 1;

    NIC_LOG_INFO("lif{}:", info.hw_lif_id);

    // Disable all sequencer queues
    for (qid = 0; qid < spec->seq_created_count; qid++) {
        qstate_addr = GetQstateAddr(STORAGE_SEQ_QTYPE_SQ, qid);
        WRITE_MEM(qstate_addr + offsetof(storage_seq_qstate_t, abort),
                  (uint8_t *)&abort, sizeof(abort));
        WRITE_MEM(qstate_addr + offsetof(storage_seq_qstate_t, enable),
                  (uint8_t *)&enable, sizeof(enable));
        invalidate_txdma_cacheline(qstate_addr);
    }

    for (qid = 0; qid < spec->adminq_count; qid++) {
        qstate_addr = GetQstateAddr(STORAGE_SEQ_QTYPE_ADMIN, qid);
        WRITE_MEM(qstate_addr + offsetof(eth_admin_qstate_t, p_index0),
                  (uint8_t *)blank_qstate + offsetof(eth_admin_qstate_t, p_index0),
                  sizeof(blank_qstate) - offsetof(eth_admin_qstate_t, p_index0));
        invalidate_txdma_cacheline(qstate_addr);
    }

    // Wait for P4+ to quiesce all sequencer queues
    static_assert((offsetof(storage_seq_qstate_t, c_ndx) - 
                  offsetof(storage_seq_qstate_t, p_ndx)) > 0);
    static_assert((offsetof(storage_seq_qstate_t, abort) - 
                  offsetof(storage_seq_qstate_t, p_ndx)) > 0);

    auto seq_queues_quiesce_check = [this, &qid] () -> int
    {
        uint64_t                qstate_addr;
        storage_seq_qstate_t    seq_qstate;

        while (seq_qid_init_high && (qid <= seq_qid_init_high)) {
            qstate_addr = GetQstateAddr(STORAGE_SEQ_QTYPE_SQ, qid);
            READ_MEM(qstate_addr + offsetof(storage_seq_qstate_t, p_ndx),
                     (uint8_t *)&seq_qstate.p_ndx,
                     (offsetof(storage_seq_qstate_t, abort) - 
                      offsetof(storage_seq_qstate_t, p_ndx) +
                      sizeof(seq_qstate.abort)));

            // As part of abort, P4+ would set c_ndx = p_ndx
            if (seq_qstate.c_ndx != seq_qstate.p_ndx) {
                return -1;
            }
            qid++;
        }
        return 0;
    };

    qid = 0;
    Poller poll(ACCEL_DEV_SEQ_QUEUES_QUIESCE_TIME_US);
    poll(seq_queues_quiesce_check);

    NIC_LOG_INFO("[lif{}: last qid quiesced: {} seq_qid_init_high: {}", 
                 info.hw_lif_id, qid, seq_qid_init_high);

    // Reset and reenable accelerator rings
    accel_ring_reset_all();
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdLifInit(void *req, void *req_data,
                         void *resp, void *resp_data)
{
    lif_init_cmd_t  *cmd = (lif_init_cmd_t *)req;

    NIC_LOG_INFO("lif{}: lif_index {}", info.hw_lif_id, cmd->index);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdAdminQueueInit(void *req, void *req_data,
                                void *resp, void *resp_data)
{
    adminq_init_cmd_t           *cmd = (adminq_init_cmd_t *)req;
    adminq_init_cpl_t           *cpl = (adminq_init_cpl_t *)resp;
    storage_seq_admin_qstate_t  admin_qstate;
    uint64_t                    addr;

    NIC_LOG_INFO("lif{}: queue_index {} ring_base {:#x} ring_size {} intr_index {}",
        info.hw_lif_id, cmd->index, cmd->ring_base,cmd->ring_size, cmd->intr_index);

    if (cmd->index >= spec->adminq_count) {
        NIC_LOG_ERR("lif{}: bad qid {}",
               info.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif{}: bad intr {}", info.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif{}: bad ring size {}", info.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = GetQstateAddr(STORAGE_SEQ_QTYPE_ADMIN, cmd->index);

    READ_MEM(addr, (uint8_t *)&admin_qstate, sizeof(admin_qstate));
    //NOTE: admin_qstate.cosA is ignored for Admin Queues. Db should ring on cosB.
    admin_qstate.cosA = 0;
    //NOTE: admin_qstate.cosB is set by HAL LifCreate
    admin_qstate.host = 1;
    admin_qstate.total = 1;
    admin_qstate.pid = cmd->pid;
    admin_qstate.enable = 1;
    admin_qstate.color = 1;
    admin_qstate.rsvd1 = 0x1f;
    admin_qstate.p_index0 = 0;
    admin_qstate.c_index0 = 0;
    admin_qstate.comp_index = 0;
    admin_qstate.ci_fetch = 0;
    admin_qstate.host_queue = ACCEL_PHYS_ADDR_HOST_GET(cmd->ring_base);
    admin_qstate.ring_base = cmd->ring_base;
    if (admin_qstate.host_queue && !ACCEL_PHYS_ADDR_LIF_GET(cmd->ring_base)) {
        ACCEL_PHYS_ADDR_LIF_SET(admin_qstate.ring_base, info.hw_lif_id);
    }
    admin_qstate.ring_size = cmd->ring_size;
    admin_qstate.cq_ring_base = roundup(admin_qstate.ring_base + (64 << cmd->ring_size), 4096);
    admin_qstate.intr_assert_addr = intr_assert_addr(spec->intr_base + cmd->intr_index);
    if (nicmgr_lif_info) {
        admin_qstate.nicmgr_qstate_addr = nicmgr_lif_info->qstate_addr[NICMGR_QTYPE_REQ];
        NIC_LOG_INFO("lif{}: nicmgr_qstate_addr RX {:#x}", info.hw_lif_id,
            admin_qstate.nicmgr_qstate_addr);
    }
    WRITE_MEM(addr, (uint8_t *)&admin_qstate, sizeof(admin_qstate));

    invalidate_txdma_cacheline(addr);

    cpl->qid = spec->adminq_base + cmd->index;
    cpl->qtype = STORAGE_SEQ_QTYPE_ADMIN;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueInit(void *req, void *req_data,
                              void *resp, void *resp_data)
{
    seq_queue_init_cmd_t    *cmd = (seq_queue_init_cmd_t *)req;
    seq_queue_init_cpl_t    *cpl = (seq_queue_init_cpl_t *)resp;
    uint64_t                qstate_addr;
    uint64_t                next_pc_addr;
    uint32_t                qid = spec->seq_queue_base + cmd->index;
    storage_seq_qstate_t    seq_qstate = {0};
    const char              *desc0_pgm_name = nullptr;
    const char              *desc1_pgm_name = nullptr;
    enum DevcmdStatus       status = DEVCMD_ERROR;

    switch (cmd->qgroup) {

    case STORAGE_SEQ_QGROUP_CPDC:
        desc0_pgm_name = STORAGE_SEQ_PGM_NAME_SQ_GEN;
        break;

    case STORAGE_SEQ_QGROUP_CPDC_STATUS:
        desc0_pgm_name = STORAGE_SEQ_PGM_NAME_CPDC_STATUS0;
        desc1_pgm_name = STORAGE_SEQ_PGM_NAME_CPDC_STATUS1;
        break;

    case STORAGE_SEQ_QGROUP_CRYPTO:
        desc0_pgm_name = STORAGE_SEQ_PGM_NAME_SQ_GEN;
        break;

    case STORAGE_SEQ_QGROUP_CRYPTO_STATUS:
        desc0_pgm_name = STORAGE_SEQ_PGM_NAME_CRYPTO_STATUS0;
        desc1_pgm_name = STORAGE_SEQ_PGM_NAME_CRYPTO_STATUS1;
        break;

    default:
        break;
    }

    if (cmd->index >= spec->seq_created_count) {
        NIC_LOG_ERR("lif{}: qgroup {} index {} exceeds max {}", info.hw_lif_id,
            cmd->qgroup, cmd->index, spec->seq_created_count);
        goto devcmd_done;
    }

    qstate_addr = GetQstateAddr(STORAGE_SEQ_QTYPE_SQ, qid);
    seq_qid_init_high = std::max(seq_qid_init_high, qid);
    READ_MEM(qstate_addr, (uint8_t *)&seq_qstate, sizeof(seq_qstate));

    seq_qstate.cosA = cmd->cos;
    //NOTE: seq_qstate.cosB is ignored for TX queues.
    seq_qstate.host_wrings = cmd->host_wrings;
    seq_qstate.total_wrings = cmd->total_wrings;
    seq_qstate.pid = cmd->pid;
    seq_qstate.p_ndx = 0;
    seq_qstate.c_ndx = 0;
    seq_qstate.enable = cmd->enable;
    seq_qstate.abort = 0;
    seq_qstate.wring_base = cmd->wring_base;
    if (ACCEL_PHYS_ADDR_HOST_GET(cmd->wring_base) && 
        !ACCEL_PHYS_ADDR_LIF_GET(cmd->wring_base)) {
        ACCEL_PHYS_ADDR_LIF_SET(seq_qstate.wring_base, info.hw_lif_id);
    }

    NIC_LOG_INFO("lif{}: qid {} qgroup {} wring_base {:#x} wring_size {} entry_size {}",
           info.hw_lif_id, qid, cmd->qgroup, seq_qstate.wring_base, cmd->wring_size,
           cmd->entry_size);

    seq_qstate.wring_base = htonll(seq_qstate.wring_base);
    seq_qstate.wring_size = htons(cmd->wring_size);
    seq_qstate.entry_size = htons(cmd->entry_size);

    if (hal->PgmBaseAddrGet(desc0_pgm_name, &next_pc_addr)) {
        NIC_LOG_ERR("Failed to get base for program {}", desc0_pgm_name);
        goto devcmd_done;
    }
    seq_qstate.desc0_next_pc = htonl(next_pc_addr >> 6);
    seq_qstate.desc1_next_pc = 0;
    seq_qstate.desc1_next_pc_valid = 0;

    if (desc1_pgm_name) {
        if (hal->PgmBaseAddrGet(desc1_pgm_name, &next_pc_addr)) {
            NIC_LOG_ERR("Failed to get base for program {}", desc1_pgm_name);
            goto devcmd_done;
        }
        seq_qstate.desc1_next_pc = htonl(next_pc_addr >> 6);
        seq_qstate.desc1_next_pc_valid = true;
    }

    WRITE_MEM(qstate_addr, (uint8_t *)&seq_qstate, sizeof(seq_qstate));
    invalidate_txdma_cacheline(qstate_addr);

    cpl->qid = qid;
    cpl->qtype = STORAGE_SEQ_QTYPE_SQ;
    status = DEVCMD_SUCCESS;

devcmd_done:

    /*
     * Special support for Storage DOL
     */
    if (cmd->dol_req_devcmd_done) {
        _PostDevcmdDone(status);
    }
    return status;
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueControl(void *req, void *req_data,
                                 void *resp, void *resp_data,
                                 bool enable)
{
    seq_queue_control_cmd_t *cmd = (seq_queue_control_cmd_t *)req;
    uint64_t                qstate_addr;
    uint8_t                 value;

    if (cmd->qtype >= STORAGE_SEQ_QTYPE_MAX) {
        NIC_LOG_ERR("lif{}: bad qtype {}", info.hw_lif_id, cmd->qtype);
        return (DEVCMD_ERROR);
    }

    NIC_LOG_INFO(" lif{}: qtype {} qid {} enable {}", info.hw_lif_id,
        cmd->qtype, cmd->qid, enable);

    value = enable;
    switch (cmd->qtype) {
    case STORAGE_SEQ_QTYPE_SQ:
        if (cmd->qid >= spec->seq_created_count) {
            NIC_LOG_ERR("lif{}: qtype {} qid {} exceeds count {}", info.hw_lif_id,
                cmd->qtype, cmd->qid, spec->seq_created_count);
            return (DEVCMD_ERROR);
        }
        qstate_addr = GetQstateAddr(cmd->qtype, cmd->qid);
        WRITE_MEM(qstate_addr + offsetof(storage_seq_qstate_t, enable),
                  (uint8_t *)&value, sizeof(value));
        invalidate_txdma_cacheline(qstate_addr);
        break;
    case STORAGE_SEQ_QTYPE_ADMIN:
        if (cmd->qid >= spec->adminq_count) {
            NIC_LOG_ERR("lif{}: qtype {} qid {} exceeds count {}", info.hw_lif_id,
                cmd->qtype, cmd->qid, spec->adminq_count);
            return (DEVCMD_ERROR);
        }
        qstate_addr = GetQstateAddr(cmd->qtype, cmd->qid);
        WRITE_MEM(qstate_addr + 16, (uint8_t *)&value, sizeof(value));
        invalidate_txdma_cacheline(qstate_addr);
        break;
    default:
        return (DEVCMD_ERROR);
        break;
    }

    return (DEVCMD_SUCCESS);
}

/*
 * Populate accel_ring_tbl with info read from HW 
 */
int
Accel_PF::accel_ring_info_get_all(void)
{
    accel_rgroup_iter_c     iter;
    int                     ret_val;

    ret_val = accel_rgroup_add();
    if (ret_val == 0) {
        ret_val = accel_rgroup_rings_add();
    }
    if (ret_val == 0) {
        ret_val = accel_rgroup_rinfo_get();
    }

    if (ret_val == 0) {
        for (iter  = accel_pf_rgroup_map.begin();
             iter != accel_pf_rgroup_map.end();
             iter++) {

            const accel_rgroup_rinfo_rsp_t& info = iter->second.info;
            if (info.ring_handle < ACCEL_RING_ID_MAX) {
                accel_ring_t& spec_ring = spec->accel_ring_tbl[info.ring_handle];
                spec_ring.ring_base_pa = info.base_pa;
                spec_ring.ring_pndx_pa = info.pndx_pa;
                spec_ring.ring_shadow_pndx_pa = info.shadow_pndx_pa;
                spec_ring.ring_opaque_tag_pa = info.opaque_tag_pa;
                spec_ring.ring_opaque_tag_size = info.opaque_tag_size;
                spec_ring.ring_desc_size = info.desc_size;
                spec_ring.ring_pndx_size = info.pndx_size;
                spec_ring.ring_size = info.ring_size;

                NIC_LOG_INFO("ring {} ring_base_pa {:#x} ring_pndx_pa {:#x} "
                             "ring_shadow_pndx_pa {:#x} ring_opaque_tag_pa {:#x} "
                             "ring_size {}",
                             accel_ring_id2name_get(info.ring_handle),
                             spec_ring.ring_base_pa, spec_ring.ring_pndx_pa,
                             spec_ring.ring_shadow_pndx_pa,
                             spec_ring.ring_opaque_tag_pa, spec_ring.ring_size);
            }
        }
    }

    return ret_val;
}

/*
 * Reset all HW rings in accel_ring_tbl
 */
int
Accel_PF::accel_ring_reset_all(void)
{
    int     ret_val;

    /*
     * Reset requires rings to be disabled which helps with convergence
     * to the quiesce state. For those rings that do not support disablement
     * (such as XTS/GCM rings), we'll wait for them to go empty. The expectation
     * here is that the Accel driver has already stopped submitting I/O requests
     * prior to asking for rings to be reset.
     */
    accel_rgroup_enable_set(false);
    ret_val = accel_ring_wait_quiesce_all();
    if (ret_val == 0) {
        usleep(ACCEL_DEV_RING_OP_QUIESCE_TIME_US);
        ret_val = accel_rgroup_reset_set(true);
    }

    /*
     * Note that model does not have support for ring reset/disablement
     * i.e., cndx would not get cleared as we would expect.
     * Consequently, pndx should never be overwritten (particularly once 
     * the ring has been active). To avoid any issues, we only
     * write pndx conditionally.
     */
    if (ret_val == 0) {
        ret_val = accel_rgroup_pndx_set(0, true);
    }

    /*
     * Bring out of reset
     */
    accel_rgroup_reset_set(false);

    /*
     * Reenable rings
     */
    accel_rgroup_enable_set(true);
    return ret_val;
}

/*
 * Wait for all HW rings to quiesce
 */
int
Accel_PF::accel_ring_wait_quiesce_all(void)
{
    uint64_t    timeout_us;
    uint32_t    max_pendings;
    int         ret_val;

    /*
     * Return 0 if all rings have quiesced
     */
    auto quiesce_check = [this] () -> int
    {
        accel_rgroup_iter_c iter;
        int                 ret_val;

        ret_val = accel_rgroup_rindices_get();
        if (ret_val == 0) {

            for (iter  = accel_pf_rgroup_map.begin();
                 iter != accel_pf_rgroup_map.end();
                 iter++) {

                if (accel_ring_num_pendings_get(iter->second)) {
                    return -1;
                }
            }

            NIC_LOG_INFO("lif{}: all rings quiesced", info.hw_lif_id);
            return 0;
        }

        NIC_LOG_ERR("lif{}: unable to wait for rings to quiesce", info.hw_lif_id);
        return 0;
    };

    ret_val = accel_ring_max_pendings_get(max_pendings);
    if (ret_val == 0) {
        NIC_LOG_INFO("lif{}: max requests pending {}", info.hw_lif_id,
                     max_pendings);
        timeout_us = max_pendings ? 
                     (uint64_t)max_pendings * ACCEL_DEV_RING_OP_QUIESCE_TIME_US :
                     ACCEL_DEV_RING_OP_QUIESCE_TIME_US;
        Poller poll(std::min(timeout_us,
                             (uint64_t)ACCEL_DEV_ALL_RINGS_MAX_QUIESCE_TIME_US));
        poll(quiesce_check);
    }

    return ret_val;
}

/*
 * Create accelerator ring group
 */
int
Accel_PF::accel_rgroup_add(void)
{
    int     ret_val;

    ret_val = hal->AccelRGroupAdd(accel_pf_rgroup_name);
    if (ret_val) {
        NIC_LOG_ERR("lif{}:: failed to add ring group {}",
                    info.hw_lif_id, accel_pf_rgroup_name);
    }
    return ret_val;
}

/*
 * Add relevant rings to accelerator ring group
 */
int
Accel_PF::accel_rgroup_rings_add(void)
{
    int     ret_val;

    ret_val = hal->AccelRGroupRingAdd(accel_pf_rgroup_name,
                                      accel_pf_ring_vec);
    if (ret_val) {
        NIC_LOG_ERR("lif{}: failed to add ring vector",
                    info.hw_lif_id, accel_pf_rgroup_name);
    }
    return ret_val;
}

/*
 * Reset/Un-reset a HW ring
 */
int
Accel_PF::accel_rgroup_reset_set(bool reset_sense)
{
    int     ret_val;

    ret_val = hal->AccelRGroupResetSet(accel_pf_rgroup_name, ACCEL_SUB_RING_ALL,
                                       reset_sense);
    if (ret_val) {
        NIC_LOG_ERR("lif{}:: failed to reset ring group {} sense {}",
                    info.hw_lif_id, accel_pf_rgroup_name, reset_sense);
    }
    return ret_val;
}

/*
 * Enable/disable a HW ring
 */
int
Accel_PF::accel_rgroup_enable_set(bool enable_sense)
{
    int     ret_val;

    ret_val = hal->AccelRGroupEnableSet(accel_pf_rgroup_name, ACCEL_SUB_RING_ALL,
                                        enable_sense);
    if (ret_val) {
        NIC_LOG_ERR("lif{}:: failed to enable ring group {} sense {}",
                    info.hw_lif_id, accel_pf_rgroup_name, enable_sense);
    }
    return ret_val;
}

/*
 * Set producer index for a HW ring
 */
int
Accel_PF::accel_rgroup_pndx_set(uint32_t val,
                                bool conditional)
{
    int     ret_val;

    ret_val = hal->AccelRGroupPndxSet(accel_pf_rgroup_name, ACCEL_SUB_RING_ALL,
                                      val, conditional);
    if (ret_val) {
        NIC_LOG_ERR("lif{}:: failed to set pndx for ring group {} val {}",
                    info.hw_lif_id, accel_pf_rgroup_name, val);
    }
    return ret_val;
}

/*
 * Accelerator ring group ring info response callback handler
 */
static void
accel_rgroup_rinfo_rsp_cb(void *user_ctx,
                          const accel_rgroup_rinfo_rsp_t& info)
{
    Accel_PF                *accel_pf = (Accel_PF *)user_ctx;
    accel_rgroup_ring_key_t key;

    if (info.ring_handle >= ACCEL_RING_ID_MAX) {
        NIC_LOG_ERR("lif{}:: unrecognized ring_handle {}",
                    accel_pf->info.hw_lif_id, info.ring_handle);
    }

    key = accel_rgroup_ring_key_make(info.ring_handle, info.sub_ring);
    accel_rgroup_ring_t& rgroup_ring = accel_pf_rgroup_map[key];
    rgroup_ring.info = info;
}

/*
 * Get ring group info
 */
int
Accel_PF::accel_rgroup_rinfo_get(void)
{
    uint32_t    num_rings;
    int         ret_val;

    ret_val = hal->AccelRGroupInfoGet(accel_pf_rgroup_name, ACCEL_SUB_RING_ALL,
                                      accel_rgroup_rinfo_rsp_cb, this,
                                      num_rings);
    if (ret_val) {
        NIC_LOG_ERR("lif{}:: failed to get info for ring group {}",
                    info.hw_lif_id, accel_pf_rgroup_name);
        return ret_val;
    }
    if (num_rings < accel_pf_ring_vec.size()) {
        NIC_LOG_ERR("lif{}: {} too few num_rings {} expected at least {}", 
                    info.hw_lif_id, accel_pf_rgroup_name,
                    num_rings, accel_pf_ring_vec.size());
        ret_val = -1;
    }
    return ret_val;
}

/*
 * Accelerator ring group ring indices response callback handler
 */
static void
accel_rgroup_rindices_rsp_cb(void *user_ctx,
                             const accel_rgroup_rindices_rsp_t& indices)
{
    Accel_PF                *accel_pf = (Accel_PF *)user_ctx;
    accel_rgroup_ring_key_t key;
    accel_rgroup_iter_t     iter;

    if (indices.ring_handle >= ACCEL_RING_ID_MAX) {
        NIC_LOG_ERR("lif{}:: unrecognized ring_handle {}",
                    accel_pf->info.hw_lif_id, indices.ring_handle);
    }

    key = accel_rgroup_ring_key_make(indices.ring_handle, indices.sub_ring);
    iter = accel_pf_rgroup_map.find(key);
    if (iter == accel_pf_rgroup_map.end()) {
        NIC_LOG_ERR("lif{}:: ring_handle {} sub_ring {} not found",
                    accel_pf->info.hw_lif_id, indices.ring_handle,
                    indices.sub_ring);
    } else {
        iter->second.indices = indices;
    }
}

/*
 * Get ring group indices
 */
int
Accel_PF::accel_rgroup_rindices_get(void)
{
    uint32_t    num_rings;
    int         ret_val;

    ret_val = hal->AccelRGroupIndicesGet(accel_pf_rgroup_name, ACCEL_SUB_RING_ALL,
                                         accel_rgroup_rindices_rsp_cb, this,
                                         num_rings);
    if (ret_val) {
        NIC_LOG_ERR("lif{}:: failed to get indices for ring group {}",
                    info.hw_lif_id, accel_pf_rgroup_name);
        return ret_val;
    }
    if (num_rings < accel_pf_ring_vec.size()) {
        NIC_LOG_ERR("lif{}: {} too few num_rings {} expected at least {}", 
                    info.hw_lif_id, accel_pf_rgroup_name,
                    num_rings, accel_pf_ring_vec.size());
        ret_val = -1;
    }
    return ret_val;
}

/*
 * Return number of requests outstanding on a HW ring.
 */
uint32_t
Accel_PF::accel_ring_num_pendings_get(const accel_rgroup_ring_t& rgroup_ring)
{
    uint32_t        pndx = 0;
    uint32_t        cndx = 0;
    uint32_t        num_pendings = 0;

    if (rgroup_ring.info.ring_size) {
        pndx = rgroup_ring.indices.pndx % rgroup_ring.info.ring_size;
        cndx = rgroup_ring.indices.cndx % rgroup_ring.info.ring_size;
        num_pendings = pndx < cndx ?
                       (pndx + rgroup_ring.info.ring_size) - cndx :
                       pndx - cndx;

        /*
         * If ring had disablement capability it would have been disabled
         * so we can consider the ring as empty.
         */ 
        if (rgroup_ring.info.sw_enable_capable) {
            num_pendings = 0;
        }
    }

    if (num_pendings == 0) {
        NIC_LOG_INFO("ring {} pndx {} cndx {}",
                     accel_ring_id2name_get(rgroup_ring.info.ring_handle),
                     pndx, cndx);
    }
    return num_pendings;
}

/*
 * Calculate the largest number of requests outstanding
 * of all the rings.
 */
int
Accel_PF::accel_ring_max_pendings_get(uint32_t& max_pendings)
{
    accel_rgroup_iter_c     iter;
    uint32_t                num_pendings;
    int                     ret_val;

    max_pendings = 0;
    ret_val = accel_rgroup_rindices_get();
    if (ret_val == 0) {

        for (iter  = accel_pf_rgroup_map.begin();
             iter != accel_pf_rgroup_map.end();
             iter++) {

            num_pendings = accel_ring_num_pendings_get(iter->second);
            max_pendings = std::max(max_pendings, num_pendings);
        }
    }

    return ret_val;
}

/*
 * rounded up log2
 */
static uint32_t
log_2(uint32_t x)
{
  uint32_t log = 0;

  while ((uint64_t)(1 << log) < (uint64_t)x) {
      log++;
  }
  return log;
}

static uint64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

/*
 * Poll with timeout
 */
int
Poller::operator()(std::function<int(void)> poll_func)
{
    uint64_t    tm_start;
    uint64_t    tm_stop;
    int         ret;

    tm_start = tm_stop = timestamp();
    while ((tm_stop - tm_start) < timeout_us) {
        ret = poll_func();
        if (ret == 0) {
            return ret;
        }
        usleep(5000);
        tm_stop = timestamp();
    }

    NIC_LOG_INFO("Polling timeout_us {} exceeded - Giving up!", timeout_us);
    return -1;
}

ostream &operator<<(ostream& os, const Accel_PF& obj) {

    os << "LIF INFO:" << endl;
    os << "\tlif_id = " << obj.spec->lif_id << endl;
    os << "\thw_lif_id = " << obj.info.hw_lif_id << endl;
    os << "\tqstate_addr: " << endl;
    for (int i = 0; i < STORAGE_SEQ_QTYPE_MAX; i++) {
        os << "\t\tqtype = " << i
           << ", qstate = 0x" << hex << obj.info.qstate_addr[i] << resetiosflags(ios::hex)
           << endl;
    }

    return os;
}
