//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/iris/upgrade/upg_ipc.hpp"
#include "nic/hal/iris/delphi/delphi.hpp"
#include "nic/hal/iris/delphi/delphic.hpp"
#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "nic/linkmgr/delphi/linkmgr_delphi.hpp"
#include "platform/src/app/nicmgrd/src/upgrade_rel_a2b.hpp"
#include "gen/proto/device.delphi.hpp"

namespace hal {
namespace svc {

using namespace std;
using grpc::Status;
using hal::upgrade::upgrade_handler;

shared_ptr<delphi_client>    g_delphic;
//------------------------------------------------------------------------------
// starting point for delphi thread
//------------------------------------------------------------------------------
void *
delphi_client_start (void *ctxt)
{
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    g_delphic = make_shared<delphi_client>(sdk);
    SDK_ASSERT(g_delphic != NULL);
    HAL_TRACE_DEBUG("HAL delphi thread started ...");

    // init linkmgr services
    Status ret = linkmgr::port_svc_init(sdk);
    SDK_ASSERT_TRACE_RETURN(ret.ok(), NULL,
                            "Port service initialization failure");
    // init nicmgr service. only for rel_A to rel_B
    nicmgr::nicmgr_delphi_init(sdk);

    // register delphi client
    sdk->RegisterService(g_delphic);

    // register for sdk ipc for nicmgr events
    hal::upgrade::upg_event_init();

    // sit in main loop
    sdk->MainLoop();

    // shouldn't hit
    return NULL;
}

//------------------------------------------------------------------------------
// delphi_client constructor
//------------------------------------------------------------------------------
delphi_client::delphi_client(delphi::SdkPtr &sdk)
{
    sdk_ = sdk;
    sysmgr_ = hal::sysmgr::create_sysmgr_client(sdk_);
    upgsdk_ =
        make_shared<::upgrade::UpgSdk>(sdk_, make_shared<upgrade_handler>(),
                                       "hal", ::upgrade::NON_AGENT, nullptr);

    // create the InterfaceSpec reactor
    if_svc_ = std::make_shared<if_svc>(sdk);

    mount_ok = false;
    init_ok = false;

    // mount InterfaceSpec objects
    delphi::objects::InterfaceSpec::Mount(sdk, delphi::ReadMode);

    // Register InterfaceSpec reactor
    delphi::objects::InterfaceSpec::Watch(sdk, if_svc_);

    // mount interface status objects
    delphi::objects::InterfaceStatus::Mount(sdk, delphi::ReadWriteMode);

}

// OnMountComplete gets called when all the objects are mounted
void
delphi_client::OnMountComplete(void)
{
    HAL_TRACE_DEBUG("OnMountComplete got called ...");
    mount_ok = true;
    if (init_ok && this->mount_ok) {
       sysmgr_->init_done();
    }
    // inform nicmgr service. only for rel_A to rel_B
    nicmgr::nicmgr_delphi_mount_complete();
}

// indicate HAL init done to rest of the system
void
delphi_client::init_done(void)
{
   HAL_TRACE_DEBUG("Init done called ...");
   init_ok = true;
   if (init_ok && mount_ok) {
      sysmgr_->init_done();
   }
}

// send upgrade stage status
void
delphi_client::send_upg_stage_status(bool status)
{
   if (status) {
        upgsdk_->SendAppRespSuccess();
   } else {
        upgsdk_->SendAppRespFail("Timeout occurred");
   }
}

// API to invoke when HAL is ready for external world
void
hal_init_done (void)
{
    if (!g_delphic) {
        return;
    }
    g_delphic->init_done();
}

delphi::SdkPtr
delphi_client::sdk (void)
{
    return sdk_;
}

std::string delphi_client::Name()
{
    return "hal";
}

static void 
microseg_sync_result (sdk::ipc::ipc_msg_ptr msg, const void *status)
{
    hal::core::micro_seg_info_t *response = (hal::core::micro_seg_info_t *)status;
    hal::core::micro_seg_info_t *src = (hal::core::micro_seg_info_t *)msg->data();

    HAL_TRACE_DEBUG("Processing microseg response from nicmgr micro_seg_en: {}, rsp_ret: {}",
                    src->status, src->rsp_ret);
    response->status = src->status;
    response->rsp_ret = src->rsp_ret;
}

sdk_ret_t
micro_seg_mode_notify (MicroSegMode mode)
{
    hal::core::micro_seg_info_t      info, rsp;

    rsp.rsp_ret = SDK_RET_OK;

    HAL_TRACE_DEBUG("Notifying nicmgr about micro seg change. New mode: {}",
                    MicroSegMode_Name(mode));
    memset(&info, 0, sizeof(info));
    info.status = mode == sys::MICRO_SEG_ENABLE ? true : false;
    // sdk::ipc::broadcast(event_id_t::EVENT_ID_MICRO_SEG, &event, sizeof(event));

    if (!hal_thread_ready(hal::HAL_THREAD_ID_NICMGR)) {
        HAL_TRACE_ERR("Nicmgr not up, micro seg msg failed");
        goto end;
    }

    sdk::ipc::request(hal::HAL_THREAD_ID_NICMGR, event_id_t::EVENT_ID_MICRO_SEG, &info,
                      sizeof(info), microseg_sync_result, &rsp);

    HAL_TRACE_DEBUG("microseg msg result.. msg {} rspcode {}",
                    rsp.status, rsp.rsp_ret);
    if (rsp.rsp_ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Micro seg msg to nicmgr failed with err: {}", rsp.rsp_ret);
        goto end;
    }

end:
    return rsp.rsp_ret;
}

}    // namespace svc
}    // namespace hal
