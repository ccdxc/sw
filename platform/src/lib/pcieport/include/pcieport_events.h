/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_EVENTS_H__
#define __PCIEPORT_EVENTS_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef enum pcieport_event_type_e {
    PCIEPORT_EVENT_HOSTUP,
    PCIEPORT_EVENT_HOSTDN,
    PCIEPORT_EVENT_BUSCHG,
} pcieport_event_type_t;

typedef struct pcieport_event_hostup_s {
    int port;                   /* pcie port number */
    int gen;                    /* pcie speed genX */
    int width;                  /* pcie width xX lanes */
    int genid;                  /* port up generation id */
} pcieport_event_hostup_t;

typedef struct pcieport_event_hostdn_s {
    int port;                   /* pcie port number */
    int genid;                  /* port down generation id */
} pcieport_event_hostdn_t;

typedef struct pcieport_event_buschg_s {
    int port;                   /* pcie port number */
    u_int8_t secbus;            /* new secondary bus number */
} pcieport_event_buschg_t;

typedef struct pcieport_event_s {
    pcieport_event_type_t type;
    union {
        pcieport_event_hostup_t hostup;
        pcieport_event_hostdn_t hostdn;
        pcieport_event_buschg_t buschg;
    };
} pcieport_event_t;

typedef void (*pcieport_event_handler_t)(pcieport_event_t *event, void *arg);

int pcieport_register_event_handler(pcieport_event_handler_t h, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEPORT_EVENTS_H__ */
