/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEMGRD_IMPL_H__
#define __PCIEMGRD_IMPL_H__

struct pcieport_s; typedef struct pcieport_s pcieport_t;

typedef struct pciemgrenv_s {
    u_int8_t enabled_ports;
    pcieport_t *pport[PCIEPORT_NPORTS];
} pciemgrenv_t;

void verbose(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
pciemgrenv_t *pciemgrenv_get(void);
void cli_loop(void);
void server_loop(void);
void logger_init(void);

#endif /* __PCIEMGRD_IMPL_H__ */
