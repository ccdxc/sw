/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __BDF_H__
#define __BDF_H__

static inline int
bdf_to_bus(const int bdf)
{
    return (bdf >> 8) & 0xff;
}

static inline int
bdf_to_dev(const int bdf)
{
    return (bdf >> 3) & 0x1f;
}

static inline int
bdf_to_fnc(const int bdf)
{
    return bdf & 0x7;
}

static inline int
bdf_make(const int b, const int d, const int f)
{
    return ((b & 0xff) << 8) | ((d & 0x1f) << 3) | (f & 0x7);
}

static inline int
bdf_ari_make(const int b, const int f)
{
    return ((b & 0xff) << 8) | (f & 0xff);
}

static inline char *
bdf_to_buf(const int bdf, char *buf, size_t bufsz)
{
    const int b = bdf_to_bus(bdf);
    const int d = bdf_to_dev(bdf);
    const int f = bdf_to_fnc(bdf);
    snprintf(buf, bufsz, "%02x:%02x.%d", b, d, f);
    return buf;
}

static inline char *
bdf_to_str(const int bdf)
{
#define NBUFS 8
    static char buf[NBUFS][16];
    static int bufi;
    return bdf_to_buf(bdf, buf[bufi++ % NBUFS], sizeof(buf[0]));
}

static inline int
bdf_from_str(const char *bdfstr)
{
    int b, d, f;
    int n = sscanf(bdfstr, "%02x:%02x.%d", &b, &d, &f);
    if (n != 3) return -1;
    return bdf_make(b, d, f);
}

#endif /* __BDF_H__ */
