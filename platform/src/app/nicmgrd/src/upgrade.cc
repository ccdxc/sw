//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <sys/stat.h>
#include "gen/proto/nicmgr/nicmgr.pb.h"

#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"
#include "nic/include/hal_cfg.hpp"

#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"

#include "upgrade.hpp"
#include "nicmgr_delphic_ipc.hpp"
#include "upgrade_rel_a2b.hpp"

using namespace upgrade;
using namespace nicmgr;

extern DeviceManager *devmgr;
extern char* nicmgr_upgrade_state_file;
extern char* nicmgr_rollback_state_file;

evutil_timer ServiceTimer;
UpgradeEvent UpgEvent;
UpgradeState ExpectedState;
uint32_t ServiceTimeout;
bool MoveToNextState;
bool SendAppResp;
bool IsUpgFailed = false;

namespace nicmgr {

static nicmgr_upg_hndlr *upg_handler;

#define NICMGR_BKUP_SHM_NAME       "nicmgr_upgdata"
#define NICMGR_BKUP_SHM_SIZE       (50 * 1024)
#define NICMGR_BKUP_OBJ_MEM_OFFSET (256)
#define NICMGR_BKUP_DIR            "/update/"

typedef enum backup_obj_id_s {
    NICMGR_BKUP_OBJ_INVALID = 0,
    NICMGR_BKUP_OBJ_DEVINFO_ID,
    NICMGR_BKUP_OBJ_UPLINKINFO_ID
} backup_obj_id_t;

typedef struct __attribute__((packed)) backup_mem_hdr_s {
    backup_obj_id_t id;           //< objects id
    uint32_t        obj_count;    //< number of objects saved for this id
    uint32_t        mem_offset;   //< pointer to the first object
} backup_mem_hdr_t;


#define NICMGR_BKUP_PBUF(pbuf_info, ret, str) {                            \
    uint32_t pbuf_byte_size = pbuf_info.ByteSizeLong();              \
    if ((obj_mem_offset_ + pbuf_byte_size + 4) > obj_mem_size_) {     \
        NIC_LOG_ERR("Failed to serialize {}", str);                   \
        ret = SDK_RET_OOM;                                            \
    }                                                                 \
    *(uint32_t *)&mem_[obj_mem_offset_] = pbuf_byte_size;             \
    if (pbuf_info.SerializeToArray(&mem_[obj_mem_offset_ + 4],       \
                                  pbuf_byte_size) == false) {         \
        NIC_LOG_ERR("Failed to serialize {}", str);                   \
        ret = SDK_RET_OOM;                                            \
    }                                                                 \
    obj_mem_offset_ += (pbuf_byte_size + 4);                          \
}

#define NICMGR_RESTORE_PBUF(pbuf_info, cb, arg, oid, ret, str) {                                \
    uint32_t pbuf_byte_size;                                           \
    backup_mem_hdr_t *hdr = (backup_mem_hdr_t *)mem_;    \
                                                                       \
    obj_mem_offset_ = hdr[oid].mem_offset;                             \
    if (hdr[oid].obj_count) {                                                  \
        SDK_ASSERT(hdr[oid].id == oid);                                  \
    }                                                                  \
    for (uint32_t i = 0; i < hdr[oid].obj_count; i++) {                        \
        pbuf_byte_size =  *(uint32_t *)&mem_[obj_mem_offset_];         \
                                                                       \
        if (pbuf_info.ParseFromArray(&mem_[obj_mem_offset_ + 4],       \
                                     pbuf_byte_size) == false) {       \
            NIC_LOG_ERR("Failed to de-serialize {}", str);           \
            ret = SDK_RET_OOM;                                        \
        }                                                              \
        cb(&pbuf_info, arg);                                                 \
        obj_mem_offset_ += (pbuf_byte_size + 4);                            \
    } \
}

static int
writefile(const char *file, const void *buf, const size_t bufsz)
{
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        goto error_out;
    }
    if (fwrite(buf, bufsz, 1, fp) != 1) {
        goto error_out;
    }
    if (fclose(fp) == EOF) {
        fp = NULL;
        goto error_out;
    }
    return bufsz;

 error_out:
    if (fp) (void)fclose(fp);
    (void)unlink(file);
    return -1;
}

static int
copyfile(const char *srcfile, const char *dstfile)
{
    FILE *srcfp, *dstfp;
    size_t n, nbytes;
    char buf[BUFSIZ];

    srcfp = NULL;
    dstfp = NULL;

    srcfp = fopen(srcfile, "r");
    if (srcfp == NULL) {
        NIC_LOG_ERR("Src file open failed");
        goto error_out;
    }
    dstfp = fopen(dstfile, "w");
    if (dstfp == NULL) {
        NIC_LOG_ERR("Dst file open failed");
        goto error_out;
    }

    nbytes = 0;
    while ((n = fread(buf, 1, sizeof(buf), srcfp)) > 0) {
        if (fwrite(buf, 1, n, dstfp) != n) {
            NIC_LOG_ERR("Fwrite failed for size {}, nbytes {}", n, nbytes);
            goto error_out;
        }
        nbytes += n;
    }
    NIC_LOG_DEBUG("Written src {} dst {} bytes {}", srcfile, dstfile, nbytes);
    if (ferror(srcfp)) {
        NIC_LOG_ERR("Src fp has error bit set");
        goto error_out;
    }
    if (fclose(dstfp) == EOF) {
        NIC_LOG_ERR("Dst fp close failed");
        dstfp = NULL;
        goto error_out;
    }
    (void)fclose(srcfp);

    return nbytes;

 error_out:
    if (srcfp) fclose(srcfp);
    if (dstfp) fclose(dstfp);
    return -1;
}

static bool
file_exist (const char *fname)
{
    FILE *fp;

    fp = fopen(fname, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

void
SaveUplinkInfo(uplink_t *up, UplinkInfo *proto_obj)
{
    proto_obj->set_key(up->id);
    proto_obj->set_id(up->id);
    proto_obj->set_port(up->port);
    proto_obj->set_is_oob(up->is_oob);
    NIC_LOG_DEBUG("Saving uplink id {} port {}", proto_obj->id(), proto_obj->port());
}

void
SaveEthDevInfo(struct EthDevInfo *eth_dev_info, EthDeviceInfo *proto_obj)
{
    //device name is the key
    proto_obj->set_key(eth_dev_info->eth_spec->name);

    //save eth_dev resources
    proto_obj->mutable_eth_dev_res()->set_lif_base(eth_dev_info->eth_res->lif_base);
    proto_obj->mutable_eth_dev_res()->set_intr_base(eth_dev_info->eth_res->intr_base);
    proto_obj->mutable_eth_dev_res()->set_regs_mem_addr(eth_dev_info->eth_res->regs_mem_addr);
    proto_obj->mutable_eth_dev_res()->set_port_info_addr(eth_dev_info->eth_res->port_info_addr);
    proto_obj->mutable_eth_dev_res()->set_cmb_mem_addr(eth_dev_info->eth_res->cmb_mem_addr);
    proto_obj->mutable_eth_dev_res()->set_cmb_mem_size(eth_dev_info->eth_res->cmb_mem_size);
    proto_obj->mutable_eth_dev_res()->set_rom_mem_addr(eth_dev_info->eth_res->rom_mem_addr);
    proto_obj->mutable_eth_dev_res()->set_rom_mem_size(eth_dev_info->eth_res->rom_mem_size);

    //save eth_dev specs
    proto_obj->mutable_eth_dev_spec()->set_dev_uuid(eth_dev_info->eth_spec->dev_uuid);
    proto_obj->mutable_eth_dev_spec()->set_eth_type(eth_dev_info->eth_spec->eth_type);
    proto_obj->mutable_eth_dev_spec()->set_name(eth_dev_info->eth_spec->name);
    proto_obj->mutable_eth_dev_spec()->set_oprom(eth_dev_info->eth_spec->oprom);
    proto_obj->mutable_eth_dev_spec()->set_pcie_port(eth_dev_info->eth_spec->pcie_port);
    proto_obj->mutable_eth_dev_spec()->set_pcie_total_vfs(eth_dev_info->eth_spec->pcie_total_vfs);
    proto_obj->mutable_eth_dev_spec()->set_host_dev(eth_dev_info->eth_spec->host_dev);
    proto_obj->mutable_eth_dev_spec()->set_uplink_port_num(eth_dev_info->eth_spec->uplink_port_num);
    proto_obj->mutable_eth_dev_spec()->set_qos_group(eth_dev_info->eth_spec->qos_group);
    proto_obj->mutable_eth_dev_spec()->set_lif_count(eth_dev_info->eth_spec->lif_count);
    proto_obj->mutable_eth_dev_spec()->set_rxq_count(eth_dev_info->eth_spec->rxq_count);
    proto_obj->mutable_eth_dev_spec()->set_txq_count(eth_dev_info->eth_spec->txq_count);
    proto_obj->mutable_eth_dev_spec()->set_eq_count(eth_dev_info->eth_spec->eq_count);
    proto_obj->mutable_eth_dev_spec()->set_adminq_count(eth_dev_info->eth_spec->adminq_count);
    proto_obj->mutable_eth_dev_spec()->set_intr_count(eth_dev_info->eth_spec->intr_count);
    proto_obj->mutable_eth_dev_spec()->set_mac_addr(eth_dev_info->eth_spec->mac_addr);
    proto_obj->mutable_eth_dev_spec()->set_enable_rdma(eth_dev_info->eth_spec->enable_rdma);
    proto_obj->mutable_eth_dev_spec()->set_pte_count(eth_dev_info->eth_spec->pte_count);
    proto_obj->mutable_eth_dev_spec()->set_key_count(eth_dev_info->eth_spec->key_count);
    proto_obj->mutable_eth_dev_spec()->set_ah_count(eth_dev_info->eth_spec->ah_count);
    proto_obj->mutable_eth_dev_spec()->set_rdma_sq_count(eth_dev_info->eth_spec->rdma_sq_count);
    proto_obj->mutable_eth_dev_spec()->set_rdma_rq_count(eth_dev_info->eth_spec->rdma_rq_count);
    proto_obj->mutable_eth_dev_spec()->set_rdma_cq_count(eth_dev_info->eth_spec->rdma_eq_count);
    proto_obj->mutable_eth_dev_spec()->set_rdma_eq_count(eth_dev_info->eth_spec->rdma_eq_count);
    proto_obj->mutable_eth_dev_spec()->set_rdma_aq_count(eth_dev_info->eth_spec->rdma_aq_count);
    proto_obj->mutable_eth_dev_spec()->set_rdma_pid_count(eth_dev_info->eth_spec->rdma_pid_count);
    proto_obj->mutable_eth_dev_spec()->set_barmap_size(eth_dev_info->eth_spec->barmap_size);

    NIC_LOG_DEBUG("Saving eth dev {}", eth_dev_info->eth_spec->name);
    return;
}

static void
upgrade_state_save_delphi (void)
{
    nicmgr_delphic_msg_t ethdev_msg;
    nicmgr_delphic_msg_t uplink_msg;


    ethdev_msg.msg_id = NICMGR_DELPHIC_MSG_SET_UPG_ETHDEVINFO;

    NIC_LOG_DEBUG("SaveState sending IPC to save ethdevinfo Delphi");
    // sending async upgrade status event to hal from nicmgr
    sdk::ipc::request(hal::HAL_THREAD_ID_DELPHI_CLIENT,
                          event_id_t::EVENT_ID_NICMGR_DELPHIC,
                          &ethdev_msg, sizeof(ethdev_msg), NULL);


    uplink_msg.msg_id = NICMGR_DELPHIC_MSG_SET_UPG_UPLINKINFO;

    NIC_LOG_DEBUG("SaveState sending IPC to save uplinkinfo Delphi");
    // sending async upgrade status event to hal from nicmgr
    sdk::ipc::request(hal::HAL_THREAD_ID_DELPHI_CLIENT,
                          event_id_t::EVENT_ID_NICMGR_DELPHIC,
                          &uplink_msg, sizeof(uplink_msg), NULL);
}

// returns void, as save state fails only in case of out of memory.
void
nicmgr_upg_hndlr::upgrade_state_save(void) {
    backup_mem_hdr_t *hdr = (backup_mem_hdr_t *)mem_;
    sdk_ret_t ret = SDK_RET_OK;
    UplinkInfo uplinkinfo;
    EthDeviceInfo devinfo;
    std::vector <struct EthDevInfo*> dev_info;
    std::map<uint32_t, uplink_t*> up_links;
    struct stat st = { 0 };
    std::string dst = std::string(NICMGR_BKUP_DIR) + std::string(NICMGR_BKUP_SHM_NAME);
    std::string src = std::string("/dev/shm/") + std::string(NICMGR_BKUP_SHM_NAME);

    dev_info = devmgr->GetEthDevStateInfo();
    NIC_FUNC_DEBUG("Saving {} objects of EthDevInfo to shm", dev_info.size());

    hdr[NICMGR_BKUP_OBJ_DEVINFO_ID].id = NICMGR_BKUP_OBJ_DEVINFO_ID;
    hdr[NICMGR_BKUP_OBJ_DEVINFO_ID].obj_count = dev_info.size();
    hdr[NICMGR_BKUP_OBJ_DEVINFO_ID].mem_offset = obj_mem_offset_;

    //for each element in dev_info convert it to protobuf and then setobject to shm
    for (uint32_t eth_dev_idx = 0; eth_dev_idx < dev_info.size(); eth_dev_idx++) {
        SaveEthDevInfo(dev_info[eth_dev_idx], &devinfo);
        NICMGR_BKUP_PBUF(devinfo, ret, "ethdev");
        SDK_ASSERT(ret == SDK_RET_OK);
    }

    up_links = devmgr->GetUplinks();
    NIC_FUNC_DEBUG("Saving {} objects of UplinkInfo to shm", up_links.size());

    hdr[NICMGR_BKUP_OBJ_UPLINKINFO_ID].id = NICMGR_BKUP_OBJ_UPLINKINFO_ID;
    hdr[NICMGR_BKUP_OBJ_UPLINKINFO_ID].obj_count = up_links.size();
    hdr[NICMGR_BKUP_OBJ_UPLINKINFO_ID].mem_offset = obj_mem_offset_;

    for (auto it = up_links.begin(); it != up_links.end(); it++) {
        uplink_t *up = it->second;
        SaveUplinkInfo(up, &uplinkinfo);
        NICMGR_BKUP_PBUF(uplinkinfo, ret, "uplink");
        SDK_ASSERT(ret == SDK_RET_OK);
    }

    // check if the bkup dir exists
    if (stat(NICMGR_BKUP_DIR, &st) == -1) {
        // doesn't exist, try to create
        if (mkdir(NICMGR_BKUP_DIR, 0755) < 0) {
            NIC_LOG_ERR("Backup directory %s/ doesn't exist, failed to create one\n",
                          NICMGR_BKUP_DIR);
        }
    }

    if ((copyfile(src.c_str(), dst.c_str())) == -1) {
        NIC_LOG_ERR("Saving state file failed, src {} to dst {}", src, dst);
    } else {
        NIC_LOG_DEBUG("Saving state file completed, src {} to dst {}", src, dst);
    }
}


// Constructor method
nicmgr_upg_hndlr::nicmgr_upg_hndlr()
{
}

// Perform compat check
HdlrResp
nicmgr_upg_hndlr::CompatCheckHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    IsUpgFailed = false;

    NIC_LOG_INFO("Upgrade: CompatCheck");

    if (!devmgr->UpgradeCompatCheck())
        resp.resp=FAIL;

    return resp;
}

HdlrResp
nicmgr_upg_hndlr::ProcessQuiesceHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: ProcessQuiesce");

    return resp;
}

// Bring link-down
HdlrResp
nicmgr_upg_hndlr::LinkDownHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=INPROGRESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: LinkDown");

    if (devmgr->HandleUpgradeEvent(UPG_EVENT_QUIESCE)) {
        NIC_FUNC_DEBUG("UPG_EVENT_QUIESCE event Failed! Cannot continue upgrade FSM");
        resp.resp = FAIL;
        return resp;
    }

    NIC_FUNC_DEBUG("Starting timer thread to check whether all devices are quiesced or not ...");

    ExpectedState = DEVICES_QUIESCED_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = false;
    SendAppResp = true;
    evutil_timer_start(devmgr->ev_loop(), &ServiceTimer, nicmgr_upg_hndlr::upg_timer_func, this, 0.0, 0.1);

    return resp;
}

// Post-binary restart handling
HdlrResp
nicmgr_upg_hndlr::PostRestartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: PostRestart");

    return resp;
}

// Bring link-up
HdlrResp
nicmgr_upg_hndlr::LinkUpHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: LinkUp");

    return resp;
}

// Wipeout state
void
nicmgr_upg_hndlr::ResetState(UpgCtx& upgCtx)
{
}

// Handle upgrade success
void
nicmgr_upg_hndlr::SuccessHandler(UpgCtx& upgCtx)
{
    nicmgr_delphic_msg_t ethdev_msg;
    nicmgr_delphic_msg_t uplink_msg;

    NIC_LOG_INFO("Upgrade: Success");

    ethdev_msg.msg_id = NICMGR_DELPHIC_MSG_CLR_UPG_ETHDEVINFO;
    // sending async upgrade status event to hal from nicmgr
    sdk::ipc::request(hal::HAL_THREAD_ID_DELPHI_CLIENT,
                          event_id_t::EVENT_ID_NICMGR_DELPHIC,
                          &ethdev_msg, sizeof(ethdev_msg), NULL);

    NIC_LOG_INFO("Clearing ethdevinfo in Delphi");

    uplink_msg.msg_id = NICMGR_DELPHIC_MSG_CLR_UPG_UPLINKINFO;
    // sending async upgrade status event to hal from nicmgr
    sdk::ipc::request(hal::HAL_THREAD_ID_DELPHI_CLIENT,
                          event_id_t::EVENT_ID_NICMGR_DELPHIC,
                          &uplink_msg, sizeof(uplink_msg), NULL);

    NIC_LOG_INFO("Clearing uplinkinfo in Delphi");

    ResetState(upgCtx);
}

// Handle upgrade failure
HdlrResp
nicmgr_upg_hndlr::FailedHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    IsUpgFailed = true;

    NIC_LOG_INFO("Upgrade: Failed");

    ResetState(upgCtx);

    unlink(nicmgr_upgrade_state_file);

    NIC_LOG_DEBUG("Previous execution state {}", UpgCtxApi::UpgCtxGetUpgState(upgCtx));
    if (UpgCtxApi::UpgCtxGetUpgState(upgCtx) == CompatCheck) {
        NIC_FUNC_DEBUG("FW upgrade failed during compat check");
        return resp;
    }

    // If fw upgrade failed after compat_check then we will rollback to running fw
    writefile(nicmgr_rollback_state_file, "in progress", 11);
    resp.resp = INPROGRESS;

    upg_handler->upgrade_state_save();

    // we will run through all the state machine for nicmgr upgrade if somehow upgrade failed
    NIC_FUNC_DEBUG("Sending LinkDown event to eth drivers");
    if (devmgr->HandleUpgradeEvent(UPG_EVENT_QUIESCE)) {
        NIC_FUNC_DEBUG("UPG_EVENT_QUIESCE event Failed! Cannot continue upgrade FSM");
        resp.resp = FAIL;
        return resp;
    }

    NIC_FUNC_DEBUG("Waiting for LinkDown event response from eth drivers");

    ExpectedState = DEVICES_QUIESCED_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = true;
    SendAppResp = false;
    evutil_timer_start(devmgr->ev_loop(), &ServiceTimer, nicmgr_upg_hndlr::upg_timer_func, this, 0.0, 0.1);

    return resp;
}

// Handle upgrade abort
void
nicmgr_upg_hndlr::AbortHandler(UpgCtx& upgCtx)
{
    NIC_LOG_INFO("Upgrade: Abort");
}

static void
restore_eth_device_info_cb (EthDeviceInfo *EthDevProtoObj, void *arg)
{
    struct EthDevInfo *eth_dev_info = new EthDevInfo();
    struct eth_devspec *eth_spec = new eth_devspec();
    struct eth_dev_res *eth_res = new eth_dev_res();

    //populate the eth_res
    eth_res->lif_base = EthDevProtoObj->eth_dev_res().lif_base();
    eth_res->intr_base = EthDevProtoObj->eth_dev_res().intr_base();
    eth_res->regs_mem_addr = EthDevProtoObj->eth_dev_res().regs_mem_addr();
    eth_res->port_info_addr = EthDevProtoObj->eth_dev_res().port_info_addr();
    eth_res->cmb_mem_addr = EthDevProtoObj->eth_dev_res().cmb_mem_addr();
    eth_res->cmb_mem_size = EthDevProtoObj->eth_dev_res().cmb_mem_size();
    eth_res->rom_mem_addr = EthDevProtoObj->eth_dev_res().rom_mem_addr();
    eth_res->rom_mem_size = EthDevProtoObj->eth_dev_res().rom_mem_size();

    //populate the eth_spec
    eth_spec->dev_uuid = EthDevProtoObj->eth_dev_spec().dev_uuid();
    eth_spec->eth_type = (EthDevType)EthDevProtoObj->eth_dev_spec().eth_type();
    eth_spec->name = EthDevProtoObj->eth_dev_spec().name();
    eth_spec->oprom = (OpromType)EthDevProtoObj->eth_dev_spec().oprom();
    eth_spec->pcie_port = EthDevProtoObj->eth_dev_spec().pcie_port();
    eth_spec->pcie_total_vfs = EthDevProtoObj->eth_dev_spec().pcie_total_vfs();
    eth_spec->host_dev = EthDevProtoObj->eth_dev_spec().host_dev();
    eth_spec->uplink_port_num = EthDevProtoObj->eth_dev_spec().uplink_port_num();
    eth_spec->qos_group = EthDevProtoObj->eth_dev_spec().qos_group();
    eth_spec->lif_count = EthDevProtoObj->eth_dev_spec().lif_count();
    eth_spec->rxq_count = EthDevProtoObj->eth_dev_spec().rxq_count();
    eth_spec->txq_count = EthDevProtoObj->eth_dev_spec().txq_count();
    eth_spec->adminq_count = EthDevProtoObj->eth_dev_spec().adminq_count();
    eth_spec->eq_count = EthDevProtoObj->eth_dev_spec().eq_count();

    eth_spec->intr_count = EthDevProtoObj->eth_dev_spec().intr_count();
    eth_spec->mac_addr = EthDevProtoObj->eth_dev_spec().mac_addr();
    eth_spec->enable_rdma = EthDevProtoObj->eth_dev_spec().enable_rdma();
    eth_spec->pte_count = EthDevProtoObj->eth_dev_spec().pte_count();
    eth_spec->key_count = EthDevProtoObj->eth_dev_spec().key_count();
    eth_spec->ah_count = EthDevProtoObj->eth_dev_spec().ah_count();
    eth_spec->rdma_sq_count = EthDevProtoObj->eth_dev_spec().rdma_sq_count();
    eth_spec->rdma_rq_count = EthDevProtoObj->eth_dev_spec().rdma_rq_count();
    eth_spec->rdma_cq_count = EthDevProtoObj->eth_dev_spec().rdma_cq_count();
    eth_spec->rdma_eq_count = EthDevProtoObj->eth_dev_spec().rdma_eq_count();
    eth_spec->rdma_aq_count = EthDevProtoObj->eth_dev_spec().rdma_aq_count();
    eth_spec->rdma_pid_count = EthDevProtoObj->eth_dev_spec().rdma_pid_count();
    eth_spec->barmap_size = EthDevProtoObj->eth_dev_spec().barmap_size();

    eth_dev_info->eth_res = eth_res;
    eth_dev_info->eth_spec = eth_spec;

    NIC_LOG_DEBUG("Restore eth dev {}", EthDevProtoObj->eth_dev_spec().name());
    devmgr->RestoreDevice(ETH, eth_dev_info);
}

static void
restore_uplink_info_cb (UplinkInfo *UplinkProtoObj,  void *arg)
{
    NIC_LOG_DEBUG("Restore uplink id {} port {}", UplinkProtoObj->id(), UplinkProtoObj->port());
    devmgr->CreateUplink(UplinkProtoObj->id(), UplinkProtoObj->port(), UplinkProtoObj->is_oob());
}

void
nicmgr_upg_hndlr::upg_restore_states(void)
{
    EthDeviceInfo proto_devobj;
    UplinkInfo proto_uplinkobj;
    sdk_ret_t ret = SDK_RET_OK;


    NIC_FUNC_DEBUG("Retrieving saved objects from shm");

    NICMGR_RESTORE_PBUF(proto_devobj, restore_eth_device_info_cb,
                NULL, NICMGR_BKUP_OBJ_DEVINFO_ID, ret, "ethdev");
    SDK_ASSERT(ret == SDK_RET_OK);

    NICMGR_RESTORE_PBUF(proto_uplinkobj, restore_uplink_info_cb,
                NULL, NICMGR_BKUP_OBJ_UPLINKINFO_ID, ret, "uplink");
    SDK_ASSERT(ret == SDK_RET_OK);
}

HdlrResp
nicmgr_upg_hndlr::SaveStateHandler(UpgCtx& upgCtx, upg_msg_t *msg) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    NIC_LOG_INFO("Upgrade: SaveState");

    if (msg->save_state_delphi == 1) {
        upgrade_state_save_delphi();
    } else {
        upg_handler->upgrade_state_save();
    }

    return resp;
}


HdlrResp
nicmgr_upg_hndlr::PostLinkUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: PostLinkUp");

    return resp;
}

string UpgStateToStr(UpgradeState state)
{
    switch (state) {
        case (DEVICES_ACTIVE_STATE):
            return "DEVICES_ACTIVE_STATE";
        case (DEVICES_QUIESCED_STATE):
            return "DEVICES_QUIESCED_STATE";
        case (DEVICES_RESET_STATE):
            return "DEVICES_RESET_STATE";
        default:
            return "UNKNOWN_STATE";
    }
}

void
nicmgr_upg_hndlr::upg_timer_func(void *obj)
{
    static uint32_t max_retry = 0;
    int ret = 0;
    bool status;

    if ((devmgr->GetUpgradeState() < ExpectedState) && (++max_retry < ServiceTimeout)) {
        return;
    }
    else {
        if (max_retry < ServiceTimeout)
            NIC_FUNC_DEBUG("All devices are in {} state after {} retry", UpgStateToStr(devmgr->GetUpgradeState()), max_retry);
        else
            NIC_LOG_ERR("Timeout occurred while waiting for all devices to go in {} state", ExpectedState);
    }

    if (MoveToNextState) {

        if (ExpectedState == DEVICES_QUIESCED_STATE) {
            UpgEvent = UPG_EVENT_DEVICE_RESET;
            ExpectedState = DEVICES_RESET_STATE;
            devmgr->HandleUpgradeEvent(UpgEvent);
            MoveToNextState = false;
            SendAppResp = true;
            max_retry = 0;

        }
        else if (ExpectedState == DEVICES_ACTIVE_STATE) {
            UpgEvent = UPG_EVENT_QUIESCE;
            ExpectedState = DEVICES_QUIESCED_STATE;
            devmgr->HandleUpgradeEvent(UpgEvent);
            MoveToNextState = true;
            SendAppResp = false;
            max_retry = 0;
        }
        else {
            NIC_LOG_ERR("Undefined device's next state from current state: {}", UpgStateToStr(devmgr->GetUpgradeState()));
        }

        NIC_FUNC_DEBUG("Moving upgrade state machine to next level {}", UpgStateToStr(ExpectedState));
        return;
    }

    if (SendAppResp) {
        if (ExpectedState == DEVICES_RESET_STATE) {
            if (!IsUpgFailed) {
                ret = writefile(nicmgr_upgrade_state_file, "in progress", 11);
                if (ret == -1)
                    return;
            }
        }

        NIC_FUNC_DEBUG("Sending App Response to hal upgrade client");

        if (max_retry >= ServiceTimeout) {
            status = false;
        } else {
            status = true;
        }

        // sending async upgrade status event to hal from nicmgr
        sdk::ipc::request(hal::HAL_THREAD_ID_DELPHI_CLIENT,
                          event_id_t::EVENT_ID_UPG_STAGE_STATUS,
                          &status, sizeof(status), NULL);

        evutil_timer_stop(devmgr->ev_loop(), &ServiceTimer);

        return;
    }
}

HdlrResp
nicmgr_upg_hndlr::HostDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=INPROGRESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: HostDown");

    if (devmgr->HandleUpgradeEvent(UPG_EVENT_DEVICE_RESET)) {
        NIC_FUNC_DEBUG("UPG_EVENT_DEVICE_RESET event Failed! Cannot continue upgrade FSM");
        resp.resp = FAIL;
        return resp;
    }

    NIC_FUNC_DEBUG("Starting timer thread to check whether all devices are in reset or not ...");
    ExpectedState = DEVICES_RESET_STATE;
    ServiceTimeout = 600; //60 seconds
    MoveToNextState = false;
    SendAppResp = true;
    evutil_timer_start(devmgr->ev_loop(), &ServiceTimer, nicmgr_upg_hndlr::upg_timer_func, this, 0.0, 0.1);

    return resp;
}

HdlrResp
nicmgr_upg_hndlr::PostHostDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: PostHostDown");

    return resp;
}

HdlrResp
nicmgr_upg_hndlr::HostUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    NIC_LOG_INFO("Upgrade: HostUp");

    return resp;
}



sdk_ret_t
nicmgr_upg_hndlr::upg_shm_alloc(const char *name, uint32_t size, bool create) {
    sdk::lib::shm_mode_e mode;
    std::string src = std::string(NICMGR_BKUP_DIR) + std::string(NICMGR_BKUP_SHM_NAME);
    std::string dst = std::string("/dev/shm/") + std::string(NICMGR_BKUP_SHM_NAME);

    mode = create ? sdk::lib::SHM_CREATE_ONLY : sdk::lib::SHM_OPEN_ONLY;
    if (create) {
        shmmgr::remove(name);
    } else {
        if (copyfile(src.c_str(), dst.c_str()) == -1) {
            NIC_LOG_ERR("Restoring state file failed, src {} to dst {}", src, dst);
            return SDK_RET_ERR;
        }
        NIC_LOG_DEBUG("Restoring state file completed, src {} to dst {}", src, dst);
    }
    try {
        shm_mmgr_ = shmmgr::factory(name, size + 1024, mode, NULL);
        if (shm_mmgr_ == NULL) {
            NIC_LOG_ERR("Bkup shared memory create failed");
            return SDK_RET_OOM;
        }
    } catch (...) {
        NIC_LOG_ERR("Bkup shared memory create failed");
        return SDK_RET_OOM;
    }
    try {
        mem_ = (char *)shm_mmgr_->segment_alloc("state_mem",
                                                size, create);
        if (!mem_) {
            NIC_LOG_ERR("Bkup shared memory create failed");
            return SDK_RET_OOM;
        }
    } catch (...) {
        NIC_LOG_ERR("Bkup shared memory create failed");
        return SDK_RET_OOM;
    }
    obj_mem_size_ = size - NICMGR_BKUP_OBJ_MEM_OFFSET;
    obj_mem_offset_ = NICMGR_BKUP_OBJ_MEM_OFFSET;
    if (create) {
        memset(mem_, 0, NICMGR_BKUP_OBJ_MEM_OFFSET);
    }
    return SDK_RET_OK;
}


void
nicmgr_upg_hndlr::upg_ipc_handler_(sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_msg_t *upg_msg = (upg_msg_t *)msg->data();
    HdlrResp rsp = {.resp=FAIL, .errStr="Invalid message id"};
    upg_msg_t rsp_msg;
    UpgCtx upg_ctx;

    // basic validation
    assert(likely(upg_msg != NULL));
    NIC_LOG_DEBUG("Rcvd UPG message, msg {}", upg_msg->msg_id);
    switch(upg_msg->msg_id) {
    case MSG_ID_UPG_COMPAT_CHECK:
        rsp = CompatCheckHandler(upg_ctx);
        break;
    case MSG_ID_UPG_QUIESCE:
        rsp = ProcessQuiesceHandler(upg_ctx);
        break;
    case MSG_ID_UPG_LINK_UP:
        rsp = LinkUpHandler(upg_ctx);
        break;
    case MSG_ID_UPG_LINK_DOWN:
        rsp = LinkDownHandler(upg_ctx);
        break;
    case MSG_ID_UPG_HOSTUP:
        rsp = HostUpHandler(upg_ctx);
        break;
    case MSG_ID_UPG_HOSTDOWN:
        rsp = HostDownHandler(upg_ctx);
        break;
   case MSG_ID_UPG_SUCCESS:
        SuccessHandler(upg_ctx);
        rsp.resp = SUCCESS;
        rsp.errStr = "";
        break;
    case MSG_ID_UPG_FAIL:
        upg_ctx.prevExecState = (UpgReqStateType)upg_msg->prev_exec_state;
        rsp = FailedHandler(upg_ctx);
        break;
    case MSG_ID_UPG_ABORT:
        AbortHandler(upg_ctx);
        rsp.resp = SUCCESS;
        rsp.errStr = "";
        break;
    case MSG_ID_UPG_SAVE_STATE:
        rsp = SaveStateHandler(upg_ctx, upg_msg);
        break;
    case MSG_ID_UPG_POST_LINK_UP:
        rsp = PostLinkUpHandler(upg_ctx);
        break;
     case MSG_ID_UPG_POST_HOSTDOWN:
        rsp = PostHostDownHandler(upg_ctx);
        break;
    case MSG_ID_UPG_POST_RESTART:
        rsp = PostRestartHandler(upg_ctx);
        break;
    default:
        NIC_LOG_ERR("Invalid message id");
    }

    NIC_LOG_DEBUG("UPG message result {} {}", rsp.resp, rsp.errStr);
    switch(rsp.resp) {
    case SUCCESS:
        rsp_msg.rsp_code = UPG_RSP_SUCCESS;
        break;
    case INPROGRESS:
        rsp_msg.rsp_code = UPG_RSP_INPROGRESS;
        break;
    case FAIL:
    // passthrough
    default:
        rsp_msg.rsp_code = UPG_RSP_FAIL;
        break;
    }
    rsp_msg.msg_id = upg_msg->msg_id;
    rsp_msg.rsp_err_string[0] = '\0';
    strncpy(rsp_msg.rsp_err_string, rsp.errStr.c_str(),
            sizeof(rsp_msg.rsp_err_string));
    NIC_LOG_DEBUG("UPG IPC respond msg {} rspcode {} err {}",
                  rsp_msg.msg_id, rsp_msg.rsp_code, rsp_msg.rsp_err_string);
    sdk::ipc::respond(msg, &rsp_msg, sizeof(rsp_msg));
}

void
nicmgr_upg_hndlr::upg_ipc_handler_cb(sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    return upg_handler->upg_ipc_handler_(msg, ctxt);
}

void
nicmgr_upg_init (void)
{
    sdk_ret_t ret;
    bool shm_create = false;
    bool states_in_delphi = false;
    upg_handler = new nicmgr_upg_hndlr();
    std::string src = std::string(NICMGR_BKUP_DIR) + std::string(NICMGR_BKUP_SHM_NAME);

    SDK_ASSERT(devmgr);

    if (devmgr->GetUpgradeMode() == FW_MODE_NORMAL_BOOT) {
        shm_create = true;
    } else if (devmgr->GetUpgradeMode() == FW_MODE_UPGRADE) {
        if (file_exist(src.c_str()) == false) {
            states_in_delphi = true;
            shm_create = true;
        }
    }

    ret = upg_handler->upg_shm_alloc(NICMGR_BKUP_SHM_NAME, NICMGR_BKUP_SHM_SIZE, shm_create);
    SDK_ASSERT(ret == SDK_RET_OK);
    sdk::ipc::reg_request_handler(event_id_t::EVENT_ID_UPG,
                                  nicmgr_upg_hndlr::upg_ipc_handler_cb, NULL);

    if (devmgr->GetUpgradeMode() == FW_MODE_UPGRADE) {
        if (states_in_delphi) {
            restore_from_delphi();
        } else {
            upg_handler->upg_restore_states();
        }
    }

    if (devmgr->GetUpgradeMode() == FW_MODE_ROLLBACK) {
        upg_handler->upg_restore_states();
        unlink(nicmgr_rollback_state_file);
    }
}

} // namespace nicmgr
