//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_SYSMGR_HPP__
#define __HAL_SYSMGR_HPP__

#include <string>
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

namespace hal {
namespace sysmgr {

class sysmgr_client {
public:
   sysmgr_client(delphi::SdkPtr &sdk) {
      this->sysmgr_ = ::sysmgr::CreateClient(sdk, "hal");
      init_ok = false;
      mount_ok = false;
   }
   void init_done();
   void mount_done();
      

private:
   ::sysmgr::ClientPtr sysmgr_;
   bool init_ok;
   bool mount_ok;
};

}    // namespace sysmgr
}    // namespace hal

#endif    // __HAL_SYSMGR_HPP__
