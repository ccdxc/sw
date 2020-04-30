//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <cerrno>
#include <sys/un.h>
#include "nic/sdk/include/sdk/uds.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/agent/svc/svc_thread.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/core/mem.hpp"
#include "gen/proto/types.pb.h"

#define SVC_SERVER_SOCKET_PATH          "/var/run/pds_svc_server_sock"
#define CMD_IOVEC_DATA_LEN              (1024 * 1024)

#define UPG_EV_PDS_AGENT_NAME "pdsagent"

namespace core {

static thread_local sdk::event_thread::io_t cmd_accept_io;

static void
svc_server_read_cb (sdk::event_thread::io_t *io, int fd, int events)
{
    char *iov_data;
    types::ServiceRequestMessage proto_req;
    int cmd_fd, bytes_read;

    // allocate memory to read from socket
    iov_data = (char *)SDK_CALLOC(PDS_MEM_ALLOC_CMD_READ_IO, CMD_IOVEC_DATA_LEN);
    // read from existing connection
    if ((bytes_read = uds_recv(fd, &cmd_fd, iov_data, CMD_IOVEC_DATA_LEN)) < 0) {
        PDS_TRACE_ERR("Read from unix domain socket failed");
        return;
    }
    // execute command
    if (bytes_read > 0) {
        // convert to proto msg
        if (proto_req.ParseFromArray(iov_data, bytes_read)) {
            // handle cmd
            handle_svc_req(fd, &proto_req, cmd_fd);
        } else {
            PDS_TRACE_ERR("Parse service request message from socket failed, "
                          "bytes_read {}", bytes_read);
        }
        // close fd
        if (cmd_fd >= 0) {
            close(cmd_fd);
        }
    }
    // free iov data
    SDK_FREE(PDS_MEM_ALLOC_CMD_READ_IO, iov_data);
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
