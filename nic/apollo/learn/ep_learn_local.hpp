//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn object for local endpoints
///
//----------------------------------------------------------------------------

#ifndef __LEARN_EP_LEARN_LOCAL_HPP__
#define __LEARN_EP_LEARN_LOCAL_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"
#include "nic/apollo/learn/ep_mac_state.hpp"
#include "nic/apollo/learn/ep_ip_state.hpp"

namespace learn {

/// \brief information passed across local learn functions
///  mac_entry and ip_entry are cached so that we look them up only once
typedef struct local_learn_ctxt_s {
    ep_mac_key_t            mac_key;            ///< MAC key from incoming pkt
    ep_ip_key_t             ip_key;             ///< IP key from incoming pkt
    ep_mac_entry            *mac_entry;         ///< MAC entry ptr
    ep_ip_entry             *ip_entry;          ///< IP entry ptr
    ep_learn_type_t         mac_learn_type;     ///< L2 learn type
    ep_learn_type_t         ip_learn_type;      ///< IP mapping learn type
    uint8_t                 pkt_drop_reason;    ///< pkt drop reason
    pds_batch_ctxt_t        bctxt;              ///< api batch context
    impl::learn_info_t      impl_info;          ///< learn info provided by impl

    void reset(void) {
        if (mac_entry && mac_entry->state() == EP_STATE_LEARNING) {
            ep_mac_entry::destroy(mac_entry);
        }
        if (ip_entry && ip_entry->state() == EP_STATE_LEARNING) {
            ep_ip_entry::destroy(ip_entry);
        }
        if (bctxt != PDS_BATCH_CTXT_INVALID) {
            sdk_ret_t ret;
            ret = pds_batch_destroy(bctxt);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to destroy pds api batch, error code %u",
                              ret);
            }
        }
    }

    const char *str(void) const {
        static string str;

        str = "(" +
              std::string(mac_key.subnet.str()) + ", " +
              std::string(macaddr2str(mac_key.mac_addr)) + ", " +
              std::string(ip_key.vpc.str()) + ", " +
              std::string(ipaddr2str(&ip_key.ip_addr)) + ", " +
              "mac learn type " + to_string(mac_learn_type) + ", " +
              "ip learn type " + to_string(ip_learn_type) +
              ")";
        return str.c_str();
    }
} local_learn_ctxt_t;

/// \brief process packets received on learn lif
void process_learn_pkt(void *pkt);

}    // namespace learn

#endif    // __LEARN_EP_LEARN_LOCAL_HPP__
