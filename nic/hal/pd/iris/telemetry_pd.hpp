#ifndef __HAL_TELEMETRY_PD_HPP__
#define __HAL_TELEMETRY_PD_HPP__

#include <base.h>
#include <pd.hpp>
#include <pd_api.hpp>
#include <telemetry.hpp>

namespace hal {
namespace pd {

hal_ret_t telemetry_pd_mirror_session_create(mirror_session_t *session);
hal_ret_t telemetry_pd_mirror_session_delete(mirror_session_t *session);
hal_ret_t telemetry_pd_mirror_session_get(uint8_t id, mirror_session_t *session);

}
}

#endif __HAL_TELEMETRY_PD_HPP__