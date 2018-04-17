/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PORTCFG_H__
#define __PORTCFG_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

u_int8_t  portcfg_readb(const int port, const u_int16_t addr);
u_int16_t portcfg_readw(const int port, const u_int16_t addr);
u_int32_t portcfg_readd(const int port, const u_int16_t addr);

void portcfg_writeb(const int port, const u_int16_t addr, const u_int8_t val);
void portcfg_writew(const int port, const u_int16_t addr, const u_int16_t val);
void portcfg_writed(const int port, const u_int16_t addr, const u_int32_t val);

#ifdef __cplusplus
}
#endif

#endif /* __PORTCFG_H__ */
