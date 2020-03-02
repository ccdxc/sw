// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __METRICS_HPP__
#define __METRICS_HPP__

#include <stdint.h>
#include <vector>

#include "shm.hpp"
#include "kvstore.hpp"
#include "htable.hpp"

namespace sdk {
namespace metrics {

typedef unsigned __int128 key_t;

typedef enum metrics_counter_type_ {
    METRICS_COUNTER_VALUE64   = 1, // A 64bit value
    METRICS_COUNTER_POINTER64 = 2, // A pointer to a 64bit value
    METRICS_COUNTER_RSVD64    = 3, // A reserved counter (to be ignored)
} metrics_counter_type_t;

typedef struct metrics_counter_ {
    const char *name; // Name of the metric. e.g. InboundPackets
    metrics_counter_type_t type;
} metrics_counter_t;

typedef struct metrics_schema_ {
    const char *name; // e.g. Port
    metrics_counter_t counters[]; // to finish the last counter should have NULL name
} metrics_schema_t;

typedef std::pair<std::string, uint64_t> metrics_counter_pair_t;
typedef std::vector<metrics_counter_pair_t> metrics_counters_t;

// Returns a handle. NULL in case of failure
extern void *metrics_register(metrics_schema_t *schema);

extern void metrics_set_address(void *handler, key_t key, unsigned int counter,
                                void *address);

extern void metrics_update(void *handler, key_t key, unsigned int counter,
                           uint64_t value);

// For reader
extern void *metrics_open(const char *name);
extern metrics_counters_t metrics_read(void *handler, key_t key);
extern uint64_t metrics_read(void *handler, key_t key, unsigned int counter);

} // namespace metrics
} // namespace sdk

#endif    // __METRICS_HPP__
