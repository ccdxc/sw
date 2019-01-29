/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __EVENT_H__
#define __EVENT_H__

struct pciehdev_eventdata_s;
typedef struct pciehdev_eventdata_s pciehdev_eventdata_t;

void pciehw_event(pciehwdev_t *phwdev, const pciehdev_eventdata_t *evd);

#endif /* __EVENT_H__ */
