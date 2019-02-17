//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vnic test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_VNIC_HPP__
#define __TEST_UTILS_VNIC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/include/api/oci_vnic.hpp"

namespace api_test {

/// VNIC test utility class
class vnic_util {
public:
    oci_vcn_id_t vcn_id;           ///< VCN id
    oci_subnet_id_t sub_id;        ///< Subnet id
    oci_vnic_id_t vnic_id;         ///< VNIC id
    uint16_t vlan_tag;             ///< VLAN tag
    oci_slot_id_t mpls_slot;       ///< MPLS slot
    std::string vnic_mac;          ///< VNIC mac
    oci_rsc_pool_id_t rsc_pool_id; ///< Resource pool id
    bool src_dst_check;            ///< Source destination check

    /// \brief Create VNIC
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create();

    /// \brief Create many VNIC for the given subnet and VCN
    ///
    /// This incements the vnic_id, vlan_tag, and mpls_slot.
    ///
    /// \param num_vnics Number of vnics to be created
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t many_create(uint32_t num_vnics);
};

} // namespace api_test

#endif // __TEST_UTILS_VNIC_HPP__
