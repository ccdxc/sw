// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __METRICS_HPP__
#define __METRICS_HPP__

#include <stdint.h>
#include <vector>

#include "include/sdk/types.hpp"
#include "lib/metrics/shm.hpp"
#include "lib/metrics/kvstore.hpp"
#include "lib/metrics/htable.hpp"

namespace sdk {
namespace metrics {

typedef unsigned __int128 key_t;

typedef enum metrics_type_ {
    SW  = 1, // Metrics are located in the process' memory
    HBM = 2, // Metrics are located in HW/HBM
} metrics_type_t;

typedef struct schema_ {
    const char *name; // e.g. Port
    metrics_type_t type;
    const char *counters[]; // to finish the last counter should have NULL name
} schema_t;

typedef std::pair<std::string, uint64_t> metrics_counter_pair_t;
typedef std::vector<metrics_counter_pair_t> metrics_counters_t;

// Returns a handle. NULL in case of failure
extern void *create(schema_t *schema);

extern void row_address(void *handler, key_t key, void *address);

extern void metrics_update(void *handler, key_t key, uint64_t values[]);

// For reader
extern void *metrics_open(const char *name);
extern metrics_counters_t metrics_read(void *handler, key_t key);

} // namespace metrics
} // namespace sdk

#endif    // __METRICS_HPP__
