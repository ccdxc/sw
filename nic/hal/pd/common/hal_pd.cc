#include <string>
#include "nic/include/hal.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/include/hal_control.hpp"

namespace hal {

extern bool gl_super_user;

namespace pd {

std::string
hal_pd_csr_dump(char *csr_str)
{
    HAL_TRACE_DEBUG("{} csr string {}", __FUNCTION__, csr_str);
    return asic_pd_csr_dump(csr_str);
}

//------------------------------------------------------------------------------
// PD init routine to
// - start USD thread that inits the ASIC, which will then start ASIC RW thread
// TODO: for now we direcly spawn ASIC RW thread from here !!
//------------------------------------------------------------------------------
hal_ret_t
hal_pd_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t ret;
    int thread_id = HAL_THREAD_ID_CONTROL;

    HAL_ASSERT(hal_cfg != NULL);

    ret = hal_pd_mem_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD init failed, err : {}", ret);
        goto cleanup;
    }

    // start HAL control thread
    HAL_TRACE_DEBUG("Starting hal-control thread ...");
    g_hal_threads[thread_id] =
        thread::factory(std::string("hal-control").c_str(),
                thread_id, HAL_CONTROL_CORE_ID,
                hal::pd::hal_control_start,
                sched_get_priority_max(SCHED_RR),
                gl_super_user ? SCHED_RR : SCHED_OTHER,
                true);
    HAL_ABORT(g_hal_threads[thread_id] != NULL);

    // set custom data
    g_hal_threads[thread_id]->set_data(hal_cfg);

    // invoke with thread instance reference
    g_hal_threads[thread_id]->start(g_hal_threads[thread_id]);

    HAL_TRACE_DEBUG("Waiting for asic-rw thread to be ready ...");
    // wait for ASIC RW thread to be ready before initializing table entries
    while (!is_asic_rw_ready()) {
        pthread_yield();
    }

    ret = hal_pd_pgm_def_entries();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL Programming default entries, err : {}", ret);
        goto cleanup;
    }

    ret = hal_pd_pgm_def_p4plus_entries();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL Programming default p4plus entries failed, err : {}", ret);
        goto cleanup;
    }

    return HAL_RET_OK;

cleanup:

    return ret;
}

}    // namespace pd
}    // namespace hal
