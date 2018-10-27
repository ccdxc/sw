//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "nic/hal/core/heartbeat/heartbeat.hpp"

namespace hal {
namespace sysmgr {

   void sysmgr_client::init_done()
   {
      this->init_ok = true;
      if (init_ok && mount_ok) {
	 this->sysmgr_->init_done();
      }
   }

   void sysmgr_client::mount_done()
   {
      this->mount_ok = true;
      if (init_ok && mount_ok) {
	 this->sysmgr_->init_done();
      }
   }
   
}    // namespace sysmgr
}    // namespace hal
