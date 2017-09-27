#include <string>
#include "nic/include/hal.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/include/asic_rw.hpp"

namespace hal {
namespace pd {

//------------------------------------------------------------------------------
// PD init routine to
// - start USD thread that inits the ASIC, which will then start ASIC RW thread
// TODO: for now we direcly spawn ASIC RW thread from here !!
//------------------------------------------------------------------------------
hal_ret_t
hal_pd_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t ret;

    HAL_ASSERT(hal_cfg != NULL);

    ret = hal_pd_mem_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("HAL PD init failed, err : {}", ret);
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Starting asic-rw thread ...");
    g_hal_threads[HAL_THREAD_ID_ASIC_RW] =
        thread::factory(std::string("asic-rw").c_str(),
                        HAL_THREAD_ID_ASIC_RW, HAL_CONTROL_CORE_ID,
                        hal::pd::asic_rw_start,
                        sched_get_priority_max(SCHED_FIFO), SCHED_FIFO, true);
    HAL_ABORT(g_hal_threads[HAL_THREAD_ID_ASIC_RW] != NULL);
    g_hal_threads[HAL_THREAD_ID_ASIC_RW]->start(hal_cfg);

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
