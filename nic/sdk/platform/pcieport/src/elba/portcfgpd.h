/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __PORTCFGPD_H__
#define __PORTCFGPD_H__

u_int32_t portcfgpd_readdw(const int port, const u_int16_t addr);
void portcfgpd_writedw(const int port, const u_int16_t addr, u_int32_t val);

#endif /* __PORTCFGPD_H__ */
