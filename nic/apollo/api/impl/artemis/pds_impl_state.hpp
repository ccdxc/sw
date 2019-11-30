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
#include "nic/apollo/api/impl/artemis/artemis_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/device_impl.hpp"
#include "nic/apollo/api/impl/artemis/vpc_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/vnic_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/mapping_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/route_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/security_policy_impl_state.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/meter_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/tag_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/service_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/vpc_peer_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/nexthop_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/tep_impl_state.hpp"

namespace api {
namespace impl {

enum {
    PDS_SLAB_ID_IMPL_MIN = 8192,
    PDS_SLAB_ID_MAPPING_IMPL,
    PDS_SLAB_ID_MIRROR_IMPL,
    PDS_SLAB_ID_SVC_MAPPING_IMPL,
    PDS_SLAB_ID_VPC_PEER_IMPL,
};

/// \defgroup PDS_IMPL_STATE - Internal state
/// @{

class pds_impl_state {
public:
    sdk_ret_t init(pds_state *state);
    static void destroy(pds_impl_state *impl_state);
    pds_impl_state();
    ~pds_impl_state();
    artemis_impl_state *artemis_impl_db(void) { return artemis_impl_db_; }
    lif_impl_state *lif_impl_db(void) { return lif_impl_db_; }
    vpc_impl_state *vpc_impl_db(void) { return vpc_impl_db_; }
    vnic_impl_state *vnic_impl_db(void) { return vnic_impl_db_; }
    mapping_impl_state *mapping_impl_db(void) { return mapping_impl_db_; }
    route_table_impl_state *route_table_impl_db(void) {
        return route_table_impl_db_;
    }
    security_policy_impl_state *security_policy_impl_db(void) {
        return security_policy_impl_db_;
    }
    //mirror_impl_state *mirror_impl_db(void) {
        //return mirror_impl_db_;
    //}
    meter_impl_state *meter_impl_db(void) {
        return meter_impl_db_;
    }
    tag_impl_state *tag_impl_db(void) {
        return tag_impl_db_;
    }
    svc_mapping_impl_state *svc_mapping_impl_db(void) {
        return svc_mapping_impl_db_;
    }
    vpc_peer_impl_state *vpc_peer_impl_db(void) {
        return vpc_peer_impl_db_;
    }
    nexthop_impl_state *nexthop_impl_db(void) {
        return nexthop_impl_db_;
    }
    tep_impl_state *tep_impl_db(void) {
        return tep_impl_db_;
    }

private:
    artemis_impl_state             *artemis_impl_db_;
    lif_impl_state                *lif_impl_db_;
    vpc_impl_state                *vpc_impl_db_;
    vnic_impl_state               *vnic_impl_db_;
    mapping_impl_state            *mapping_impl_db_;
    route_table_impl_state        *route_table_impl_db_;
    security_policy_impl_state    *security_policy_impl_db_;
    //mirror_impl_state             *mirror_impl_db_;
    meter_impl_state              *meter_impl_db_;
    tag_impl_state                *tag_impl_db_;
    svc_mapping_impl_state        *svc_mapping_impl_db_;
    vpc_peer_impl_state           *vpc_peer_impl_db_;
    nexthop_impl_state            *nexthop_impl_db_;
    tep_impl_state                *tep_impl_db_;
};
extern pds_impl_state g_pds_impl_state;

static inline artemis_impl_state *
artemis_impl_db (void)
{
    return  g_pds_impl_state.artemis_impl_db();
}

static inline vpc_impl_state *
vpc_impl_db (void)
{
    return g_pds_impl_state.vpc_impl_db();
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

#if 0
static inline mirror_impl_state *
mirror_impl_db (void)
{
    return g_pds_impl_state.mirror_impl_db();
}
#endif

static inline meter_impl_state *
meter_impl_db (void)
{
    return g_pds_impl_state.meter_impl_db();
}

static inline tag_impl_state *
tag_impl_db (void)
{
    return g_pds_impl_state.tag_impl_db();
}

static inline svc_mapping_impl_state *
svc_mapping_impl_db (void)
{
    return g_pds_impl_state.svc_mapping_impl_db();
}

static inline vpc_peer_impl_state *
vpc_peer_impl_db (void)
{
    return g_pds_impl_state.vpc_peer_impl_db();
}

static inline nexthop_impl_state *
nexthop_impl_db (void)
{
    return g_pds_impl_state.nexthop_impl_db();
}

static inline tep_impl_state *
tep_impl_db (void)
{
    return g_pds_impl_state.tep_impl_db();
}

/// @}

}    // namespace  impl
}    // namespace api

#endif    // __PDS_IMPL_STATE_HPP__
