//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_UPGRADE_REL_ATOB_HPP__
#define __NICMGR_UPGRADE_REL_ATOB_HPP__

#include "nic/delphi/sdk/delphi_sdk.hpp"

namespace nicmgr {

void restore_from_delphi(void);
extern bool g_device_restored;
void nicmgr_delphi_init(delphi::SdkPtr sdk);
void nicmgr_delphi_mount_complete(void);

}   // namespace nicmgr

#endif   // __NICMGR_UPGRADE_REL_ATOB_HPP__
