// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __DELPHIC_HPP__
#define __DELPHIC_HPP__

#include "gen/proto/upgrade.delphi.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "nic/hal/iris/delphi/if_svc.hpp"
#include "nic/hal/iris/upgrade/upgrade.hpp"

namespace dobj = delphi::objects;
using hal::upgrade::upgrade_handler;

namespace hal {
namespace svc {

// delphi_client is the HAL’s delphi client obj
class delphi_client : public delphi::Service {
public:
   delphi_client(delphi::SdkPtr &sdk);
   void OnMountComplete(void);
   void init_done(void);
   delphi::SdkPtr sdk(void);

private:
    delphi::SdkPtr                                 sdk_;
    std::shared_ptr<hal::sysmgr::sysmgr_client>    sysmgr_;
    ::upgrade::UpgSdkPtr                           upgsdk_;
    if_svc_ptr_t                                   if_svc_;
    bool                                           mount_ok;
    bool                                           init_ok;
};
std::shared_ptr<delphi_client> delphic(void);
void init_done(void);
void set_hal_status(hal::hal_status_t hal_status);

}    // namespace svc
}    // namespace hal

#endif    // __DELPHIC_HPP__

