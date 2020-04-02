/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "upgrade_rel_a2b.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"

DeviceManager *devmgr;

std::vector<struct EthDevInfo *>
DeviceManager::GetEthDevStateInfo() {
    std::vector<struct EthDevInfo *> eth;

    return eth;
}

namespace nicmgr {

void
nicmgr_init (platform_type_t platform,
             struct sdk::event_thread::event_thread *thread)
{

}

void
nicmgr_exit (void)
{

}

void
nicmgr_delphi_mount_complete (void)
{

}

// below function is called from delphi thread
void
nicmgr_delphi_init (delphi::SdkPtr sdk)
{

}

}   // namespace hal
