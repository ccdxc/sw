//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// captures all the state maintained in impl layer
///
//----------------------------------------------------------------------------

#ifndef __PDS_IMPL_STATE_HPP__
#define __PDS_IMPL_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl_state.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/if_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/device_impl.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/mapping_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/route_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/security_policy_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/mirror_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/service_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/policer_impl_state.hpp"

namespace api {
namespace impl {

/// \brief slab types
enum {
    PDS_SLAB_ID_IMPL_MIN     = 8192,
    PDS_SLAB_ID_VNIC_IMPL,
    PDS_SLAB_ID_MAPPING_IMPL,
    PDS_SLAB_ID_SVC_MAPPING_IMPL,
    PDS_SLAB_ID_MIRROR_IMPL,
};

/// \brief impl state/db types
enum {
    PDS_IMPL_STATE_MIN,
    PDS_IMPL_STATE_APULU = PDS_IMPL_STATE_MIN,
    PDS_IMPL_STATE_LIF,
    PDS_IMPL_STATE_IF,
    PDS_IMPL_STATE_TEP,
    PDS_IMPL_STATE_VPC,
    PDS_IMPL_STATE_VNIC,
    PDS_IMPL_STATE_SUBNET,
    PDS_IMPL_STATE_MAPPING,
    PDS_IMPL_STATE_ROUTE_TABLE,
    PDS_IMPL_STATE_SECURITY_POLICY,
    PDS_IMPL_STATE_MIRROR,
    PDS_IMPL_STATE_NEXTHOP,
    PDS_IMPL_STATE_NEXTHOP_GROUP,
    PDS_IMPL_STATE_SVC_MAPPING,
    PDS_IMPL_STATE_POLICER,
    PDS_IMPL_STATE_MAX,
};

/// \defgroup PDS_IMPL_STATE - internal state
/// \ingroup PDS_IMPL_STATE
/// @{

/// \brief state implementation at impl layer
class pds_impl_state {
public:
    pds_impl_state();
    ~pds_impl_state();
    sdk_ret_t init(pds_state *state);
    static void destroy(pds_impl_state *impl_state);
    sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt);
    apulu_impl_state *apulu_impl_db(void) const {
        return (apulu_impl_state *)impl_state_[PDS_IMPL_STATE_APULU];
    }
    lif_impl_state *lif_impl_db(void) const {
        return (lif_impl_state *)impl_state_[PDS_IMPL_STATE_LIF];
    }
    if_impl_state *if_impl_db(void) const {
        return (if_impl_state *)impl_state_[PDS_IMPL_STATE_IF];
    }
    tep_impl_state *tep_impl_db(void) const {
        return (tep_impl_state *)impl_state_[PDS_IMPL_STATE_TEP];
    }
    vpc_impl_state *vpc_impl_db(void) const {
        return (vpc_impl_state *)impl_state_[PDS_IMPL_STATE_VPC];
    }
    subnet_impl_state *subnet_impl_db(void) const {
        return (subnet_impl_state *)impl_state_[PDS_IMPL_STATE_SUBNET];
    }
    vnic_impl_state *vnic_impl_db(void) const {
        return (vnic_impl_state*)impl_state_[PDS_IMPL_STATE_VNIC];
    }
    mapping_impl_state *mapping_impl_db(void) const {
        return (mapping_impl_state *)impl_state_[PDS_IMPL_STATE_MAPPING];
    }
    route_table_impl_state *route_table_impl_db(void) const {
        return (route_table_impl_state *)impl_state_[PDS_IMPL_STATE_ROUTE_TABLE];
    }
    security_policy_impl_state *security_policy_impl_db(void) const {
        return (security_policy_impl_state  *)impl_state_[PDS_IMPL_STATE_SECURITY_POLICY];
    }
    mirror_impl_state *mirror_impl_db(void) const {
        return (mirror_impl_state *)impl_state_[PDS_IMPL_STATE_MIRROR];
    }
    nexthop_impl_state *nexthop_impl_db(void) const {
        return (nexthop_impl_state *)impl_state_[PDS_IMPL_STATE_NEXTHOP];
    }
    nexthop_group_impl_state *nexthop_group_impl_db(void) const {
        return (nexthop_group_impl_state *)impl_state_[PDS_IMPL_STATE_NEXTHOP_GROUP];
    }
    svc_mapping_impl_state *svc_mapping_impl_db(void) const {
        return (svc_mapping_impl_state *)impl_state_[PDS_IMPL_STATE_SVC_MAPPING];
    }
    policer_impl_state *policer_impl_db(void) const {
        return (policer_impl_state *)impl_state_[PDS_IMPL_STATE_POLICER];
    }

private:
    state_base *impl_state_[PDS_IMPL_STATE_MAX];

};
extern pds_impl_state g_pds_impl_state;

static inline apulu_impl_state *
apulu_impl_db (void)
{
    return  g_pds_impl_state.apulu_impl_db();
}

static inline if_impl_state *
if_impl_db (void)
{
    return g_pds_impl_state.if_impl_db();
}

static inline tep_impl_state *
tep_impl_db (void)
{
    return g_pds_impl_state.tep_impl_db();
}

static inline vpc_impl_state *
vpc_impl_db (void)
{
    return g_pds_impl_state.vpc_impl_db();
}

static inline subnet_impl_state *
subnet_impl_db (void)
{
    return g_pds_impl_state.subnet_impl_db();
}

static inline vnic_impl_state *
vnic_impl_db (void)
{
    return g_pds_impl_state.vnic_impl_db();
}

static inline mapping_impl_state *
mapping_impl_db (void)
{
    return g_pds_impl_state.mapping_impl_db();
}

static inline route_table_impl_state *
route_table_impl_db (void)
{
    return g_pds_impl_state.route_table_impl_db();
}

static inline security_policy_impl_state *
security_policy_impl_db (void)
{
    return g_pds_impl_state.security_policy_impl_db();
}

static inline mirror_impl_state *
mirror_impl_db (void)
{
    return g_pds_impl_state.mirror_impl_db();
}

static inline nexthop_impl_state *
nexthop_impl_db (void)
{
    return g_pds_impl_state.nexthop_impl_db();
}

static inline nexthop_group_impl_state *
nexthop_group_impl_db (void)
{
    return g_pds_impl_state.nexthop_group_impl_db();
}

static inline svc_mapping_impl_state *
svc_mapping_impl_db (void)
{
    return g_pds_impl_state.svc_mapping_impl_db();
}

static inline policer_impl_state *
policer_impl_db (void)
{
    return g_pds_impl_state.policer_impl_db();
}

/// \@}

}    // namespace  impl
}    // namespace api

#endif    // __PDS_IMPL_STATE_HPP__
