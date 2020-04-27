//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <cmath>

#include "platform/elba/elba_p4.hpp"
#include "platform/elba/elba_tm_rw.hpp"
#include "platform/elba/elba_tm_utils.hpp"
#include "platform/elba/elba_state.hpp"
#include "asic/pd/pd.hpp"

namespace sdk {
namespace platform {
namespace elba {

using sdk::asic::pd::queue_credits_get_cb_t;
using sdk::asic::pd::port_queue_credit_t;
using sdk::asic::pd::queue_credit_t;

// ----------------------------------------------------------------------------
// elba thresholds get
// ----------------------------------------------------------------------------
sdk_ret_t
elba_thresholds_get (elba_thresholds_t *thresholds)
{
    return SDK_RET_OK; // TBD-ELBA-REBASE: Missing function from Capri

}

sdk_ret_t
elba_populate_queue_stats (tm_port_t port, elba_queue_t iqs[ELBA_TM_MAX_IQS],
                           elba_queue_t oqs[ELBA_TM_MAX_OQS],
                           elba_queue_stats_t *stats)
{
    return SDK_RET_OK; // TBD-ELBA-REBASE: Missing function from Capri
}

sdk_ret_t
elba_queue_stats_get (tm_port_t port, elba_queue_stats_t *stats)
{
    return SDK_RET_OK; // TBD-ELBA-REBASE: Missing function from Capri
}

sdk_ret_t
elba_queue_credits_get (queue_credits_get_cb_t cb, void *ctxt)
{
    return SDK_RET_OK; // TBD-ELBA-REBASE: Missing function from Capri
}

} // namespace elba
} // namespace platform
} // namespace sdk
