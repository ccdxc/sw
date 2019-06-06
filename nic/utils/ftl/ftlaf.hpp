//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#undef __ftl_make_aftype_exp
#undef __ftl_make_aftype
#undef FTL_MAKE_AFTYPE
#undef FTL_AFPFX

#define __ftl_make_aftype_exp(_p, _a) _p##_##_a
#define __ftl_make_aftype(_p, _a) __ftl_make_aftype_exp(_p, _a)
#define FTL_MAKE_AFTYPE(_a) __ftl_make_aftype(FTL_ADDRESS_PREFIX, _a)
#define FTL_AFPFX() FTL_ADDRESS_PREFIX

#undef __ftl_afpfx_str2
#undef __ftl_afpfx_str
#undef FTL_AFPFX_STR

#define __ftl_afpfx_str2(_a) #_a
#define __ftl_afpfx_str(_a) __ftl_afpfx_str2(_a)
#define FTL_AFPFX_STR() __ftl_afpfx_str(FTL_ADDRESS_PREFIX)
