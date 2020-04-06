//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file captures PDS memory related helpers
///
//----------------------------------------------------------------------------

#ifndef __CORE_TRACE_HPP__
#define __CORE_TRACE_HPP__

#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/api/include/pds_init.hpp"

extern sdk_logger::trace_cb_t g_trace_cb;

static void inline
register_trace_cb (sdk_logger::trace_cb_t trace_cb)
{
    g_trace_cb = trace_cb;
}

#define PDS_HAL_TRACE(...)                                                \
{                                                                         \
    g_trace_cb(pds_mod_id_t::PDS_MOD_ID_PDS, ##__VA_ARGS__);              \
}

#define PDS_HMON_TRACE(...)                                               \
{                                                                         \
    g_trace_cb(sdk_mod_id_t::SDK_MOD_ID_HMON, ##__VA_ARGS__);             \
}

#define PDS_INTR_TRACE(...)                                               \
{                                                                         \
    g_trace_cb(sdk_mod_id_t::SDK_MOD_ID_INTR, ##__VA_ARGS__);             \
}

#define PDS_TRACE_ERR(fmt, ...)                                           \
{                                                                         \
    PDS_HAL_TRACE(sdk::lib::SDK_TRACE_LEVEL_ERR, "[%s:%u] "               \
                  fmt, __FNAME__, __LINE__, ##__VA_ARGS__);               \
}

#define PDS_TRACE_WARN(fmt, ...)                                          \
{                                                                         \
    PDS_HAL_TRACE(sdk::lib::SDK_TRACE_LEVEL_WARN, "[%s:%u] "              \
                  fmt, __FNAME__, __LINE__, ##__VA_ARGS__);               \
}

#define PDS_TRACE_INFO(fmt, ...)                                          \
{                                                                         \
    PDS_HAL_TRACE(sdk::lib::SDK_TRACE_LEVEL_INFO, "[%s:%u] "              \
                  fmt, __FNAME__, __LINE__,  ##__VA_ARGS__);              \
}

#define PDS_TRACE_DEBUG(fmt, ...)                                         \
{                                                                         \
    PDS_HAL_TRACE(sdk::lib::SDK_TRACE_LEVEL_DEBUG, "[%s:%u] "             \
                  fmt, __FNAME__, __LINE__, ##__VA_ARGS__);               \
}

#define PDS_TRACE_DEBUG_NO_HEADER(fmt, ...)                               \
{                                                                         \
    PDS_HAL_TRACE(sdk::lib::SDK_TRACE_LEVEL_DEBUG, fmt, ##__VA_ARGS__);   \
}

#define PDS_TRACE_VERBOSE(fmt, ...)                                       \
{                                                                         \
    PDS_HAL_TRACE(sdk::lib::SDK_TRACE_LEVEL_VERBOSE, "[%s:%u] "           \
                  fmt, __FNAME__, __LINE__, ##__VA_ARGS__);               \
}

#define PDS_HMON_TRACE_ERR(fmt, ...)                                      \
{                                                                         \
    PDS_HMON_TRACE(sdk::lib::SDK_TRACE_LEVEL_ERR, "[%s:%u] "              \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_HMON_TRACE_WARN(fmt, ...)                                     \
{                                                                         \
    PDS_HMON_TRACE(sdk::lib::SDK_TRACE_LEVEL_WARN, "[%s:%u] "             \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_HMON_TRACE_INFO(fmt, ...)                                     \
{                                                                         \
    PDS_HMON_TRACE(sdk::lib::SDK_TRACE_LEVEL_INFO, "[%s:%u] "             \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_HMON_TRACE_DEBUG(fmt, ...)                                    \
{                                                                         \
    PDS_HMON_TRACE(sdk::lib::SDK_TRACE_LEVEL_DEBUG, "[%s:%u] "            \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_HMON_TRACE_VERBOSE(fmt, ...)                                  \
{                                                                         \
    PDS_HMON_TRACE(sdk::lib::SDK_TRACE_LEVEL_VERBOSE, "[%s:%u] "          \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_INTR_TRACE_ERR(fmt, ...)                                      \
{                                                                         \
    PDS_INTR_TRACE(sdk::lib::SDK_TRACE_LEVEL_ERR, "[%s:%u] "              \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_INTR_TRACE_WARN(fmt, ...)                                     \
{                                                                         \
    PDS_INTR_TRACE(sdk::lib::SDK_TRACE_LEVEL_WARN, "[%s:%u] "             \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_INTR_TRACE_INFO(fmt, ...)                                     \
{                                                                         \
    PDS_INTR_TRACE(sdk::lib::SDK_TRACE_LEVEL_INFO, "[%s:%u] "             \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_INTR_TRACE_DEBUG(fmt, ...)                                    \
{                                                                         \
    PDS_INTR_TRACE(sdk::lib::SDK_TRACE_LEVEL_DEBUG, "[%s:%u] "            \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#define PDS_INTR_TRACE_VERBOSE(fmt, ...)                                  \
{                                                                         \
    PDS_INTR_TRACE(sdk::lib::SDK_TRACE_LEVEL_VERBOSE, "[%s:%u] "          \
                   fmt, __FNAME__, __LINE__, ##__VA_ARGS__);              \
}

#endif    // __CORE_TRACE_HPP__
