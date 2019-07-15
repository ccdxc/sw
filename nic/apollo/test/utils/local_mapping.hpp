//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_LOCAL_MAPPING_HPP__
#define __TEST_UTILS_LOCAL_MAPPING_HPP__

#include "nic/apollo/api/include/pds_mapping.hpp"

#define PDS_MAX_VNIC_IP 33

namespace api_test {

#define LOCAL_MAPPING_CREATE(obj)                                              \
    ASSERT_TRUE(obj.create() == sdk::SDK_RET_OK)

#define LOCAL_MAPPING_DELETE(obj)                                              \
    ASSERT_TRUE(obj.del() == sdk::SDK_RET_OK)

#define LOCAL_MAPPING_MANY_CREATE(seed)                                        \
    ASSERT_TRUE(local_mapping_util::many_create(seed) == sdk::SDK_RET_OK)

#define LOCAL_MAPPING_MANY_READ(seed, expected_res)                            \
    ASSERT_TRUE(local_mapping_util::many_read(                                 \
        seed, expected_res) == sdk::SDK_RET_OK)

#define LOCAL_MAPPING_MANY_UPDATE(seed)                                        \
    ASSERT_TRUE(local_mapping_util::many_update(seed) == sdk::SDK_RET_OK)

#define LOCAL_MAPPING_MANY_DELETE(seed)                                        \
    ASSERT_TRUE(local_mapping_util::many_delete(seed) == sdk::SDK_RET_OK)

#define LOCAL_MAPPING_SEED_INIT local_mapping_util::local_mapping_stepper_seed_init

typedef struct local_mapping_stepper_seed_s {
    uint32_t vpc_id;
    uint32_t subnet_id;
    uint16_t vnic_id_stepper;
    std::string vnic_ip_stepper;
    uint64_t vnic_mac_stepper;
    pds_encap_type_t encap_type;
    uint32_t encap_val_stepper;
    bool is_public_ip_valid;
    std::string public_ip_stepper;

    // Used to create total local mappings
    // part of workflows creation
    uint32_t num_vnics;
    uint32_t num_ip_per_vnic;
} local_mapping_stepper_seed_t;

class local_mapping_util {
public:
    uint32_t vpc_id;
    uint32_t sub_id;
    uint16_t vnic_id;
    std::string vnic_ip;
    std::string vnic_mac;
    pds_encap_type_t encap_type;
    union {
        pds_mpls_tag_t mpls_slot;
        pds_vnid_id_t vxlan_id;
    };
    bool is_public_ip_valid;
    std::string public_ip;

    local_mapping_util();

    local_mapping_util(uint32_t vpc_id, uint32_t sub_id,
                       std::string vnic_ip, uint16_t vnic_id, uint64_t vnic_mac,
                       pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
                       uint32_t encap_val = 1, bool is_public_ip_valid = false,
                       std::string public_ip = "");

    ~local_mapping_util();

    /// \brief Create local IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void) const;

    /// \brief Read local IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_local_mapping_info_t *info) const;

    /// \brief Update local IP mapping
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void) const;

    /// \brief Delete local/remote ip mapping
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create many local IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_ip_per_vnic * num_vnics
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(local_mapping_stepper_seed_t *seed_info);

    /// \brief Update many local IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_ip_per_vnic * num_vnics
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(local_mapping_stepper_seed_t *seed_info);

    /// \brief Delete many local IP mapping for the given VNIC
    /// \param num_mapping Number of mappings to be deleted.
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(local_mapping_stepper_seed_t *seed_info);

    /// \brief Read many local IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_ip_per_vnic * num_vnics
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(local_mapping_stepper_seed_t *seed_info,
                               sdk::sdk_ret_t expected_result = sdk::SDK_RET_OK);

    /// \brief Initialize the seed for local mappings
    /// \param[out] seed local mapping seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t local_mapping_stepper_seed_init(local_mapping_stepper_seed_t *seed,
                                       uint32_t vpc_id, uint32_t subnet_id,
                                       uint16_t base_vnic_id, pds_encap_type_t encap_type,
                                       uint32_t base_encap_val, uint64_t base_mac_64,
                                       std::string vnic_ip_cidr, bool public_ip_valid,
                                       std::string public_ip_cidr);

    /// \brief Indicates whether mapping is stateful
    /// \returns TRUE for mapping which is stateful
    static bool is_stateful(void) { return false; }

private:
    void __init();

};

}    // namespace api_test

#endif    // __TEST_UTILS_LOCAL_MAPPING_HPP__
