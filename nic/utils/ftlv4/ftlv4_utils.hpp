//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLV4_UTILS_HPP__
#define __FTLV4_UTILS_HPP__

#ifdef SIM
#define FTLV4_TRACES_ENABLE
#endif

#ifdef FTLV4_TRACES_ENABLE
#define FTLV4_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define FTLV4_TRACE_DEBUG(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define FTLV4_SNPRINTF(_buf, _len, _fmt, _args...) snprintf(_buf, _len, _fmt, ##_args)
#else
#define FTLV4_TRACE_VERBOSE(_msg, _args...)
#define FTLV4_TRACE_DEBUG(_msg, _args...)
#define FTLV4_SNPRINTF(_buf, _len, _fmt, _args...)
#endif

#define FTLV4_TRACE_INFO(_msg, _args...) SDK_TRACE_INFO(_msg, ##_args)
#define FTLV4_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)

#define FTLV4_RET_CHECK_AND_GOTO(_status, _label, _msg, _args...) {\
    if (unlikely((_status) != sdk::SDK_RET_OK)) {\
        SDK_TRACE_ERR(_msg, ##_args);\
        goto _label;\
    }\
}

char*
ftlu_rawstr(void *data, uint32_t len);

#endif // __FTLV4_UTILS_HPP__
