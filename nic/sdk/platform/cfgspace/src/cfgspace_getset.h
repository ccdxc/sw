/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __CFGSPACE_GETSET_H__
#define __CFGSPACE_GETSET_H__

/*
 * These functions do the actual work of reading/writing
 * the configuration space and associated mask region.
 * These functions understand the implementation details
 * and should not be called directly by external clients.
 *
 * Note that the config space memory region (in cfg->cur[]) is the
 * actual representation of config space for devices exposed across
 * the PCIe bus to the host.  PCIe config space is little-endian.
 * These functions are implemented to be endian-agnostic to run on
 * either big- or little-endian cpus.
 */

static inline u_int8_t
_cfgspace_getb_fld(u_int8_t *fld, const u_int16_t offset)
{
    return fld[offset];
}

static inline u_int16_t
_cfgspace_getw_fld(u_int8_t *fld, const u_int16_t offset)
{
    u_int16_t val;

    val = (((u_int16_t)fld[offset + 1] << 8) |
           ((u_int16_t)fld[offset + 0] << 0));
    return val;
}

static inline u_int32_t
_cfgspace_getd_fld(u_int8_t *fld, const u_int16_t offset)
{
    u_int32_t val;

    val = (((u_int32_t)fld[offset + 3] << 24) |
           ((u_int32_t)fld[offset + 2] << 16) |
           ((u_int32_t)fld[offset + 1] <<  8) |
           ((u_int32_t)fld[offset + 0] <<  0));
    return val;
}

static inline void
_cfgspace_setb_fld(u_int8_t *fld, const u_int16_t offset, const u_int8_t val)
{
    fld[offset] = val;
}

static inline void
_cfgspace_setw_fld(u_int8_t *fld, const u_int16_t offset, const u_int16_t val)
{
    fld[offset + 0] = val;
    fld[offset + 1] = val >> 8;
}

static inline void
_cfgspace_setd_fld(u_int8_t *fld, const u_int16_t offset, const u_int32_t val)
{
    fld[offset + 0] = val;
    fld[offset + 1] = val >> 8;
    fld[offset + 2] = val >> 16;
    fld[offset + 3] = val >> 24;
}

static inline u_int8_t
cfgspace_getb_cur(cfgspace_t *cs, const u_int16_t offset)
{
    return _cfgspace_getb_fld(cs->cur, offset);
}

static inline u_int16_t
cfgspace_getw_cur(cfgspace_t *cs, const u_int16_t offset)
{
    return _cfgspace_getw_fld(cs->cur, offset);
}

static inline u_int32_t
cfgspace_getd_cur(cfgspace_t *cs, const u_int16_t offset)
{
    return _cfgspace_getd_fld(cs->cur, offset);
}

static inline void
cfgspace_setb_cur(cfgspace_t *cs, const u_int16_t offset, const u_int8_t val)
{
    _cfgspace_setb_fld(cs->cur, offset, val);
}

static inline void
cfgspace_setw_cur(cfgspace_t *cs, const u_int16_t offset, const u_int16_t val)
{
    _cfgspace_setw_fld(cs->cur, offset, val);
}

static inline void
cfgspace_setd_cur(cfgspace_t *cs, const u_int16_t offset, const u_int32_t val)
{
    _cfgspace_setd_fld(cs->cur, offset, val);
}

static inline u_int8_t
cfgspace_getb_msk(cfgspace_t *cs, const u_int16_t offset)
{
    return _cfgspace_getb_fld(cs->msk, offset);
}

static inline u_int16_t
cfgspace_getw_msk(cfgspace_t *cs, const u_int16_t offset)
{
    return _cfgspace_getw_fld(cs->msk, offset);
}

static inline u_int32_t
cfgspace_getd_msk(cfgspace_t *cs, const u_int16_t offset)
{
    return _cfgspace_getd_fld(cs->msk, offset);
}

static inline void
cfgspace_setb_msk(cfgspace_t *cs, const u_int16_t offset, const u_int8_t val)
{
    _cfgspace_setb_fld(cs->msk, offset, val);
}

static inline void
cfgspace_setw_msk(cfgspace_t *cs, const u_int16_t offset, const u_int16_t val)
{
    _cfgspace_setw_fld(cs->msk, offset, val);
}

static inline void
cfgspace_setd_msk(cfgspace_t *cs, const u_int16_t offset, const u_int32_t val)
{
    _cfgspace_setd_fld(cs->msk, offset, val);
}

#endif /* __CFGSPACE_GETSET_H__ */
