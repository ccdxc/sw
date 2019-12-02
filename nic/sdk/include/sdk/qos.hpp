//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __SDK_QOS_HPP__
#define __SDK_QOS_HPP__

#include "include/sdk/base.hpp"

namespace sdk {

#define SDK_DEFAULT_POLICER_REFRESH_INTERVAL    250 // in usecs
#define SDK_MAX_POLICER_TOKENS_PER_INTERVAL     ((1ull<<39)-1)

typedef enum policer_type_e {
    POLICER_TYPE_NONE = 0,
    POLICER_TYPE_PPS  = 1,    ///< packets-per-second policer
    POLICER_TYPE_BPS  = 2,    ///< bytes-per-second policer
} policer_type_t;

typedef struct policer_s {
    policer_type_t    type;    ///< type of the policer
    uint64_t          rate;    ///< rate in bytes-per-sec or packets-per-sec
    uint64_t          burst;   ///< burst size in bytes or packets
} policer_t;

static inline sdk_ret_t
policer_to_token_rate (policer_t *policer, uint64_t refresh_interval_us,
                       uint64_t max_policer_tokens_per_interval,
                       uint64_t *token_rate, uint64_t *token_burst)
{
    uint64_t    rate_per_sec = policer->rate;
    uint64_t    burst = policer->burst;
    uint64_t    rate_tokens;

    if (rate_per_sec > UINT64_MAX/refresh_interval_us) {
        SDK_TRACE_ERR("Policer rate %u is too high", rate_per_sec);
        return SDK_RET_INVALID_ARG;
    }
    rate_tokens = (refresh_interval_us * rate_per_sec)/1000000;

    if (rate_tokens == 0) {
        SDK_TRACE_ERR("Policer rate %u too low for refresh interval %uus",
                      rate_per_sec, refresh_interval_us);
        return SDK_RET_INVALID_ARG;
    }

    if ((burst + rate_tokens) > max_policer_tokens_per_interval) {
        SDK_TRACE_ERR("Policer rate %u is too high for "
                      "refresh interval %uus",
                      rate_per_sec, refresh_interval_us);
        return SDK_RET_INVALID_ARG;
    }
    *token_rate = rate_tokens;
    *token_burst = rate_tokens + burst;
    return SDK_RET_OK;
}

static inline sdk_ret_t
policer_token_to_rate (uint64_t token_rate, uint64_t token_burst,
                       uint64_t refresh_interval_us,
                       uint64_t *rate, uint64_t *burst)
{
    *rate = (token_rate * 1000000)/refresh_interval_us;
    *burst = (token_burst - token_rate);
    return SDK_RET_OK;
}

}    // namespace sdk

#endif    // __SDK_QOS_HPP__
