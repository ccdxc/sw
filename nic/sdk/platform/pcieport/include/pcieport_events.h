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
    PCIEPORT_EVENT_LINKUP,
    PCIEPORT_EVENT_LINKDN,
    PCIEPORT_EVENT_HOSTUP,
    PCIEPORT_EVENT_HOSTDN,
    PCIEPORT_EVENT_MACUP,
    PCIEPORT_EVENT_BUSCHG,
    PCIEPORT_EVENT_FAULT,
} pcieport_event_type_t;

typedef struct pcieport_event_linkinfo_s {
    int gen;                    /* pcie speed genX */
    int width;                  /* pcie width xX lanes */
    int reversed;               /* pcie lanes reversed */
    int genid;                  /* port event generation id */
} pcieport_event_linkinfo_t;

typedef pcieport_event_linkinfo_t pcieport_event_linkup_t;
typedef pcieport_event_linkinfo_t pcieport_event_linkdn_t;
typedef pcieport_event_linkinfo_t pcieport_event_hostup_t;
typedef pcieport_event_linkinfo_t pcieport_event_hostdn_t;
typedef pcieport_event_linkinfo_t pcieport_event_macup_t;

typedef struct pcieport_event_buschg_s {
    u_int8_t pribus;            /* new primary     bus number */
    u_int8_t secbus;            /* new secondary   bus number */
    u_int8_t subbus;            /* new subordinate bus number */
} pcieport_event_buschg_t;

typedef struct pcieport_event_fault_s {
    char reason[80];
} pcieport_event_fault_t;

typedef struct pcieport_event_s {
    pcieport_event_type_t type;
    int port;                   /* pcie port number */
    union {
        pcieport_event_linkup_t linkup;
        pcieport_event_linkdn_t linkdn;
        pcieport_event_hostup_t hostup;
        pcieport_event_hostdn_t hostdn;
        pcieport_event_macup_t  macup;
        pcieport_event_buschg_t buschg;
        pcieport_event_fault_t  fault;
    };
} pcieport_event_t;

typedef void (*pcieport_event_handler_t)(pcieport_event_t *event, void *arg);

int pcieport_register_event_handler(pcieport_event_handler_t h, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEPORT_EVENTS_H__ */
