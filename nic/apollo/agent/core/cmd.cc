//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <cerrno>
#include <sys/un.h>
#include "nic/sdk/include/sdk/fd.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/core/mem.hpp"
#include "gen/proto/debug.pb.h"

#define CMD_SERVER_SOCKET_PATH          "/var/run/cmd_server_sock"
#define CMD_IOVEC_DATA_LEN              (256)

namespace core {

static thread_local sdk::event_thread::io_t cmd_accept_io;

static void
cmd_server_read_cb (sdk::event_thread::io_t *io, int fd, int events)
{
    char iov_data[CMD_IOVEC_DATA_LEN];
    int cmd_fd,bytes_read;

    // read from existing connection
    if ((bytes_read = fd_recv(fd, &cmd_fd, &iov_data, CMD_IOVEC_DATA_LEN)) < 0) {
        PDS_TRACE_ERR("Receive fd failed");
    }
    // execute command
    if (bytes_read > 0) {
        pds::CommandCtxt proto_cmd_ctxt;
        pds::CommandResponse cmd_resp;
        cmd_ctxt_t cmd_ctxt = { 0 };
        sdk_ret_t ret;

        // convert to proto msg
        proto_cmd_ctxt.ParseFromArray(iov_data, bytes_read);
        // parse cmd ctxt
        pds_cmd_proto_to_cmd_ctxt(&cmd_ctxt, &proto_cmd_ctxt, cmd_fd);
        // handle cmd
        ret = debug::pds_handle_cmd(&cmd_ctxt);
        // close fd
        close(cmd_fd);
        // send response
        cmd_resp.set_apistatus(sdk_ret_to_api_status(ret));
        cmd_resp.SerializeToArray(iov_data, cmd_resp.ByteSizeLong());
        send(fd, iov_data, cmd_resp.ByteSizeLong(), 0);
    }
    // close connection
    close(fd);
    // stop the watcher
    sdk::event_thread::io_stop(io);
    // free the watcher
    SDK_FREE(PDS_MEM_ALLOC_CMD_READ_IO, io);
}

static void
cmd_server_accept_cb (sdk::event_thread::io_t *io, int fd, int events)
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
    sdk::event_thread::io_init(cmd_read_io, cmd_server_read_cb, fd2,
                               EVENT_READ);
    sdk::event_thread::io_start(cmd_read_io);
}

void
cmd_server_thread_init (void *ctxt)
{
    int fd;
    struct sockaddr_un sock_addr;

    // initialize unix socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        PDS_TRACE_ERR("Failed to open unix domain socket for cmd server thread");
        return;
    }

    memset(&sock_addr, 0, sizeof (sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, CMD_SERVER_SOCKET_PATH);

    if (bind(fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        PDS_TRACE_ERR ("Failed to bind unix domain socket for cmd server thread");
        return;
    }

    if (listen(fd, 1) == -1) {
        PDS_TRACE_ERR ("Failed to bind unix domain socket for fd receive");
        return;
    }

    sdk::event_thread::io_init(&cmd_accept_io, cmd_server_accept_cb, fd,
                               EVENT_READ);
    sdk::event_thread::io_start(&cmd_accept_io);
}

void
cmd_server_thread_exit (void *ctxt)
{
    sdk::event_thread::io_stop(&cmd_accept_io);
}

}    // namespace core
