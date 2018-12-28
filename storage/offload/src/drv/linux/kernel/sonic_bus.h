/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef _SONIC_BUS_H_
#define _SONIC_BUS_H_

int sonic_bus_get_irq(struct sonic *sonic, unsigned int num);
const char *sonic_bus_info(struct sonic *sonic);
int sonic_bus_alloc_irq_vectors(struct sonic *sonic, unsigned int nintrs);
void sonic_bus_free_irq_vectors(struct sonic *sonic);
int sonic_bus_register_driver(void);
void sonic_bus_unregister_driver(void);

#endif /* _SONIC_BUS_H_ */
