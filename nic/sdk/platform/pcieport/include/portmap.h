/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __PORTMAP_H__
#define __PORTMAP_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef struct pcieport_spec_s {
    int host;
    int port;
    int gen;
    int width;
} pcieport_spec_t;

int portmap_init(void);
int portmap_addhost(const pcieport_spec_t *ps);
int portmap_getspec(const int port, pcieport_spec_t *ps);
int portmap_hostport(const int pcieport);
int portmap_pcieport(const int hostport);
uint32_t portmap_portmask(void);
uint32_t portmap_hostmask(void);

typedef void (*portmap_cb_t)(const int port_or_host, void *arg);
void portmap_foreach_port(portmap_cb_t callbackf, void *arg);
void portmap_foreach_host(portmap_cb_t callbackf, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __PORTMAP_H__ */
