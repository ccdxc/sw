/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __INTR_H__
#define __INTR_H__

void pciehw_intrhw_init(pciehw_t *phw);
void pciehw_intr_init(pciehwdev_t *phwdev);
void pciehw_intr_reset(pciehwdev_t *phwdev);
void pciehw_intr_config(pciehwdev_t *phwdev,
                        const int legacy, const int fmask);

#endif /* __INTR_H__ */
