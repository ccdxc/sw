//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/iris/delphi/delphic.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "gen/proto/hal.delphi.hpp"

namespace hal {
namespace svc {

using namespace std;

static ::hal::HalState
hal_state (hal::hal_status_t hal_status)
{
    switch (hal_status) {
    case HAL_STATUS_NONE:
        return ::hal::HalState::HAL_STATE_NONE;
    case HAL_STATUS_ASIC_INIT_DONE:
        return ::hal::HalState::HAL_STATE_ASIC_INIT_DONE;
    case HAL_STATUS_MEM_INIT_DONE:
        return ::hal::HalState::HAL_STATE_MEM_INIT_DONE;
    case HAL_STATUS_PACKET_BUFFER_INIT_DONE:
        return ::hal::HalState::HAL_STATE_PACKET_BUFFER_INIT_DONE;
    case HAL_STATUS_DATA_PLANE_INIT_DONE:
        return ::hal::HalState::HAL_STATE_DATA_PLANE_INIT_DONE;
    case HAL_STATUS_SCHEDULER_INIT_DONE:
        return ::hal::HalState::HAL_STATE_SCHEDULER_INIT_DONE;
    case HAL_STATUS_INIT_ERR:
        return ::hal::HalState::HAL_STATE_INIT_ERR;
    case HAL_STATUS_UP:
        return ::hal::HalState::HAL_STATE_UP;
    default:
        return hal::HalState::HAL_STATE_NONE;
    }
}

// API to update HAL status
void
set_hal_status (hal::hal_status_t hal_status, delphi::SdkPtr sdk)
{
    dobj::HalStatusPtr    status;
    ::hal::HalState       state;

    state = hal_state(hal_status);
    status = std::make_shared<dobj::HalStatus>();
    status->set_state(state);
    sdk->QueueUpdate(status);
}

}    // namespace svc
}    // namespace hal
