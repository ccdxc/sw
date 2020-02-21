/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include <vppinfra/file.h>
#include "pdsa_vpp_hdlr.h"

// logging class
static vlib_log_class_t pds_ipc_log = 0;

// callback function called from the main VPP poll loop.  Called when there's
// data in the IPC fd
static clib_error_t *
pds_vpp_ipc_fd_read (clib_file_t * uf) {
    vlib_worker_thread_barrier_sync(vlib_get_main());
    pds_ipc_read_fd(uf->file_descriptor);
    vlib_worker_thread_barrier_release(vlib_get_main());

    return 0;
}

// register the FD with the VPP main poll loop
void
pds_vpp_fd_register (int fd) {
    clib_file_t clib_file = { 0 };

    // now that we have the fd, register with VPP poll infra
    clib_file.read_function   = pds_vpp_ipc_fd_read;
    clib_file.private_data    = 0; // perhaps change to something else?
    clib_file.file_descriptor = fd;
    clib_file.flags           = UNIX_FILE_EVENT_EDGE_TRIGGERED;

    // register file desc
    clib_file_add(&file_main, &clib_file);
    ipc_log_notice("IPC file descriptor %d registered with main loop", fd);
}

// wrapper over vlib_log callable from C++
int
ipc_log_notice (const char *fmt, ...) {
    va_list va;
    int ret;
    static char buf[128];

    va_start(va, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    vlib_log(VLIB_LOG_LEVEL_NOTICE, pds_ipc_log, buf);
    return ret;
}

// wrapper over vlib_log callable from C++
int
ipc_log_error (const char *fmt, ...) {
    int ret;
    va_list va;
    static char buf[128];

    va_start(va, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    vlib_log(VLIB_LOG_LEVEL_ERR, pds_ipc_log, buf);
    return ret;
}

// initialize the IPC server to receive messages
int
pds_vpp_ipc_init (void)
{
    pds_ipc_log = vlib_log_register_class("pds-ipc", 0);
    assert(pds_ipc_log != 0);

    ipc_log_notice("IPC request handler registered");

    pds_ipc_init();
    return 0;
}
