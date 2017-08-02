#include <base.h>
#include <thread.hpp>
#include <hal.hpp>
#include <hal_mem.hpp>

namespace hal {

thread_local thread *t_curr_thread;

hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    return HAL_RET_OK;
}

namespace pd {

hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
