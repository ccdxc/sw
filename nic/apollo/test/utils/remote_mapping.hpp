//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mapping test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_REMOTE_MAPPING_HPP__
#define __TEST_UTILS_REMOTE_MAPPING_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"

#define PDS_MAX_VNIC_IP 33

namespace api_test {

typedef struct remote_mapping_stepper_seed_s {
    uint32_t encap_val_stepper;
    uint64_t vnic_mac_stepper;
    std::string vnic_ip_stepper;
    std::string tep_ip_stepper;
} remote_mapping_stepper_seed_t;

class remote_mapping_util {
public:
    pds_vpc_id_t vpc_id;
    std::string vnic_ip;
    pds_subnet_id_t sub_id;
    std::string tep_ip;
    std::string vnic_mac;
    pds_encap_type_t encap_type;
    union {
        pds_mpls_tag_t mpls_tag;
        pds_vnid_id_t vxlan_id;
    };

    remote_mapping_util();

    remote_mapping_util(pds_vpc_id_t vpc_id, pds_subnet_id_t sub_id,
                        std::string vnic_ip, std::string tep_ip,
                        uint64_t vnic_mac,
                        pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
                        uint32_t encap_val = 1);

    ~remote_mapping_util();

    /// \brief Create remote IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void) const;

    /// \brief Read remote IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_remote_mapping_info_t *info) const;

    /// \brief Update remote IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(pds_remote_mapping_spec_t *spec) const;

    /// \brief Delete remote ip mapping
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create many remote IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_vnics * num_teps
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t
    many_create(uint16_t num_vnics, uint16_t num_teps, pds_vpc_id_t vpc_id,
                pds_subnet_id_t sub_id, remote_mapping_stepper_seed_t *seed,
                pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP);

    /// \brief Delete many remote IP mapping for the given VNIC
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(uint16_t num_vnics, uint16_t num_teps,
                                 pds_vpc_id_t vpc_id,
                                 remote_mapping_stepper_seed_t *seed);

    /// \brief Read many remote IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_vnics * num_teps
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t
    many_read(uint16_t num_vnics, uint16_t num_teps, pds_vpc_id_t vpc_id,
              pds_subnet_id_t sub_id, remote_mapping_stepper_seed_t *seed,
              pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
              sdk::sdk_ret_t expected_result = sdk::SDK_RET_OK);
};

}    // namespace api_test

#endif    // __TEST_UTILS_LOCAL_MAPPING_HPP__
