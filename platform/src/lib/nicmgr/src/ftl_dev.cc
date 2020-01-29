/*
 * Copyright (c) 2020, Pensando Systems Inc.
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
#include "nic/sdk/platform/misc/include/misc.h"


#include "logger.hpp"
#include "ftl_dev.hpp"
#include "ftl_lif.hpp"
#include "pd_client.hpp"


/*
 * Devapi availability check
 */
#define FTL_DEVAPI_CHECK(ret_val)                                               \
    if (!dev_api) {                                                             \
        NIC_LOG_ERR("{}: Uninitialized devapi", DevNameGet());                  \
        return ret_val;                                                         \
    }

#define FTL_DEVAPI_CHECK_VOID()                                                 \
    if (!dev_api) {                                                             \
        NIC_LOG_ERR("{}: Uninitialized devapi", DevNameGet());                  \
        return;                                                                 \
    }

/*
 * LIFs iterator
 */
#define FOR_EACH_FTL_DEV_LIF(idx, lif)                                          \
    for (idx = 0, lif = LifFind(0); lif; idx++, lif = LifFind(idx))


FtlDev::FtlDev(devapi *dapi,
               void *dev_spec,
               PdClient *pd_client,
               EV_P) :
    spec((ftl_devspec_t *)dev_spec),
    pd(pd_client),
    dev_api(dapi),
    delphi_mounted(false),
    EV_A(EV_A)
{
    ftl_lif_res_t       lif_res;
    sdk_ret_t           ret = SDK_RET_OK;

    /*
     * Allocate LIFs
     */
    ret = pd->lm_->alloc_id(&lif_base, spec->lif_count);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to allocate lifs. ret: {}", DevNameGet(), ret);
        throw;
    }
    NIC_LOG_DEBUG("{}: lif_base {} lif_count {}",
                  DevNameGet(), lif_base, spec->lif_count);

    /*
     * Create LIF
     */
    lif_res.lif_id = lif_base;
    lif_res.index = 0;
    auto lif = new FtlLif(*this, lif_res, EV_DEFAULT);
    if (!lif) {
        NIC_LOG_ERR("{}: failed to create FtlLif {}",
                    DevNameGet(), lif_res.lif_id);
        throw;
    }
    lif_vec.push_back(lif);
}

FtlDev::~FtlDev()
{
    FtlLif      *lif;
    uint32_t    i;

    FOR_EACH_FTL_DEV_LIF(i, lif) {
        delete lif;
    }
    lif_vec.clear();
    pd->lm_->free_id(lif_base, spec->lif_count);
}

void
FtlDev::HalEventHandler(bool status)
{
    FtlLif      *lif;
    uint32_t    i;

    FOR_EACH_FTL_DEV_LIF(i, lif) {
        lif->HalEventHandler(status);
    }
}

void
FtlDev::DelphiMountEventHandler(bool mounted)
{
    delphi_mounted = mounted;
}

void
FtlDev::SetHalClient(devapi *dapi)
{
    FtlLif      *lif;
    uint32_t    i;

    dev_api = dapi;
    FOR_EACH_FTL_DEV_LIF(i, lif) {
        lif->SetHalClient(dapi);
    }
}

struct ftl_devspec *
FtlDev::ParseConfig(boost::property_tree::ptree::value_type node)
{
    ftl_devspec *ftl_spec;
    auto        val = node.second;

    ftl_spec = new struct ftl_devspec;
    memset(ftl_spec, 0, sizeof(*ftl_spec));

    ftl_spec->name = val.get<string>("name");
    ftl_spec->lif_count = 1;
    ftl_spec->session_hw_scanners = val.get<uint32_t>("session_hw_scanners");
    ftl_spec->session_burst_size = val.get<uint32_t>("session_burst_size");
    ftl_spec->session_burst_resched_time_us = val.get<uint32_t>("session_burst_resched_time_us");
    ftl_spec->conntrack_hw_scanners = val.get<uint32_t>("conntrack_hw_scanners");
    ftl_spec->conntrack_burst_size = val.get<uint32_t>("conntrack_burst_size");
    ftl_spec->conntrack_burst_resched_time_us = val.get<uint32_t>("conntrack_burst_resched_time_us");
    ftl_spec->sw_pollers = val.get<uint32_t>("sw_pollers");
    ftl_spec->sw_poller_qdepth = val.get<uint32_t>("sw_poller_qdepth");

    ftl_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
    NIC_LOG_DEBUG("Creating FTL device with name: {}, qos_group: {}",
                  ftl_spec->name, ftl_spec->qos_group);

    return ftl_spec;
}

const char *
ftl_dev_opcode_str(uint32_t opcode)
{
    switch(opcode) {
        FTL_DEVCMD_OPCODE_CASE_TABLE
        default: return "FTL_DEVCMD_UNKNOWN";
    }
}

static const char   *status_str_table[] = {
    FTL_RC_STR_TABLE
};

const char *
ftl_dev_status_str(ftl_status_code_t status)
{
    if (status < (ftl_status_code_t)ARRAY_SIZE(status_str_table)) {
        return status_str_table[status];
    }

    return "FTL_RC_UNKNOWN";
}

ftl_status_code_t
FtlDev::CmdHandler(ftl_devcmd_t *req,
                   void *req_data,
                   ftl_devcmd_cpl_t *rsp,
                   void *rsp_data)
{
    ftl_status_code_t   status;

    NIC_LOG_DEBUG("{}: Handling cmd: {}", DevNameGet(),
                  ftl_dev_opcode_str(req->cmd.opcode));

    switch (req->cmd.opcode) {

    case FTL_DEVCMD_OPCODE_NOP:
        status = FTL_RC_SUCCESS;
        break;

    case FTL_DEVCMD_OPCODE_RESET:
        status = devcmd_reset(&req->dev_reset, req_data,
                              &rsp->dev_reset, rsp_data);
        break;

    case FTL_DEVCMD_OPCODE_IDENTIFY:
        status = devcmd_identify(&req->dev_identify, req_data,
                                 &rsp->dev_identify, rsp_data);
        break;

    default:
        auto lif = LifFind(req->cmd.lif_index);
        if (!lif) {
            NIC_LOG_ERR("{}: lif {} unexpectedly missing", DevNameGet(),
                        req->cmd.lif_index);
            return FTL_RC_ENOENT;
        }

        status = lif->CmdHandler(req, req_data, rsp, rsp_data);
        break;
    }

    rsp->status = status;
    NIC_LOG_DEBUG("{}: Done cmd: {}, status: {}", DevNameGet(),
                  ftl_dev_opcode_str(req->cmd.opcode), status);
    return status;
}


ftl_status_code_t
FtlDev::devcmd_reset(dev_reset_cmd_t *cmd,
                     void *req_data,
                     dev_reset_cpl_t *cpl,
                     void *resp_data)
{

    FtlLif              *lif;
    ftl_status_code_t   status = FTL_RC_SUCCESS;
    uint32_t            i;

    NIC_LOG_DEBUG("{}: FTL_DEVCMD_OPCODE_RESET", DevNameGet());

    FOR_EACH_FTL_DEV_LIF(i, lif) {

        /*
         * Reset at the device level will reset and delete the LIF.
         */
        status = lif->reset(true);
        if (status != FTL_RC_SUCCESS) {
            break;
        }
    }

    return status;
}

ftl_status_code_t
FtlDev::devcmd_identify(dev_identify_cmd_t *cmd,
                        void *req_data,
                        dev_identify_cpl_t *cpl,
                        void *rsp_data)
{
    dev_identity_t      *rsp = (dev_identity_t *)rsp_data;

    NIC_LOG_DEBUG("{}: FTL_DEVCMD_OPCODE_IDENTIFY", DevNameGet());

    if (cmd->type >= FTL_DEV_TYPE_MAX) {
        NIC_LOG_ERR("{}: bad device type {}", DevNameGet(), cmd->type);
        return FTL_RC_EINVAL;
    }
    if (cmd->ver != IDENTITY_VERSION_1) {
        NIC_LOG_ERR("{}: unsupported version {}", DevNameGet(), cmd->ver);
        return FTL_RC_EVERSION;
    }
    memset(&rsp->base, 0, sizeof(rsp->base));
    rsp->base.version = IDENTITY_VERSION_1;
    rsp->base.nlifs = spec->lif_count;

    cpl->ver = IDENTITY_VERSION_1;
    return FTL_RC_SUCCESS;
}

FtlLif *
FtlDev::LifFind(uint32_t lif_index)
{
    if (lif_index >= lif_vec.size()) {
        return nullptr;
    }
    return lif_vec.at(lif_index);
}

