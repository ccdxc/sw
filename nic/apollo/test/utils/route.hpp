//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the route table test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_ROUTE_HPP__
#define __TEST_UTILS_ROUTE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

namespace api_test {

/// route test utility class
class route_util {
public:
    // Test parameters
    ip_prefix_t ip_pfx;      ///< route prefix
    ip_addr_t nh_ip;         ///< next hop IP
    pds_nh_type_t nh_type;   ///< nexthop type
    pds_vcn_id_t vcn_id;     ///< vcn id

    /// \brief default constructor
    route_util();

    /// \brief destructor
    ~route_util();
};

/// route table test utility class
class route_table_util {
public:
    // Test parameters
    pds_route_table_id_t id;
    uint8_t af;
    uint32_t num_routes;
    ip_prefix_t first_route_pfx;
    ip_addr_t first_nh_ip;
    route_util *routes;

    /// \brief default constructor
    route_table_util();

    /// \brief parameterized constructor
    route_table_util(pds_route_table_id_t id, std::string first_route_pfx_str,
                     std::string first_nh_ip_str, uint8_t af=IP_AF_IPV4,
                     uint32_t num_routes=1);

    /// \brief destructor
    ~route_table_util();

    /// \brief Create route table
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Read route table
    ///
    /// \param[in] compare_spec validation to be done or not
    /// \param[out] info route information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_route_table_info_t *info, bool compare_spec=TRUE);

    /// \brief Update route table
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void);

    /// \brief Delete route table
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void);
};

}    // namespace api_test

#endif    // __TEST_UTILS_ROUTE_HPP__
