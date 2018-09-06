
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

// Tell accel_dev.hpp to emumerate definitions of all devcmds
#define ACCEL_DEV_CMD_ENUMERATE  1

#include "base.hpp"
#include "logger.hpp"
#include "intrutils.h"
#include "accel_dev.hpp"
#include "hal_client.hpp"
#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_hens_csr_define.h"

// Tell capri_barco_rings.hpp to define config values only
#define BARCO_CRYPTO_RINGS_CFG_VAL_ONLY  1
#include "capri_barco_rings.hpp"


static uint32_t log_2(uint32_t x);

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

/**
 * Accelerator configuration space registers
 */
#define ACCEL_CSR_OFFS_VOID     0xffffffff

typedef struct accel_csr {
    const char  *ring_name;
    uint32_t    base_addr_offs;
    uint32_t    pndx_addr_offs;
    uint32_t    opa_tag_addr_offs;
    uint32_t    reset_addr_offs;
    uint32_t    reset_data;
    uint32_t    engine_en_addr_offs;
    uint32_t    engine_en_data;
    uint32_t    ring_en_addr_offs;
    uint32_t    ring_en_data;
    uint32_t    ring_size_offs;
    uint32_t    ring_size_shft;
    uint32_t    ring_size_mask;
    uint32_t    ring_desc_size;
    uint32_t    ring_pndx_size;
} accel_csr_t;

#define ACCEL_RING_ID_NAME(id)  #id

static const accel_csr_t    accel_csr_tbl[ACCEL_RING_ID_MAX] = {

    [ACCEL_RING_CP] = {
        .ring_name = ACCEL_RING_ID_NAME(ACCEL_RING_CP),
        .base_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q_BASE_ADR_W0_BYTE_ADDRESS,
        .pndx_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_Q_PD_IDX_BYTE_ADDRESS,
        .opa_tag_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOST_OPAQUE_TAG_ADR_W0_BYTE_ADDRESS,
        .reset_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_GLB_BYTE_ADDRESS,
        .reset_data = BARCO_CRYPTO_CP_CFG_GLB_SOFT_RESET,
        .engine_en_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_UENG_W0_BYTE_ADDRESS,
        .engine_en_data = BARCO_CRYPTO_CP_UENG_LO_EN_ALL,
        .ring_en_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_BYTE_ADDRESS,
        .ring_en_data = BARCO_CRYPTO_CP_DIST_DESC_Q_EN,
        .ring_size_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_BYTE_ADDRESS,
        .ring_size_shft = BARCO_CRYPTO_CP_DIST_DESC_Q_SIZE_SHFT,
        .ring_size_mask = BARCO_CRYPTO_CP_DIST_DESC_Q_SIZE_MASK,
        .ring_desc_size = 64,
        .ring_pndx_size = sizeof(uint32_t),
    },
    [ACCEL_RING_CP_HOT] = {
        .ring_name = ACCEL_RING_ID_NAME(ACCEL_RING_CP_HOT),
        .base_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOTQ_BASE_ADR_W0_BYTE_ADDRESS,
        .pndx_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_HOTQ_PD_IDX_BYTE_ADDRESS,
        .opa_tag_addr_offs = ACCEL_CSR_OFFS_VOID,
        .reset_addr_offs = ACCEL_CSR_OFFS_VOID,
        .reset_data = 0,
        .engine_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .engine_en_data = 0,
        .ring_en_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_BYTE_ADDRESS,
        .ring_en_data = BARCO_CRYPTO_CP_DIST_DESC_HOTQ_EN,
        .ring_size_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_CP_CFG_DIST_BYTE_ADDRESS,
        .ring_size_shft = BARCO_CRYPTO_CP_DIST_DESC_HOTQ_SIZE_SHFT,
        .ring_size_mask = BARCO_CRYPTO_CP_DIST_DESC_HOTQ_SIZE_MASK,
        .ring_desc_size = 64,
        .ring_pndx_size = sizeof(uint32_t),
    },
    [ACCEL_RING_DC] = {
        .ring_name = ACCEL_RING_ID_NAME(ACCEL_RING_DC),
        .base_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q_BASE_ADR_W0_BYTE_ADDRESS,
        .pndx_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_Q_PD_IDX_BYTE_ADDRESS,
        .opa_tag_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_HOST_OPAQUE_TAG_ADR_W0_BYTE_ADDRESS,
        .reset_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_GLB_BYTE_ADDRESS,
        .reset_data = BARCO_CRYPTO_DC_CFG_GLB_SOFT_RESET,
        .engine_en_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_UENG_W0_BYTE_ADDRESS,
        .engine_en_data = BARCO_CRYPTO_DC_UENG_LO_EN_ALL,
        .ring_en_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_BYTE_ADDRESS,
        .ring_en_data = BARCO_CRYPTO_DC_DIST_DESC_Q_EN,
        .ring_size_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_BYTE_ADDRESS,
        .ring_size_shft = BARCO_CRYPTO_DC_DIST_DESC_Q_SIZE_SHFT,
        .ring_size_mask = BARCO_CRYPTO_DC_DIST_DESC_Q_SIZE_MASK,
        .ring_desc_size = 64,
        .ring_pndx_size = sizeof(uint32_t),
    },
    [ACCEL_RING_DC_HOT] = {
        .ring_name = ACCEL_RING_ID_NAME(ACCEL_RING_DC_HOT),
        .base_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_HOTQ_BASE_ADR_W0_BYTE_ADDRESS,
        .pndx_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_HOTQ_PD_IDX_BYTE_ADDRESS,
        .opa_tag_addr_offs = ACCEL_CSR_OFFS_VOID,
        .reset_addr_offs = ACCEL_CSR_OFFS_VOID,
        .reset_data = 0,
        .engine_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .engine_en_data = 0,
        .ring_en_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_BYTE_ADDRESS,
        .ring_en_data = BARCO_CRYPTO_DC_DIST_DESC_HOTQ_EN,
        .ring_size_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_DC_CFG_DIST_BYTE_ADDRESS,
        .ring_size_shft = BARCO_CRYPTO_DC_DIST_DESC_HOTQ_SIZE_SHFT,
        .ring_size_mask = BARCO_CRYPTO_DC_DIST_DESC_HOTQ_SIZE_MASK,
        .ring_desc_size = 64,
        .ring_pndx_size = sizeof(uint32_t),
    },
    [ACCEL_RING_XTS0] = {
        .ring_name = ACCEL_RING_ID_NAME(ACCEL_RING_XTS0),
        .base_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_RING_BASE_W0_BYTE_ADDRESS,
        .pndx_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_PRODUCER_IDX_BYTE_ADDRESS,
        .opa_tag_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_OPA_TAG_ADDR_W0_BYTE_ADDRESS,
        .reset_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_SOFT_RST_BYTE_ADDRESS,
        .reset_data = 0xffffffff,
        .engine_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .engine_en_data = 0,
        .ring_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .ring_en_data = 0,
        .ring_size_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_ENC_RING_SIZE_BYTE_ADDRESS,
        .ring_size_shft = 0,
        .ring_size_mask = 0,
        .ring_desc_size = 128,
        .ring_pndx_size = sizeof(uint32_t),
    },
    [ACCEL_RING_XTS1] = {
        .ring_name = ACCEL_RING_ID_NAME(ACCEL_RING_XTS1),
        .base_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_RING_BASE_W0_BYTE_ADDRESS,
        .pndx_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_PRODUCER_IDX_BYTE_ADDRESS,
        .opa_tag_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_OPA_TAG_ADDR_W0_BYTE_ADDRESS,
        .reset_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_SOFT_RST_BYTE_ADDRESS,
        .reset_data = 0xffffffff,
        .engine_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .engine_en_data = 0,
        .ring_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .ring_en_data = 0,
        .ring_size_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_XTS_RING_SIZE_BYTE_ADDRESS,
        .ring_size_shft = 0,
        .ring_size_mask = 0,
        .ring_desc_size = 128,
        .ring_pndx_size = sizeof(uint32_t),
    },
    [ACCEL_RING_GCM0] = {
        .ring_name = ACCEL_RING_ID_NAME(ACCEL_RING_GCM0),
        .base_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_RING_BASE_W0_BYTE_ADDRESS,
        .pndx_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_PRODUCER_IDX_BYTE_ADDRESS,
        .opa_tag_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_OPA_TAG_ADDR_W0_BYTE_ADDRESS,
        .reset_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_SOFT_RST_BYTE_ADDRESS,
        .reset_data = 0xffffffff,
        .engine_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .engine_en_data = 0,
        .ring_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .ring_en_data = 0,
        .ring_size_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM0_RING_SIZE_BYTE_ADDRESS,
        .ring_size_shft = 0,
        .ring_size_mask = 0,
        .ring_desc_size = 128,
        .ring_pndx_size = sizeof(uint32_t),
    },
    [ACCEL_RING_GCM1] = {
        .ring_name = ACCEL_RING_ID_NAME(ACCEL_RING_GCM1),
        .base_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_RING_BASE_W0_BYTE_ADDRESS,
        .pndx_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_PRODUCER_IDX_BYTE_ADDRESS,
        .opa_tag_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_OPA_TAG_ADDR_W0_BYTE_ADDRESS,
        .reset_addr_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_SOFT_RST_BYTE_ADDRESS,
        .reset_data = 0xffffffff,
        .engine_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .engine_en_data = 0,
        .ring_en_addr_offs = ACCEL_CSR_OFFS_VOID,
        .ring_en_data = 0,
        .ring_size_offs = CAP_HENS_CSR_DHS_CRYPTO_CTL_GCM1_RING_SIZE_BYTE_ADDRESS,
        .ring_size_shft = 0,
        .ring_size_mask = 0,
        .ring_desc_size = 128,
        .ring_pndx_size = sizeof(uint32_t),
    },
};

Accel_PF::Accel_PF(HalClient *hal_client, void *dev_spec,
                   const struct lif_info *nicmgr_lif_info) :
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

    // Find devcmd/devcmddb/rings shadow pndx, etc., area
    if (hal->AllocHbmAddress(ACCEL_DEVCMD_HBM_HANDLE, &hbm_addr, &hbm_size)) {
        NIC_LOG_ERR("Failed to get HBM base for {}", ACCEL_DEVCMD_HBM_HANDLE);
        return;
    }

    // devcmd page needs to be page aligned, then all subsequent pages
    // would fall into the same alignment.
    hbm_size *= 1024;
    assert(hbm_size >= ((ACCEL_DEV_BAR0_NUM_PAGES_MAX +
                         ACCEL_DEV_SHADOW_PINDEX_NUM_PAGES_MAX + 1) *
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

    // rings shadow pindex page
    shadow_pndx_page_addr = pci_resources.devcmdpa +
                            (ACCEL_DEV_BAR0_NUM_PAGES_MAX * ACCEL_DEV_PAGE_SIZE);
    WRITE_MEM(shadow_pndx_page_addr, (uint8_t *)blank_page, ACCEL_DEV_PAGE_SIZE);

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

    lif_handle = hal->LifCreate(spec->lif_id, qinfo, &info, false, 0, 0, 0);
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
    uint64_t    qstate_addr;
    uint8_t     enable = 0;
    uint8_t     abort = 1;

    NIC_LOG_INFO("lif{}:", info.hw_lif_id);

    // Disable all sequencer queues
    for (uint32_t qid = 0; qid < spec->seq_created_count; qid++) {
        qstate_addr = GetQstateAddr(STORAGE_SEQ_QTYPE_SQ, qid);
        WRITE_MEM(qstate_addr + offsetof(storage_seq_qstate_t, abort),
                  (uint8_t *)&abort, sizeof(abort));
        WRITE_MEM(qstate_addr + offsetof(storage_seq_qstate_t, enable),
                  (uint8_t *)&enable, sizeof(enable));
        invalidate_txdma_cacheline(qstate_addr);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        qstate_addr = GetQstateAddr(STORAGE_SEQ_QTYPE_ADMIN, qid);
        WRITE_MEM(qstate_addr + offsetof(eth_admin_qstate_t, p_index0),
                  (uint8_t *)blank_qstate + offsetof(eth_admin_qstate_t, p_index0),
                  sizeof(blank_qstate) - offsetof(eth_admin_qstate_t, p_index0));
        invalidate_txdma_cacheline(qstate_addr);
    }

    // Reset and reenable accelerator rings
    accel_ring_reset_all();
    accel_engine_enable_all();
    accel_ring_enable_all();
    
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
void
Accel_PF::accel_ring_info_get_all(void)
{
    accel_ring_t        *accel_ring;
    const accel_csr_t   *csr;
    uint64_t            shadow_addr;
    uint32_t            reset;
    accel_ring_id_t     id;

    /*
     * Temporary workaround for not having cap_top_init take the MD block
     * out of reset for us.
     */
    reset = accel_csr_get32(CAP_HENS_CSR_CFG_HE_CTL_BYTE_ADDRESS);
    if (reset & 0xff) {
        accel_csr_set32(CAP_HENS_CSR_CFG_HE_CTL_BYTE_ADDRESS, reset & ~0xff);
    }

    assert(shadow_pndx_page_addr);
    shadow_addr = shadow_pndx_page_addr;
    for (id = ACCEL_RING_ID_FIRST, accel_ring = &spec->accel_ring_tbl[0], csr = &accel_csr_tbl[0];
         id < ACCEL_RING_ID_MAX;
         id++, accel_ring++, csr++) {

        accel_ring->ring_id = id;
        accel_ring->ring_base_pa = accel_csr_get64(csr->base_addr_offs);
        accel_ring->ring_pndx_pa = CAP_ADDR_BASE_MD_HENS_OFFSET + csr->pndx_addr_offs;
        accel_ring->ring_shadow_pndx_pa = shadow_addr;
        shadow_addr += csr->ring_pndx_size;

        if (csr->opa_tag_addr_offs != ACCEL_CSR_OFFS_VOID) {
            accel_ring->ring_opaque_tag_pa = accel_csr_get64(csr->opa_tag_addr_offs);
        }
        accel_ring->ring_desc_size = csr->ring_desc_size;
        accel_ring->ring_pndx_size = csr->ring_pndx_size;
        accel_ring->ring_size = accel_csr_get32(csr->ring_size_offs);

        /*
         * Some accelerator rings store size in shared register which requires
         * shift and mask.
         */
        if (csr->ring_size_mask) {
            accel_ring->ring_size = (accel_ring->ring_size >> csr->ring_size_shft) &
                                    csr->ring_size_mask;
            if (accel_ring->ring_size == 0) {
                accel_ring->ring_size = csr->ring_size_mask + 1;
            }
        }
        NIC_LOG_INFO("ring {} ring_base_pa {:#x} ring_pndx_pa {:#x} "
               "ring_shadow_pndx_pa {:#x} ring_opaque_tag_pa {:#x} "
               "ring_size {}",
               csr->ring_name, accel_ring->ring_base_pa,
               accel_ring->ring_pndx_pa, accel_ring->ring_shadow_pndx_pa,
               accel_ring->ring_opaque_tag_pa, accel_ring->ring_size);
    }

    shadow_pndx_bytes_used = shadow_addr - shadow_pndx_page_addr;
}

/*
 * Reset all HW rings in accel_ring_tbl
 */
void
Accel_PF::accel_ring_reset_all(void)
{
    accel_ring_t        *accel_ring;
    const accel_csr_t   *csr;
    accel_ring_id_t     id;
    uint32_t            csr_val;

    for (id = ACCEL_RING_ID_FIRST, accel_ring = &spec->accel_ring_tbl[0], csr = &accel_csr_tbl[0];
         id < ACCEL_RING_ID_MAX;
         id++, accel_ring++, csr++) {

        if (csr->reset_addr_offs != ACCEL_CSR_OFFS_VOID) {
            NIC_LOG_INFO("ring {}", csr->ring_name);

            csr_val = accel_csr_get32(csr->reset_addr_offs);
            accel_csr_set32(csr->reset_addr_offs,
                            csr_val | csr->reset_data);
            /*
             * Bring out of reset
             */
            accel_csr_set32(csr->reset_addr_offs,
                            csr_val & ~csr->reset_data);
            /*
             * Reset would have cleared cndx but would not have touched any
             * other registers. Here we clear pndx to match.
             */
            accel_csr_set32(csr->pndx_addr_offs, 0);
        }
    }

    NIC_LOG_INFO("clearing {} shadow_pndx_page_addr bytes", shadow_pndx_bytes_used);
    WRITE_MEM(shadow_pndx_page_addr, (uint8_t *)blank_page, shadow_pndx_bytes_used);
}

/*
 * Enable all HW ring engines in accel_ring_tbl
 */
void
Accel_PF::accel_engine_enable_all(void)
{
    accel_ring_t        *accel_ring;
    const accel_csr_t   *csr;
    accel_ring_id_t     id;
    uint32_t            csr_val;

    for (id = ACCEL_RING_ID_FIRST, accel_ring = &spec->accel_ring_tbl[0], csr = &accel_csr_tbl[0];
         id < ACCEL_RING_ID_MAX;
         id++, accel_ring++, csr++) {

        if (csr->engine_en_addr_offs != ACCEL_CSR_OFFS_VOID) {
            NIC_LOG_INFO("ring {}", csr->ring_name);

            csr_val = accel_csr_get32(csr->engine_en_addr_offs);
            accel_csr_set32(csr->engine_en_addr_offs,
                            csr_val | csr->engine_en_data);
        }
    }
}

/*
 * Enable all HW queues in accel_ring_tbl
 */
void
Accel_PF::accel_ring_enable_all(void)
{
    accel_ring_t        *accel_ring;
    const accel_csr_t   *csr;
    accel_ring_id_t     id;
    uint32_t            csr_val;

    for (id = ACCEL_RING_ID_FIRST, accel_ring = &spec->accel_ring_tbl[0], csr = &accel_csr_tbl[0];
         id < ACCEL_RING_ID_MAX;
         id++, accel_ring++, csr++) {

        if (csr->ring_en_addr_offs != ACCEL_CSR_OFFS_VOID) {
            NIC_LOG_INFO("ring {}", csr->ring_name);

            csr_val = accel_csr_get32(csr->ring_en_addr_offs);
            accel_csr_set32(csr->ring_en_addr_offs,
                            csr_val | csr->ring_en_data);
        }
    }
}

/*
 * Read 64-bit data from Accelerator CSR 
 */
uint64_t
accel_csr_get64(uint64_t csr_offs)
{
    uint64_t    csr_addr = CAP_ADDR_BASE_MD_HENS_OFFSET + csr_offs;
    uint32_t    val32[2];

    READ_REG(csr_addr, val32, 2);
    return ((uint64_t)val32[1] << 32) | val32[0];
}

/*
 * Write 64-bit data to Accelerator CSR 
 */
void
accel_csr_set64(uint64_t csr_offs,
                uint64_t csr_val)
{
    uint64_t    csr_addr = CAP_ADDR_BASE_MD_HENS_OFFSET + csr_offs;
    uint32_t    *val32 = (uint32_t *)&csr_val;

    WRITE_REG(csr_addr, val32, 2);
}

/*
 * Read 32-bit data from Accelerator CSR 
 */
uint32_t
accel_csr_get32(uint64_t csr_offs)
{
    uint64_t    csr_addr = CAP_ADDR_BASE_MD_HENS_OFFSET + csr_offs;
    uint32_t    val32;

    READ_REG(csr_addr, &val32, 1);
    return val32;
}

/*
 * Write 64-bit data to Accelerator CSR 
 */
void
accel_csr_set32(uint64_t csr_offs,
                uint32_t csr_val)
{
    WRITE_REG32(CAP_ADDR_BASE_MD_HENS_OFFSET + csr_offs, csr_val);
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
