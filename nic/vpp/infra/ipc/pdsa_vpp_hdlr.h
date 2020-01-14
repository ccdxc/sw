//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_IPC_VPP_HDLR_H__
#define __VPP_INFRA_IPC_VPP_HDLR_H__

#ifdef __cplusplus
extern "C" {
#endif

// function prototypes
int pds_vpp_ipc_init(void);

// ipc log, to be used by CB functions
int ipc_log_notice(const char *fmt, ...);
int ipc_log_error(const char *fmt, ...);

// ipc infra internal functions
void pds_vpp_fd_register(int fd);
void pds_ipc_init(void);
void pds_ipc_read_fd(int fd);

#ifndef __cplusplus

#define PDS_MAX_KEY_LEN 16

// inline functions
always_inline u8
pds_id_equals(const u8 id1[PDS_MAX_KEY_LEN], const u8 id2[PDS_MAX_KEY_LEN])
{
    return !memcmp(id1, id2, PDS_MAX_KEY_LEN);
}

always_inline void
pds_id_set(u8 id1[PDS_MAX_KEY_LEN], const u8 id2[PDS_MAX_KEY_LEN])
{
    clib_memcpy(id1, id2, PDS_MAX_KEY_LEN);
}
#endif


#ifdef __cplusplus
}
#endif

#endif    // __VPP_INFRA_IPC_VPP_HDLR_H__
