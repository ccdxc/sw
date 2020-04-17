//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// PDS config/command msg related helper APIs
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/core/msg.hpp"
#include "nic/apollo/api/core/msg.h"

namespace core {

pds_msg_list_t *
pds_msg_list_alloc (pds_msg_type_t type, pds_epoch_t epoch, uint32_t num_msgs)
{
    pds_msg_list_t *msg_list;
    msg_list = (pds_msg_list_t *)SDK_MALLOC(SDK_MEM_PDS_MSG_LIST,
                                            sizeof(pds_msg_list_t) +
                                                (num_msgs * sizeof(pds_msg_t)));
    if (msg_list) {
        msg_list->type = type;
        msg_list->epoch = epoch;
        msg_list->num_msgs = num_msgs;
    }
    return msg_list;
}

void
pds_msg_list_free (pds_msg_list_t *msg_list)
{
    if (msg_list) {
        SDK_FREE(SDK_MEM_PDS_MSG_LIST, msg_list);
    }
}

uint32_t
pds_msg_list_size (pds_msg_list_t *msg_list)
{
    return (sizeof(pds_msg_list_t) +
                (msg_list->num_msgs * sizeof(pds_msg_t)));
}

pds_msg_t *
pds_msg (pds_msg_list_t *msg_list, uint32_t idx)
{
    SDK_ASSERT(idx < msg_list->num_msgs);
    return &msg_list->msgs[idx];
}

/// \brief handle command msg reply from VPP
void
pds_cmd_response_handler_cb (sdk::ipc::ipc_msg_ptr msg, const void *ret)
{
    pds_cmd_reply_msg_t *response = (pds_cmd_reply_msg_t *)ret;

    if (msg->length() != sizeof(pds_cmd_reply_msg_t)) {
        response->status = sdk::SDK_RET_INVALID_ARG;
        return;
    }
    memcpy(response, msg->data(), msg->length());
}


}    // namespace core
