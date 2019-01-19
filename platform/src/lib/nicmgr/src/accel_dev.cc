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
#include <sys/time.h>

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"

// Tell accel_dev.hpp to emumerate definitions of all devcmds
#define ACCEL_DEV_CMD_ENUMERATE  1

#include "nic/include/base.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"

#include "gen/proto/nicmgr/accel_metrics.pb.h"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"

#include "platform/src/lib/intrutils/include/intrutils.h"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

#include "logger.hpp"
#include "accel_dev.hpp"
#include "pd_client.hpp"
#include "hal_client.hpp"


using namespace nicmgr;

extern class pciemgr *pciemgr;


// Amount of time to wait for sequencer queues to be quiesced
#define ACCEL_DEV_SEQ_QUEUES_QUIESCE_TIME_US    5000000
#define ACCEL_DEV_RING_OP_QUIESCE_TIME_US       1000000
#define ACCEL_DEV_ALL_RINGS_MAX_QUIESCE_TIME_US (10 * ACCEL_DEV_RING_OP_QUIESCE_TIME_US)
#define ACCEL_DEV_NUM_CRYPTO_KEYS_MIN           512     // arbitrary low water mark
#define ACCEL_QID_PUBLISH_YIELD_POINT           128

static void
accel_ring_info_publish(const accel_rgroup_ring_t& rgroup_ring);

static accel_rgroup_ring_t&
accel_pf_rgroup_find_create(uint32_t ring_handle,
                            uint32_t sub_ring);
void *
seq_queue_info_publish_thread(void *ctx);

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

static std::vector<AccelWorkThread *> accel_threads_vec;
static AccelWorkThread *delphi_qinfo_thread;
static uint32_t accel_thread_id;

static types::CryptoKeyType crypto_key_type_tbl[] = {
    [CMD_CRYPTO_KEY_TYPE_AES128] = types::CRYPTO_KEY_TYPE_AES128,
    [CMD_CRYPTO_KEY_TYPE_AES256] = types::CRYPTO_KEY_TYPE_AES256,
};

/*
 * Short lived crypto key accumulator
 */
typedef struct {
    uint8_t     key_data[CMD_CRYPTO_KEY_PART_MAX][CMD_CRYPTO_KEY_PART_SIZE];
    uint32_t    num_key_parts;
} crypto_key_accum_t;

typedef std::unordered_map<uint32_t,crypto_key_accum_t> accel_crypto_key_map_t;
typedef accel_crypto_key_map_t::iterator                accel_crypto_key_iter_t;

static accel_crypto_key_map_t   accel_crypto_key_map;

static crypto_key_accum_t *crypto_key_accum_find_add(uint32_t key_index);
static void crypto_key_accum_del(uint32_t key_index);

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

/*
 * Common accel ring group API return error checking
 */
#define ACCEL_RGROUP_GET_CB_HANDLE_CHECK(_handle)                               \
    do {                                                                        \
        if (_handle >= ACCEL_RING_ID_MAX) {                                     \
            NIC_LOG_ERR("lif-{}: unrecognized ring_handle {}",                   \
                        accel_pf->GetHalLifInfo()->hw_lif_id, _handle);         \
        }                                                                       \
    } while (false)

#define ACCEL_RGROUP_GET_RET_CHECK(_ret_val, _num_rings, _name)                 \
    do {                                                                        \
        if (_ret_val != HAL_RET_OK) {                                           \
            NIC_LOG_ERR("lif-{}: failed to getv" _name " for ring group {}",     \
                        hal_lif_info_.hw_lif_id, accel_pf_rgroup_name);         \
            return _ret_val;                                                    \
        }                                                                       \
        if (_num_rings < accel_pf_ring_vec.size()) {                            \
            NIC_LOG_ERR("lif-{}: {} too few num_rings {} expected at least {}",  \
                        hal_lif_info_.hw_lif_id, accel_pf_rgroup_name,          \
                        _num_rings, accel_pf_ring_vec.size());                  \
            return HAL_RET_ERR;                                                 \
        }                                                                       \
    } while (false)


Accel_PF::Accel_PF(HalClient *hal_client, void *dev_spec,
                   const hal_lif_info_t *nicmgr_lif_info,
                   PdClient *pd_client) :
    nicmgr_lif_info(nicmgr_lif_info),
    seq_qid_init_high(0)
{
    uint32_t    num_keys_max;

    hal = hal_client;
    spec = (accel_devspec_t *)dev_spec;
    pd = pd_client;

    // Allocate lifs
    lif_base = pd->lm_->LIFRangeAlloc(-1, spec->lif_count);
    if (lif_base < 0) {
        NIC_LOG_ERR("{}: Failed to allocate lifs", spec->name);
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
    devcmd_mem_addr = pd->devcmd_mem_alloc(ACCEL_DEV_PAGE_SIZE);
    MEM_SET(devcmd_mem_addr, 0, ACCEL_DEV_PAGE_SIZE, 0);
    // TODO: mmap instead of calloc after porting to real pal
    devcmd = (dev_cmd_regs_t *)calloc(1, sizeof(dev_cmd_regs_t));
    if (devcmd == NULL) {
        NIC_LOG_ERR("{}: Failed to map devcmd region", spec->name);
        throw;
    }
    devcmd->signature = DEV_CMD_SIGNATURE;
    WRITE_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(*devcmd), 0);

    devcmddb_mem_addr = pd->devcmd_mem_alloc(ACCEL_DEV_PAGE_SIZE);
    MEM_SET(devcmddb_mem_addr, 0, ACCEL_DEV_PAGE_SIZE, 0);

    NIC_LOG_DEBUG("{}: devcmd_addr {:#x} devcmddb_addr {:#x}",
        spec->name, devcmd_mem_addr, devcmddb_mem_addr);

    // Locate HBM region dedicated to crypto keys
    cmb_mem_addr = pd->mp_->start_addr(CAPRI_BARCO_KEY_DESC);
    cmb_mem_size = pd->mp_->size(CAPRI_BARCO_KEY_DESC);
    if (cmb_mem_addr == INVALID_MEM_ADDRESS || cmb_mem_size == 0) {
        NIC_LOG_ERR("{}: Failed to get HBM base for {}", spec->name, CAPRI_BARCO_KEY_DESC);
        throw;
    }

    // hal/pd/capri/capri_hbm.cc stores size in KB;
    // split the key region and use the second half for accel device.
    num_keys_max = std::min(cmb_mem_size / (CMD_CRYPTO_KEY_PART_SIZE *
                                            CMD_CRYPTO_KEY_PART_MAX),
                            (uint32_t)CRYPTO_KEY_COUNT_MAX);
    num_crypto_keys_max = num_keys_max / 2;
    crypto_key_idx_base = num_crypto_keys_max;
    NIC_LOG_DEBUG("{}: Key region size {} bytes crypto_key_idx_base {} "
                 "num_crypto_keys_max {}", spec->name, cmb_mem_size,
                 crypto_key_idx_base, num_crypto_keys_max);
    if (num_crypto_keys_max < ACCEL_DEV_NUM_CRYPTO_KEYS_MIN) {
        NIC_LOG_ERR("{}: num_crypto_keys_max {} too small, expected at least {}",
                    spec->name, num_crypto_keys_max, ACCEL_DEV_NUM_CRYPTO_KEYS_MIN);
        throw;
    }

    // Locate HBM region dedicated to STORAGE_SEQ_HBM_HANDLE
    cmb_mem_addr = pd->mp_->start_addr(STORAGE_SEQ_HBM_HANDLE);
    cmb_mem_size = pd->mp_->size(STORAGE_SEQ_HBM_HANDLE);
    if (cmb_mem_addr == INVALID_MEM_ADDRESS || cmb_mem_size == 0) {
        NIC_LOG_ERR("{}: Failed to get HBM base for {}", spec->name,
            STORAGE_SEQ_HBM_HANDLE);
        throw;
    }

    // First, ensure size is a power of 2, then per PCIe BAR mapping
    // requirement, align the region on its natural boundary, i.e.,
    // if size is 64MB then the region must be aligned on a 64MB boundary!
    // This means we could potentially waste half of the space if
    // the region was not already aligned.
    assert(cmb_mem_size && !(cmb_mem_size & (cmb_mem_size - 1)));
    if (cmb_mem_addr & (cmb_mem_size - 1)) {
        cmb_mem_size /= 2;
        cmb_mem_addr = ACCEL_DEV_ADDR_ALIGN(cmb_mem_addr, cmb_mem_size);
    }

    NIC_LOG_DEBUG("{}: cmb_mem_addr: {:#x} cmb_mem_size: {}",
        spec->name, cmb_mem_addr, cmb_mem_size);

    // Create the device
    if (spec->pcie_port == 0xff) {
        NIC_LOG_DEBUG("{}: Skipped creating PCI device", spec->name);
    } else {
        if (!CreateHostDevice()) {
            NIC_LOG_ERR("{}: Failed to create device", spec->name);
            throw;
        }
    }

    // Create LIF
    memset(qinfo, 0, sizeof(qinfo));
    seq_created_count = 1 << log_2(spec->seq_queue_count);

    qinfo[STORAGE_SEQ_QTYPE_SQ] = {
        .type_num = STORAGE_SEQ_QTYPE_SQ,
        .size = HW_CB_MULTIPLE(STORAGE_SEQ_CB_SIZE_SHFT),
        .entries = log_2(spec->seq_queue_count),   // modified at runtime
    };

    qinfo[STORAGE_SEQ_QTYPE_UNUSED] = {
        .type_num = STORAGE_SEQ_QTYPE_UNUSED,
        .size = HW_CB_MIN_MULTIPLE,
        .entries = 0,
    };

    qinfo[STORAGE_SEQ_QTYPE_ADMIN] = {
        .type_num = STORAGE_SEQ_QTYPE_ADMIN,
        .size = HW_CB_MULTIPLE(ADMIN_QSTATE_SIZE_SHFT),
        .entries = 0,
    };

    memset(&hal_lif_info_, 0, sizeof(hal_lif_info_));
    hal_lif_info_.hw_lif_id = lif_base;
    hal_lif_info_.name = spec->name;
    hal_lif_info_.type = types::LIF_TYPE_NONE;
    hal_lif_info_.pinned_uplink_port_num = 0;
    hal_lif_info_.enable_rdma = false;
    hal_lif_info_.pushed_to_hal = false;
    memcpy(hal_lif_info_.queue_info, qinfo, sizeof(hal_lif_info_.queue_info));

    evutil_timer_start(&devcmd_timer, &Accel_PF::DevcmdPoll, this, 0.0, 0.01);
}

bool
Accel_PF::CreateHostDevice()
{
    pciehdevice_resources_t pci_resources = {0};

    pci_resources.lif_valid = 1;
    pci_resources.lif = lif_base;
    pci_resources.intrb = intr_base;
    pci_resources.intrc = spec->intr_count;
    pci_resources.port = spec->pcie_port;
    pci_resources.npids = 1;
    pci_resources.devcmdpa = devcmd_mem_addr;
    pci_resources.devcmddbpa = devcmddb_mem_addr;
    pci_resources.cmbpa = cmb_mem_addr;
    pci_resources.cmbsz = cmb_mem_size;

    // Create PCI device
    NIC_LOG_DEBUG("{}: Creating PCI device", spec->name);
    pdev = pciehdev_accel_new(spec->name.c_str(), &pci_resources);
    if (pdev == NULL) {
        NIC_LOG_ERR("{}: Failed to create PCI device", spec->name);
        return false;
    }
    pciehdev_set_priv(pdev, (void *)this);

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

int
Accel_PF::DelphiDeviceInit(void)
{
    accel_metrics::AccelSeqQueueKey     seq_qkey;
    uint64_t                            qmetrics_addr;
    uint32_t                            qid;
    int64_t                             addr;

    if (g_nicmgr_svc) {
        delphi_qinfo_thread = 
            new AccelWorkThread(spec->name + "_qinfo",
                                ACCEL_DEV_CONTROL_CORES_MASK,
                                this, seq_queue_info_publish_thread);
        accel_threads_vec.push_back(delphi_qinfo_thread);

        delphi_pf = make_shared<delphi::objects::AccelPfInfo>();
        delphi::objects::AccelPfInfoPtr shared_pf(delphi_pf);
        shared_pf->set_key(std::to_string(hal_lif_info_.hw_lif_id));
        shared_pf->set_hwlifid(hal_lif_info_.hw_lif_id);
        shared_pf->set_numseqqueues(seq_created_count);
        shared_pf->set_cryptokeyidxbase(crypto_key_idx_base);
        shared_pf->set_numcryptokeysmax(num_crypto_keys_max);
        shared_pf->set_intrbase(intr_base);
        shared_pf->set_intrcount(spec->intr_count);
        g_nicmgr_svc->sdk()->SetObject(shared_pf);

        seq_qkey.set_lifid(std::to_string(hal_lif_info_.hw_lif_id));
        for (qid = 0; qid < seq_created_count; qid++) {
            seq_qkey.set_qid(std::to_string(qid));

            addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id,
                                             STORAGE_SEQ_QTYPE_SQ, qid);
            if (addr < 0) {
                NIC_LOG_ERR("lif-{}: Failed to get qstate address for SEQ qid {}",
                    hal_lif_info_.hw_lif_id, qid);
                return -1;
            }

            qmetrics_addr = addr +
                            offsetof(storage_seq_qstate_t, metrics);
            auto qmetrics = delphi::objects::AccelSeqQueueMetrics::
                            NewAccelSeqQueueMetrics(seq_qkey, qmetrics_addr);
            delphi_qmetrics_vec.push_back(std::move(qmetrics));

            /*
             * Some of the fields below will be updated when driver
             * issues _DevcmdSeqQueueInit().
             */
            auto qinfo = make_shared<delphi::objects::AccelSeqQueueInfo>();
            qinfo->mutable_key()->set_lifid(std::to_string(hal_lif_info_.hw_lif_id));
            qinfo->mutable_key()->set_qid(std::to_string(qid));
            qinfo->set_qstateaddr(addr);
            qinfo->set_qgroup((::accel_metrics::AccelSeqQGroup)STORAGE_SEQ_QGROUP_CPDC);
            qinfo->set_coreid(0);
            delphi_qinfo_vec.push_back(std::move(qinfo));
        }

        delphi_qinfo_thread->work_notify();
    }

    return 0;
}

void
Accel_PF::DevcmdPoll(void *obj)
{
    Accel_PF        *dev = (Accel_PF *)obj;
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
Accel_PF::DevcmdHandler()
{
    enum DevcmdStatus   status = DEVCMD_SUCCESS;

    NIC_HEADER_TRACE("Devcmd");

    // read devcmd region
    READ_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(dev_cmd_regs_t), 0);

    if (devcmd->done) {
        NIC_LOG_ERR("{}: Devcmd done is set before processing command, opcode {}",
            spec->name,
            opcode_to_str((enum cmd_opcode)devcmd->cmd.cmd.opcode));
        status = DEVCMD_ERROR;
        goto devcmd_done;
    }

    if (devcmd->signature != DEV_CMD_SIGNATURE) {
        NIC_LOG_ERR("{}: Devcmd signature mismatch, opcode {}",
            spec->name,
            opcode_to_str((enum cmd_opcode)devcmd->cmd.cmd.opcode));
        status = DEVCMD_ERROR;
        goto devcmd_done;
    }

    status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->cpl, &devcmd->data);

    // write data
    if (status == DEVCMD_SUCCESS) {
        WRITE_MEM(devcmd_mem_addr + offsetof(dev_cmd_regs_t, data),
                  (uint8_t *)devcmd + offsetof(dev_cmd_regs_t, data),
                  sizeof(devcmd->data), 0);
    }

devcmd_done:
    devcmd->cpl.cpl.status = status;
    devcmd->done = 1;

    // write completion
    WRITE_MEM(devcmd_mem_addr + offsetof(dev_cmd_regs_t, cpl),
              (uint8_t *)devcmd + offsetof(dev_cmd_regs_t, cpl),
              sizeof(devcmd->cpl), 0);

    // write done
    WRITE_MEM(devcmd_mem_addr + offsetof(dev_cmd_regs_t, done),
              (uint8_t *)devcmd + offsetof(dev_cmd_regs_t, done),
              sizeof(devcmd->done), 0);

    NIC_HEADER_TRACE("Devcmd End");
}

#define CASE(opcode) case opcode: return #opcode

const char*
Accel_PF::opcode_to_str(enum cmd_opcode opcode)
{
    switch(opcode) {
        CASE(CMD_OPCODE_NOP);
        CASE(CMD_OPCODE_RESET);
        CASE(CMD_OPCODE_IDENTIFY);
        CASE(CMD_OPCODE_LIF_INIT);
        CASE(CMD_OPCODE_ADMINQ_INIT);
        CASE(CMD_OPCODE_SEQ_QUEUE_INIT);
        CASE(CMD_OPCODE_SEQ_QUEUE_INIT_COMPLETE);
        CASE(CMD_OPCODE_SEQ_QUEUE_ENABLE);
        CASE(CMD_OPCODE_SEQ_QUEUE_DISABLE);
        CASE(CMD_OPCODE_CRYPTO_KEY_UPDATE);
        CASE(CMD_OPCODE_HANG_NOTIFY);
        CASE(CMD_OPCODE_SEQ_QUEUE_DUMP);
        CASE(CMD_OPCODE_SEQ_QUEUE_BATCH_INIT);
        CASE(CMD_OPCODE_SEQ_QUEUE_BATCH_ENABLE);
        CASE(CMD_OPCODE_SEQ_QUEUE_BATCH_DISABLE);
        default: return "DEVCMD_UNKNOWN";
    }
}

enum DevcmdStatus
Accel_PF::AdminCmdHandler(uint64_t lif_id,
    void *req, void *req_data,
    void *resp, void *resp_data)
{
    return CmdHandler(req, req_data, resp, resp_data);
}

enum DevcmdStatus
Accel_PF::CmdHandler(void *req, void *req_data,
                     void *resp, void *resp_data)
{
    dev_cmd_t       *cmd = (dev_cmd_t *)req;
    dev_cmd_cpl_t   *cpl = (dev_cmd_cpl_t *)resp;
    enum DevcmdStatus status;

    NIC_LOG_DEBUG("{}: Handling cmd: {}", spec->name,
        opcode_to_str((enum cmd_opcode)cmd->cmd.opcode));

    switch (cmd->cmd.opcode) {

    case CMD_OPCODE_NOP:
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

    case CMD_OPCODE_SEQ_QUEUE_INIT_COMPLETE:
        status = this->_DevcmdSeqQueueInitComplete(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_SEQ_QUEUE_BATCH_INIT:
        status = this->_DevcmdSeqQueueBatchInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_CRYPTO_KEY_UPDATE:
        status = this->_DevcmdCryptoKeyUpdate(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_HANG_NOTIFY:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_SEQ_QUEUE_ENABLE:
        status = this->_DevcmdSeqQueueControl(req, req_data, resp, resp_data, true);
        break;

    case CMD_OPCODE_SEQ_QUEUE_DISABLE:
        status = this->_DevcmdSeqQueueControl(req, req_data, resp, resp_data, false);
        break;

    case CMD_OPCODE_SEQ_QUEUE_BATCH_ENABLE:
        status = this->_DevcmdSeqQueueBatchControl(req, req_data, resp, resp_data, true);
        break;

    case CMD_OPCODE_SEQ_QUEUE_BATCH_DISABLE:
        status = this->_DevcmdSeqQueueBatchControl(req, req_data, resp, resp_data, false);
        break;

    case CMD_OPCODE_SEQ_QUEUE_DUMP:
        status = DEVCMD_SUCCESS;
        break;

    default:
        NIC_LOG_ERR("{}: Unknown Opcode {}", spec->name, cmd->cmd.opcode);
        status = DEVCMD_UNKNOWN;
        break;
    }

    cpl->cpl.status = status;
    cpl->cpl.rsvd = 0xff;

    NIC_LOG_DEBUG("{}: Done cmd: {}, status: {}", spec->name,
        opcode_to_str((enum cmd_opcode)cmd->cmd.opcode), status);

    return (status);
}

enum DevcmdStatus
Accel_PF::_DevcmdIdentify(void *req, void *req_data,
                          void *resp, void *resp_data)
{
    identity_t      *rsp = (identity_t *)resp_data;
    identify_cpl_t  *cpl = (identify_cpl_t *)resp;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_IDENTIFY", hal_lif_info_.hw_lif_id);

    memset(&devcmd->data[0], 0, sizeof(devcmd->data));

    // TODO: Get these from hw
    rsp->dev.asic_type = 0x00;
    rsp->dev.asic_rev = 0xA0;
    sprintf((char *)&rsp->dev.serial_num, "haps");
    // TODO: Get this from sw
    sprintf((char *)&rsp->dev.fw_version, "v0.0.1");
    rsp->dev.num_lifs = 1;
    memset(&rsp->dev.lif_tbl[0], 0, sizeof(identify_lif_t));
    rsp->dev.lif_tbl[0].hw_lif_id = hal_lif_info_.hw_lif_id;
    rsp->dev.lif_tbl[0].hw_lif_local_dbaddr =
             ACCEL_LIF_LOCAL_DBADDR_SET(hal_lif_info_.hw_lif_id, STORAGE_SEQ_QTYPE_SQ);
    rsp->dev.lif_tbl[0].hw_host_prefix = ACCEL_PHYS_ADDR_HOST_SET(1) |
                                         ACCEL_PHYS_ADDR_LIF_SET(hal_lif_info_.hw_lif_id);
    rsp->dev.lif_tbl[0].hw_host_mask = ACCEL_PHYS_ADDR_HOST_SET(ACCEL_PHYS_ADDR_HOST_MASK) |
                                       ACCEL_PHYS_ADDR_LIF_SET(ACCEL_PHYS_ADDR_LIF_MASK);
    rsp->dev.lif_tbl[0].hw_key_idx_base = crypto_key_idx_base;
    rsp->dev.lif_tbl[0].num_crypto_keys_max = num_crypto_keys_max;
    rsp->dev.db_pages_per_lif = 1;
    rsp->dev.admin_queues_per_lif = spec->adminq_count;
    rsp->dev.seq_queues_per_lif = seq_created_count;
    rsp->dev.num_intrs = spec->intr_count;
    rsp->dev.intr_assert_stride = intr_assert_stride();
    rsp->dev.intr_assert_data = intr_assert_data();
    rsp->dev.intr_assert_addr = intr_assert_addr(intr_base);
    rsp->dev.cm_base_pa = cmb_mem_addr;
    memcpy(rsp->dev.accel_ring_tbl, accel_ring_tbl,
           sizeof(rsp->dev.accel_ring_tbl));
    cpl->ver = IDENTITY_VERSION_1;

    NIC_LOG_DEBUG("lif-{} local_dbaddr {:#x} host_prefix {:#x} host_mask {:#x} size {}",
                 hal_lif_info_.hw_lif_id, rsp->dev.lif_tbl[0].hw_lif_local_dbaddr,
                 rsp->dev.lif_tbl[0].hw_host_prefix, rsp->dev.lif_tbl[0].hw_host_mask,
                 (int)sizeof(rsp->dev));

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdReset(void *req, void *req_data,
                       void *resp, void *resp_data)
{
    int64_t                 qstate_addr;
    uint32_t                qid;
    uint8_t                 enable = 0;
    uint8_t                 abort = 1;

    NIC_LOG_INFO("lif-{}: CMD_OPCODE_RESET", hal_lif_info_.hw_lif_id);

    if (!hal_lif_info_.pushed_to_hal) {
        return (DEVCMD_SUCCESS);
    }

    if (delphi_qinfo_thread) {
        delphi_qinfo_thread->stop_work_set(true);
    }

    // Disable all sequencer queues
    for (qid = 0; qid < seq_created_count; qid++) {
        qstate_addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, STORAGE_SEQ_QTYPE_SQ, qid);
        if (qstate_addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for SEQ qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        WRITE_MEM(qstate_addr + offsetof(storage_seq_qstate_t, abort),
                  (uint8_t *)&abort, sizeof(abort), 0);
        WRITE_MEM(qstate_addr + offsetof(storage_seq_qstate_t, enable),
                  (uint8_t *)&enable, sizeof(enable), 0);
        invalidate_txdma_cacheline(qstate_addr);
    }

    // Disable all adminq
    for (qid = 0; qid < spec->adminq_count; qid++) {
        qstate_addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, STORAGE_SEQ_QTYPE_ADMIN, qid);
        if (qstate_addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for SEQ qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        MEM_SET(qstate_addr + offsetof(admin_qstate_t, p_index0), 0,
                sizeof(admin_qstate_t) - offsetof(admin_qstate_t, p_index0), 0);
        invalidate_txdma_cacheline(qstate_addr);
    }

    // Wait for P4+ to quiesce all sequencer queues
    static_assert((offsetof(storage_seq_qstate_t, c_ndx) -
                   offsetof(storage_seq_qstate_t, p_ndx)) > 0);
    static_assert((offsetof(storage_seq_qstate_t, abort) -
                   offsetof(storage_seq_qstate_t, p_ndx)) > 0);

    auto seq_queues_quiesce_check = [this, &qid] () -> int
    {
        int64_t                 qstate_addr;
        storage_seq_qstate_t    seq_qstate;

        while (seq_qid_init_high && (qid <= seq_qid_init_high)) {
            qstate_addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, STORAGE_SEQ_QTYPE_SQ, qid);
            if (qstate_addr < 0) {
                NIC_LOG_ERR("lif-{}: Failed to get qstate address for SEQ qid {}",
                    hal_lif_info_.hw_lif_id, qid);
                return (DEVCMD_ERROR);
            }
            READ_MEM(qstate_addr + offsetof(storage_seq_qstate_t, p_ndx),
                     (uint8_t *)&seq_qstate.p_ndx,
                     (offsetof(storage_seq_qstate_t, abort) -
                      offsetof(storage_seq_qstate_t, p_ndx) +
                      sizeof(seq_qstate.abort)),
                     0);

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

    NIC_LOG_DEBUG("lif-{}: last qid quiesced: {} seq_qid_init_high: {}",
                 hal_lif_info_.hw_lif_id, qid, seq_qid_init_high);

    // Reset and reenable accelerator rings
    accel_ring_reset_all();

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_pba_clear(intr_base + intr);
        intr_drvcfg(intr_base + intr);
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdLifInit(void *req, void *req_data,
                         void *resp, void *resp_data)
{
    lif_init_cmd_t  *cmd = (lif_init_cmd_t *)req;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_LIF_INIT: lif_index {}, pushed_to_hal: {}",
                spec->name,
                cmd->index,
                hal_lif_info_.pushed_to_hal);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdAdminQueueInit(void *req, void *req_data,
                                void *resp, void *resp_data)
{
    adminq_init_cmd_t           *cmd = (adminq_init_cmd_t *)req;
    adminq_init_cpl_t           *cpl = (adminq_init_cpl_t *)resp;
    admin_qstate_t              qstate;
    int64_t                     addr;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_ADMINQ_INIT: "
                 "queue_index {} ring_base {:#x} ring_size {} intr_index {}",
                 hal_lif_info_.hw_lif_id,
                 cmd->index,
                 cmd->ring_base,
                 cmd->ring_size,
                 cmd->intr_index);

    if (cmd->index >= spec->adminq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}",
               hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", hal_lif_info_.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring size {}", hal_lif_info_.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, STORAGE_SEQ_QTYPE_ADMIN, cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for SEQ qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    uint8_t off;
    if (pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "adminq_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: adminq_stage0");
        return (DEVCMD_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = cosB;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.ci_fetch = 0;
    qstate.sta.color = 1;
    qstate.cfg.enable = 1;
    qstate.cfg.host_queue = ACCEL_PHYS_ADDR_HOST_GET(cmd->ring_base);
    qstate.cfg.intr_enable = 1;
    qstate.ring_base = cmd->ring_base;
    if (qstate.cfg.host_queue && !ACCEL_PHYS_ADDR_LIF_GET(cmd->ring_base)) {
        qstate.ring_base |= ACCEL_PHYS_ADDR_LIF_SET(hal_lif_info_.hw_lif_id);
    }
    qstate.ring_size = cmd->ring_size;
    qstate.cq_ring_base = roundup(qstate.ring_base + (64 << cmd->ring_size), 4096);
    qstate.intr_assert_index = intr_base + cmd->intr_index;
    if (nicmgr_lif_info) {
        qstate.nicmgr_qstate_addr = nicmgr_lif_info->qstate_addr[NICMGR_QTYPE_REQ];
        NIC_LOG_DEBUG("lif-{}: nicmgr_qstate_addr {:#x}", hal_lif_info_.hw_lif_id,
            qstate.nicmgr_qstate_addr);
    }
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    invalidate_txdma_cacheline(addr);

    cpl->qid = cmd->index;
    cpl->qtype = STORAGE_SEQ_QTYPE_ADMIN;
    NIC_LOG_DEBUG("lif-{}: qid {} qtype {}",
                 hal_lif_info_.hw_lif_id, cpl->qid, cpl->qtype);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueSingleInit(const seq_queue_init_cmd_t *cmd)
{
    uint64_t                qstate_addr;
    uint64_t                desc0_pgm_pc = 0;
    uint64_t                desc1_pgm_pc = 0;
    uint32_t                qid = cmd->index;
    storage_seq_qstate_t    qstate = {0};
    const char              *desc0_pgm_name = nullptr;
    const char              *desc1_pgm_name = nullptr;
    uint64_t                wring_base = 0;

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

    if (cmd->index >= seq_created_count) {
        NIC_LOG_ERR("lif-{}: qgroup {} index {} exceeds max {}", hal_lif_info_.hw_lif_id,
                    cmd->qgroup, cmd->index, seq_created_count);
        return (DEVCMD_ERROR);
    }

    qstate_addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, STORAGE_SEQ_QTYPE_SQ, qid);
    if (qstate_addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for SEQ qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (desc0_pgm_name && hal->PgmBaseAddrGet(desc0_pgm_name, &desc0_pgm_pc)) {
        NIC_LOG_ERR("Failed to get base for program {}", desc0_pgm_name);
        return (DEVCMD_ERROR);
    }

    if (desc1_pgm_name && hal->PgmBaseAddrGet(desc1_pgm_name, &desc1_pgm_pc)) {
        NIC_LOG_ERR("Failed to get base for program {}", desc1_pgm_name);
        return (DEVCMD_ERROR);
    }

    wring_base = cmd->wring_base;
    if (ACCEL_PHYS_ADDR_HOST_GET(cmd->wring_base) &&
        !ACCEL_PHYS_ADDR_LIF_GET(cmd->wring_base)) {
        wring_base |= ACCEL_PHYS_ADDR_LIF_SET(hal_lif_info_.hw_lif_id);
    }

    seq_qid_init_high = std::max(seq_qid_init_high, qid);

    uint8_t off;
    if (pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "storage_seq_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: storage_seq_stage0");
        return (DEVCMD_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = cmd->cos;
    qstate.host_wrings = cmd->host_wrings;
    qstate.total_wrings = cmd->total_wrings;
    qstate.pid = cmd->pid;
    qstate.p_ndx = 0;
    qstate.c_ndx = 0;
    qstate.enable = cmd->enable;
    qstate.abort = 0;
    qstate.wring_base = htonll(wring_base);
    qstate.wring_size = htons(cmd->wring_size);
    qstate.entry_size = htons(cmd->entry_size);

    if (desc0_pgm_name) {
        qstate.desc0_next_pc = htonl(desc0_pgm_pc >> 6);
    }

    if (desc1_pgm_name) {
        qstate.desc1_next_pc = htonl(desc1_pgm_pc >> 6);
        qstate.desc1_next_pc_valid = 1;
    }

    qstate.qgroup = cmd->qgroup;
    qstate.core_id = cmd->core_id;

    WRITE_MEM(qstate_addr, (uint8_t *)&qstate, sizeof(qstate), 0);
    invalidate_txdma_cacheline(qstate_addr);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueInit(void *req, void *req_data,
                              void *resp, void *resp_data)
{
    seq_queue_init_cmd_t    *cmd = (seq_queue_init_cmd_t *)req;
    seq_queue_init_cpl_t    *cpl = (seq_queue_init_cpl_t *)resp;
    enum DevcmdStatus       status;

    NIC_LOG_DEBUG("lif-{} CMD_OPCODE_SEQ_QUEUE_INIT: "
                  "qid {} qgroup {} core_id {} wring_base {:#x} "
                  "wring_size {} entry_size {}", hal_lif_info_.hw_lif_id,
                  cmd->index, cmd->qgroup, cmd->core_id, cmd->wring_base,
                  cmd->wring_size, cmd->entry_size);
    /*
     * Stop any in-progress delphi uploads until INIT_COMPLETE received.
     */
    if (delphi_qinfo_thread) {
        delphi_qinfo_thread->stop_work_set(true);
    }
    status = _DevcmdSeqQueueSingleInit(cmd);

    cpl->qid = cmd->index;
    cpl->qtype = STORAGE_SEQ_QTYPE_SQ;

    return status;
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueInitComplete(void *req, void *req_data,
                                     void *resp, void *resp_data)
{
    NIC_LOG_DEBUG("lif-{} CMD_OPCODE_SEQ_QUEUE_INIT_COMPLETE: ",
                  hal_lif_info_.hw_lif_id);
    if (delphi_qinfo_thread) {
        delphi_qinfo_thread->work_notify();
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueBatchInit(void *req, void *req_data,
                                   void *resp, void *resp_data)
{
    seq_queue_batch_init_cmd_t *batch_cmd = (seq_queue_batch_init_cmd_t *)req;
    seq_queue_init_cmd_t       cmd = {0};
    enum DevcmdStatus          status = DEVCMD_SUCCESS;
    uint64_t                   seq_q_size;
    uint32_t                   i;

    NIC_LOG_DEBUG("lif-{} CMD_OPCODE_SEQ_QUEUE_BATCH_INIT: "
                  "qid {} qgroup {} num_queues {} wring_base {:#x} "
                  "wring_size {} entry_size {}", hal_lif_info_.hw_lif_id,
                  batch_cmd->index, batch_cmd->qgroup, batch_cmd->num_queues,
                  batch_cmd->wring_base, batch_cmd->wring_size, batch_cmd->entry_size);
    /*
     * Stop any in-progress delphi uploads until INIT_COMPLETE received.
     */
    if (delphi_qinfo_thread) {
        delphi_qinfo_thread->stop_work_set(true);
    }
    cmd.opcode = CMD_OPCODE_SEQ_QUEUE_INIT;
    cmd.index = batch_cmd->index;
    cmd.pid = batch_cmd->pid;
    cmd.qgroup = batch_cmd->qgroup;
    cmd.enable = batch_cmd->enable;
    cmd.cos = batch_cmd->cos;
    cmd.total_wrings = batch_cmd->total_wrings;
    cmd.host_wrings = batch_cmd->host_wrings;
    cmd.entry_size = batch_cmd->entry_size;
    cmd.wring_size = batch_cmd->wring_size;
    cmd.wring_base = batch_cmd->wring_base;

    seq_q_size = (1ULL << batch_cmd->entry_size) *
                 (1ULL << batch_cmd->wring_size);
    for (i = 0; i < batch_cmd->num_queues; i++) {
        status = _DevcmdSeqQueueSingleInit(&cmd);
        if (status != DEVCMD_SUCCESS) {
            break;
        }
        cmd.index++;
        cmd.wring_base += seq_q_size;
    }

    return status;
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueSingleControl(const seq_queue_control_cmd_t *cmd,
                                       bool enable)
{
    int64_t                 qstate_addr;
    uint8_t                 value;
    struct admin_cfg_qstate admin_cfg = {0};

    if (cmd->qtype >= STORAGE_SEQ_QTYPE_MAX) {
        NIC_LOG_ERR("lif-{}: bad qtype {}", hal_lif_info_.hw_lif_id, cmd->qtype);
        return (DEVCMD_ERROR);
    }

    value = enable;
    switch (cmd->qtype) {
    case STORAGE_SEQ_QTYPE_SQ:
        if (cmd->qid >= seq_created_count) {
            NIC_LOG_ERR("lif-{}: qtype {} qid {} exceeds count {}",
                        hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid,
                        seq_created_count);
            return (DEVCMD_ERROR);
        }
        qstate_addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);
        if (qstate_addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        WRITE_MEM(qstate_addr + offsetof(storage_seq_qstate_t, enable),
                  (uint8_t *)&value, sizeof(value), 0);
        invalidate_txdma_cacheline(qstate_addr);
        break;
    case STORAGE_SEQ_QTYPE_ADMIN:
        if (cmd->qid >= spec->adminq_count) {
            NIC_LOG_ERR("lif-{}: qtype {} qid {} exceeds count {}",
                        hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid,
                        spec->adminq_count);
            return (DEVCMD_ERROR);
        }
        qstate_addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);
        if (qstate_addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        admin_cfg.enable = enable;
        admin_cfg.host_queue = 0x1;
        WRITE_MEM(qstate_addr + offsetof(admin_qstate_t, cfg), (uint8_t *)&admin_cfg,
                  sizeof(admin_cfg), 0);
        invalidate_txdma_cacheline(qstate_addr);
        break;
    default:
        return (DEVCMD_ERROR);
        break;
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueControl(void *req, void *req_data,
                                 void *resp, void *resp_data,
                                 bool enable)
{
    seq_queue_control_cmd_t *cmd = (seq_queue_control_cmd_t *)req;

    NIC_LOG_DEBUG(" lif-{}: qtype {} qid {} enable {}", hal_lif_info_.hw_lif_id,
                  cmd->qtype, cmd->qid, enable);
    return _DevcmdSeqQueueSingleControl(cmd, enable);
}

enum DevcmdStatus
Accel_PF::_DevcmdSeqQueueBatchControl(void *req, void *req_data,
                                      void *resp, void *resp_data,
                                      bool enable)
{
    seq_queue_batch_control_cmd_t *batch_cmd = (seq_queue_batch_control_cmd_t *)req;
    seq_queue_control_cmd_t       cmd = {0};
    enum DevcmdStatus             status = DEVCMD_SUCCESS;
    uint32_t                      i;

    NIC_LOG_DEBUG(" lif-{}: qtype {} qid {} mum_queues {} enable {}",
                  hal_lif_info_.hw_lif_id, batch_cmd->qtype,
                  batch_cmd->qid, batch_cmd->num_queues, enable);
    cmd.opcode = enable ? CMD_OPCODE_SEQ_QUEUE_ENABLE : CMD_OPCODE_SEQ_QUEUE_DISABLE;
    cmd.qid = batch_cmd->qid;
    cmd.qtype = batch_cmd->qtype;

    for (i = 0; i < batch_cmd->num_queues; i++) {
        status = _DevcmdSeqQueueSingleControl(&cmd, enable);
        if (status != DEVCMD_SUCCESS) {
            break;
        }
        cmd.qid++;
    }

    return status;
}

enum DevcmdStatus
Accel_PF::_DevcmdCryptoKeyUpdate(void *req, void *req_data,
                                 void *resp, void *resp_data)
{
    crypto_key_update_cmd_t     *cmd = (crypto_key_update_cmd_t *)req;
    crypto_key_accum_t          *key_accum;
    uint8_t                     *dst_key_data;
    int                         ret_val;

    NIC_LOG_DEBUG(" lif-{}  key_index {} key_type {} key_size {} key_part {}",
                 hal_lif_info_.hw_lif_id, cmd->key_index, cmd->key_type,
                 cmd->key_size, cmd->key_part);

    if (cmd->key_type >= CMD_CRYPTO_KEY_TYPE_MAX) {
        NIC_LOG_ERR("lif-{}: unrecognized key_type {}", hal_lif_info_.hw_lif_id,
                    cmd->key_type);
        return (DEVCMD_ERROR);
    }

    if (cmd->key_size > CMD_CRYPTO_KEY_PART_SIZE) {
        NIC_LOG_ERR("lif-{}: invalid key_size {}", hal_lif_info_.hw_lif_id,
                    cmd->key_size);
        return (DEVCMD_ERROR);
    }

    if (cmd->key_part >= CMD_CRYPTO_KEY_PART_MAX) {
        NIC_LOG_ERR("lif-{}: invalid key_part {}", hal_lif_info_.hw_lif_id,
                    cmd->key_part);
        return (DEVCMD_ERROR);
    }

    key_accum = crypto_key_accum_find_add(cmd->key_index);
    if (!key_accum) {
        NIC_LOG_ERR("lif-{}: unable to obtain key accumulator for key_index {}",
                    hal_lif_info_.hw_lif_id, cmd->key_index);
        return (DEVCMD_ERROR);
    }

    dst_key_data = &key_accum->key_data[cmd->key_part][0];
    memcpy(dst_key_data, cmd->key_data, cmd->key_size);
    if (CMD_CRYPTO_KEY_PART_SIZE - cmd->key_size) {
        memset(dst_key_data + cmd->key_size, 0,
               CMD_CRYPTO_KEY_PART_SIZE - cmd->key_size);
    }

    /*
     * This devcmd assumes that each time the caller makes a key update,
     * all the key parts would eventually have been submitted.
     */
    key_accum->num_key_parts = std::min(key_accum->num_key_parts + 1,
                                        (uint32_t)CMD_CRYPTO_KEY_PART_MAX);
    if (cmd->trigger_update) {
        ret_val = hal->CryptoKeyIndexUpdate(cmd->key_index,
                              crypto_key_type_tbl[cmd->key_type],
                              &key_accum->key_data[0][0],
                              key_accum->num_key_parts * CMD_CRYPTO_KEY_PART_SIZE);
        /*
         * Wipe and erase accumulator
         */
        memset(key_accum, 0, sizeof(*key_accum));
        crypto_key_accum_del(cmd->key_index);
        if (ret_val) {
            NIC_LOG_ERR("lif {}: failed to update crypto key for key_index {}",
                        hal_lif_info_.hw_lif_id, cmd->key_index);
            return (DEVCMD_ERROR);
        }
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
                accel_ring_t& spec_ring = accel_ring_tbl[info.ring_handle];
                spec_ring.ring_base_pa = info.base_pa;
                spec_ring.ring_pndx_pa = info.pndx_pa;
                spec_ring.ring_shadow_pndx_pa = info.shadow_pndx_pa;
                spec_ring.ring_opaque_tag_pa = info.opaque_tag_pa;
                spec_ring.ring_opaque_tag_size = info.opaque_tag_size;
                spec_ring.ring_desc_size = info.desc_size;
                spec_ring.ring_pndx_size = info.pndx_size;
                spec_ring.ring_size = info.ring_size;

                NIC_LOG_DEBUG("ring {} ring_base_pa {:#x} ring_pndx_pa {:#x} "
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

            NIC_LOG_DEBUG("lif-{}: all rings quiesced", hal_lif_info_.hw_lif_id);
            return 0;
        }

        NIC_LOG_ERR("lif-{}: unable to wait for rings to quiesce", hal_lif_info_.hw_lif_id);
        return 0;
    };

    ret_val = accel_ring_max_pendings_get(max_pendings);
    if (ret_val == 0) {
        NIC_LOG_DEBUG("lif-{}: max requests pending {}", hal_lif_info_.hw_lif_id,
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
        NIC_LOG_ERR("lif-{}:: failed to add ring group {}",
                    hal_lif_info_.hw_lif_id, accel_pf_rgroup_name);
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
        NIC_LOG_ERR("lif-{}: failed to add ring vector",
                    hal_lif_info_.hw_lif_id, accel_pf_rgroup_name);
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
        NIC_LOG_ERR("lif-{}:: failed to reset ring group {} sense {}",
                    hal_lif_info_.hw_lif_id, accel_pf_rgroup_name, reset_sense);
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
        NIC_LOG_ERR("lif-{}:: failed to enable ring group {} sense {}",
                    hal_lif_info_.hw_lif_id, accel_pf_rgroup_name, enable_sense);
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
        NIC_LOG_ERR("lif-{}:: failed to set pndx for ring group {} val {}",
                    hal_lif_info_.hw_lif_id, accel_pf_rgroup_name, val);
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
    Accel_PF    *accel_pf = (Accel_PF *)user_ctx;

    ACCEL_RGROUP_GET_CB_HANDLE_CHECK(info.ring_handle);
    accel_rgroup_ring_t& rgroup_ring =
            accel_pf_rgroup_find_create(info.ring_handle, info.sub_ring);
    rgroup_ring.info = info;
    accel_ring_info_publish(rgroup_ring);
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
    ACCEL_RGROUP_GET_RET_CHECK(ret_val, num_rings, "rinfo");
    return ret_val;
}

/*
 * Accelerator ring group ring indices response callback handler
 */
void
accel_rgroup_rindices_rsp_cb(void *user_ctx,
                             const accel_rgroup_rindices_rsp_t& indices)
{
    Accel_PF    *accel_pf = (Accel_PF *)user_ctx;

    ACCEL_RGROUP_GET_CB_HANDLE_CHECK(indices.ring_handle);
    accel_rgroup_ring_t& rgroup_ring =
            accel_pf_rgroup_find_create(indices.ring_handle, indices.sub_ring);
    if (memcmp(&rgroup_ring.indices, &indices, sizeof(indices))) {
        accel_pf->rgroup_indices_chg = true;
    }
    rgroup_ring.indices = indices;
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
    ACCEL_RGROUP_GET_RET_CHECK(ret_val, num_rings, "indices");
    return ret_val;
}

/*
 * Accelerator ring group ring metrics response callback handler
 */
void
accel_rgroup_rmetrics_rsp_cb(void *user_ctx,
                             const accel_rgroup_rmetrics_rsp_t& metrics)
{
    Accel_PF    *accel_pf = (Accel_PF *)user_ctx;

    ACCEL_RGROUP_GET_CB_HANDLE_CHECK(metrics.ring_handle);
    accel_rgroup_ring_t& rgroup_ring =
            accel_pf_rgroup_find_create(metrics.ring_handle, metrics.sub_ring);
    if (memcmp(&rgroup_ring.metrics, &metrics, sizeof(metrics))) {
        accel_pf->rgroup_metrics_chg = true;
    }
    rgroup_ring.metrics = metrics;
}

/*
 * Get ring group metrics
 */
int
Accel_PF::accel_rgroup_rmetrics_get(void)
{
    uint32_t    num_rings;
    int         ret_val;

    ret_val = hal->AccelRGroupMetricsGet(accel_pf_rgroup_name, ACCEL_SUB_RING_ALL,
                                         accel_rgroup_rmetrics_rsp_cb, this,
                                         num_rings);
    ACCEL_RGROUP_GET_RET_CHECK(ret_val, num_rings, "metrics");
    return ret_val;
}

/*
 * Send Delphi updates for all rings
 */
void
Accel_PF::delphi_update(void)
{
    accel_rgroup_iter_c     iter;

    if (g_nicmgr_svc) {
        rgroup_indices_chg = false;
        rgroup_metrics_chg = false;

        accel_rgroup_rindices_get();
        accel_rgroup_rmetrics_get();

        if (rgroup_indices_chg || rgroup_metrics_chg) {
            for (iter  = accel_pf_rgroup_map.begin();
                 iter != accel_pf_rgroup_map.end();
                 iter++) {

                accel_ring_info_publish(iter->second);
            }
        }
    }
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
        NIC_LOG_DEBUG("ring {} pndx {} cndx {}",
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
 * Periodic timer, mainly for metrics update
 */
void
Accel_PF::periodic_sync(ev::timer &watcher, int revents)
{
    delphi_update();
}

/*
 * Publish AccelSeqQueueInfo objects to Delphi.
 */
void *
seq_queue_info_publish_thread(void *ctx)
{
    AccelWorkThread *thread = (AccelWorkThread *)ctx;
    Accel_PF        *accel_pf = (Accel_PF *)thread->user_ctx_get();
    uint32_t        qid;

    thread->work_lock();
    while (!thread->terminate_get()) {
        thread->wait_for_work();

        for (qid = 0;
             (qid < accel_pf->delphi_qinfo_vec.size()) &&
             !thread->stop_work_get() && !thread->terminate_get();
             qid++) {

            g_nicmgr_svc->sdk()->QueueUpdate(
                     accel_pf->delphi_qinfo_vec.at(qid));
            if ((qid % ACCEL_QID_PUBLISH_YIELD_POINT) == 0) {
                thread->yield();
            }
        }

        thread->stop_work_set(true);
    }

    thread->work_unlock();
    return nullptr;
}

/*
 * Publish AccelHwRingInfo object to Delphi.
 */
static void
accel_ring_info_publish(const accel_rgroup_ring_t& rgroup_ring)
{
    if (g_nicmgr_svc) {
        delphi::objects::AccelHwRingInfoPtr shared_ring(rgroup_ring.delphi_ring);
        shared_ring->set_pindex(rgroup_ring.indices.pndx);
        shared_ring->set_cindex(rgroup_ring.indices.cndx);
        g_nicmgr_svc->sdk()->SetObject(shared_ring);

        rgroup_ring.delphi_metrics->InputBytes()->
                                    Set(rgroup_ring.metrics.input_bytes);
        rgroup_ring.delphi_metrics->OutputBytes()->
                                    Set(rgroup_ring.metrics.output_bytes);
        rgroup_ring.delphi_metrics->SoftResets()->
                                    Set(rgroup_ring.metrics.soft_resets);
    }
}

/*
 * Find or create an accelerator ring group ring
 */
static accel_rgroup_ring_t&
accel_pf_rgroup_find_create(uint32_t ring_handle,
                            uint32_t sub_ring)
{
    accel_rgroup_ring_key_t         key;
    accel_rgroup_iter_t             iter;
    accel_metrics::AccelHwRingKey   delphi_key;

    key = accel_rgroup_ring_key_make(ring_handle, sub_ring);
    iter = accel_pf_rgroup_map.find(key);
    if (iter != accel_pf_rgroup_map.end()) {
        return iter->second;
    }

    accel_rgroup_ring_t& rgroup_ring = accel_pf_rgroup_map[key];
    rgroup_ring.delphi_ring = make_shared<delphi::objects::AccelHwRingInfo>();
    rgroup_ring.delphi_ring->mutable_key()->set_rid(accel_ring_id2name_get(ring_handle));
    rgroup_ring.delphi_ring->mutable_key()->set_subrid(std::to_string(sub_ring));

    delphi_key.set_rid(accel_ring_id2name_get(ring_handle));
    delphi_key.set_subrid(std::to_string(sub_ring));
    rgroup_ring.delphi_metrics = delphi::objects::AccelHwRingMetrics::
                                 NewAccelHwRingMetrics(delphi_key);
    return rgroup_ring;
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
 * Crypto key accumulator
 */
static crypto_key_accum_t *
crypto_key_accum_find_add(uint32_t key_index)
{
    std::pair<accel_crypto_key_iter_t,bool> iter;
    crypto_key_accum_t empty_key_accum = {0};

    try {
        iter = accel_crypto_key_map.insert(std::make_pair(key_index,
                                                          empty_key_accum));
        return &iter.first->second;
    } catch (std::exception& e) {
        return nullptr;
    }
}

static void
crypto_key_accum_del(uint32_t key_index)
{
    accel_crypto_key_map.erase(key_index);
}

void
Accel_PF::HalEventHandler(bool status)
{
    if (!status) {
        return;
    }

    // FIXME: This should be done from LifInit but putting it there
    // causes devcmd timeouts because DelphiDeviceInit trigger very
    // long running timer callbacks.

    // Trigger Hal for Lif create if this is the first time
    if (!hal_lif_info_.pushed_to_hal) {

        uint64_t ret = hal->LifCreate(&hal_lif_info_);
        if (ret != 0) {
            NIC_LOG_ERR("{}: Failed to create LIF {}",
                spec->name, hal_lif_info_.hw_lif_id);
            return;
        }

        NIC_LOG_INFO("lif-{}: created", hal_lif_info_.hw_lif_id);

        cosA = 1;
        cosB = 0;
        uint8_t coses = (((cosB & 0x0f) << 4) | (cosA & 0x0f));

        // program the queue state
        pd->program_qstate((struct queue_info*)hal_lif_info_.queue_info,
            &hal_lif_info_, coses);

        // acquire rings info as initialized by HAL
        accel_ring_info_get_all();

        // establish sequencer queues metrics with Delphi
        if (DelphiDeviceInit()) {
            NIC_LOG_ERR("lif-{}: Failed to establish qmetrics",
                hal_lif_info_.hw_lif_id);
        }

        if (spec->pub_intv_frac) {
            sync_timer.set<Accel_PF, &Accel_PF::periodic_sync>(this);
            float intv = 1.0 / (float)spec->pub_intv_frac;
            sync_timer.start(0.0, intv);
        }
    }
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

    NIC_LOG_DEBUG("Polling timeout_us {} exceeded - Giving up!", timeout_us);
    return -1;
}

void
Accel_PF::SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client)
{
    hal = hal_client;
    hal_common_client = hal_cmn_client;
}

void
Accel_PF::ThreadsWaitJoin(void)
{
    for (uint32_t i = 0; i < accel_threads_vec.size(); i++) {
        auto thread = accel_threads_vec.at(i);

        thread->terminate_set(true);
        thread->work_notify();
        thread->join();
    }
}

/*
 * Accelerator work thread
 */
AccelWorkThread::AccelWorkThread(std::string thread_name,
                                 uint64_t cores_mask,
                                 void *user_ctx,
                                 sdk::lib::thread_entry_func_t entry_func) :
    thread_name(thread_name),
    thread(nullptr),
    user_ctx(user_ctx),
    stop_work(true),
    terminate(false)
{
    if (pthread_mutex_init(&work_mutex, NULL)) {
         NIC_LOG_ERR("{} failed work mutex init: error {}",
                     thread_name, errno);
         throw;
    }
    if (pthread_cond_init(&work_cond, NULL)) {
         NIC_LOG_ERR("{} failed work cond init: error {}",
                     thread_name, errno);
         throw;
    }

    sdk::lib::thread::control_cores_mask_set(cores_mask);
    thread = sdk::lib::thread::factory(thread_name.c_str(),
                                       accel_thread_id++,
                                       sdk::lib::THREAD_ROLE_CONTROL,
                                       cores_mask,
                                       entry_func,
                                       sched_get_priority_max(SCHED_OTHER),
                                       SCHED_OTHER, true);
    if (!thread) {
        NIC_LOG_ERR("failed to create thread {}", thread_name);
        throw;
    }

    thread->start(this);
}

AccelWorkThread::~AccelWorkThread()
{
    if (thread) {
        sdk::lib::thread::destroy(thread);
    }
}

void
AccelWorkThread::work_lock(void)
{
    if (pthread_mutex_lock(&work_mutex)) {
        NIC_LOG_ERR("{} failed work mutex lock: error {}",
                    thread_name, errno);
        throw;
    }
}

void
AccelWorkThread::work_unlock(void)
{
    if (pthread_mutex_unlock(&work_mutex)) {
        NIC_LOG_ERR("{} failed work mutex unlock: error {}",
                    thread_name, errno);
    }
}

void
AccelWorkThread::work_notify(void)
{
    work_lock();
    stop_work = false;
    if (pthread_cond_signal(&work_cond)) {
        NIC_LOG_ERR("{} failed to notify work: error {}",
                    thread_name, errno);
        throw;
    }
    work_unlock();
}

void
AccelWorkThread::wait_for_work(void)
{
    if (pthread_cond_wait(&work_cond, &work_mutex)) {
        NIC_LOG_ERR("{} failed to wait for work: error {}",
                    thread_name, errno);
        throw;
    }
}

void
AccelWorkThread::join(void)
{
    if (pthread_join(thread->pthread_id(), NULL)) {
        NIC_LOG_ERR("{} failed to join: error {}",
                    thread_name, errno);
    }
}

