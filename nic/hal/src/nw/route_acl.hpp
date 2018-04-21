// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __ROUTE_ACL_HPP__
#define __ROUTE_ACL_HPP__

#include "nic/fte/acl/acl.hpp"
#include "nic/hal/src/nw/route.hpp"

namespace hal {

using acl::acl_ctx_t;
using acl::acl_config_t;
using acl::ref_t;
using namespace acl;

#define ROUTE_ACL_FLD_DEF(typ, struct_name, fld_name)      \
    {typ, sizeof(((struct_name*)0)->fld_name),         \
            offsetof(struct_name, fld_name) }

// route tuple for route "ACL"
typedef struct route_tuple_s {
    uint32_t vrf_id;
    uint32_t ip_pfx;
} route_tuple_t;

// route tuple for route "ACL"
typedef enum route_tuple_field_e {
    ROUTE_TUPLE_VRF_ID = 0,
    ROUTE_TUPLE_IP_PREFIX,
    ROUTE_TUPLE_MAX
} route_tuple_field_t;

// route "ACL" rule
ACL_RULE_DEF(route_acl_rule_t, ROUTE_TUPLE_MAX);

// public APIs
hal_ret_t route_acl_add_route(route_t *route);
hal_ret_t route_acl_del_route(route_t *route);
hal_ret_t route_acl_lookup(route_key_t *key, hal_handle_t *handle);
}    // namespace hal

#endif    // __ROUTE_ACL_HPP__
