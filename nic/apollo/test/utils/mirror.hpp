//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_MIRROR_HPP__
#define __TEST_UTILS_MIRROR_HPP__

#include "nic/apollo/api/include/pds_mirror.hpp"

namespace api_test {

// Mirror session object seed used as base seed in many_* operations
typedef struct mirror_session_stepper_seed_s {
    pds_mirror_session_key_t key;
    pds_vpc_id_t vpc_id;
    pds_mirror_session_type_t type;
    uint32_t tep_id;
    ip_addr_t dst_ip;
    ip_addr_t src_ip;
    uint32_t span_id;
    uint32_t dscp;
    pds_ifindex_t interface;
    pds_encap_t encap;
    uint8_t num_ms;
} mirror_session_stepper_seed_t;

/// Mirror session test utility class
class mirror_session_util {
public:
    pds_mirror_session_key_t key;       ///< Mirror session id
    pds_mirror_session_type_t type;     ///< Mirror session type
    uint16_t snap_len;                  ///< max length of packet mirrored
    pds_erspan_spec_t erspan_spec;      ///< Erspan specification
    pds_rspan_spec_t rspan_spec;        ///< Rspan specification

    /// \brief Constructor
    mirror_session_util() {}

    /// \brief Parameterized constructor
    mirror_session_util(mirror_session_stepper_seed_t *seed);

    /// \brief Destructor
    ~mirror_session_util() {}

    /// \brief Create mirror session
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void) const;

    /// \brief Read mirror session info
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_mirror_session_info_t *info) const;

    /// \brief Update mirror session configuration
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void) const;

    /// \brief Delete mirror session
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create many mirror sessions for the given type
    ///
    /// \param num_mirror_sessions Number of mirror sessions to be created
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(mirror_session_stepper_seed_t *seed);

    /// \brief Read many mirror sessions
    ///
    /// \param num_mirror_sessions Number of mirror sessions to be read
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(mirror_session_stepper_seed_t *seed,
                               sdk::sdk_ret_t exp_result = sdk::SDK_RET_OK);

    /// \brief update many mirror sessions for the given type and id
    ///
    /// \param num_mirror_sessions Number of mirror sessions to be created
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(mirror_session_stepper_seed_t *seed);

    /// \brief Delete many mirror sessions
    ///
    /// \param num_mirror_sessions Number of mirror sessions to be deleted
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(mirror_session_stepper_seed_t *seed);

    /// \brief Indicates whether mirror is stateful
    ///
    /// \returns FALSE for mirror which is stateless
    static bool is_stateful(void) { return FALSE; }
};

}    // namespace api_test

#endif    // __TEST_UTILS_MIRROR_HPP__
