//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the tag table test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_TAG_HPP__
#define __TEST_UTILS_TAG_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/test/utils/feeder.hpp"
#include "nic/apollo/test/utils/api_base.hpp"

namespace api_test {

#define MAX_NUM_RULES 1024

/// Tag test utility class
class tag_rule {
public:
    // Test parameter
    uint32_t tag;
    uint32_t priority;
    uint32_t num_prefixes;
    ip_prefix_t *prefixes;

    // brief Destructor
    ~tag_rule() { };
};

/// Tag tbl test utility class
class tag_feeder : public feeder {
public:
    // Test parameters
    pds_tag_key_t key;
    uint8_t af;
    uint32_t num_rules;
    uint32_t tag_base;
    uint32_t priority_base;
    uint32_t num_prefixes;
    uint32_t num_prefixes_all;
    ip_prefix_t base_route_pfx_v4;
    ip_prefix_t base_route_pfx_v6;
    std::string base_route_pfx_v4_str;
    std::string base_route_pfx_v6_str;
    // PDS_MAX_TAG + 1 for testing max + 1 tags
    tag_rule rules[MAX_NUM_RULES + 1];

    tag_feeder();
    ~tag_feeder();
    tag_feeder(const tag_feeder& feeder);

    // Initialize feeder with the base set of values
    void init(uint32_t base_tag_table_id,
              std::string base_tag_pfx_str_v4,
              std::string base_tag_pfx_str_v6,
              uint32_t tag, uint32_t priority,
              uint8_t af=IP_AF_IPV4,
              uint32_t num_tag_tables=PDS_MAX_TAG,
              uint32_t num_prefixes=PDS_MAX_PREFIX_PER_TAG);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_tag_key_t *key);
    void spec_build(pds_tag_spec_t *spec);

    // Compare routines
    bool key_compare(pds_tag_key_t *key);
    bool spec_compare(pds_tag_spec_t *spec);
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const tag_feeder& obj) {
    os << "Tag feeder =>"
       << "id: " << obj.key.id
       << "num prefixes: " << obj.num_prefixes_all;
    return os;
}

// Export variables
extern pds_tag_key_t k_tag_key;

// Tag test CRUD routines
API_CREATE(tag);
API_READ(tag);
API_UPDATE(tag);
API_DELETE(tag);

}    // namespace api_test

#endif    // __TEST_UTILS_TAG_HPP__
