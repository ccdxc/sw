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

#include "nic/include/base.hpp"
#include "gen/proto/nicmgr/accel_metrics.pb.h"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

#ifdef __aarch64__
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#endif
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"


#include "logger.hpp"
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

/*
 * LIFs iterator
 */
#define FOR_EACH_ACCEL_DEV_LIF(idx, lif)                                        \
    for (idx = 0, lif = _LifFind(0); lif; idx++, lif = _LifFind(idx))

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
                   PdClient *pd_client,
                   EV_P) :
    spec((accel_devspec_t *)dev_spec),
    pd(pd_client),
    dev_api(dapi),
    delphi_mounted(false),
    EV_A(EV_A)
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

    // Allocate & Init Device registers
    regs_mem_addr = pd->devcmd_mem_alloc(sizeof(dev_regs_t));
    if (regs_mem_addr == 0) {
        NIC_LOG_ERR("{}: Failed to allocate registers", DevNameGet());
        throw;
    }
    devcmd_mem_addr = regs_mem_addr + offsetof(dev_regs_t, devcmd);

    // TODO: mmap instead of calloc after porting to real pal
    regs = (dev_regs_t *)calloc(1, sizeof(dev_regs_t));
    if (regs == NULL) {
        NIC_LOG_ERR("{}: Failed to map register region", DevNameGet());
        throw;
    }
    devcmd = &regs->devcmd;

    NIC_LOG_DEBUG("{}: regs_mem_addr {:#x} devcmd_mem_addr {:#x}",
                  DevNameGet(), regs_mem_addr, devcmd_mem_addr);
    _DevInfoRegsInit();

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
    lif_res.index = 0;
    lif_res.intr_base = intr_base;
    lif_res.cmb_mem_addr = cmb_mem_addr;
    lif_res.cmb_mem_size = cmb_mem_size;
    auto lif = new AccelLif(*this, lif_res, EV_DEFAULT);
    if (!lif) {
        NIC_LOG_ERR("{}: failed to create AccelLif {}",
                    DevNameGet(), lif_res.lif_id);
        throw;
    }
    lif_vec.push_back(lif);

    _MetricsInit();
    memset(&devcmd_timer, 0, sizeof(devcmd_timer));
    evutil_timer_start(EV_A_ &devcmd_timer, &AccelDev::_DevcmdPoll, this, 0.0, 0.01);
}

AccelDev::~AccelDev()
{
    AccelLif    *lif;
    uint32_t    i;

    /*
     * Most HBM related allocs don't have corresponding free API so
     * we'll leave them alone. Same with intr_alloc().
     */
    evutil_timer_stop(EV_A_ &devcmd_timer);
    _MetricsFini();

    FOR_EACH_ACCEL_DEV_LIF(i, lif) {
        delete lif;
    }
    lif_vec.clear();

    _DestroyHostDevice();
    pd->lm_->free_id(lif_base, spec->lif_count);
}

void
AccelDev::HalEventHandler(bool status)
{
    AccelLif    *lif;
    uint32_t    i;

    FOR_EACH_ACCEL_DEV_LIF(i, lif) {
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
    AccelLif    *lif;
    uint32_t    i;

    dev_api = dapi;
    FOR_EACH_ACCEL_DEV_LIF(i, lif) {
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
    pciehdevice_resources_t pres;

    memset(&pres, 0, sizeof(pres));
    pres.type = PCIEHDEVICE_ACCEL;
    strncpy0(pres.pfres.name, DevNameGet().c_str(), sizeof(pres.pfres.name));
    pres.pfres.port = spec->pcie_port;
    pres.pfres.lifb = lif_base;
    pres.pfres.lifc = spec->lif_count;
    pres.pfres.intrb = intr_base;
    pres.pfres.intrc = spec->intr_count;
    pres.pfres.intrdmask = 1;
    pres.pfres.npids = 1;
    pres.pfres.cmbpa = cmb_mem_addr;
    pres.pfres.cmbsz = cmb_mem_size;
    pres.pfres.cmbprefetch = true;
    pres.pfres.accel.devregspa = regs_mem_addr;
    pres.pfres.accel.devregssz = sizeof(dev_regs_t);

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
AccelDev::_DestroyHostDevice(void)
{
}

void
AccelDev::_DevInfoRegsInit(void)
{
    const uint32_t sta_ver = READ_REG32(HW_CHIP_VER_CSR);
    std::string sn;

    regs->info.base.signature = ACCEL_DEV_CMD_SIGNATURE;
    regs->info.base.version = 0x1;
    regs->info.base.asic_type = HW_ASIC_TYPE_GET(sta_ver);
    regs->info.base.asic_rev  = HW_ASIC_REV_GET(sta_ver);
    if (platform_is_hw(pd->platform_)) {
        sdk::platform::readFruKey(SERIALNUMBER_KEY, sn);
        strncpy0(regs->info.base.serial_num, sn.c_str(),
                 sizeof(regs->info.base.serial_num));

        boost::property_tree::ptree ver;
        boost::property_tree::read_json(VERSION_FILE, ver);
        strncpy0(regs->info.base.fw_version,
                 ver.get<std::string>("sw.version").c_str(),
                 sizeof(regs->info.base.fw_version));
    } else {
        strncpy0(regs->info.base.serial_num, "sim",
                 sizeof(regs->info.base.serial_num));
        strncpy0(regs->info.base.fw_version, "0.1.0-sim",
                 sizeof(regs->info.base.fw_version));
    }
    WRITE_MEM(regs_mem_addr, (uint8_t *)regs, sizeof(*regs), 0);
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

    READ_MEM(dev->devcmd_mem_addr, (uint8_t *)&db, sizeof(db), 0);
    if (db.v & 1) {
        NIC_LOG_INFO("{}: Devcmd doorbell", dev->DevNameGet());
        WRITE_MEM(dev->devcmd_mem_addr, (uint8_t *)&db_clear,
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

    status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->cpl, &devcmd->data);

    // write data
    if (status == ACCEL_RC_SUCCESS) {
        WRITE_MEM(devcmd_mem_addr + offsetof(dev_cmd_regs_t, data),
                  (uint8_t *)devcmd->data, sizeof(devcmd->data), 0);
    }

devcmd_done:
    devcmd->cpl.status = status;
    devcmd->done = 1;

    // write completion
    WRITE_MEM(devcmd_mem_addr + offsetof(dev_cmd_regs_t, cpl),
              (uint8_t *)&devcmd->cpl, sizeof(devcmd->cpl), 0);

    // write done
    WRITE_MEM(devcmd_mem_addr + offsetof(dev_cmd_regs_t, done),
              (uint8_t *)&devcmd->done, sizeof(devcmd->done), 0);

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
        status = _AdminCmdHandler(cmd->cmd.lif_index, req, req_data,
                                  resp, resp_data);
        break;
    }

    cpl->status = status;
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
AccelDev::_AdminCmdHandler(uint32_t lif_index,
                           void *req,
                           void *req_data,
                           void *resp,
                           void *resp_data)
{
    auto lif = _LifFind(lif_index);
    if (!lif) {
        NIC_LOG_ERR("{}: lif {} unexpectedly missing", DevNameGet(), lif_index);
        return ACCEL_RC_ENOENT;
    }

    return lif->CmdHandler(req, req_data, resp, resp_data);
}

accel_status_code_t
AccelDev::_DevcmdReset(void *req,
                       void *req_data,
                       void *resp,
                       void *resp_data)
{

    AccelLif            *lif;
    accel_status_code_t status = ACCEL_RC_SUCCESS;
    uint32_t            i;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_RESET", DevNameGet());
    IntrClear();

    FOR_EACH_ACCEL_DEV_LIF(i, lif) {

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
    dev_identify_cmd_t  *cmd = (dev_identify_cmd_t *)req;
    dev_identity_t      *rsp = (dev_identity_t *)resp_data;
    dev_identify_cpl_t  *cpl = (dev_identify_cpl_t *)resp;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_IDENTIFY", DevNameGet());

    if (cmd->type >= ACCEL_DEV_TYPE_MAX) {
        NIC_LOG_ERR("{}: bad device type {}", DevNameGet(), cmd->type);
        return ACCEL_RC_EINVAL;
    }
    if (cmd->ver != IDENTITY_VERSION_1) {
        NIC_LOG_ERR("{}: unsupported version {}", DevNameGet(), cmd->ver);
        return ACCEL_RC_EVERSION;
    }
    memset(&rsp->base, 0, sizeof(rsp->base));
    rsp->base.version = IDENTITY_VERSION_1;
    rsp->base.nlifs = spec->lif_count;
    rsp->base.ndbpgs_per_lif = 1;
    rsp->base.nintrs = spec->intr_count;

    int mul, div;
    intr_coal_get_params(&mul, &div);
    rsp->base.intr_coal_mult = mul;
    rsp->base.intr_coal_div = div;
    rsp->base.intr_assert_stride = intr_assert_stride();
    rsp->base.intr_assert_data = intr_assert_data();
    rsp->base.intr_assert_addr = intr_assert_addr(intr_base);
    rsp->base.cm_base_pa = cmb_mem_addr;

    cpl->ver = IDENTITY_VERSION_1;
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
AccelDev::_LifFind(uint32_t lif_index)
{
    if (lif_index >= lif_vec.size()) {
        return nullptr;
    }
    return lif_vec.at(lif_index);
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

