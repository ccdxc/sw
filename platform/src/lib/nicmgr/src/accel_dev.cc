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
#include <sys/time.h>

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"
#include "cap_ms_c_hdr.h"

#include "nic/include/base.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"

#include "gen/proto/nicmgr/accel_metrics.pb.h"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "nic/sdk/platform/fru/fru.hpp"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

#ifdef __aarch64__
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#endif
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"


#include "logger.hpp"
#include "accel_if.h"
#include "accel_dev.hpp"
#include "accel_lif.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"


using namespace nicmgr;

extern class pciemgr *pciemgr;

/*
 * Devapi availability check
 */
#define ACCEL_DEVAPI_CHECK(ret_val)                                             \
    if (!dev_api) {                                                             \
        NIC_LOG_ERR("{}: Uninitialized devapi", DevNameGet());                  \
        return ret_val;                                                         \
    }

#define ACCEL_DEVAPI_CHECK_VOID()                                               \
    if (!dev_api) {                                                             \
        NIC_LOG_ERR("{}: Uninitialized devapi", DevNameGet());                  \
        return;                                                                 \
    }

#define ACCEL_DEV_NUM_CRYPTO_KEYS_MIN           512     // arbitrary low water mark

// Minimum amount of STORAGE_SEQ_HBM_HANDLE we want to enforce
#define ACCEL_DEV_SEQ_HBM_SIZE_MIN              (256 * 1024 * 1024)

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

AccelDev::AccelDev(devapi *dapi,
                   void *dev_spec,
                   PdClient *pd_client) :
    spec((accel_devspec_t *)dev_spec),
    pd(pd_client),
    dev_api(dapi),
    pdev(nullptr),
    delphi_mounted(false)
{
    accel_lif_res_t     lif_res;
    sdk_ret_t           ret = SDK_RET_OK;
    uint32_t            num_keys_max;

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
    devcmd_mem_addr = pd->devcmd_mem_alloc(ACCEL_DEV_PAGE_SIZE);
    MEM_SET(devcmd_mem_addr, 0, ACCEL_DEV_PAGE_SIZE, 0);
    // TODO: mmap instead of calloc after porting to real pal
    devcmd = (dev_cmd_regs_t *)calloc(1, sizeof(dev_cmd_regs_t));
    if (devcmd == NULL) {
        NIC_LOG_ERR("{}: Failed to map devcmd region", DevNameGet());
        throw;
    }
    devcmd->signature = DEV_CMD_SIGNATURE;
    WRITE_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(*devcmd), 0);

    devcmddb_mem_addr = pd->devcmd_mem_alloc(ACCEL_DEV_PAGE_SIZE);
    MEM_SET(devcmddb_mem_addr, 0, ACCEL_DEV_PAGE_SIZE, 0);

    NIC_LOG_DEBUG("{}: devcmd_addr {:#x} devcmddb_addr {:#x}",
                  DevNameGet(), devcmd_mem_addr, devcmddb_mem_addr);

    // Locate HBM region dedicated to crypto keys
    cmb_mem_addr = pd->mp_->start_addr(CAPRI_BARCO_KEY_DESC);
    cmb_mem_size = pd->mp_->size(CAPRI_BARCO_KEY_DESC);
    if (cmb_mem_addr == INVALID_MEM_ADDRESS || cmb_mem_size == 0) {
        NIC_LOG_ERR("{}: Failed to get HBM base for {}",
                    DevNameGet(), CAPRI_BARCO_KEY_DESC);
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
                 "num_crypto_keys_max {}", DevNameGet(), cmb_mem_size,
                 crypto_key_idx_base, num_crypto_keys_max);
    if (num_crypto_keys_max < ACCEL_DEV_NUM_CRYPTO_KEYS_MIN) {
        NIC_LOG_ERR("{}: num_crypto_keys_max {} too small, expected at least {}",
                    DevNameGet(), num_crypto_keys_max,
                    ACCEL_DEV_NUM_CRYPTO_KEYS_MIN);
        throw;
    }

    // Locate HBM region dedicated to STORAGE_SEQ_HBM_HANDLE
    cmb_mem_addr = pd->mp_->start_addr(STORAGE_SEQ_HBM_HANDLE);
    cmb_mem_size = pd->mp_->size(STORAGE_SEQ_HBM_HANDLE);
    if (cmb_mem_addr == INVALID_MEM_ADDRESS || cmb_mem_size == 0) {
        NIC_LOG_ERR("{}: failed to get HBM base for {}", DevNameGet(),
            STORAGE_SEQ_HBM_HANDLE);
        throw;
    }

    // First, ensure size is a power of 2, then per PCIe BAR mapping
    // requirement, align the region on its natural boundary, i.e.,
    // if size is 64MB then the region must be aligned on a 64MB boundary!
    // This means we could potentially waste half of the space if
    // the region was not already aligned.
    assert(is_power_of_2(cmb_mem_size));
    if (cmb_mem_addr & (cmb_mem_size - 1)) {
        cmb_mem_size /= 2;
        cmb_mem_addr = ACCEL_DEV_ADDR_ALIGN(cmb_mem_addr, cmb_mem_size);
    }

    if (platform_is_hw(pd->platform_) &&
        (pd->fwd_mode_ == sdk::platform::FWD_MODE_CLASSIC) &&
        (cmb_mem_size < ACCEL_DEV_SEQ_HBM_SIZE_MIN)) {
        NIC_LOG_ERR("{}: HBM aligned size {} is too small for {}",
                    DevNameGet(), cmb_mem_size, STORAGE_SEQ_HBM_HANDLE);
        throw;
    }

    NIC_LOG_DEBUG("{}: cmb_mem_addr: {:#x} cmb_mem_size: {}",
                  DevNameGet(), cmb_mem_addr, cmb_mem_size);

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
    auto lif = new AccelLif(*this, lif_res);
    if (!lif) {
        NIC_LOG_ERR("{}: failed to create AccelLif {}",
                    DevNameGet(), lif_res.lif_id);
        throw;
    }
    lif_map[lif_base] = lif;

    _MetricsInit();
    evutil_timer_start(&devcmd_timer, &AccelDev::_DevcmdPoll, this, 0.0, 0.01);
}

AccelDev::~AccelDev()
{
    /*
     * Most HBM related allocs don't have corresponding free API so
     * we'll leave them alone. Same with intr_alloc().
     */
    evutil_timer_stop(&devcmd_timer);
    _MetricsFini();

    auto iter = lif_map.begin();
    while (iter != lif_map.end()) {
        delete iter->second;
        iter = lif_map.erase(iter);
    }

    _DestroyHostDevice();
    pd->lm_->free_id(lif_base, spec->lif_count);
}

void
AccelDev::HalEventHandler(bool status)
{
    for (auto iter = lif_map.cbegin(); iter != lif_map.cend(); iter++) {
        AccelLif *lif = iter->second;
        lif->HalEventHandler(status);
    }
}

void
AccelDev::DelphiMountEventHandler(bool mounted)
{
    delphi_mounted = mounted;
    _MetricsInit();
}

void
AccelDev::SetHalClient(devapi *dapi)
{
    dev_api = dapi;

    for (auto iter = lif_map.cbegin(); iter != lif_map.cend(); iter++) {
        AccelLif *lif = iter->second;
        lif->SetHalClient(dapi);
    }
}

struct accel_devspec *
AccelDev::ParseConfig(boost::property_tree::ptree::value_type node)
{
    accel_devspec* accel_spec;
    auto val = node.second;

    accel_spec = new struct accel_devspec;
    memset(accel_spec, 0, sizeof(*accel_spec));

    accel_spec->name = val.get<string>("name");
    accel_spec->lif_count = val.get<uint64_t>("lif_count");
    accel_spec->seq_queue_count = val.get<uint32_t>("seq_queue_count");
    accel_spec->adminq_count = val.get<uint32_t>("adminq_count");
    accel_spec->intr_count = val.get<uint32_t>("intr_count");
    if (val.get_optional<string>("rate_limit")) {
        if (val.get_optional<string>("rate_limit.rx_limit_gbps")) {
            accel_spec->rx_limit_gbps = val.get<uint32_t>("rate_limit.rx_limit_gbps");
        }
        if (val.get_optional<string>("rate_limit.rx_burst_gb")) {
            accel_spec->rx_burst_gb = val.get<uint32_t>("rate_limit.rx_burst_gb");
        }
        if (val.get_optional<string>("rate_limit.tx_limit_gbps")) {
            accel_spec->tx_limit_gbps = val.get<uint32_t>("rate_limit.tx_limit_gbps");
        }
        if (val.get_optional<string>("rate_limit.tx_burst_gb")) {
            accel_spec->tx_burst_gb = val.get<uint32_t>("rate_limit.tx_burst_gb");
        }
    }

    accel_spec->pub_intv_frac = ACCEL_DEV_PUB_INTV_FRAC_DFLT;
    if (val.get_optional<string>("publish_interval")) {
        accel_spec->pub_intv_frac = val.get<uint32_t>("publish_interval.sec_fraction");
    }

    accel_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
    accel_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
    NIC_LOG_DEBUG("Creating accel device with name: {}, qos_group: {}",
            accel_spec->name,
            accel_spec->qos_group);

    return accel_spec;
}

bool
AccelDev::_CreateHostDevice(void)
{
    pciehdevice_resources_t pres = {0};

    pres.pfres.port = spec->pcie_port;
    pres.pfres.lifb = lif_base;
    pres.pfres.lifc = spec->lif_count;
    pres.pfres.intrb = intr_base;
    pres.pfres.intrc = spec->intr_count;
    pres.pfres.intrdmask = 1;
    pres.pfres.npids = 1;
    pres.pfres.devcmdpa = devcmd_mem_addr;
    pres.pfres.devcmddbpa = devcmddb_mem_addr;
    pres.pfres.cmbpa = cmb_mem_addr;
    pres.pfres.cmbsz = cmb_mem_size;
    pres.pfres.cmbprefetch = true;

    // Create PCI device
    NIC_LOG_DEBUG("{}: Creating PCI device", DevNameGet());
    pdev = pciehdevice_new("accel", DevNameGet().c_str(), &pres);
    if (pdev == NULL) {
        NIC_LOG_ERR("{}: Failed to create PCI device", DevNameGet());
        return false;
    }

    // Add device to PCI topology
    if (pciemgr) {
        int ret = pciemgr->add_device(pdev);
        if (ret != 0) {
            NIC_LOG_ERR("{}: Failed to add PCI device to topology",
                        DevNameGet());
            return false;
        }
    }

    return true;
}

void
AccelDev::_DestroyHostDevice(void)
{
    if (pdev) {
        pciehdev_delete(pdev);
        pdev = nullptr;
    }
}

void
AccelDev::_MetricsInit(void)
{
    if (g_nicmgr_svc && delphi_mounted && !delphi_pf) {
        NIC_LOG_INFO("{}: Registering AccelPfInfo", DevNameGet());
        delphi_pf = make_shared<delphi::objects::AccelPfInfo>();
        delphi_pf->set_key(std::to_string(lif_base));
        delphi_pf->set_hwlifid(lif_base);
        delphi_pf->set_numseqqueues(spec->seq_queue_count);
        delphi_pf->set_cryptokeyidxbase(crypto_key_idx_base);
        delphi_pf->set_numcryptokeysmax(num_crypto_keys_max);
        delphi_pf->set_intrbase(intr_base);
        delphi_pf->set_intrcount(spec->intr_count);
        g_nicmgr_svc->sdk()->SetObject(delphi_pf);
    }
}

void
AccelDev::_MetricsFini(void)
{
    if (g_nicmgr_svc && delphi_pf) {
        NIC_LOG_INFO("{}: Deregistering AccelPfInfo", DevNameGet());
        g_nicmgr_svc->sdk()->DeleteObject(delphi_pf);
        delphi_pf.reset();
    }
}

void
AccelDev::_DevcmdPoll(void *obj)
{
    AccelDev        *dev = (AccelDev *)obj;
    dev_cmd_db_t    db = {0};
    dev_cmd_db_t    db_clear = {0};

    READ_MEM(dev->devcmddb_mem_addr, (uint8_t *)&db, sizeof(db), 0);
    if (db.v) {
        NIC_LOG_INFO("{}: Devcmd doorbell", dev->DevNameGet());
        WRITE_MEM(dev->devcmddb_mem_addr, (uint8_t *)&db_clear,
                  sizeof(db_clear), 0);
        dev->DevcmdHandler();
    }
}

void
AccelDev::DevcmdHandler()
{
    accel_status_code_t status = ACCEL_RC_SUCCESS;

    NIC_HEADER_TRACE("Devcmd");

    // read devcmd region
    READ_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(dev_cmd_regs_t), 0);

    if (devcmd->done) {
        NIC_LOG_ERR("{}: Devcmd done is set before processing command, opcode {}",
            DevNameGet(), accel_dev_opcode_str(devcmd->cmd.cmd.opcode));
        status = ACCEL_RC_ERROR;
        goto devcmd_done;
    }

    if (devcmd->signature != DEV_CMD_SIGNATURE) {
        NIC_LOG_ERR("{}: Devcmd signature mismatch, opcode {}", DevNameGet(),
                    accel_dev_opcode_str(devcmd->cmd.cmd.opcode));
        status = ACCEL_RC_ERROR;
        goto devcmd_done;
    }

    status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->cpl, &devcmd->data);

    // write data
    if (status == ACCEL_RC_SUCCESS) {
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
    PAL_barrier();
    WRITE_MEM(devcmd_mem_addr + offsetof(dev_cmd_regs_t, done),
              (uint8_t *)devcmd + offsetof(dev_cmd_regs_t, done),
              sizeof(devcmd->done), 0);

    NIC_HEADER_TRACE("Devcmd End");
}

const char *
accel_dev_opcode_str(uint32_t opcode)
{
    switch(opcode) {
        ACCEL_DEVCMD_OPCODE_CASE_TABLE
        default: return "DEVCMD_UNKNOWN";
    }
}

accel_status_code_t
AccelDev::CmdHandler(void *req, void *req_data,
                     void *resp, void *resp_data)
{
    dev_cmd_t       *cmd = (dev_cmd_t *)req;
    dev_cmd_cpl_t   *cpl = (dev_cmd_cpl_t *)resp;
    accel_status_code_t status;

    NIC_LOG_DEBUG("{}: Handling cmd: {}", DevNameGet(),
                  accel_dev_opcode_str(cmd->cmd.opcode));

    switch (cmd->cmd.opcode) {

    case CMD_OPCODE_NOP:
        status = ACCEL_RC_SUCCESS;
        break;

    case CMD_OPCODE_RESET:
        status = _DevcmdReset(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_IDENTIFY:
        status = _DevcmdIdentify(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_CRYPTO_KEY_UPDATE:
        status = _DevcmdCryptoKeyUpdate(req, req_data, resp, resp_data);
        break;

    default:

        /*
         * Other devcmds are expected to come thru per-LIF adminQ.
         * However, DOL does not use adminQ so allow it to enter here,
         * albeit with the base LIF only.
         */
        status = _AdminCmdHandler(lif_base, req, req_data, resp, resp_data);
        break;
    }

    cpl->cpl.status = status;
    cpl->cpl.rsvd = 0xff;

    NIC_LOG_DEBUG("{}: Done cmd: {}, status: {}", DevNameGet(),
                  accel_dev_opcode_str(cmd->cmd.opcode), status);

    return status;
}

void
AccelDev::IntrClear(void)
{
    intr_reset_dev(intr_base, spec->intr_count, 1);
}

accel_status_code_t
AccelDev::_AdminCmdHandler(uint64_t lif_id,
                           void *req,
                           void *req_data,
                           void *resp,
                           void *resp_data)
{
    AccelLif    *lif = _LifFind(lif_id);
    accel_status_code_t status = ACCEL_RC_ENOENT;

    if (lif) {
        status = lif->CmdHandler(req, req_data, resp, resp_data);
    }

    return status;
}

accel_status_code_t
AccelDev::_DevcmdReset(void *req,
                       void *req_data,
                       void *resp,
                       void *resp_data)
{

    accel_status_code_t status = ACCEL_RC_SUCCESS;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_RESET", DevNameGet());
    IntrClear();

    for (auto iter = lif_map.cbegin(); iter != lif_map.cend(); iter++) {
        AccelLif *lif = iter->second;

        /*
         * Reset at the device level will reset and delete the LIF.
         */
        status = lif->reset(true);
        if (status != ACCEL_RC_SUCCESS) {
            break;
        }
    }

    return status;
}

accel_status_code_t
AccelDev::_DevcmdIdentify(void *req,
                          void *req_data,
                          void *resp,
                          void *resp_data)
{
    identity_t      *rsp = (identity_t *)resp_data;
    identify_cpl_t  *cpl = (identify_cpl_t *)resp;
    std::string sn;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_IDENTIFY", DevNameGet());

    memset(&devcmd->data[0], 0, sizeof(devcmd->data));
    const uint32_t sta_ver = READ_REG32(CAP_ADDR_BASE_MS_MS_OFFSET +
                                        CAP_MS_CSR_STA_VER_BYTE_ADDRESS);
    rsp->dev.asic_type = sta_ver & 0xf;
    rsp->dev.asic_rev  = (sta_ver >> 4) & 0xfff;

    if (platform_is_hw(pd->platform_)) {
        sdk::platform::readFruKey(SERIALNUMBER_KEY, sn);
        strncpy0(rsp->dev.serial_num, sn.c_str(), sizeof(rsp->dev.serial_num));

        boost::property_tree::ptree ver;
        boost::property_tree::read_json(VERSION_FILE, ver);
        strncpy0(rsp->dev.fw_version, ver.get<std::string>("sw.version").c_str(),
                 sizeof(rsp->dev.fw_version));
    } else {
        strncpy0(rsp->dev.serial_num, "sim", sizeof(rsp->dev.serial_num));
        strncpy0(rsp->dev.fw_version, "0.1.0-sim", sizeof(rsp->dev.fw_version));
    }

    AccelLif *lif = _LifFind(lif_base);
    rsp->dev.num_lifs = spec->lif_count;
    memset(&rsp->dev.lif_tbl[0], 0, sizeof(identify_lif_t));
    rsp->dev.lif_tbl[0].hw_lif_id = lif_base;
    rsp->dev.lif_tbl[0].hw_lif_local_dbaddr =
             ACCEL_LIF_LOCAL_DBADDR_SET(lif_base, STORAGE_SEQ_QTYPE_SQ);
    rsp->dev.lif_tbl[0].hw_host_prefix = ACCEL_PHYS_ADDR_HOST_SET(1) |
                                         ACCEL_PHYS_ADDR_LIF_SET(lif_base);
    rsp->dev.lif_tbl[0].hw_host_mask = ACCEL_PHYS_ADDR_HOST_SET(ACCEL_PHYS_ADDR_HOST_MASK) |
                                       ACCEL_PHYS_ADDR_LIF_SET(ACCEL_PHYS_ADDR_LIF_MASK);
    rsp->dev.lif_tbl[0].hw_key_idx_base = crypto_key_idx_base;
    rsp->dev.lif_tbl[0].num_crypto_keys_max = num_crypto_keys_max;
    rsp->dev.db_pages_per_lif = 1;
    rsp->dev.admin_queues_per_lif = spec->adminq_count;
    rsp->dev.seq_queues_per_lif = lif->SeqCreatedCountGet();
    rsp->dev.num_intrs = spec->intr_count;
    rsp->dev.intr_assert_stride = intr_assert_stride();
    rsp->dev.intr_assert_data = intr_assert_data();
    rsp->dev.intr_assert_addr = intr_assert_addr(intr_base);
    rsp->dev.cm_base_pa = cmb_mem_addr;
    memcpy(rsp->dev.accel_ring_tbl, lif->AccelRingTableGet(),
           sizeof(rsp->dev.accel_ring_tbl));
    cpl->ver = IDENTITY_VERSION_1;

    NIC_LOG_DEBUG("{} local_dbaddr {:#x} host_prefix {:#x} host_mask {:#x} size {}",
                  DevNameGet(), rsp->dev.lif_tbl[0].hw_lif_local_dbaddr,
                  rsp->dev.lif_tbl[0].hw_host_prefix,
                  rsp->dev.lif_tbl[0].hw_host_mask, (int)sizeof(rsp->dev));

    return ACCEL_RC_SUCCESS;
}

accel_status_code_t
AccelDev::_DevcmdCryptoKeyUpdate(void *req,
                                 void *req_data,
                                 void *resp,
                                 void *resp_data)
{
    crypto_key_update_cmd_t     *cmd = (crypto_key_update_cmd_t *)req;
    crypto_key_accum_t          *key_accum;
    uint8_t                     *dst_key_data;
    int                         ret_val = 0;

    NIC_LOG_DEBUG("{}  key_index {} key_type {} key_size {} key_part {}",
                  DevNameGet(), cmd->key_index, cmd->key_type,
                  cmd->key_size, cmd->key_part);

    if (cmd->key_type >= CMD_CRYPTO_KEY_TYPE_MAX) {
        NIC_LOG_ERR("{}: unrecognized key_type {}", DevNameGet(),
                    cmd->key_type);
        return ACCEL_RC_ERANGE;
    }

    if (cmd->key_size > CMD_CRYPTO_KEY_PART_SIZE) {
        NIC_LOG_ERR("{}: invalid key_size {}", DevNameGet(), cmd->key_size);
        return ACCEL_RC_ERANGE;
    }

    if (cmd->key_part >= CMD_CRYPTO_KEY_PART_MAX) {
        NIC_LOG_ERR("{}: invalid key_part {}", DevNameGet(), cmd->key_part);
        return ACCEL_RC_ERANGE;
    }

    key_accum = crypto_key_accum_find_add(cmd->key_index);
    if (!key_accum) {
        NIC_LOG_ERR("{}: unable to obtain key accumulator for key_index {}",
                    DevNameGet(), cmd->key_index);
        return ACCEL_RC_ENOMEM;
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
        ACCEL_DEVAPI_CHECK(ACCEL_RC_ERROR);
        ret_val = dev_api->crypto_key_index_upd(cmd->key_index,
                           (crypto_key_type_t)crypto_key_type_tbl[cmd->key_type],
                           &key_accum->key_data[0][0],
                           key_accum->num_key_parts * CMD_CRYPTO_KEY_PART_SIZE);
        /*
         * Wipe and erase accumulator
         */
        memset(key_accum, 0, sizeof(*key_accum));
        crypto_key_accum_del(cmd->key_index);
        if (ret_val != SDK_RET_OK) {
            NIC_LOG_ERR("{}: failed to update crypto key for key_index {}",
                        DevNameGet(), cmd->key_index);
            return ACCEL_RC_ERROR;
        }
    }

    return ACCEL_RC_SUCCESS;
}

AccelLif *
AccelDev::_LifFind(uint64_t lif_id)
{
    auto iter = lif_map.find(lif_id);
    if (iter != lif_map.end()) {
        return iter->second;
    }
    return nullptr;
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

