//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __SLTCAM_UTILS_HPP__
#define __SLTCAM_UTILS_HPP__

//#define SLTCAM_TRACES_ENABLED 1

#ifdef SLTCAM_TRACES_ENABLED
#define SLTCAM_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define SLTCAM_TRACE_DEBUG(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#else
#define SLTCAM_TRACE_VERBOSE(_msg, _args...)
#define SLTCAM_TRACE_DEBUG(_msg, _args...)
#endif

#define SLTCAM_TRACE_PRINT(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define SLTCAM_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)
#define SLTCAM_TRACE_ERR_GOTO(_label, _msg, _args...) {\
        SDK_TRACE_ERR(_msg, ##_args);\
        goto _label;\
}

#endif // __SLTCAM_UTILS_HPP__
