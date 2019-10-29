/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __VPD_H__
#define __VPD_H__

union pciehwdev_s;
typedef union pciehwdev_u pciehwdev_t;

struct vpd_table_s;
typedef struct vpd_table_s vpd_table_t;

void pciehw_vpd_finalize(pciehwdev_t *phwdev, const vpd_table_t *vpdtab);
uint32_t pciehw_vpd_read(pciehwdev_t *phwdev, const uint16_t addr);
void pciehw_vpd_write(pciehwdev_t *phwdev,
                      const uint16_t addr, const uint32_t data);

#endif /* __VPD_H__ */
