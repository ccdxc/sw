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
} pcieport_event_type_t;

typedef struct pcieport_event_hostup_s {
    int port;
    int gen;
    int width;
} pcieport_event_hostup_t;

typedef struct pcieport_event_hostdn_s {
    int port;
} pcieport_event_hostdn_t;

typedef struct pcieport_event_s {
    pcieport_event_type_t type;
    union {
        pcieport_event_hostup_t hostup;
        pcieport_event_hostdn_t hostdn;
    };
} pcieport_event_t;

typedef void (*pcieport_event_handler_t)(pcieport_event_t *event, void *arg);

int pcieport_register_event_handler(pcieport_event_handler_t h, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEPORT_EVENTS_H__ */
