//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/iris/delphi/delphi.hpp"
#include "nic/hal/iris/delphi/delphic.hpp"
#include "nic/hal/iris/delphi/utils/utils.hpp"
#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "nic/linkmgr/delphi/linkmgr_delphi.hpp"
#include "gen/proto/hal.delphi.hpp"
#include "gen/proto/device.delphi.hpp"

namespace hal {
namespace svc {

using namespace std;
using grpc::Status;
using hal::upgrade::upgrade_handler;
using delphi::objects::SystemSpec;

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

    // mount system object for nicmgr
    SystemSpec::Mount(sdk, delphi::ReadWriteMode);

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

    // mount hal status objects
    delphi::objects::HalStatus::Mount(sdk, delphi::ReadWriteMode);

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

// API to update HAL status
void
set_hal_status (hal::hal_status_t hal_status)
{
    if (!g_delphic) {
        return;
    }
    set_hal_status(hal_status, g_delphic->sdk());
}

void
micro_seg_mode_notify (MicroSegMode mode)
{
    delphi::SdkPtr        sdk = g_delphic ? g_delphic->sdk() : NULL;
    dobj::SystemSpecPtr   spec;

    if (!g_delphic) {
        return;
    }

    HAL_TRACE_DEBUG("Notifying nicmgr about micro seg change. New mode: {}",
                    MicroSegMode_Name(mode));
    spec = std::make_shared<dobj::SystemSpec>();
    spec->set_micro_seg_mode((mode == sys::MICRO_SEG_ENABLE) ? 
                             device::MICRO_SEG_ENABLE : device::MICRO_SEG_DISABLE);
    sdk->QueueUpdate(spec);
}

}    // namespace svc
}    // namespace hal
