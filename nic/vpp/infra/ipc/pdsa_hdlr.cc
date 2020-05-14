//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <map>
#include <assert.h>
#include <nic/sdk/lib/ipc/ipc.hpp>
#include <nic/vpp/infra/cfg/pdsa_db.hpp>
#include <nic/vpp/infra/upgrade/upgrade_hdlr.hpp>
#include "pdsa_vpp_hdlr.h"
#include "pdsa_hdlr.hpp"

// maps storing the callback functions and context associated with a file
// descriptor. multiple file descriptors are provided to be monitored by the
// IPC infra. when poll reports a descriptor is ready, these maps provide the
// callback and context for the specific FD
static std::map<int, sdk::ipc::handler_cb> g_cb_func;
static std::map<int, const void *> g_cb_ctx;

// message rx counters for debugging
static unsigned int g_type_count[PDS_MSG_TYPE_MAX] = {0};

// callbacks registered for processing VPP commands received from PDS agent
static std::map<int, pds_cmd_cb_t> g_cmd_cbs;

// called from VPP main poll loop, whenere data is available on the zmq
// IPC file descriptor. Calls the SDK IPC library to read messages and
// dispatch them
void
pds_ipc_read_fd (int fd) {
    auto cb_func_it = g_cb_func.find(fd);
    auto cb_ctx_it = g_cb_ctx.find(fd);

    if (cb_func_it == g_cb_func.end()) {
        ipc_log_error("Attempting read on unknown fd %u", fd);
        return;
    }

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

    ipc_log_notice("Received msg with type:%u [count:%u] Returning OK", type,
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

    ipc_log_notice("Received msg with type:%u [count:%u] Returning Invalid OP",
                   type, g_type_count[type]);

    // always return ok
    ret = sdk::SDK_RET_INVALID_OP;
    sdk::ipc::respond(ipc_msg, (const void *)&ret, sizeof(sdk::sdk_ret_t));
}

// register callbacks from plugins for command messages
int
pds_ipc_register_cmd_callbacks (pds_cmd_msg_id_t msg_id, pds_cmd_cb_t cmd_hdl_cb)
{
    if (cmd_hdl_cb == NULL) {
        ipc_log_error("Registration request for msg id %d has invalid function"
                      "pointer", msg_id);
        return -1;
    }

    g_cmd_cbs[msg_id] = cmd_hdl_cb;

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

    g_type_count[PDS_MSG_TYPE_CFG_OBJ_SET]++;
    ipc_log_notice("Received msglist with %u message(s) [epoch:%u]",
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
        ipc_log_error("Execution fail of msglist [epoch:%u] ret:%u",
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
    pds_cmd_msg_t *msg;
    pds_cmd_rsp_t response;
    sdk::sdk_ret_t retcode = sdk::SDK_RET_OK;
    auto config_data = vpp_config_data::get();
    auto config_batch = vpp_config_batch::get();
    std::map<int, pds_cmd_cb_t>::iterator cb_fun_it;

    g_type_count[PDS_MSG_TYPE_CMD]++;

    // validate received message
    if (ipc_msg->length() != sizeof(pds_cmd_msg_t)) {
        retcode = sdk::SDK_RET_INVALID_ARG;
        goto error;
    }

    msg = (pds_cmd_msg_t *)ipc_msg->data();
    if (msg->id >= PDS_MSG_ID_MAX) {
        retcode = sdk::SDK_RET_INVALID_ARG;
        goto error;
    }

    cb_fun_it = g_cmd_cbs.find(msg->id);
    if (cb_fun_it != g_cmd_cbs.end()) {
        pds_cmd_cb_t cb_fun = cb_fun_it->second;
        retcode = (*cb_fun)(msg, &response);

        response.status = retcode;
        sdk::ipc::respond(ipc_msg, (const void *)&response, sizeof(response));
        ipc_log_notice("Execution success of command msg id:%u [count:%u]",
                       msg->id, g_type_count[PDS_MSG_TYPE_CMD]);
        return;
    } else {
        retcode = sdk::SDK_RET_INVALID_ARG;
    }

error:
    ipc_log_error("Execution fail of command msg [count:%u] ret:%u",
                  g_type_count[PDS_MSG_TYPE_CMD], retcode);
    response.status = (uint32_t )retcode;
    sdk::ipc::respond(ipc_msg, (const void *)&response, sizeof(response));
}

// handler for PDS IPC config get call. cfg get callbacks will send a reply msg
static void
pds_ipc_cfg_get_cb (sdk::ipc::ipc_msg_ptr ipc_msg, const void *ctx)
{
    pds_cfg_get_req_t *req;
    pds_cfg_get_rsp_t reply;
    sdk::sdk_ret_t retcode = sdk::SDK_RET_OK;
    auto config_data = vpp_config_data::get();
    auto config_batch = vpp_config_batch::get();

    g_type_count[PDS_MSG_TYPE_CFG_OBJ_GET]++;

    // validate received message
    if (ipc_msg->length() != sizeof(pds_cfg_get_req_t)) {
        retcode = sdk::SDK_RET_INVALID_ARG;
        goto error;
    }

    req = (pds_cfg_get_req_t *)ipc_msg->data();

    // read in the spec from the cfg store
    if (!config_data.exists(req->obj_id, req->key)) {
        retcode = sdk::SDK_RET_ENTRY_NOT_FOUND;
        goto error;
    }

    reply.obj_id = req->obj_id;
    config_data.get(req->key, reply);

    // read in status/stats from node plugin
    config_batch.read(reply);

    ipc_log_notice("Execution success of cfg get msg [count:%u]",
                   g_type_count[PDS_MSG_TYPE_CFG_OBJ_GET]);
    reply.status = (uint32_t )retcode;
    sdk::ipc::respond(ipc_msg, (const void *)&reply, sizeof(reply));
    return;

error:
    ipc_log_error("Execution fail of cfg get msg [count:%u] ret:%u",
                  g_type_count[PDS_MSG_TYPE_CFG_OBJ_GET], retcode);
    reply.status = (uint32_t )retcode;
    sdk::ipc::respond(ipc_msg, (const void *)&reply, sizeof(reply));
}

static void
pds_walk_cb (pds_cfg_msg_t *msg, void *cb_msg)
{
    pds_cfg_get_all_rsp_t *response = (pds_cfg_get_all_rsp_t *)cb_msg;
    auto config_batch = vpp_config_batch::get();

    switch(msg->obj_id) {
#define _(obj, data)                                                   \
    case OBJ_ID_##obj:                                                 \
        memcpy(&response->data[response->count].spec, &msg->data.spec, \
               sizeof(pds_##data##_spec_t));                           \
        config_batch.read(msg->obj_id,                                 \
                          (void *)&response->data[response->count]);   \
        break;

    _(DHCP_POLICY, dhcp_policy)
    _(NAT_PORT_BLOCK, nat_port_block)
    _(SECURITY_PROFILE, security_profile)

#undef _
    default:
        // other objects currently unsupported
        return;
    }
    response->count++;
}

// handler for PDS IPC config get call. cfg get callbacks will send a reply msg
static void
pds_ipc_cfg_getall_cb (sdk::ipc::ipc_msg_ptr ipc_msg, const void *ctx)
{
    pds_cfg_get_all_req_t *req;
    pds_cfg_get_all_rsp_t *response = NULL;
    sdk::sdk_ret_t retcode = sdk::SDK_RET_OK;
    vpp_config_data &config_data = vpp_config_data::get();
    int respsz = sizeof(pds_cfg_get_all_rsp_t);
    int num_inst;

    g_type_count[PDS_MSG_TYPE_CFG_OBJ_GET_ALL]++;

    // validate received message
    if (ipc_msg->length() != sizeof(pds_cfg_get_all_req_t)) {
        retcode = sdk::SDK_RET_INVALID_ARG;
        goto error;
    }

    req = (pds_cfg_get_all_req_t *)ipc_msg->data();
    if (req->obj_id >= OBJ_ID_MAX) {
        retcode = sdk::SDK_RET_INVALID_ARG;
        goto error;
    }

    // read in the spec from the cfg store
    num_inst = config_data.size(req->obj_id);
    if (num_inst == 0) {
        response = (pds_cfg_get_all_rsp_t *)calloc(1, respsz);
        response->status = sdk::SDK_RET_OK;
        response->count = 0;
        sdk::ipc::respond(ipc_msg, (const void *)response, respsz);
        free(response);
        return;
    }

    respsz = sizeof(pds_cfg_get_all_rsp_t) +
                    ((num_inst) * config_data.objsize(req->obj_id));
    response = (pds_cfg_get_all_rsp_t *)calloc(1, respsz);

    config_data.walk(req->obj_id, pds_walk_cb, (void *)response);
    response->status = (uint32_t )sdk::SDK_RET_OK;

    ipc_log_notice("Execution success of cfg getall msg [count:%u]",
                   g_type_count[PDS_MSG_TYPE_CFG_OBJ_GET_ALL]);
    sdk::ipc::respond(ipc_msg, (const void *)response, respsz);

    free(response);
    return;

error:
    ipc_log_error("Execution fail of cfg getall msg [count:%u] ret:%u",
                  g_type_count[PDS_MSG_TYPE_CFG_OBJ_GET_ALL], retcode);
    if (response == NULL) {
        respsz = sizeof(pds_cfg_get_all_rsp_t);
        response = (pds_cfg_get_all_rsp_t *)calloc(1, respsz);
    }
    response->status = (uint32_t )retcode;
    sdk::ipc::respond(ipc_msg, (const void *)response, respsz);
    free(response);
}


// VPP IPC initialization. Register VPP with IPC infra, and install callbacks
// for handling base message types
void
pds_ipc_init (void)
{
    sdk::ipc::ipc_init_async(PDS_IPC_ID_VPP, pds_vpp_fd_watch_cb,
                             (const void *)pds_vpp_fd_watch_cb);

    // register with the upgrade manager for upgrade IPC messages.
    vpp_upg_init();

    // register "NONE" messages with INVALID handler
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_NONE, pds_ipc_invalid_type_cb,
                                  (const void *)PDS_MSG_TYPE_NONE);

    // register placeholder callbacks.
    // TODO: must be removed when actual handlers are implemented
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_EVENT, pds_ipc_ok_type_cb,
                                  (const void *)PDS_MSG_TYPE_EVENT);

    // register handler for configuration batches
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_CFG_OBJ_SET, pds_ipc_msglist_cb,
                                  NULL);

    // register handler for configuration read
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_CFG_OBJ_GET, pds_ipc_cfg_get_cb,
                                  NULL);

    // register handler for configuration read all
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_CFG_OBJ_GET_ALL,
                                  pds_ipc_cfg_getall_cb, NULL);

    // register handler for command messages
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_CMD, pds_ipc_cmd_msg_cb, NULL);

    ipc_log_notice("Registered callbacks for IPC messages");
}
