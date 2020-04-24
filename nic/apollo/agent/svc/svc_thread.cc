//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <cerrno>
#include <sys/un.h>
#include "nic/sdk/include/sdk/fd.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/vpc_svc.hpp"
#include "nic/apollo/agent/svc/vnic_svc.hpp"
#include "nic/apollo/agent/svc/subnet_svc.hpp"
#include "nic/apollo/agent/svc/policy_svc.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/core/mem.hpp"
#include "gen/proto/types.pb.h"

#define SVC_SERVER_SOCKET_PATH          "/var/run/pds_svc_server_sock"
#define CMD_IOVEC_DATA_LEN              (256)

#define UPG_EV_PDS_AGENT_NAME "pdsagent"

namespace core {

static thread_local sdk::event_thread::io_t cmd_accept_io;

static sdk_ret_t
pds_handle_cfg (int fd, cfg_ctxt_t *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    char *iov_data;
    types::ServiceResponseMessage proto_rsp;
    google::protobuf::Any *any_resp = proto_rsp.mutable_response();

    PDS_TRACE_VERBOSE("Received UDS config message {}", ctxt->cfg);

    switch (ctxt->cfg) {
    case CFG_MSG_VPC_CREATE:
    case CFG_MSG_VPC_UPDATE:
    case CFG_MSG_VPC_DELETE:
    case CFG_MSG_VPC_GET:
        ret = pds_svc_vpc_handle_cfg(ctxt, any_resp);
        break;
    case CFG_MSG_VPC_PEER_CREATE:
    case CFG_MSG_VPC_PEER_DELETE:
    case CFG_MSG_VPC_PEER_GET:
        ret = pds_svc_vpc_peer_handle_cfg(ctxt, any_resp);
        break;
    case CFG_MSG_VNIC_CREATE:
    case CFG_MSG_VNIC_UPDATE:
    case CFG_MSG_VNIC_DELETE:
    case CFG_MSG_VNIC_GET:
        ret = pds_svc_vnic_handle_cfg(ctxt, any_resp);
        break;
    case CFG_MSG_SUBNET_CREATE:
    case CFG_MSG_SUBNET_UPDATE:
    case CFG_MSG_SUBNET_DELETE:
    case CFG_MSG_SUBNET_GET:
        ret = pds_svc_subnet_handle_cfg(ctxt, any_resp);
        break;
    case CFG_MSG_SECURITY_POLICY_CREATE:
    case CFG_MSG_SECURITY_POLICY_UPDATE:
    case CFG_MSG_SECURITY_POLICY_DELETE:
    case CFG_MSG_SECURITY_POLICY_GET:
        ret = pds_svc_security_policy_handle_cfg(ctxt, any_resp);
        break;
    case CFG_MSG_SECURITY_PROFILE_CREATE:
    case CFG_MSG_SECURITY_PROFILE_UPDATE:
    case CFG_MSG_SECURITY_PROFILE_DELETE:
    case CFG_MSG_SECURITY_PROFILE_GET:
        ret = pds_svc_security_profile_handle_cfg(ctxt, any_resp);
        break;
    case CFG_MSG_SECURITY_RULE_CREATE:
    case CFG_MSG_SECURITY_RULE_UPDATE:
    case CFG_MSG_SECURITY_RULE_DELETE:
    case CFG_MSG_SECURITY_RULE_GET:
        ret = pds_svc_security_rule_handle_cfg(ctxt, any_resp);
        break;
    default:
        return SDK_RET_INVALID_ARG;
    }

    proto_rsp.set_apistatus(sdk_ret_to_api_status(ret));
    iov_data = (char *)SDK_CALLOC(PDS_MEM_ALLOC_CMD_READ_IO, proto_rsp.ByteSizeLong());
    if (proto_rsp.SerializeToArray(iov_data, proto_rsp.ByteSizeLong())) {
        if (send(fd, iov_data, proto_rsp.ByteSizeLong(), 0) < 0) {
            PDS_TRACE_ERR("Send on socket failed. Error {}", errno);
        }
    } else {
        PDS_TRACE_ERR("Serializing config message {} response failed", ctxt->cfg);
    }
    SDK_FREE(PDS_MEM_ALLOC_CMD_READ_IO, iov_data);

    return SDK_RET_OK;
}

static sdk_ret_t
handle_svc_req (int fd, svc_req_ctxt_t *req)
{
    sdk_ret_t ret;

    switch (req->type) {
    case SVC_REQ_TYPE_CFG:
        ret = pds_handle_cfg(fd, &req->cfg_ctxt);
        break;
    case SVC_REQ_TYPE_CMD:
        {
            types::ServiceResponseMessage proto_rsp;
            char iov_data[CMD_IOVEC_DATA_LEN];

            ret = debug::pds_handle_cmd(&req->cmd_ctxt);
            proto_rsp.set_apistatus(sdk_ret_to_api_status(ret));
            proto_rsp.SerializeToArray(iov_data, proto_rsp.ByteSizeLong());
            send(fd, iov_data, proto_rsp.ByteSizeLong(), 0);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
    }
    return ret;
}

static void
svc_server_read_cb (sdk::event_thread::io_t *io, int fd, int events)
{
    char iov_data[CMD_IOVEC_DATA_LEN];
    types::ServiceRequestMessage proto_req;
    svc_req_ctxt_t svc_req;
    int cmd_fd, bytes_read;

    // read from existing connection
    if ((bytes_read = fd_recv(fd, &cmd_fd, &iov_data, CMD_IOVEC_DATA_LEN)) < 0) {
        PDS_TRACE_ERR("Receive fd failed");
    }
    // execute command
    if (bytes_read > 0) {
        // convert to proto msg
        if (proto_req.ParseFromArray(iov_data, bytes_read)) {
            // parse cmd ctxt
            pds_svc_req_proto_to_svc_req_ctxt(&svc_req, &proto_req, cmd_fd);
            // handle cmd
            handle_svc_req(fd, &svc_req);
        } else {
            PDS_TRACE_ERR("Parse service request message from socket failed");
        }
        // close fd
        if (cmd_fd >= 0) {
            close(cmd_fd);
        }
    }
    // close connection
    close(fd);
    // stop the watcher
    sdk::event_thread::io_stop(io);
    // free the watcher
    SDK_FREE(PDS_MEM_ALLOC_CMD_READ_IO, io);
}

static void
svc_server_accept_cb (sdk::event_thread::io_t *io, int fd, int events)
{
    sdk::event_thread::io_t *cmd_read_io;
    int fd2;

    // accept incoming connection
    if ((fd2 = accept(fd, NULL, NULL)) == -1) {
        PDS_TRACE_ERR("Accept failed");
        return;
    }

    // allocate memory for cmd_read_io
    cmd_read_io = (sdk::event_thread::io_t *)
                    SDK_MALLOC(PDS_MEM_ALLOC_CMD_READ_IO,
                               sizeof(sdk::event_thread::io_t));
    if (cmd_read_io == NULL) {
        PDS_TRACE_ERR("Memory allocation for cmd_read_io failed");
        return;
    }

    // Initialize and start watcher to read client requests
    sdk::event_thread::io_init(cmd_read_io, svc_server_read_cb, fd2,
                               EVENT_READ);
    sdk::event_thread::io_start(cmd_read_io);
}

static void
upg_ev_fill (sdk::upg::upg_ev_t *ev)
{
    ev->svc_ipc_id = PDS_AGENT_THREAD_ID_SVC_SERVER;
    strncpy(ev->svc_name, UPG_EV_PDS_AGENT_NAME, sizeof(ev->svc_name));
    ev->compat_check_hdlr = pds_upgrade;
    ev->start_hdlr = pds_upgrade;
    ev->backup_hdlr = pds_upgrade;
    ev->prepare_hdlr = pds_upgrade;
    ev->prepare_switchover_hdlr = pds_upgrade;
    ev->switchover_hdlr = pds_upgrade;
    ev->rollback_hdlr = pds_upgrade;
    ev->ready_hdlr = pds_upgrade;
    ev->repeal_hdlr = pds_upgrade;
    ev->finish_hdlr = pds_upgrade;
}


void
svc_server_thread_init (void *ctxt)
{
    int fd;
    struct sockaddr_un sock_addr;
    sdk::upg::upg_ev_t ev;

    // register for upgrade events
    upg_ev_fill(&ev);
    sdk::upg::upg_ev_hdlr_register(ev);

    // initialize unix socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        PDS_TRACE_ERR("Failed to open unix domain socket for cmd server thread");
        return;
    }

    memset(&sock_addr, 0, sizeof (sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, SVC_SERVER_SOCKET_PATH);

    if (bind(fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        PDS_TRACE_ERR ("Failed to bind unix domain socket for cmd server thread");
        return;
    }

    if (listen(fd, 1) == -1) {
        PDS_TRACE_ERR ("Failed to bind unix domain socket for fd receive");
        return;
    }

    sdk::event_thread::io_init(&cmd_accept_io, svc_server_accept_cb, fd,
                               EVENT_READ);
    sdk::event_thread::io_start(&cmd_accept_io);
}

void
svc_server_thread_exit (void *ctxt)
{
    sdk::event_thread::io_stop(&cmd_accept_io);
}

}    // namespace core
