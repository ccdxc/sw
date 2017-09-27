#ifndef __HAL_TELEMETRY_PD_HPP__
#define __HAL_TELEMETRY_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/telemetry.hpp"

namespace hal {
namespace pd {

hal_ret_t telemetry_pd_mirror_session_create(mirror_session_t *session);
hal_ret_t telemetry_pd_mirror_session_delete(mirror_session_t *session);
hal_ret_t telemetry_pd_mirror_session_get(uint8_t id, mirror_session_t *session);

}
}

#endif __HAL_TELEMETRY_PD_HPP__