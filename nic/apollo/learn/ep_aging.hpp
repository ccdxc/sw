//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// endpoint aging management apis
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/api/include/pds.hpp"

#ifndef __LEARN_EP_AGING_HPP__
#define __LEARN_EP_AGING_HPP__

namespace learn {

/// \brief number of ARP probes sent before deleting IP on endpoint
#define MAX_NUM_ARP_PROBES          3

/// \brief initialize aging timer
void aging_timer_init(sdk::event_thread::timer_t *timer, void *ctx,
                      pds_mapping_type_t map_type);

/// \brief reset aging timer to configured timeout value and restart
static inline void
aging_timer_restart (sdk::event_thread::timer_t *timer)
{
    // TODO: see if timer_again() is better
    // there is no harm calling timer_stop() without timer_start()
    sdk::event_thread::timer_stop(timer);

    // set timeout again as it could have been reset to arp probe timeout value
    sdk::event_thread::timer_set(timer, (float) learn_db()->ep_timeout(), 0.0);

    sdk::event_thread::timer_start(timer);
}

/// \brief stop aging timer
static inline void
aging_timer_stop (sdk::event_thread::timer_t *timer)
{
    sdk::event_thread::timer_stop(timer);
}

}    // namespace learn

#endif    // __LEARN_EP_AGING_HPP__

