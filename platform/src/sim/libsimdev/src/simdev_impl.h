/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __SIMDEV_IMPL_H__
#define __SIMDEV_IMPL_H__

#include <sys/queue.h>

#include "src/sim/libsimlib/include/simmsg.h"

struct simdev_s;
typedef struct simdev_s simdev_t;

typedef struct dev_ops_s {
    int  (*init) (simdev_t *sd, const char *devparams);
    void (*free) (simdev_t *sd);
    int  (*cfgrd)(simdev_t *sd, simmsg_t *m, u_int64_t *valp);
    void (*cfgwr)(simdev_t *sd, simmsg_t *m);
    int  (*memrd)(simdev_t *sd, simmsg_t *m, u_int64_t *valp);
    void (*memwr)(simdev_t *sd, simmsg_t *m);
    int  (*iord) (simdev_t *sd, simmsg_t *m, u_int64_t *valp);
    void (*iowr) (simdev_t *sd, simmsg_t *m);
} dev_ops_t;

struct simdev_s {
    int fd;
    u_int16_t bdf;
    void *priv;
    dev_ops_t *ops;
    TAILQ_ENTRY(simdev_s) list;
};

int  generic_init(simdev_t *sd, const char *devparams);
void generic_free(simdev_t *sd);
int  generic_cfgrd(simdev_t *sd, simmsg_t *m, u_int64_t *valp);
void generic_cfgwr(simdev_t *sd, simmsg_t *m);
int  generic_memrd(simdev_t *sd, simmsg_t *m, u_int64_t *valp);
void generic_memwr(simdev_t *sd, simmsg_t *m);
int  generic_iord (simdev_t *sd, simmsg_t *m, u_int64_t *valp);
void generic_iowr (simdev_t *sd, simmsg_t *m);

void simdev_log(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
void simdev_error(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
int simdev_doorbell(u_int64_t addr, u_int64_t data);
int simdev_read_reg(u_int64_t addr, u_int32_t *data);
int simdev_read_regs(u_int64_t addr, u_int32_t *data, int nw);
int simdev_write_reg(u_int64_t addr, u_int32_t data);
int simdev_write_regs(u_int64_t addr, u_int32_t *data, int nw);
int simdev_read_mem(u_int64_t addr, void *buf, size_t size);
int simdev_write_mem(u_int64_t addr, void *buf, size_t size);
void simdev_set_user(const char *user);

simdev_t *simdev_by_bdf(const u_int16_t bdf);

int devparam_str(const char *devparams, const char *key,
                 char *buf, const size_t bufsz);
int devparam_mac(const char *devparams, const char *key, mac_t *m);
int devparam_int(const char *devparams, const char *key, int *val);
int devparam_u64(const char *devparams, const char *key, u_int64_t *val);
int devparam_bdf(const char *devparams, const char *key, u_int16_t *bdf);

void devparam_get_str(const char *devparams, const char *key,
                      char *buf, const size_t bufsz, char *dval);
void devparam_get_int(const char *devparams, const char *key,
                      int *val, int dval);
void devparam_get_bdf(const char *devparams, const char *key,
                      u_int16_t *bdf, u_int16_t dbdf);
void devparam_get_mac(const char *devparams, const char *key,
                      mac_t *m, mac_t *dmac);

int genericdev_init(void);
simdev_t *genericdev_get(const u_int16_t bdf);

extern dev_ops_t generic_ops;
extern dev_ops_t eth_ops;
extern dev_ops_t accel_ops;

#endif /* __SIMDEV_IMPL_H__ */
