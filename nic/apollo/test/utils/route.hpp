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

#define ROUTE_TABLE_SEED_INIT route_table_util::route_table_stepper_seed_init

#define ROUTE_TABLE_CREATE(obj)                                               \
    ASSERT_TRUE(obj.create() == sdk::SDK_RET_OK)

#define ROUTE_TABLE_READ(obj, info)                                           \
    ASSERT_TRUE(obj.read(info) == sdk::SDK_RET_OK)

#define ROUTE_TABLE_UPDATE(obj)                                               \
    ASSERT_TRUE(obj.update() == sdk::SDK_RET_OK)

#define ROUTE_TABLE_DELETE(obj)                                               \
    ASSERT_TRUE(obj.del() == sdk::SDK_RET_OK)

#define ROUTE_TABLE_MANY_CREATE(seed)                                         \
    ASSERT_TRUE(route_table_util::many_create(seed) == sdk::SDK_RET_OK)

#define ROUTE_TABLE_MANY_READ(seed, exp_result)                               \
    ASSERT_TRUE(route_table_util::many_read(seed,                             \
                                      exp_result) == sdk::SDK_RET_OK)

#define ROUTE_TABLE_MANY_UPDATE(seed)                                         \
    ASSERT_TRUE(route_table_util::many_update(seed) == sdk::SDK_RET_OK)

#define ROUTE_TABLE_MANY_DELETE(seed)                                         \
    ASSERT_TRUE(route_table_util::many_delete(seed) == sdk::SDK_RET_OK)

// Route object seed used as base seed in many_* operations
typedef struct route_table_seed_s {
    uint32_t num_route_tables;
    uint32_t base_route_table_id;
    ip_prefix_t base_route_pfx;
    ip_addr_t base_nh_ip;
    uint8_t af;
    uint32_t num_routes;
} route_table_seed_t;

typedef struct route_table_stepper_seed_s {
    route_table_seed_t v4_rt_seed;
    route_table_seed_t v6_rt_seed;
} route_table_stepper_seed_t;

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
    ip_prefix_t base_route_pfx;
    ip_addr_t base_nh_ip;
    // PDS_MAX_ROUTE_PER_TABLE + 1 for testing max + 1 routes
    route_util routes[PDS_MAX_ROUTE_PER_TABLE + 1];

    /// \brief Constructor
    route_table_util();

    /// \brief Parameterized constructor
    route_table_util(pds_route_table_id_t id, ip_prefix_t base_route_pfx,
                     ip_addr_t base_nh_ip, uint8_t af = IP_AF_IPV4,
                     uint32_t num_routes=1);

    /// \brief Destructor
    ~route_table_util();

    /// \brief Create route table
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void) const;

    /// \brief Read route table
    ///
    /// \param[out] info route information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_route_table_info_t *info) const;

    /// \brief Update route table
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void) const;

    /// \brief Delete route table
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create many route tables
    ///
    /// \param[in] seed route table seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(route_table_stepper_seed_t *seed);

    /// \brief Read many route tables
    ///
    /// \param[in] seed route table seed
    /// \param[in] exp_result expected result for read operation
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(route_table_stepper_seed_t *seed,
                               sdk::sdk_ret_t exp_result = sdk::SDK_RET_OK);

    /// \brief Update many route tables
    ///
    /// \param[in] seed route table seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(route_table_stepper_seed_t *seed);

    /// \brief Delete many route tables
    ///
    /// \param[in] seed route table seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(route_table_stepper_seed_t *seed);

    /// \brief Initialize the seed for route table
    ///
    /// \param[in] num_route_tables number of route tables
    /// \param[in] base_route_table_id route table id base
    /// \param[in] base_route_pfx_str route table starting prefix
    /// \param[in] base_nh_ip_str base next hop ip for route
    /// \param[in] af route table address family
    /// \param[in] num_routes number of routes per route table
    /// \param[out] seed route table seed
    static void route_table_stepper_seed_init(uint32_t num_route_tables,
                                              uint32_t base_route_table_id,
                                              std::string base_route_pfx_str,
                                              std::string base_nh_ip_str,
                                              uint8_t af, uint32_t num_routes,
                                              route_table_stepper_seed_t *seed
                                              );

    /// \brief Indicates whether route table is stateful
    ///
    /// \returns FALSE for route table which is stateless
    static bool is_stateful(void) { return FALSE; }
};

}    // namespace api_test

#endif    // __TEST_UTILS_ROUTE_HPP__
