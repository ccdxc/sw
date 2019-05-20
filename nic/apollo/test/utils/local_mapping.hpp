//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mapping test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_LOCAL_MAPPING_HPP__
#define __TEST_UTILS_LOCAL_MAPPING_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"

#define PDS_MAX_VNIC_IP 33

namespace api_test {

typedef struct local_mapping_stepper_seed_s {
    uint16_t vnic_id_stepper;
    uint32_t encap_val_stepper;
    uint64_t vnic_mac_stepper;
    std::string vnic_ip_stepper;
    std::string public_ip_stepper;
} local_mapping_stepper_seed_t;

class local_mapping_util {
public:
    pds_vpc_id_t vpc_id;
    std::string vnic_ip;
    pds_subnet_id_t sub_id;
    std::string vnic_mac;
    pds_encap_type_t encap_type;
    union {
        pds_mpls_tag_t mpls_slot;
        pds_vnid_id_t vxlan_id;
    };
    bool is_public_ip_valid;
    pds_vnic_id_t vnic_id;
    std::string public_ip;

    local_mapping_util();

    local_mapping_util(pds_vpc_id_t vpc_id, pds_subnet_id_t sub_id,
                       std::string vnic_ip, pds_vnic_id_t vnic_id,
                       uint64_t vnic_mac,
                       pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
                       uint32_t encap_val = 1, bool is_public_ip_valid = false,
                       std::string public_ip = "");

    ~local_mapping_util();

    /// \brief Create local IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Read local IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_local_mapping_info_t *info);

    /// \brief Update local IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(pds_local_mapping_spec_t *spec);

    /// \brief Delete local/remote ip mapping
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void);

    /// \brief Create many local IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_ip_per_vnic * num_vnics
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(
        uint16_t num_ip_per_vnic, uint16_t num_vnics, pds_vpc_id_t vpc_id,
        pds_subnet_id_t sub_id, local_mapping_stepper_seed_t *seed_info,
        pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
        bool is_public_ip_valid = false, std::string public_ip_cidr_str = "");

    /// \brief Delete many local IP mapping for the given VNIC
    /// \param num_mapping Number of mappings to be deleted.
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(uint16_t num_ip_per_vnic, uint16_t num_vnics,
                                 pds_vpc_id_t vpc_id,
                                 local_mapping_stepper_seed_t *seed_info);

    /// \brief Read many local IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_ip_per_vnic * num_vnics
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t
    many_read(uint16_t num_ip_per_vnic, uint16_t num_vnics, pds_vpc_id_t vpc_id,
              pds_subnet_id_t sub_id, local_mapping_stepper_seed_t *seed_info,
              pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
              bool is_public_ip_valid = false,
              sdk::sdk_ret_t expected_result = sdk::SDK_RET_OK);
};

}    // namespace api_test

#endif    // __TEST_UTILS_LOCAL_MAPPING_HPP__
