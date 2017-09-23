#include <hal_test_utils.hpp>
#include <hal_state.hpp>
#include <hal_state_pd.hpp>

using hal::g_hal_state;
using hal::pd::g_hal_state_pd;

// extern class hal_state_pd    *g_hal_state_pd;

void 
hal_test_utils_slab_disable_delete()
{
    hal::utils::slab::g_delay_delete = false;
}

slab_stats_t *
hal_test_utils_collect_slab_stats() 
{
    slab_stats_t    *stats = NULL, *t_stats = NULL;
    slab            *slab = NULL;

    stats = (slab_stats_t *)malloc(sizeof(slab_stats_t) * hal::HAL_SLAB_MAX);
    memset(stats, 0, sizeof(slab_stats_t) * hal::HAL_SLAB_MAX);

    t_stats = stats;
    for (int i = hal::HAL_SLAB_NONE; i < hal::HAL_SLAB_MAX; i++) {
        slab = hal_test_utils_get_slab((hal::hal_slab_t)i);
        if (slab) {
            hal_test_utils_populate(t_stats, slab);
        }
        t_stats++;
    }

    return stats;
}

slab *
hal_test_utils_get_slab(hal_slab_t slab_id) 
{
    slab *pi_slab = NULL;
    pi_slab = g_hal_state->get_slab(slab_id);

    if (pi_slab) {
        return pi_slab;
    }

    return g_hal_state_pd->get_slab(slab_id);
}

void
hal_test_utils_populate (slab_stats_t *stats, slab *slab)
{
    if (stats == NULL || slab == NULL) {
        return;
    }

    stats->slab_id = slab->get_slab_id();
    stats->num_in_use = slab->num_in_use();
    stats->num_allocs = slab->num_allocs();
    stats->num_frees = slab->num_frees();

    return;
}

void 
hal_test_utils_slab_stats_free(slab_stats_t *stats)
{
    free(stats);
}

void
hal_test_utils_check_slab_leak(slab_stats_t *pre, slab_stats_t *post, bool *is_leak)
{
    *is_leak = false;
    if (pre == NULL || post == NULL) {
        return;
    }
    for (int i = hal::HAL_SLAB_NONE; i < hal::HAL_SLAB_MAX; i++) {
        // if (memcmp(pre, post, sizeof(slab_stats_t))) {
        if (pre->num_in_use != post->num_in_use) {
            *is_leak = true;
            HAL_TRACE_DEBUG("-----  Leaked  ------");
            hal_test_utils_trace(pre, post);
        } else {
            // HAL_TRACE_DEBUG("-----  Passed  ------");
            // hal_test_utils_trace(pre, post);
        }
        pre++;
        post++;
    }

    return;
}

void
hal_test_utils_trace(slab_stats_t *pre, slab_stats_t *post)
{
    if (pre == NULL || post == NULL) {
        return;
    }
    HAL_ASSERT(pre->slab_id == post->slab_id);

    HAL_TRACE_DEBUG("slab_id: {}", pre->slab_id);
    HAL_TRACE_DEBUG("           Pre             Post");
    HAL_TRACE_DEBUG(" In-Use    {}              {}  ", 
            pre->num_in_use, post->num_in_use);       
    HAL_TRACE_DEBUG(" Allocs    {}              {}  ", 
            pre->num_allocs, post->num_allocs);       
    HAL_TRACE_DEBUG(" Frees     {}              {}  ", 
            pre->num_frees, post->num_frees);       

    return;
}
