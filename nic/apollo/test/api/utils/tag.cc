//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/tag.hpp"
#include "nic/apollo/test/api/utils/tep.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Tag feeder class routines
//----------------------------------------------------------------------------

tag_feeder::tag_feeder() {
    memset(rules, 0, sizeof(tag_rule) * (MAX_NUM_RULES + 1));
}

tag_feeder::~tag_feeder() {
    for (uint32_t i = 0; i < this->num_rules; i++) {
        if (this->rules[i].prefixes) {
            SDK_FREE(PDS_MEM_ALLOC_TAG, this->rules[i].prefixes);
        }
    }
}

tag_feeder::tag_feeder(const tag_feeder& feeder) {
    init(feeder.key, feeder.base_route_pfx_v4_str,
         feeder.base_route_pfx_v6_str, feeder.tag_base,
         feeder.priority_base, feeder.af,
         feeder.num_obj, feeder.num_prefixes_all);
}

void
tag_feeder::init(pds_obj_key_t base_tag_table_id,
                 std::string base_tag_pfx_str_v4,
                 std::string base_tag_pfx_str_v6,
                 uint32_t tag, uint32_t priority,
                 uint8_t af, uint32_t num_tag_tables,
                 uint32_t num_prefixes) {

    ip_addr_t route_addr;
    memset(rules, 0, sizeof(tag_rule) * (MAX_NUM_RULES + 1));
    this->key = base_tag_table_id;
    this->num_rules = MAX_NUM_RULES;
    this->af = af;
    this->tag_base = tag;
    this->priority_base = priority;
    this->num_prefixes_all = num_prefixes;
    this->num_prefixes = num_prefixes/MAX_NUM_RULES;
    this->num_obj = num_tag_tables;
    this->base_route_pfx_v4_str = base_tag_pfx_str_v4;
    this->base_route_pfx_v6_str = base_tag_pfx_str_v6;
    test::extract_ip_pfx(base_tag_pfx_str_v4.c_str(), &this->base_route_pfx_v4);
    test::extract_ip_pfx(base_tag_pfx_str_v6.c_str(), &this->base_route_pfx_v6);

    for (uint32_t i = 0; i < this->num_rules; i++) {
        this->rules[i].tag = tag_base+i;
        this->rules[i].priority = priority_base+i;
        this->rules[i].num_prefixes = this->num_prefixes;

        // add leftover prefixes to last rule
        if (i == num_rules-1) {
            this->rules[i].num_prefixes += (num_prefixes % MAX_NUM_RULES);
        }

        this->rules[i].prefixes =
            (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_TAG,
                this->rules[i].num_prefixes * sizeof(ip_prefix_t));

        if (this->af == IP_AF_IPV4) {
            for (uint32_t j =0; j < this->rules[i].num_prefixes; j++) {
                this->rules[i].prefixes[j] = this->base_route_pfx_v4;
                ip_prefix_ip_next(&this->base_route_pfx_v4, &route_addr);
                this->base_route_pfx_v4.addr = route_addr;
            }
        } else if (this->af == IP_AF_IPV6) {
            for (uint32_t j = 0; j < this->rules[i].num_prefixes; j++) {
                this->rules[i].prefixes[j] = this->base_route_pfx_v6;
                ip_prefix_ip_next(&this->base_route_pfx_v6, &route_addr);
                this->base_route_pfx_v6.addr = route_addr;
            }
        }
    }
}

void
tag_feeder::iter_next(int width) {
    ip_addr_t route_addr = {0};

    this->key = int2pdsobjkey(pdsobjkey2int(key) + width);
    cur_iter_pos++;

    //create ipv4 and ipv6 tag tables alternately
    if (pdsobjkey2int(this->key) % 2 == 1) {
        this->af = IP_AF_IPV4;
    } else {
        this->af = IP_AF_IPV6;
    }
    for (uint32_t i = 0; i < this->num_rules; i++) {
        memset(rules[i].prefixes, 0, num_prefixes*sizeof(ip_prefix_t));
        if (this->af == IP_AF_IPV4) {
            for (uint32_t j = 0; j < this->rules[i].num_prefixes; j++) {
                this->rules[i].prefixes[j] = this->base_route_pfx_v4;
                ip_prefix_ip_next(&this->base_route_pfx_v4, &route_addr);
                this->base_route_pfx_v4.addr = route_addr;
            }
        } else {
            for (uint32_t j = 0; j < this->rules[i].num_prefixes; j++) {
                this->rules[i].prefixes[j] = this->base_route_pfx_v6;
                ip_prefix_ip_next(&this->base_route_pfx_v6, &route_addr);
                this->base_route_pfx_v6.addr = route_addr;
            }
        }
    }
}

void
tag_feeder::key_build(pds_obj_key_t *key) {
    memset(key, 0, sizeof(pds_obj_key_t));
    *key = this->key;
}

void
tag_feeder::spec_build(pds_tag_spec_t *spec) {
    memset(spec, 0, sizeof(pds_tag_spec_t));
    this->key_build(&spec->key);

    spec->af = this->af;
    spec->num_rules = this->num_rules;
    spec->rules = (pds_tag_rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_TAG,
        this->num_rules * sizeof(pds_tag_rule_t));

    for (uint32_t i = 0; i < this->num_rules; i++) {
        spec->rules[i].tag = this->rules[i].tag;
        spec->rules[i].priority = this->rules[i].priority;
        spec->rules[i].num_prefixes = this->rules[i].num_prefixes;
        spec->rules[i].prefixes = (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_TAG,
                spec->rules[i].num_prefixes * sizeof(ip_prefix_t));

        for (uint32_t j = 0; j < this->rules[i].num_prefixes; j++) {
            spec->rules[i].prefixes[j] = this->rules[i].prefixes[j];
        }
    }
}

}    // namespace api
}    // namespace test
