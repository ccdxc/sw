//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_INIT_HPP__
#define __NICMGR_INIT_HPP__

namespace nicmgr {
void nicmgr_init(platform_type_t platform,
                 struct sdk::event_thread::event_thread *thread);
void nicmgr_exit();
}   // namespace nicmgr
#endif //__NICMGR_INIT_HPP__
