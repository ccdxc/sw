//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_DELPHIC_IPC_HPP__
#define __NICMGR_DELPHIC_IPC_HPP__

namespace nicmgr {

typedef enum nicmgr_delphic_msg_id_e {
    NICMGR_DELPHIC_MSG_SET_UPG_ETHDEVINFO = 1,
    NICMGR_DELPHIC_MSG_CLR_UPG_ETHDEVINFO = 2,
    NICMGR_DELPHIC_MSG_SET_UPG_UPLINKINFO = 3,
    NICMGR_DELPHIC_MSG_CLR_UPG_UPLINKINFO = 4,
} nicmgr_delphic_msg_id_t;


typedef struct nicmgr_delphic_msg_s {
    nicmgr_delphic_msg_id_t msg_id;
} nicmgr_delphic_msg_t;

} // namespace nicmgr

using nicmgr::nicmgr_delphic_msg_id_t;
using nicmgr::nicmgr_delphic_msg_t;
using nicmgr_delphic_msg_id_t::NICMGR_DELPHIC_MSG_SET_UPG_ETHDEVINFO;
using nicmgr_delphic_msg_id_t::NICMGR_DELPHIC_MSG_CLR_UPG_ETHDEVINFO;
using nicmgr_delphic_msg_id_t::NICMGR_DELPHIC_MSG_SET_UPG_UPLINKINFO;
using nicmgr_delphic_msg_id_t::NICMGR_DELPHIC_MSG_CLR_UPG_UPLINKINFO;

#endif    // __NICMGR_DELPHIC_IPC_HPP__
