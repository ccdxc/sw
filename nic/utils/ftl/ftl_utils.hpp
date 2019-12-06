//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifdef SIM
#define FTL_TRACES_ENABLE
#endif

#ifdef FTL_TRACES_ENABLE
#define FTL_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define FTL_TRACE_DEBUG(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#else
#define FTL_TRACE_VERBOSE(_msg, _args...)
#define FTL_TRACE_DEBUG(_msg, _args...)
#endif

#define FTL_TRACE_INFO(_msg, _args...) SDK_TRACE_INFO(_msg, ##_args)
#define FTL_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)
#define FTL_SNPRINTF(_buf, _len, _fmt, _args...) snprintf(_buf, _len, _fmt, ##_args)

#define FTL_RET_CHECK_AND_GOTO(_status, _label, _msg, _args...) {\
    if (unlikely((_status) != sdk::SDK_RET_OK)) {\
        SDK_TRACE_ERR(_msg, ##_args);\
        goto _label;\
    }\
}

char*
ftlu_rawstr(void *data, uint32_t len);
