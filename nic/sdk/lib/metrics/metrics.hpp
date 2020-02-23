//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines generic metrics interface
///
//----------------------------------------------------------------------------

#ifndef __METRICS_HPP__
#define __METRICS_HPP__

#include <stdint.h>

typedef enum metrics_column_type_ {
    METRICS_COLUMN_COUNTER64 = 1, ///< a 64bit value
    METRICS_COLUMN_POINTER64 = 2, ///< a pointer to a 64bit value
    METRICS_COLUMN_RSVD      = 3, ///< placeholder for unused metrics
} metrics_column_type_t;

/// metrics_column_t defines single metric
typedef struct metrics_column_ {
    /// name of the metric
    const char *name;
    /// type of the metric
    metrics_column_type_t type;
} metrics_column_t;

/// metrics_schema_t defines the schema for metrics table
typedef struct metrics_schema_ {
    /// name of the object (e.g. eth-1/1/1 or uuid)
    const char *name; // e.g. Port
    /// list of metrics/counters
    /// NOTE: the last column should have NULL name
    metrics_column_t columns[];
} metrics_schema_t;

// Returns a handle. Null in case of failure
/// \brief register schema of the metrics
/// \param[in] schema    schema for a given metrics object
/// \return  handle to the metrics object or NULL in case of failure
static inline void *
metrics_register (metrics_schema_t *schema)
{
    return NULL;
}

#endif    // __METRICS_HPP__
