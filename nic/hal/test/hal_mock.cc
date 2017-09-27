#include "nic/include/base.h"
#include "nic/utils/thread/thread.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {

uint64_t  hal_handle = 1;
thread    *g_hal_threads[HAL_THREAD_ID_MAX];
bool      gl_super_user = false;
thread_local thread *t_curr_thread;
LIFManager *g_lif_manager = nullptr;
class hal_state *g_hal_state;


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

namespace pd {

hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
