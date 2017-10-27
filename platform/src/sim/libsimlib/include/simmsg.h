/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __SIMMSG_H__
#define __SIMMSG_H__

typedef enum simmsgtype_e {
    SIMMSG_INIT,
    SIMMSG_CFGRD,
    SIMMSG_CFGWR,
    SIMMSG_MEMRD,
    SIMMSG_MEMWR,
    SIMMSG_IORD,
    SIMMSG_IOWR,
    SIMMSG_RDRESP,
    SIMMSG_WRRESP,
} simmsgtype_t;

#define PACKED __attribute__((packed))

typedef struct simmsg_s {
    u_int16_t magic;
    u_int16_t msgtype;
    u_int32_t _pad;
    union {
        struct {
            char name[32];
        } PACKED init;
        struct {
            u_int16_t bdf;
            u_int8_t  bar;
            u_int64_t addr;
            u_int32_t size;
        } PACKED generic;
        struct {
            u_int16_t bdf;
            u_int8_t  bar;
            u_int64_t addr;
            u_int32_t size;
        } PACKED read;
        struct {
            u_int16_t bdf;
            u_int8_t  bar;
            u_int64_t addr;
            u_int32_t size;
            u_int64_t val;
            u_int8_t  error;
        } PACKED readres;
        struct {
            u_int16_t bdf;
            u_int8_t  bar;
            u_int64_t addr;
            u_int32_t size;
            u_int64_t val;
        } PACKED write;
        struct {
            u_int16_t bdf;
            u_int8_t  bar;
            u_int64_t addr;
            u_int32_t size;
            u_int64_t val;
            u_int8_t  error;
        } PACKED writeres;
        struct {
            /* room to grow without breaking existing clients */
            u_int8_t pad[64];
        } PACKED pad;
    } u;
} simmsg_t;

typedef void (*msg_handler_t)(int fd, simmsg_t *m);

#endif /* __SIMMSG_H__ */

