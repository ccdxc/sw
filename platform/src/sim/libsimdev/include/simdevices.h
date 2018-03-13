/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __SIMDEVICES_H__
#define __SIMDEVICES_H__

#include "src/sim/libsimlib/include/simmsg.h"

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

struct create_mr_cmd;
struct create_mr_comp;
struct create_cq_cmd;
struct create_cq_comp;
struct create_qp_cmd;
struct create_qp_comp;
struct modify_qp_cmd;
struct modify_qp_comp;

void simdev_msg_handler(int fd, simmsg_t *m);

typedef struct simdev_api_s {
    void (*set_user)(const char *user);
    void (*log)(const char *fmt, va_list ap);
    void (*error)(const char *fmt, va_list ap);
    int (*doorbell)(u_int64_t addr, u_int64_t data);
    int (*read_reg)(u_int64_t addr, u_int32_t *data);
    int (*write_reg)(u_int64_t addr, u_int32_t data);
    int (*read_mem)(u_int64_t addr, void *buf, size_t size);
    int (*write_mem)(u_int64_t addr, void *buf, size_t size);
    int (*host_read_mem)(u_int64_t addr, void *buf, size_t size);
    int (*host_write_mem)(const u_int64_t addr,
                          const void *buf,
                          const size_t size);
    void (*hal_create_mr)(struct create_mr_cmd *cmd,
                          struct create_mr_comp *comp,
                          u_int32_t *done);
    void (*hal_create_cq)(struct create_cq_cmd *cmd,
                          struct create_cq_comp *comp,
                          u_int32_t *done);
    void (*hal_create_qp)(struct create_qp_cmd *cmd,
                          struct create_qp_comp *comp,
                          u_int32_t *done);
    void (*hal_modify_qp)(struct modify_qp_cmd *cmd,
                          struct modify_qp_comp *comp,
                          u_int32_t *done);
    void (*set_lif) (u_int32_t lif);
} simdev_api_t;

int simdev_open(simdev_api_t *api);
void simdev_close(void);
int simdev_add_dev(const char *devparams);

#ifdef __cplusplus
}
#endif

#endif /* __SIMDEVICES_H__ */
