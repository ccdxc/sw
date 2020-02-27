//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __LINKMGR_EVENT_RECORDER_HPP__
#define __LINKMGR_EVENT_RECORDER_HPP__

namespace linkmgr {

sdk_ret_t event_recorder_init(void);
void port_event_recorder_notify(port_event_info_t *port_event_info);

}    // namespace linkmgr

#endif  // __LINKMGR_EVENT_RECORDER_HPP__
