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

#define PORTCFG_CAP_PCIE        0x80
#define PORTCFG_CAP_AER         0x200

void portcfg_read_bus(const int port,
                      u_int8_t *pribus, u_int8_t *secbus, u_int8_t *subbus);
void portcfg_read_genwidth(const int port, int *gen, int *width);

u_int8_t  portcfg_readb(const int port, const u_int16_t addr);
u_int16_t portcfg_readw(const int port, const u_int16_t addr);
u_int32_t portcfg_readd(const int port, const u_int16_t addr);
size_t    portcfg_read (const int port, const u_int16_t addr,
                        void *buf, size_t count);

void portcfg_writeb(const int port, const u_int16_t addr, const u_int8_t val);
void portcfg_writew(const int port, const u_int16_t addr, const u_int16_t val);
void portcfg_writed(const int port, const u_int16_t addr, const u_int32_t val);

#ifdef __cplusplus
}
#endif

#endif /* __PORTCFG_H__ */
