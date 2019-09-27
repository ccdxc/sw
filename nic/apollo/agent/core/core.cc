//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <cerrno>
#include <sys/un.h>
#include "nic/sdk/include/sdk/fd.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "gen/proto/debug.pb.h"

#define FD_RECV_SOCKET_PATH             "/var/run/fd_recv_sock"
#define CMD_IOVEC_DATA_LEN              (256)
namespace core {

void *
fd_recv_thread_start (void *ctxt)
{
    int sock_fd, sock_fd2, max_fd;
    fd_set master_set, active_set;
    struct sockaddr_un sock_addr;
    int ret;

    // initialize unix socket
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        PDS_TRACE_ERR("Failed to open unix domain socket for fd receive");
        return NULL;
    }

    memset(&sock_addr, 0, sizeof (sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, FD_RECV_SOCKET_PATH);

    if (bind(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        PDS_TRACE_ERR ("Failed to bind unix domain socket for fd receive");
        return NULL;
    }
    if (listen(sock_fd, 1) == -1) {
        PDS_TRACE_ERR ("Failed to bind unix domain socket for fd receive");
        return NULL;
    }

    FD_ZERO(&master_set);
    FD_SET(sock_fd, &master_set);
    max_fd = sock_fd;

    while (1) {
        active_set = master_set;

        if ((ret = select(max_fd + 1, &active_set, NULL, NULL, NULL)) < 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                PDS_TRACE_ERR("Select failed");
                break;
            }
        }

        for (int i = 0; i <= max_fd && ret > 0; i++) {
            // check if FD is ready
            if (FD_ISSET(i, &active_set)) {
                ret--;
                if (i == sock_fd) {
                    // accept incoming connection
                    if ((sock_fd2 = accept(sock_fd, NULL, NULL)) == -1) {
                        PDS_TRACE_ERR("Accept failed");
                        continue;
                    }
                    FD_SET(sock_fd2, &master_set);
                    if (sock_fd2 > max_fd) {
                        max_fd = sock_fd2;
                    }
                } else {
                    char iov_data[CMD_IOVEC_DATA_LEN];
                    int fd, bytes_read;

                    // read from existing connection
                    if ((bytes_read = fd_recv(i, &fd, &iov_data, CMD_IOVEC_DATA_LEN)) < 0) {
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
                        pds_cmd_proto_to_cmd_ctxt(&cmd_ctxt, &proto_cmd_ctxt, fd);
                        // handle cmd
                        ret = debug::pds_handle_cmd(&cmd_ctxt);
                        // close fd
                        close(fd);
                        // send response
                        cmd_resp.set_apistatus(sdk_ret_to_api_status(ret));
                        cmd_resp.SerializeToArray(iov_data, cmd_resp.ByteSizeLong());
                        send(i, iov_data, cmd_resp.ByteSizeLong(), 0);
                    }
                    // close connection
                    close(i);
                    FD_CLR(i, &master_set);
                    if (i == max_fd) {
                        while (FD_ISSET (max_fd, &master_set) == FALSE) {
                            max_fd -= 1;
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

}    // namespace core
