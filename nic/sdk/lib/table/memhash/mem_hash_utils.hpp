//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_UTILS_HPP__
#define __MEM_HASH_UTILS_HPP__

#ifndef PERF_DBG
#define MEMHASH_P4PD_STATS_ENABLE
#define MEMHASH_TRACES_ENABLE
#else
#define MEMHASH_PERF_DBG_ENABLE
#endif

//#define MEMHASH_TRACES_ENABLE

#ifdef MEMHASH_TRACES_ENABLE
#define MEMHASH_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_VERBOSE(_msg, ##_args)
#define MEMHASH_TRACE_DEBUG(_msg, _args...) SDK_TRACE_VERBOSE(_msg, ##_args)
#else
#define MEMHASH_TRACE_VERBOSE(_msg, _args...)
#define MEMHASH_TRACE_DEBUG(_msg, _args...)
#endif
#define MEMHASH_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)

char*
mem_hash_utils_rawstr(void *data, uint32_t len);

#endif // __MEM_HASH_UTILS_HPP__
