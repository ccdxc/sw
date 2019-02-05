//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_UTILS_HPP__
#define __MEM_HASH_UTILS_HPP__

#define MEMHASH_TRACES_ENABLED 1

#if MEMHASH_TRACES_ENABLED
#define MEMHASH_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_VERBOSE(_msg, ##_args)
#define MEMHASH_TRACE_DEBUG(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define MEMHASH_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)
#else
#define MEMHASH_TRACE_VERBOSE(_msg, _args...)
#define MEMHASH_TRACE_DEBUG(_msg, _args...)
#define MEMHASH_TRACE_ERR(_msg, _args...)
#endif

char*
mem_hash_utils_rawstr(void *data, uint32_t len);

#endif // __MEM_HASH_UTILS_HPP__
