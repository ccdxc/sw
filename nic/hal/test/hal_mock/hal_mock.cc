#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/thread.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/pd/pd_api.hpp"
namespace hal {

uint64_t               hal_handle = 1;
thread                 *g_hal_threads[HAL_THREAD_ID_MAX];
bool                   gl_super_user = false;
thread_local thread    *t_curr_thread;
hal_state              *g_hal_state;
bool                    g_delay_delete = false;

slab *
hal_cfg_db::get_slab(hal_slab_t slab_id)
{
    return NULL;
}

uint64_t
hal_state::preserve_state (void)
{
    return 0;
}

hal_ret_t
hal_state::restore_state (void)
{
    return HAL_RET_OK;
}

hal_ret_t
hal_parse_ini (const char *inifile, hal_cfg_t *hal_cfg)
{
    return HAL_RET_OK;
}

hal_ret_t
hal_parse_cfg (const char *cfgfile, hal_cfg_t *hal_cfg)
{
    return HAL_RET_OK;
}

hal_ret_t
hal_init (hal_cfg_t *hal_cfg)
{
    return HAL_RET_OK;
}

slab *
hal_handle_slab(void)
{
    return NULL;
}

slab *
hal_handle_ht_entry_slab(void)
{
    return NULL;
}

ht *
hal_handle_id_ht (void)
{
    return NULL;
}

void
hal_handle_cfg_db_lock (bool readlock, bool lock)
{
    return;
}

hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    return HAL_RET_OK;
}

hal_handle_t
hal_alloc_handle (void)
{
    return hal_handle++;
}

thread *
hal_get_current_thread (void)
{
    return t_curr_thread;
}

hal_ret_t
hal_destroy (void)
{
    return HAL_RET_OK;
}


namespace pd {

hal_ret_t
hal_pd_call (pd_func_id_t pd_func_id, pd_func_args_t *args)
{
    return HAL_RET_OK;
}

hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
