//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// utilities to handle EP MAC and IP entries
///
//----------------------------------------------------------------------------

#ifndef __LEARN_EP_LEARN_UTILS_HPP__
#define __LEARN_EP_LEARN_UTILS_HPP__

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/learn/ep_ip_state.hpp"
#include "nic/apollo/learn/ep_mac_state.hpp"

namespace learn {

/// \brief delete IP entry from endpoint
sdk_ret_t delete_ip_from_ep(ep_ip_entry *ip_entry,
                            pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief delete endpoint
/// this deletes MAC entry and all associated IP entries of the endpoint
sdk_ret_t delete_ep(ep_mac_entry *mac_entry,
                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief send ARP probe for associated IP
void send_arp_probe(ep_ip_entry *ip_entry);

}    // namespace learn

#endif    // __LEARN_EP_LEARN_UTILS_HPP__
