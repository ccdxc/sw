//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

// This file initializes a Unix Domain Socket which is used to communicate
// between VPP and HAL.

#include "uds.h"
#include "uds_internal.h"
#include <vppinfra/socket.h>
#include <vppinfra/file.h>
#include <vlib/unix/unix.h>

#define SOCKET_FILE "/run/vpp/pds.sock"
#define BUF_SIZE 4096
clib_socket_t vpp_ipc_socket;

static clib_error_t *
vpp_uds_read_ready (clib_file_t * uf)
{
    u8 *input_buf = 0;
    int n;

    vec_resize (input_buf, BUF_SIZE);
    n = read (uf->file_descriptor, input_buf, vec_len (input_buf));
    if (n < 0 && errno != EAGAIN) {
        return clib_error_return_unix (0, "read");
    } else if (n == 0 && errno != EAGAIN) {
        clib_file_del (&file_main, uf);
    }

    udswrap_process_input(uf->file_descriptor, (char *)input_buf, n);
    clib_file_del (&file_main, uf);
    vec_free (input_buf);
    return 0;
}

static clib_error_t *
vpp_uds_write_ready (clib_file_t * uf)
{
    return 0;
}

static clib_error_t *
vpp_uds_error_detected (clib_file_t * uf)
{
    clib_file_del (&file_main, uf);
    return 0;
}

// Socket has a new connection.
static clib_error_t *
vpp_uds_listen_read_ready (clib_file_t * uf)
{
    clib_socket_t *s = &vpp_ipc_socket;
    clib_error_t *error;
    clib_socket_t client;
    clib_file_t clib_file = { 0 };

    error = clib_socket_accept (s, &client);
    if (error) {
        return error;
    }

    clib_file.read_function = vpp_uds_read_ready;
    clib_file.write_function = vpp_uds_write_ready;
    clib_file.error_function = vpp_uds_error_detected;
    clib_file.file_descriptor = client.fd;
    clib_file.private_data = 0;
    clib_file.description = format (0, "VPP UDS IPC client");
    clib_file_add (&file_main, &clib_file);

    return 0;
}

// Initialize a Unix Domain Socket and register the FD with VPP event loop
int
vpp_uds_init()
{
    clib_socket_t *s = &vpp_ipc_socket;
    clib_error_t *error = 0;

    s->config = SOCKET_FILE;
    s->flags = CLIB_SOCKET_F_IS_SERVER |
        CLIB_SOCKET_F_SEQPACKET |
        CLIB_SOCKET_F_ALLOW_GROUP_WRITE;    /* PF_LOCAL socket only */

    // makedir of file socket
    u8 *tmp = format (0, "%s", "/run/vpp");
    vlib_unix_recursive_mkdir ((char *) tmp);
    vec_free (tmp);

    error = clib_socket_init (s);
    if (error) {
        clib_error_report (error);
        return 1;
    }
        
    clib_file_t clib_file = { 0 };
    clib_file.read_function = vpp_uds_listen_read_ready;
    clib_file.file_descriptor = s->fd;
    clib_file.description = format (0, "VPP UDS IPC listener %s", s->config);
    clib_file_add (&file_main, &clib_file);

    return 0;
}
