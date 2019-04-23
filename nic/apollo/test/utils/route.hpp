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

/// Route test utility class
class route_util {
public:
    // Test parameters
    ip_prefix_t ip_pfx;      ///< route prefix
    ip_addr_t nh_ip;         ///< next hop IP
    pds_nh_type_t nh_type;   ///< nexthop type
    pds_vpc_id_t vpc_id;     ///< vpc id

    /// \brief Constructor
    route_util();

    /// \brief Destructor
    ~route_util();
};

/// Route tbl test utility class
class route_table_util {
public:
    // Test parameters
    pds_route_table_id_t id;
    uint8_t af;
    uint32_t num_routes;
    ip_prefix_t first_route_pfx;
    ip_addr_t first_nh_ip;
    route_util *routes;

    /// \brief Constructor
    route_table_util();

    /// \brief Parameterized constructor
    route_table_util(pds_route_table_id_t id);

    /// \brief Parameterized constructor
    route_table_util(pds_route_table_id_t id, std::string first_route_pfx_str,
                     std::string first_nh_ip_str, uint8_t af = IP_AF_IPV4,
                     uint32_t num_routes=1);

    /// \brief Destructor
    ~route_table_util();

    /// \brief Create route table
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Create multiple route tables
    /// Create "num_route_tables" route tables of "af" address family
    /// starting with id "first_route_table_id"
    ///
    /// \param[in] num_route_tables number of route tables to be created
    /// \param[in] first_route_table_id starting route table id
    /// \param[in] first_route_pfx_str route prefix
    /// \param[in] first_nh_ip_str next hop ip address to be used for routes
    /// \param[in] af address family of route table
    /// \param[in] num_routes number of routes to be created in each route table
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(uint32_t num_route_tables,
                                 uint32_t first_route_table_id,
                                 std::string first_route_pfx_str,
                                 std::string first_nh_ip_str, uint8_t af,
                                 uint32_t num_routes);

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

    /// \brief Delete multiple route tables
    /// Delete "num_route_tables" route tables
    /// starting with id "first_route_table_id"
    ///
    /// \param[in] num_route_tables number of route tables to be deleted
    /// \param[in] first_route_table_id starting route table id
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(uint32_t num_route_tables,
                                 uint32_t first_route_table_id);
};

}    // namespace api_test

#endif    // __TEST_UTILS_ROUTE_HPP__
