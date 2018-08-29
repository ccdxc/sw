#include "nic/hal/pd/iris/event/hal_event_pd.hpp"

void table_health_monitor(uint32_t table_id,
                          char *name,
                          table_health_state_t curr_state,
                          uint32_t capacity,
                          uint32_t usage,
                          table_health_state_t *new_state)
{
    // TODO: Update new_state depending on usage and
    //       raise an event on state change.
    HAL_TRACE_DEBUG("Table Health Monior: id: {}, name: {}, capacity: {}, "
                    "usage: {}, curr_state: {}, new_state: {}",
                    table_id, name, capacity, usage, curr_state, *new_state);
}
