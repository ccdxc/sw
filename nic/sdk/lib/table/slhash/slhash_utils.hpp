//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __SLHASH_UTILS_HPP__
#define __SLHASH_UTILS_HPP__

#ifdef SLHASH_TRACES_ENABLED
#define SLHASH_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define SLHASH_TRACE_DEBUG(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#else
#define SLHASH_TRACE_VERBOSE(_msg, _args...)
#define SLHASH_TRACE_DEBUG(_msg, _args...)
#endif

#define SLHASH_TRACE_PRINT(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define SLHASH_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)
#define SLHASH_TRACE_ERROR_AND_GOTO(_label, _msg, _args...) {\
    SDK_TRACE_ERR(_msg, ##_args);\
    goto _label;\
}

#define SLHASH_RET_CHECK_AND_GOTO(_status, _label, _msg, _args...) {\
    if ((_status) != sdk::SDK_RET_OK) {\
        SDK_TRACE_ERR(_msg, ##_args);\
        goto _label;\
    }\
}

#endif // __SLHASH_UTILS_HPP__
