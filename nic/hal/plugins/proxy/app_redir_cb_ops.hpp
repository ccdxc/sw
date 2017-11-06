#pragma once

#include "nic/hal/src/rawrcb.hpp"
#include "nic/hal/src/rawccb.hpp"

namespace hal {
namespace proxy {

hal_ret_t app_redir_rawrcb_create(uint32_t cb_id,
                                  const rawrcb_t *rawrcb);
hal_ret_t app_redir_rawccb_update(uint32_t cb_id,
                                  const rawccb_t *rawccb);
static inline void
app_redir_rawrcb_init(rawrcb_t *rawrcb)
{
    memset(rawrcb, 0, sizeof(*rawrcb));
}

hal_ret_t app_redir_rawccb_create(uint32_t cb_id,
                                  const rawccb_t *rawccb);
hal_ret_t app_redir_rawccb_update(uint32_t cb_id,
                                  const rawccb_t *rawccb);
static inline void
app_redir_rawccb_init(rawccb_t *rawccb)
{
    memset(rawccb, 0, sizeof(*rawccb));
}

} // namespace proxy 
} // namespace hal

