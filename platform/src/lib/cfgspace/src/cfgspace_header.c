/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <assert.h>
#include <sys/types.h>

#include "cfgspace.h"

/*
 * Initialize specific config space registers.
 */

void
cfgspace_set_vendorid(cfgspace_t *cs, const u_int16_t vendorid)
{
    cfgspace_setw(cs, 0x0, vendorid);
}

void
cfgspace_set_deviceid(cfgspace_t *cs, const u_int16_t deviceid)
{
    cfgspace_setw(cs, 0x2, deviceid);
}

void
cfgspace_set_command(cfgspace_t *cs, const u_int16_t cmd, const u_int16_t msk)
{
    cfgspace_setwm(cs, 0x4, cmd, msk);
}

u_int16_t
cfgspace_get_status(cfgspace_t *cs)
{
    return cfgspace_getw(cs, 0x6);
}

void
cfgspace_set_status(cfgspace_t *cs, const u_int16_t status)
{
    cfgspace_setw(cs, 0x6, status);
}

void
cfgspace_set_revid(cfgspace_t *cs, const u_int8_t revid)
{
    cfgspace_setb(cs, 0x8, revid);
}

void
cfgspace_set_class(cfgspace_t *cs, const u_int32_t classcode)
{
    cfgspace_setb(cs, 0x9, classcode);
    cfgspace_setb(cs, 0xa, classcode >> 8);
    cfgspace_setb(cs, 0xb, classcode >> 16);
}

void
cfgspace_set_cachelinesz(cfgspace_t *cs, const u_int8_t sz)
{
    cfgspace_setbm(cs, 0xc, sz, 0xff);
}

void
cfgspace_set_headertype(cfgspace_t *cs, const u_int8_t headertype)
{
    cfgspace_setb(cs, 0xe, headertype);
}

u_int8_t
cfgspace_get_headertype(cfgspace_t *cs)
{
    return cfgspace_getb(cs, 0xe);
}

void
cfgspace_set_subvendorid(cfgspace_t *cs, const u_int16_t subvendorid)
{
    cfgspace_setw(cs, 0x2c, subvendorid);
}

void
cfgspace_set_subdeviceid(cfgspace_t *cs, const u_int16_t subdeviceid)
{
    cfgspace_setw(cs, 0x2e, subdeviceid);
}

u_int8_t
cfgspace_get_cap(cfgspace_t *cs)
{
    return cfgspace_getb(cs, 0x34);
}

void
cfgspace_set_cap(cfgspace_t *cs, const u_int8_t cap)
{
    cfgspace_setb(cs, 0x34, cap);
}

void
cfgspace_set_intline(cfgspace_t *cs, const u_int8_t intline)
{
    cfgspace_setbm(cs, 0x3c, intline, 0xff);
}

void
cfgspace_set_intpin(cfgspace_t *cs, const u_int8_t intpin)
{
    /* 0=none, 1=IntA, 2=IntB, 3=IntC, 4=IntD */
    assert(intpin <= 4);
    cfgspace_setb(cs, 0x3d, intpin);
}

u_int8_t
cfgspace_get_intpin(cfgspace_t *cs)
{
    return cfgspace_getb(cs, 0x3d);
}

void
cfgspace_set_pribus(cfgspace_t *cs, const u_int8_t pribus)
{
    cfgspace_setbm(cs, 0x18, pribus, 0xff);
}

void
cfgspace_set_secbus(cfgspace_t *cs, const u_int8_t secbus)
{
    cfgspace_setbm(cs, 0x19, secbus, 0xff);
}

void
cfgspace_set_subbus(cfgspace_t *cs, const u_int8_t subbus)
{
    cfgspace_setbm(cs, 0x1a, subbus, 0xff);
}

void
cfgspace_set_iobase(cfgspace_t *cs, const u_int8_t iobase)
{
    cfgspace_setbm(cs, 0x1c, iobase, 0xf0);
}

void
cfgspace_set_iobase_upper(cfgspace_t *cs, const u_int16_t iobaseup)
{
    cfgspace_setwm(cs, 0x30, iobaseup, 0xffff);
}

void
cfgspace_set_iolimit(cfgspace_t *cs, const u_int8_t iolimit)
{
    cfgspace_setbm(cs, 0x1d, iolimit, 0xf0);
}

void
cfgspace_set_iolimit_upper(cfgspace_t *cs, const u_int16_t iolimitup)
{
    cfgspace_setwm(cs, 0x32, iolimitup, 0xffff);
}

void
cfgspace_set_membase(cfgspace_t *cs, const u_int16_t membase)
{
    cfgspace_setwm(cs, 0x20, membase, 0xfff0);
}

void
cfgspace_set_memlimit(cfgspace_t *cs, const u_int16_t memlimit)
{
    cfgspace_setwm(cs, 0x22, memlimit, 0xfff0);
}

void
cfgspace_set_prefbase(cfgspace_t *cs, const u_int16_t prefbase)
{
    cfgspace_setwm(cs, 0x24, prefbase, 0xfff0);
}

void
cfgspace_set_preflimit(cfgspace_t *cs, const u_int16_t preflimit)
{
    cfgspace_setwm(cs, 0x26, preflimit, 0xfff0);
}

void
cfgspace_set_prefbase_upper(cfgspace_t *cs, const u_int32_t prefbaseup)
{
    cfgspace_setdm(cs, 0x28, prefbaseup, 0xffffffff);
}

void
cfgspace_set_preflimit_upper(cfgspace_t *cs, const u_int32_t preflimitup)
{
    cfgspace_setdm(cs, 0x2c, preflimitup, 0xffffffff);
}

void
cfgspace_set_bridgectrl(cfgspace_t *cs, const u_int16_t bridgectrl)
{
    const u_int32_t msk = ((1 << 0) |   /* parity */
                           (1 << 1) |   /* serr */
                           (1 << 2) |   /* isa */
                           (1 << 3) |   /* vga */
                           (1 << 4) |   /* vga decode 16 */
                           (1 << 6));   /* bus reset */
    cfgspace_setwm(cs, 0x3e, bridgectrl, msk);
}

/******************************************************************/

/*
 * Set PCIe common headers.
 */

static void
cfgspace_set_bar(cfgspace_t *cs, const cfgspace_bar_t *b)
{
    const u_int8_t cfgoff = 0x10 + (b->cfgidx << 2);
    u_int64_t v, m;

    switch (b->type) {
    case CFGSPACE_BARTYPE_MEM:
        v = 0x0 << 1;   /* 32-bit */
        m = ~(b->size - 1) & ~0xfUL;
        cfgspace_setdm(cs, cfgoff, v, m);
        break;
    case CFGSPACE_BARTYPE_MEM64:
        v = 0x2 << 1;   /* 64-bit */
        m = ~((u_int64_t)b->size - 1) & ~0xfULL;
        cfgspace_setdm(cs, cfgoff, v, m);
        cfgspace_setdm(cs, cfgoff + 4, v >> 32, m >> 32);
        break;
    case CFGSPACE_BARTYPE_IO:
        v = 0x1;        /* I/O */
        m = ~(b->size - 1) & ~0x3;
        cfgspace_setdm(cs, cfgoff, v, m);
        break;
    default:
        break;
    }
}

static void
cfgspace_set_bars(cfgspace_t *cs, const cfgspace_bar_t *bars, const int nbars)
{
    const u_int8_t headertype = cfgspace_get_headertype(cs) & 0x7f;
    const u_int8_t maxidx = headertype == 0 ? 5 : 1;
    const cfgspace_bar_t *b;
    int i;

    for (b = bars, i = 0; i < nbars; i++, b++) {
        assert(b->cfgidx <= maxidx);
        cfgspace_set_bar(cs, b);
    }
}

static void
cfgspace_sethdr_cmn_pf(cfgspace_t *cs, const cfgspace_header_params_t *p)
{
    u_int16_t m;

    cfgspace_set_vendorid(cs, p->vendorid);
    cfgspace_set_deviceid(cs, p->deviceid);

    m = ((1 <<  0) |    /* i/o space enable */
         (1 <<  1) |    /* mem space enable */
         (1 <<  2) |    /* master */
         (1 <<  6) |    /* parity */
         (1 <<  8) |    /* serr */
         (1 << 10));    /* interrupt disable */
    cfgspace_set_command(cs, 0, m);

    cfgspace_set_revid(cs, p->revid);
    cfgspace_set_class(cs, p->classcode);
    cfgspace_set_cachelinesz(cs, 0);

    cfgspace_set_intline(cs, 0);
    cfgspace_set_intpin(cs, p->intpin);

    cfgspace_set_bars(cs, p->bars, p->nbars);
}

static void
cfgspace_sethdr_cmn_vf(cfgspace_t *cs, const cfgspace_header_params_t *p)
{
    u_int16_t m;

    cfgspace_set_vendorid(cs, 0xffff);
    cfgspace_set_deviceid(cs, 0xffff);

    m = (1 << 2);       /* master */
    cfgspace_set_command(cs, 0, m);

    cfgspace_set_revid(cs, p->revid);
    cfgspace_set_class(cs, p->classcode);
}

static void
cfgspace_sethdr_cmn(cfgspace_t *cs, const cfgspace_header_params_t *p)
{
    if (!p->vf) {
        cfgspace_sethdr_cmn_pf(cs, p);
    } else {
        cfgspace_sethdr_cmn_vf(cs, p);
    }
}

void
cfgspace_sethdr_type0(cfgspace_t *cs, const cfgspace_header_params_t *p)
{
    cfgspace_set_headertype(cs, 0);
    cfgspace_sethdr_cmn(cs, p);

    cfgspace_set_subvendorid(cs, p->subvendorid);
    cfgspace_set_subdeviceid(cs, p->subdeviceid);
}

void
cfgspace_sethdr_type1(cfgspace_t *cs, const cfgspace_header_params_t *p)
{
    cfgspace_set_headertype(cs, 1);
    cfgspace_sethdr_cmn(cs, p);

    cfgspace_set_pribus(cs, 0);
    cfgspace_set_secbus(cs, 0);
    cfgspace_set_subbus(cs, 0);

    cfgspace_set_iobase(cs, 0x1);       /* supports 32-bit */
    cfgspace_set_iolimit(cs, 0x1);      /* supports 32-bit */
    cfgspace_set_iobase_upper(cs, 0);
    cfgspace_set_iolimit_upper(cs, 0);

    cfgspace_set_membase(cs, 0);
    cfgspace_set_memlimit(cs, 0);

    cfgspace_set_prefbase(cs, 0x1);     /* supports 64-bit */
    cfgspace_set_preflimit(cs, 0x1);    /* supports 64-bit */
    cfgspace_set_prefbase_upper(cs, 0);
    cfgspace_set_preflimit_upper(cs, 0);

    cfgspace_set_bridgectrl(cs, 0);
}
