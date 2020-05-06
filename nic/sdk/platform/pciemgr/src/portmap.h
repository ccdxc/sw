/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PORTMAP_H__
#define __PORTMAP_H__

void pciehw_portmap_init(void);
int pciehw_portmap_load(const u_int32_t lifb,
                        const u_int32_t lifc,
                        const u_int8_t port);
int pciehw_portmap_unload(const u_int32_t lifb,
                          const u_int32_t lifc);
void pciehw_portmap_dbg(int argc, char *argv[]);

#endif /* __PORTMAP_H__ */
