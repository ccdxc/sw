/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_H__
#define __PCIEPORT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "pcieport_events.h"

struct pcieport_s;
typedef struct pcieport_s pcieport_t;

typedef enum pcieport_cmd_e {
    PCIEPORT_CMD_NONE,
    PCIEPORT_CMD_HOSTCONFIG,
    PCIEPORT_CMD_RCCONFIG,
    PCIEPORT_CMD_CRS,
} pcieport_cmd_t;

#define PCIEPORT_NPORTS         8

pcieport_t *pcieport_open(const int port);
void pcieport_close(pcieport_t *p);

struct pciehdev_params_s;
typedef struct pciehdev_params_s pciehdev_params_t;

int pcieport_hostconfig(pcieport_t *p, const pciehdev_params_t *params);
int pcieport_crs(pcieport_t *p, const int on);

int pcieport_poll(pcieport_t *p);
void pcieport_dbg(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEPORT_H__ */
