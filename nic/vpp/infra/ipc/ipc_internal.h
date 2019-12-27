//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IPC_INTERNAL_H__
#define __VPP_IPC_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

// VPP ipc internal functions
void pds_vpp_fd_register(int fd);
void ipcshim_init(void);
void ipcshim_read_fd(void);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_IPC_INTERNAL_H__
