//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mapping test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_MAPPING_HPP__
#define __TEST_UTILS_MAPPING_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"

namespace api_test {

/// Mapping test utility class
class mapping_util {
public:
    // Key
    pds_vcn_id_t vcn_id; ///<  VCN id
    std::string vnic_ip; ///< VNIC ip address
    // Spec parameters
    pds_subnet_id_t sub_id;  ///< Subnet id
    pds_slot_id_t mpls_slot; ///< MPLS slot
    std::string tep_ip;      ///< TEP IP address
    std::string vnic_mac;    ///< VNIC mac
    // The below 3 parameters are applicable only for local mapping
    pds_vnic_id_t vnic_id; ///< VNIC id
    std::string public_ip; ///< Public ip address for the vnic ip

    /// \brief Constructor
    mapping_util();

    /// \brief Parameterized constructor
    mapping_util(pds_vcn_id_t vcn_id, std::string vnic_rem_ip,
                 std::string vnic_rem_mac = "");

    /// \brief Parameterized constructor with local vnic params
    mapping_util(pds_vcn_id_t vcn_id, std::string vnic_ip,
                 pds_vnic_id_t vnic_id, std::string vnic_mac = "");

    /// \brief Destructor
    ~mapping_util();

    /// \brief Create local/remote ip mapping
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Read local/remote ip mapping
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_mapping_info_t *info, bool compare_spec = true);

    /// \brief Update local/remote ip mapping
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void);

    /// \brief Delete local/remote ip mapping
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void);

    /// \brief Create many local/remote ip mapping for the given VNIC
    ///
    /// This increments the vnic_ip
    ///
    /// \param num_mapping Number of mappings to be created.
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t many_create(uint32_t num_mappings);

}; 
} // namespace api_test

#endif // __TEST_UTILS_MAPPING_HPP__
