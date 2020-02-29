//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements endpoint clear APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/core.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"

static void
learn_clear_resp_cb (sdk::ipc::ipc_msg_ptr msg, const void *ret)
{
    *(sdk_ret_t *)ret = *(sdk_ret_t *)msg->data();
}

sdk_ret_t
learn_ip_clear (ep_ip_key_t *key)
{
    sdk_ret_t ret;
    learn_clear_msg_t request;

    if (key) {
        request.id = LEARN_CLEAR_IP;
        memcpy(&request.ip_key, key, sizeof(ep_ip_key_t));
    } else {
        request.id = LEARN_CLEAR_IP_ALL;
    }
    sdk::ipc::request(core::PDS_THREAD_ID_LEARN, LEARN_MSG_ID_CLEAR_CMD,
                      &request, sizeof(learn_clear_msg_t),
                      learn_clear_resp_cb, &ret);
    return ret;
}

sdk_ret_t
learn_mac_clear (ep_mac_key_t *key)
{
    sdk_ret_t ret;
    learn_clear_msg_t request;

    if (key) {
        request.id = LEARN_CLEAR_MAC;
        memcpy(&request.mac_key, key, sizeof(ep_mac_key_t));
    } else {
        request.id = LEARN_CLEAR_MAC_ALL;
    }
    sdk::ipc::request(core::PDS_THREAD_ID_LEARN, LEARN_MSG_ID_CLEAR_CMD,
                      &request, sizeof(learn_clear_msg_t),
                      learn_clear_resp_cb, &ret);
    return ret;
}
