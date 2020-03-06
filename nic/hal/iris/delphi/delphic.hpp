// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __DELPHIC_HPP__
#define __DELPHIC_HPP__

#include "gen/proto/upgrade.delphi.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "nic/hal/iris/delphi/if_svc.hpp"
#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "gen/proto/system.delphi.hpp"

namespace dobj = delphi::objects;
using hal::upgrade::upgrade_handler;
using sys::MicroSegMode;

namespace hal {
namespace svc {

// delphi_client is the HAL’s delphi client obj
class delphi_client : public delphi::Service {
public:
   delphi_client(delphi::SdkPtr &sdk);
   void OnMountComplete(void);
   void init_done(void);
   void send_upg_stage_status(bool status);
   delphi::SdkPtr sdk(void);
   virtual std::string Name();

private:
    delphi::SdkPtr                                 sdk_;
    std::shared_ptr<hal::sysmgr::sysmgr_client>    sysmgr_;
    ::upgrade::UpgSdkPtr                           upgsdk_;
    if_svc_ptr_t                                   if_svc_;
    bool                                           mount_ok;
    bool                                           init_ok;
};
std::shared_ptr<delphi_client> delphic(void);

sdk_ret_t micro_seg_mode_notify(MicroSegMode mode);
}    // namespace svc
}    // namespace hal


#endif    // __DELPHIC_HPP__

