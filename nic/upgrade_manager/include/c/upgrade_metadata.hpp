// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_METADATA_H__
#define __UPGRADE_METADATA_H__

#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"
#include "upgrade_state_machine.hpp"

namespace upgrade {

using namespace std;

bool GetUpgCtxFromMeta(string metafile, UpgCtx& ctx);

}

#endif //__UPGRADE_METADATA_H__
