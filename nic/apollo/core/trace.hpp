/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    trace.hpp
 *
 * @brief   trace macros for OCI APIs
 */
#if !defined (__TRACE_HPP__)
#define __TRACE_HPP__

#include "nic/sdk/lib/logger/logger.hpp"

extern sdk_logger::trace_cb_t    g_trace_cb;
static void inline
register_trace_cb(sdk_logger::trace_cb_t  trace_cb)
{
    g_trace_cb = trace_cb;
}

#define OCI_TRACE_ERR(fmt, ...)          g_trace_cb(sdk::lib::SDK_TRACE_LEVEL_ERR, "[%s:%d] "\
                                                    fmt, __func__, __LINE__, ##__VA_ARGS__)
#define OCI_TRACE_WARN(fmt, ...)         g_trace_cb(sdk::lib::SDK_TRACE_LEVEL_WARN, "[%s:%d] "\
                                                    fmt, __func__, __LINE__, ##__VA_ARGS__)
#define OCI_TRACE_INFO(fmt, ...)         g_trace_cb(sdk::lib::SDK_TRACE_LEVEL_INFO, "[%s:%d] "\
                                                    fmt, __func__, __LINE__, ##__VA_ARGS__)
#define OCI_TRACE_DEBUG(fmt, ...)        g_trace_cb(sdk::lib::SDK_TRACE_LEVEL_DEBUG, "[%s:%d] "\
                                                    fmt, __func__, __LINE__, ##__VA_ARGS__)
#define OCI_TRACE_VERBOSE(fmt, ...)      g_trace_cb(sdk::lib::SDK_TRACE_LEVEL_VERBOSE, "[%s:%d] "\
                                                    fmt, __func__, __LINE__, ##__VA_ARGS__)

#endif    /** __TRACE_HPP__ */
