//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains vnic object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_VNIC_UTILS_HPP__
#define __API_VNIC_UTILS_HPP__

#include <iostream>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_key_t *key) {
    os << " id: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_spec_t *spec) {
    os << &spec->key
       << " vpc id: " << spec->vpc.id
       << " subnet id: " << spec->subnet.id
       << " vnic encap: " << pds_encap2str(&spec->vnic_encap)
       << " fabric encap: " << pds_encap2str(&spec->fabric_encap)
       << " mac: " << macaddr2str(spec->mac_addr)
       << " resource pool id: " << spec->rsc_pool_id
       << " src dst check: " << spec->src_dst_check
       << " tx_mirror_session_bmap: " << +spec->tx_mirror_session_bmap
       << " rx_mirror_session_bmap: " << +spec->rx_mirror_session_bmap
       << " v4 meter id: " << spec->v4_meter.id
       << " v6 meter id: " << spec->v6_meter.id
       << " switch vnic: " << spec->switch_vnic;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_status_t *status) {
    os << " HW id: " << status->hw_id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_stats_t *stats) {
    os << " rx pkts: " << stats->rx_pkts
       << " rx bytes: " << stats->rx_bytes
       << " tx pkts: " << stats->tx_pkts
       << " tx bytes: " << stats->tx_bytes;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vnic_info_t *obj) {
    os << " VNIC info =>"
       << &obj->spec
       << &obj->status
       << &obj->stats
       << std::endl;
    return os;
}

#endif    // __API_VNIC_UTILS_HPP__
