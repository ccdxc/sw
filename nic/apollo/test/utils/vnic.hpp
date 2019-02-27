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
#include "nic/apollo/include/api/pds_vnic.hpp"

namespace api_test {

/// VNIC test utility class
class vnic_util {
public:
    pds_vcn_id_t vcn_id;           ///< VCN id
    pds_subnet_id_t sub_id;        ///< Subnet id
    pds_vnic_id_t vnic_id;         ///< VNIC id
    uint16_t vlan_tag;             ///< VLAN tag
    pds_slot_id_t mpls_slot;       ///< MPLS slot
    std::string vnic_mac;          ///< VNIC mac
    pds_rsc_pool_id_t rsc_pool_id; ///< Resource pool id
    bool src_dst_check;            ///< Source destination check

    /// \brief Constructor
    vnic_util();

    /// \brief Parameterized constructor
    vnic_util(pds_vcn_id_t vcn_id, pds_subnet_id_t sub_id, pds_vnic_id_t vnic_id,
              std::string vnic_mac, bool src_dst_check = false);

    /// \brief Destructor
    ~vnic_util();

    /// \brief Create VNIC
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Read VNIC info
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_vnic_id_t vnic_id, pds_vnic_info_t *info);

    /// \brief Update VNIC configuration
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void);

    /// \brief Remove VNIC
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t remove(void);

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
