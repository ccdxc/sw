//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <assert.h>
#include <nic/sdk/lib/ipc/ipc.hpp>
#include "pdsa_hdlr.hpp"
#include "pdsa_vpp_cfg.hpp"
#include "pdsa_vpp_hdlr.h"
#include <map>

// maps storing the callback functions and context associated with a file
// descriptor. multiple file descriptors are provided to be monitored by the
// IPC infra. when poll reports a descriptor is ready, these maps provide the
// callback and context for the specific FD
static std::map<int, sdk::ipc::handler_cb> g_cb_func;
static std::map<int, const void *> g_cb_ctx;

// message rx counters for debugging
static unsigned int g_type_count[PDS_MSG_TYPE_MAX] = {0};

// called from VPP main poll loop, whenere data is available on the zmq
// IPC file descriptor. Calls the SDK IPC library to read messages and
// dispatch them
void
pds_ipc_read_fd (int fd) {
    auto cb_func_it = g_cb_func.find(fd);
    auto cb_ctx_it = g_cb_ctx.find(fd);

    if (cb_func_it == g_cb_func.end()) {
        ipc_log_error("Attempting read on unknown fd %d", fd);
        return;
    }

    ipc_log_notice("Data available on fd %d", fd);

    sdk::ipc::handler_cb cb_func = cb_func_it->second;
    const void *cb_ctx = cb_ctx_it->second;

    cb_func(fd, cb_ctx);
}

// callback called when initializing IPC. Provides a file desciptor to
// be monitored via poll, and a callback function to be called when
// data is available on the file descriptor
static void
pds_vpp_fd_watch_cb (int fd, sdk::ipc::handler_cb cb, const void *set_ctx,
                    const void *ctx) {
    // confirm that the context is the ptr to this fn
    assert(ctx == (const void *)pds_vpp_fd_watch_cb);

    // store provided data
    g_cb_func[fd] = cb;
    g_cb_ctx[fd] = set_ctx;

    pds_vpp_fd_register(fd);
}

/// \brief dummy "OK" callback, used until actual code is ready
static void
pds_ipc_ok_type_cb (sdk::ipc::ipc_msg_ptr ipc_msg, const void *ctx) {
    sdk::sdk_ret_t ret;
    uint64_t       type = (uint64_t )ctx;

    g_type_count[type]++;

    ipc_log_notice("Received msg with type:%d [count:%d] Returning OK", type,
                   g_type_count[type]);

    // always return ok
    ret = sdk::SDK_RET_OK;
    sdk::ipc::respond(ipc_msg, (const void *)&ret, sizeof(sdk::sdk_ret_t));
}

/// \brief dummy "NOK" callback, used for unsupported messages
static void
pds_ipc_invalid_type_cb (sdk::ipc::ipc_msg_ptr ipc_msg, const void *ctx) {
    sdk::sdk_ret_t ret;
    uint64_t       type = (uint64_t )ctx;

    g_type_count[type]++;

    ipc_log_notice("Received msg with type:%d [count:%d] Returning Invalid OP",
                   type, g_type_count[type]);

    // always return ok
    ret = sdk::SDK_RET_INVALID_OP;
    sdk::ipc::respond(ipc_msg, (const void *)&ret, sizeof(sdk::sdk_ret_t));
}

// register callbacks from plugins for messages
// return 0 indicates  registered successfully
// return non-zero indicates registration fail (invalid param)
int
pds_ipc_register_callbacks (obj_id_t id,
                            pds_cfg_set_cb set_cb_fn,
                            pds_cfg_del_cb del_cb_fn,
                            pds_cfg_act_cb act_cb_fn ) {
    if ((set_cb_fn == NULL) || (del_cb_fn == NULL)) {
        ipc_log_error("Registration request for obj id %d"
                      " has invalid function pointers", id);
        return -1;
    }

    if ((id != OBJ_ID_VNIC) &&
        (id != OBJ_ID_SUBNET) &&
        (id != OBJ_ID_DHCP_RELAY) &&
        (id != OBJ_ID_DHCP_POLICY) &&
        (id != OBJ_ID_NAT_PORT_BLOCK) &&
        (id != OBJ_ID_SECURITY_PROFILE)) {
        ipc_log_error("Registration request for invalid obj id %d", id);
        return -1;
    }

    vpp_config_batch::register_cbs(id, set_cb_fn, del_cb_fn, act_cb_fn);

    ipc_log_notice("Callbacks registered for obj id %d", id);

    return 0;
}


// handler for batch messages from HAL
static void
pds_ipc_msglist_cb (sdk::ipc::ipc_msg_ptr ipc_msg, const void *ctx) {
    sdk::sdk_ret_t  ret;
    pds_msg_list_t *msglist = NULL;
    auto config_batch = vpp_config_batch::get();

    if (ipc_msg->length() < sizeof(pds_msg_list_t)) {
        ret = sdk::SDK_RET_INVALID_ARG;
        goto error;
    }

    msglist = (pds_msg_list_t *)ipc_msg->data();

    g_type_count[PDS_MSG_TYPE_CFG]++;
    ipc_log_notice("Received msglist with %d message(s) [epoch:%u]",
                   msglist->num_msgs, msglist->epoch);

    // Stage 1: setup batch
    ret = config_batch.create(msglist);
    if (ret != sdk::SDK_RET_OK) {
        ipc_log_error("Reserve failed [epoch:%u]", msglist->epoch);
        goto error;
    }

    // Stage 2: Publish
    ret = config_batch.commit();
    if (ret != sdk::SDK_RET_OK) {
        ipc_log_error("Commit failed [epoch:%u]", msglist->epoch);
    }

error:
    // clear batch on both success and failure
    config_batch.clear();
    if (ret != sdk::SDK_RET_OK) {
        ipc_log_error("Execution fail of msglist [epoch:%u] ret:%d",
                      (msglist ? msglist->epoch : 0), ret);
    } else {
        ipc_log_notice("Successful Execution of msglist [epoch:%u]",
                       msglist->epoch);
    }
    sdk::ipc::respond(ipc_msg, (const void *)&ret, sizeof(sdk::sdk_ret_t));
}

// handler for PDS IPC command call. command callbacks will have more than a
// return code, so instead they return a pds_msg_t pointer
static void
pds_ipc_cmd_msg_cb (sdk::ipc::ipc_msg_ptr ipc_msg, const void *ctx) {
    pds_msg_t *msg, response;
    sdk::sdk_ret_t retcode;
    auto config_data = vpp_config_data::get();

    g_type_count[PDS_MSG_TYPE_CMD]++;

    // validate received message
    if (ipc_msg->length() != sizeof(pds_msg_t)) {
        retcode = sdk::SDK_RET_INVALID_ARG;
        goto error;
    }

    msg = (pds_msg_t *)ipc_msg->data();
    if (msg->id >= PDS_MSG_ID_MAX) {
        retcode = sdk::SDK_RET_INVALID_ARG;
        goto error;
    }

    if (!config_data.exists(msg->cfg_msg)) {
        retcode = sdk::SDK_RET_ENTRY_NOT_FOUND;
        goto error;
    }

    memcpy(&response.cfg_msg, &msg->cfg_msg, sizeof(pds_cfg_msg_t));
    config_data.get(response.cfg_msg);

    ipc_log_notice("Execution success of command msg [count:%d]",
                   g_type_count[PDS_MSG_TYPE_CMD]);
    sdk::ipc::respond(ipc_msg, (const void *)&response, sizeof(pds_msg_t));
    return;

error:
    ipc_log_error("Execution fail of command msg [count:%d] ret:%d",
                  g_type_count[PDS_MSG_TYPE_CMD], retcode);
    sdk::ipc::respond(ipc_msg, (const void *)&retcode, sizeof(sdk::sdk_ret_t));
}

// VPP IPC initialization. Register VPP with IPC infra, and install callbacks
// for handling base message types
void
pds_ipc_init (void)
{
    sdk::ipc::ipc_init_async(PDS_IPC_ID_VPP, pds_vpp_fd_watch_cb,
                             (const void *)pds_vpp_fd_watch_cb);

    // register "NONE" messages with INVALID handler
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_NONE, pds_ipc_invalid_type_cb,
                                  (const void *)PDS_MSG_TYPE_NONE);

    // register placeholder callbacks.
    // TODO: must be removed when actual handlers are implemented
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_EVENT, pds_ipc_ok_type_cb,
                                  (const void *)PDS_MSG_TYPE_EVENT);

    // register handler for configuration batches
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_CFG, pds_ipc_msglist_cb, NULL);

    // register handler for command messages
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_CMD, pds_ipc_cmd_msg_cb, NULL);

    ipc_log_notice("Registered callbacks for IPC messages");
}
