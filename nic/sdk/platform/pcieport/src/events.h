/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __EVENTS_H__
#define __EVENTS_H__

union pcieport_u;
typedef union pcieport_u pcieport_t;

void pcieport_event_linkup(pcieport_t *p, const int genid);
void pcieport_event_linkdn(pcieport_t *p, const int genid);
void pcieport_event_hostup(pcieport_t *p, const int genid);
void pcieport_event_hostdn(pcieport_t *p, const int genid);
void pcieport_event_buschg(pcieport_t *p);
void pcieport_event_fault(pcieport_t *p);

#endif /* __EVENTS_H__ */
