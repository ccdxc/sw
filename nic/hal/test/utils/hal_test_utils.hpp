#ifndef __HAL_TEST_UTILS_HPP__
#define __HAL_TEST_UTILS_HPP__
#include <trace.hpp>
#include <hal_mem.hpp>
#include <base.h>
#include <slab.hpp>

using hal::hal_slab_t;
using hal::utils::slab;

typedef struct slab_stats_s {
    hal_slab_t  slab_id;
    uint32_t    num_in_use;
    uint32_t    num_allocs;
    uint32_t    num_frees;
} slab_stats_t;

slab *hal_test_utils_get_slab(hal_slab_t slab_id);
void hal_test_utils_populate (slab_stats_t *stats, slab *slab);
void hal_test_utils_trace(slab_stats_t *pre, slab_stats_t *post);

// Public APIs
void hal_test_utils_slab_disable_delete(void);
slab_stats_t *hal_test_utils_collect_slab_stats();
void hal_test_utils_slab_stats_free(slab_stats_t *stats);
void hal_test_utils_check_slab_leak(slab_stats_t *pre, slab_stats_t *post, 
                                    bool *is_leak);
#endif // __HAL_TEST_UTILS_HPP__
