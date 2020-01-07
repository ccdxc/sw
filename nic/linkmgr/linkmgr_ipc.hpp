//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#ifndef __LINKMGR_IPC_HPP__
#define __LINKMGR_IPC_HPP__

namespace linkmgr {
namespace ipc {

void port_event_notify (port_event_info_t *port_event_info);
void xcvr_event_notify (xcvr_event_info_t *xcvr_event_info);

}    // namespace ipc
}    // namespace linkmgr

#endif   // __LINKMGR_IPC_HPP__
