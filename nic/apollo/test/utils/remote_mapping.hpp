//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_REMOTE_MAPPING_HPP__
#define __TEST_UTILS_REMOTE_MAPPING_HPP__

#include "nic/apollo/api/include/pds_mapping.hpp"

#define PDS_MAX_VNIC_IP 33

namespace api_test {

#define REMOTE_MAPPING_CREATE(obj)                                              \
    ASSERT_TRUE(obj.create() == sdk::SDK_RET_OK)

#define REMOTE_MAPPING_DELETE(obj)                                              \
    ASSERT_TRUE(obj.del() == sdk::SDK_RET_OK)

#define REMOTE_MAPPING_MANY_CREATE(seed)                                        \
    ASSERT_TRUE(remote_mapping_util::many_create(seed) == sdk::SDK_RET_OK)

#define REMOTE_MAPPING_MANY_READ(seed, expected_res)                            \
    ASSERT_TRUE(remote_mapping_util::many_read(                                 \
        seed, expected_res) == sdk::SDK_RET_OK)

#define REMOTE_MAPPING_MANY_UPDATE(seed)                                        \
    ASSERT_TRUE(remote_mapping_util::many_update(seed) == sdk::SDK_RET_OK)

#define REMOTE_MAPPING_MANY_DELETE(seed)                                        \
    ASSERT_TRUE(remote_mapping_util::many_delete(seed) == sdk::SDK_RET_OK)

#define REMOTE_MAPPING_SEED_INIT remote_mapping_util::remote_mapping_stepper_seed_init

typedef struct remote_mapping_stepper_seed_s {
    uint32_t vpc_id;
    uint32_t subnet_id;
    std::string vnic_ip_stepper;
    uint64_t vnic_mac_stepper;
    std::string tep_ip_stepper;
    pds_encap_type_t encap_type;
    uint32_t encap_val_stepper;

    // Used to create total remote mappings
    // part of workflows creation
    uint32_t num_vnics;
    uint32_t num_teps;

} remote_mapping_stepper_seed_t;

class remote_mapping_util {
public:
    uint32_t vpc_id;
    uint32_t sub_id;
    std::string vnic_ip;
    std::string tep_ip;
    std::string vnic_mac;
    pds_encap_type_t encap_type;
    union {
        pds_mpls_tag_t mpls_tag;
        pds_vnid_id_t vxlan_id;
    };

    remote_mapping_util();

    remote_mapping_util(uint32_t vpc_id, uint32_t sub_id,
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
    sdk_ret_t update(void) const;

    /// \brief Delete remote ip mapping
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create many remote IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_vnics * num_teps
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(remote_mapping_stepper_seed_t *seed);

    /// \brief Update many remote IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_vnics * num_teps
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(remote_mapping_stepper_seed_t *seed);

    /// \brief Delete many remote IP mapping for the given VNIC
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(remote_mapping_stepper_seed_t *seed);

    /// \brief Read many remote IP mapping for the given <vpc, subnet>
    /// max num_mappings = num_vnics * num_teps
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t
    many_read(remote_mapping_stepper_seed_t *seed,
              sdk::sdk_ret_t expected_result = sdk::SDK_RET_OK);

    /// \brief Initialize the seed for remote mappings
    /// \param[out] seed remote mapping seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t remote_mapping_stepper_seed_init(remote_mapping_stepper_seed_t *seed,
                                       uint32_t vpc_id, uint32_t subnet_id,
                                       std::string base_vnic_ip, pds_encap_type_t encap_type,
                                       uint32_t base_encap_val, uint64_t base_mac_64,
                                       std::string tep_ip_cidr);

    /// \brief Indicates whether mapping is stateful
    /// \returns TRUE for mapping which is stateful
    static bool is_stateful(void) { return false; }

private:
    void __init();
};

}    // namespace api_test

#endif    // __TEST_UTILS_REMOTE_MAPPING_HPP__
