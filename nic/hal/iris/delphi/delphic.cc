//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/iris/delphi/delphi.hpp"
#include "nic/hal/iris/delphi/delphic.hpp"
#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "nic/linkmgr/delphi/linkmgr_delphi.hpp"
#include "gen/proto/hal.delphi.hpp"

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
    HAL_ASSERT(g_delphic != NULL);
    HAL_TRACE_DEBUG("HAL delphi thread started ...");

    // init linkmgr services
    Status ret = linkmgr::port_svc_init(sdk);
    HAL_ABORT(ret.ok());

    // register delphi client
    sdk->RegisterService(g_delphic);

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
    dobj::HalStatusPtr    status;

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

    // create HAL status (singleton) obj with STATUS_NONE
    status = std::make_shared<dobj::HalStatus>();
    if (status) {
        status->set_state(::hal::HalState::HAL_STATE_NONE);
        sdk_->SetObject(status);
    }
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

// API to invoke when HAL is ready for external world
void
init_done (void)
{
    g_delphic->init_done();
}

delphi::SdkPtr
delphi_client::sdk (void)
{
    return sdk_;
}

static ::hal::HalState
hal_state (hal::hal_status_t hal_status)
{
    switch (hal_status) {
    case HAL_STATUS_NONE:
        return ::hal::HalState::HAL_STATE_NONE;
    case HAL_STATUS_ASIC_INIT_DONE:
        return ::hal::HalState::HAL_STATE_ASIC_INIT_DONE;
    case HAL_STATUS_MEM_INIT_DONE:
        return ::hal::HalState::HAL_STATE_MEM_INIT_DONE;
    case HAL_STATUS_PACKET_BUFFER_INIT_DONE:
        return ::hal::HalState::HAL_STATE_PACKET_BUFFER_INIT_DONE;
    case HAL_STATUS_DATA_PLANE_INIT_DONE:
        return ::hal::HalState::HAL_STATE_DATA_PLANE_INIT_DONE;
    case HAL_STATUS_SCHEDULER_INIT_DONE:
        return ::hal::HalState::HAL_STATE_SCHEDULER_INIT_DONE;
    case HAL_STATUS_UP:
        return ::hal::HalState::HAL_STATE_UP;
    default:
        return hal::HalState::HAL_STATE_NONE;
    }
}

// API to update HAL status
void
set_hal_status (hal::hal_status_t hal_status)
{
    dobj::HalStatusPtr    status;
    ::hal::HalState       state;

    state = hal_state(hal_status);
    status = dobj::HalStatus::FindObject(g_delphic->sdk());
    if (status) {
        status->set_state(state);
        g_delphic->sdk()->QueueUpdate(status);
    } else {
        status = std::make_shared<dobj::HalStatus>();
        status->set_state(state);
        g_delphic->sdk()->QueueUpdate(status);
    }
}

}    // namespace svc
}    // namespace hal
