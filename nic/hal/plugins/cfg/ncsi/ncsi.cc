//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
// Handles CRUD APIs for NCSI
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/plugins/cfg/ncsi/ncsi.hpp"
#include "nic/hal/plugins/cfg/ncsi/ncsi_ipc.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/utils/eventmgr/eventmgr.hpp"
#include "nic/hal/core/event_ipc.hpp"
#include "nic/hal/pd/pd_api.hpp"

namespace hal {

static void
ncsi_process_sync_result (sdk::ipc::ipc_msg_ptr msg, const void *status)
{
    ncsi_ipc_msg_t *response = (ncsi_ipc_msg_t *)status;
    ncsi_ipc_msg_t *src = (ncsi_ipc_msg_t *)msg->data();

    HAL_TRACE_DEBUG("Processing NCSI response from nicmgr msg_id: {}, rsp_ret: {}",
                    src->msg_id, src->rsp_ret);

    response->msg_id = src->msg_id;
    response->rsp_ret = src->rsp_ret;
}

hal_ret_t
ncsi_nicmgr_block_call (ncsi_ipc_msg_t *msg)
{
    hal_ret_t ret = HAL_RET_OK;
    ncsi_ipc_msg_t status;

     if (!hal_thread_ready(hal::HAL_THREAD_ID_NICMGR)) {
         HAL_TRACE_ERR("Nicmgr not up, ncsi msg failed");
         goto end;
     }
     HAL_TRACE_DEBUG("sendin msg {} to nicmgr for ncsi", msg->msg_id);
     proto_msg_dump(*(msg->msg));

     // Release read lock
     hal_handle_cfg_db_lock(true, false);

     // gRPC -> nicmgr
     sdk::ipc::request(hal::HAL_THREAD_ID_NICMGR, event_id_t::EVENT_ID_NCSI, msg, 
                       sizeof(*msg), ncsi_process_sync_result, &status);
     HAL_TRACE_DEBUG("ncsi msg result.. msg {} rspcode {}",
                     status.msg_id, status.rsp_ret);
     if (status.rsp_ret != SDK_RET_OK) {
         HAL_TRACE_ERR("ncsi nicmgr failed with err: {}", status.rsp_ret);
         ret = HAL_RET_ERR;
     }
     HAL_TRACE_DEBUG("Got response");
     proto_msg_dump(*msg->rsp);

     // Take read lock
     hal_handle_cfg_db_lock(true, true);

end:
     return ret;
}

hal_ret_t 
vlan_filter_create (VlanFilterRequest& req, VlanFilterResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI vlan filter create ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_VLAN_FILTER;
    msg.oper = NCSI_MSG_OPER_CREATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
vlan_filter_delete (VlanFilterRequest& req, VlanFilterResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI vlan filter delete ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_VLAN_FILTER;
    msg.oper = NCSI_MSG_OPER_DELETE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
vlan_filter_get (VlanFilterGetRequest& req, VlanFilterGetResponseMsg *rsp)
{
     hal_ret_t      ret = HAL_RET_OK;
     ncsi_ipc_msg_t msg;

     hal_api_trace(" API Begin: NCSI vlan filter get ");
     proto_msg_dump(req);

     msg.msg_id = NCSI_MSG_VLAN_FILTER;
     msg.oper = NCSI_MSG_OPER_GET;
     msg.msg = dynamic_cast<Message *>(&req);
     msg.rsp = dynamic_cast<Message *>(rsp);

     ret = ncsi_nicmgr_block_call(&msg);

     return ret;
}

hal_ret_t 
mac_filter_create (MacFilterRequest& req, MacFilterResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI mac filter create ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_MAC_FILTER;
    msg.oper = NCSI_MSG_OPER_CREATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);
    hal_api_trace("API End: NCSI mac filter create");

    return ret;
}

hal_ret_t
mac_filter_delete (MacFilterRequest& req, MacFilterResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI mac filter delete ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_MAC_FILTER;
    msg.oper = NCSI_MSG_OPER_DELETE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
mac_filter_get (MacFilterGetRequest& req, MacFilterGetResponseMsg *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI mac filter get ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_MAC_FILTER;
    msg.oper = NCSI_MSG_OPER_GET;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
bcast_filter_create (BcastFilterRequest& req, BcastFilterResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI bcast filter create ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_BCAST_FILTER;
    msg.oper = NCSI_MSG_OPER_CREATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
bcast_filter_update (BcastFilterRequest& req, BcastFilterResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI bcast filter update ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_BCAST_FILTER;
    msg.oper = NCSI_MSG_OPER_UPDATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
bcast_filter_delete (BcastFilterDeleteRequest& req, BcastFilterDeleteResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI bcast filter delete ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_BCAST_FILTER;
    msg.oper = NCSI_MSG_OPER_DELETE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}
hal_ret_t 
bcast_filter_get (BcastFilterGetRequest& req, BcastFilterGetResponseMsg *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI bcast filter get ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_BCAST_FILTER;
    msg.oper = NCSI_MSG_OPER_GET;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
mcast_filter_create (McastFilterRequest& req, McastFilterResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI mcast filter create ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_MCAST_FILTER;
    msg.oper = NCSI_MSG_OPER_CREATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
mcast_filter_update (McastFilterRequest& req, McastFilterResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI mcast filter update ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_MCAST_FILTER;
    msg.oper = NCSI_MSG_OPER_UPDATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
mcast_filter_delete (McastFilterDeleteRequest& req, McastFilterDeleteResponse *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI mcast filter delete ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_MCAST_FILTER;
    msg.oper = NCSI_MSG_OPER_DELETE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
mcast_filter_get (McastFilterGetRequest& req, McastFilterGetResponseMsg *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI mcast filter get ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_MCAST_FILTER;
    msg.oper = NCSI_MSG_OPER_GET;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
vlan_mode_create (VlanModeRequest& req, VlanModeResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI vlan mode create ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_VLAN_MODE;
    msg.oper = NCSI_MSG_OPER_CREATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}


hal_ret_t 
vlan_mode_update (VlanModeRequest& req, VlanModeResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI vlan mode update ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_VLAN_MODE;
    msg.oper = NCSI_MSG_OPER_UPDATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
vlan_mode_delete (VlanModeRequest& req, VlanModeResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI vlan mode delete ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_VLAN_MODE;
    msg.oper = NCSI_MSG_OPER_DELETE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
vlan_mode_get (VlanModeGetRequest& req, VlanModeGetResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;

    hal_api_trace(" API Begin: NCSI vlan mode get ");
    proto_msg_dump(req);

    return ret;
}

hal_ret_t 
channel_create (ChannelRequest& req, ChannelResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI channel create ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_CHANNEL;
    msg.oper = NCSI_MSG_OPER_CREATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
channel_update (ChannelRequest& req, ChannelResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI channel update ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_CHANNEL;
    msg.oper = NCSI_MSG_OPER_UPDATE;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t 
channel_delete (ChannelRequest& req, ChannelResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;

    hal_api_trace(" API Begin: NCSI channel delete ");
    proto_msg_dump(req);

    return ret;
}

hal_ret_t 
channel_get (ChannelGetRequest& req, ChannelGetResponseMsg *rsp)
{
    hal_ret_t      ret = HAL_RET_OK;
    ncsi_ipc_msg_t msg;

    hal_api_trace(" API Begin: NCSI channel get ");
    proto_msg_dump(req);

    msg.msg_id = NCSI_MSG_CHANNEL;
    msg.oper = NCSI_MSG_OPER_GET;
    msg.msg = dynamic_cast<Message *>(&req);
    msg.rsp = dynamic_cast<Message *>(rsp);

    ret = ncsi_nicmgr_block_call(&msg);

    return ret;
}

hal_ret_t
clock_multiplier_update (ClockMultiplierUpdateRequest& req, ClockMultiplierUpdateResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    hal::pd::pd_set_clock_multiplier_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};

    pd_func_args.pd_set_clock_multiplier = &args;
    
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_SET_CLOCK_MULTIPLIER, &pd_func_args);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
    }     
     
    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

}    // namespace hal
