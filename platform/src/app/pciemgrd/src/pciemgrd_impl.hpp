/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEMGRD_IMPL_H__
#define __PCIEMGRD_IMPL_H__

#include "pciehw_dev.h"

typedef struct pciemgrenv_s {
    u_int8_t interactive:1;
    u_int8_t gold:1;
    u_int8_t enabled_ports;
    pciehdev_params_t params;
} pciemgrenv_t;

pciemgrenv_t *pciemgrenv_get(void);
int cli_loop(void);
int server_loop(void);
int gold_loop(void);
void logger_init(void);
int open_hostports(void);
void close_hostports(void);

#endif /* __PCIEMGRD_IMPL_H__ */
