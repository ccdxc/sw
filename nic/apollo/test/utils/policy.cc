//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <iostream>
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/policy.hpp"

using namespace std;
namespace api_test {

//----------------------------------------------------------------------------
// Policy feeder class routines
//----------------------------------------------------------------------------

void
policy_feeder::init(pds_policy_key_t key,
                    uint16_t stateful_rules,
                    rule_dir_t direction,
                    policy_type_t type,
                    uint8_t af,
                    std::string cidr_str,
                    uint32_t num_policy) {
    uint32_t max_rules = ((af == IP_AF_IPV4) ?
                            PDS_MAX_RULES_PER_IPV4_SECURITY_POLICY :
                            PDS_MAX_RULES_PER_IPV6_SECURITY_POLICY);
    this->key = key;
    this->num_rules = max_rules;
    this->stateful_rules = stateful_rules;
    this->direction = direction;
    this->type = type;
    this->af = af;
    this->cidr_str = cidr_str;
    num_obj = num_policy;
}

void
policy_feeder::iter_next(int width) {
    ip_addr_t ipaddr = {0};

    ip_prefix_ip_next(&pfx, &ipaddr);
    memcpy(&pfx.addr, &ipaddr, sizeof(ip_addr_t));
    key.id += width;
    cur_iter_pos++;
}

void
policy_feeder::key_build(pds_policy_key_t *key) const {
    memset(key, 0, sizeof(pds_policy_key_t));
    key->id = this->key.id;
}

void
create_rules(std::string cidr_str, uint16_t num_rules,
             rule_t **rules, uint16_t stateful_rules)
{
    ip_prefix_t ip_pfx;

    *rules =
        (rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                             num_rules * sizeof(rule_t));
    extract_ip_pfx((char *)cidr_str.c_str(), &ip_pfx);
    for (uint32_t i = 0; i < num_rules; i++) {
        if (ip_pfx.addr.af == IP_AF_IPV4) {
            if (stateful_rules-- < PDS_MAX_RULES_PER_IPV4_SECURITY_POLICY) {
                (*rules)[i].stateful = true;
                (*rules)[i].match.l4_match.sport_range.port_lo = 0;
                (*rules)[i].match.l4_match.sport_range.port_hi = 65535;
                (*rules)[i].match.l4_match.dport_range.port_lo = 0;
                (*rules)[i].match.l4_match.dport_range.port_hi = 65535;
            } else {
                (*rules)[i].stateful = false;
                (*rules)[i].match.l4_match.icmp_type = 1;
                (*rules)[i].match.l4_match.icmp_code = 1;
            }
            (*rules)[i].match.l3_match.ip_proto = 1;
            str2ipv4pfx((char*)cidr_str.c_str(),
                        &(*rules)[i].match.l3_match.src_ip_pfx);
            ip_pfx.addr.addr.v4_addr += 1;
            cidr_str = ippfx2str(&ip_pfx);
        } else {
            if (stateful_rules-- < PDS_MAX_RULES_PER_IPV6_SECURITY_POLICY) {
                (*rules)[i].stateful = true;
                (*rules)[i].match.l4_match.sport_range.port_lo = 0;
                (*rules)[i].match.l4_match.sport_range.port_hi = 65535;
                (*rules)[i].match.l4_match.dport_range.port_lo = 0;
                (*rules)[i].match.l4_match.dport_range.port_hi = 65535;
            } else {
                (*rules)[i].stateful = false;
                (*rules)[i].match.l4_match.icmp_type = 1;
                (*rules)[i].match.l4_match.icmp_code = 1;
            }
            (*rules)[i].match.l3_match.ip_proto = 1;
            str2ipv6pfx((char*)cidr_str.c_str(),
                        &(*rules)[i].match.l3_match.src_ip_pfx);
            for (uint8_t byte = IP6_ADDR8_LEN - 1; byte >= 0 ; byte--) {
                // keep adding one until there is no rollover
                if ((++(ip_pfx.addr.addr.v6_addr.addr8[byte]))) {
                    break;
                }
            }
            cidr_str = ippfx2str(&ip_pfx);
        }
        (*rules)[i].action_data.fw_action.action = SECURITY_RULE_ACTION_ALLOW;
    }
}

void
policy_feeder::spec_build(pds_policy_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_policy_spec_t));
    this->key_build(&spec->key);

    spec->key.id = this->key.id;
    spec->num_rules = this->num_rules;
    create_rules(this->cidr_str, this->num_rules, (rule_t**)&(this->rules),
                 this->stateful_rules);
    spec->rules = this->rules;
    spec->direction = this->direction;
    spec->policy_type = this->type;
    spec->af = this->af;
}

bool
policy_feeder::key_compare(const pds_policy_key_t *key) const {
    return (memcmp(key, &this->key, sizeof(pds_policy_key_t)) == 0);
}

bool
policy_feeder::spec_compare(const pds_policy_spec_t *spec) const {
    if (spec->direction != direction)
        return false;
    if (spec->af != af)
        return false;

    return true;
}

}    // namespace api_test
