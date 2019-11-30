//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
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
#include "nic/apollo/api/impl/apollo/apollo_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/device_impl.hpp"
#include "nic/apollo/api/impl/apollo/tep_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/vnic_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/mapping_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/route_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/security_policy_impl_state.hpp"
#include "nic/apollo/api/impl/lif_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/mirror_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/nexthop_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/nexthop_group_impl_state.hpp"

namespace api {
namespace impl {

/// \brief slab types
enum {
    PDS_SLAB_ID_IMPL_MIN     = 8192,
    PDS_SLAB_ID_MAPPING_IMPL,
    PDS_SLAB_ID_MIRROR_IMPL,
};

/// \defgroup PDS_IMPL_STATE - internal state
/// \ingroup PDS_IMPL_STATE
/// @{

/// \brief state implementation at impl layer
class pds_impl_state {
public:
    sdk_ret_t init(pds_state *state);
    static void destroy(pds_impl_state *impl_state);
    pds_impl_state();
    ~pds_impl_state();
    apollo_impl_state *apollo_impl_db(void) const { return apollo_impl_db_; }
    lif_impl_state *lif_impl_db(void) const { return lif_impl_db_; }
    tep_impl_state *tep_impl_db(void) const { return tep_impl_db_; }
    vnic_impl_state *vnic_impl_db(void) const { return vnic_impl_db_; }
    mapping_impl_state *mapping_impl_db(void) const { return mapping_impl_db_; }
    route_table_impl_state *route_table_impl_db(void) const {
        return route_table_impl_db_;
    }
    security_policy_impl_state *security_policy_impl_db(void) const {
        return security_policy_impl_db_;
    }
    mirror_impl_state *mirror_impl_db(void) const {
        return mirror_impl_db_;
    }
    nexthop_impl_state *nexthop_impl_db(void) const {
        return nexthop_impl_db_;
    }
    nexthop_group_impl_state *nexthop_group_impl_db(void) const {
        return nexthop_group_impl_db_;
    }

private:
    apollo_impl_state          *apollo_impl_db_;
    lif_impl_state             *lif_impl_db_;
    tep_impl_state             *tep_impl_db_;
    vnic_impl_state            *vnic_impl_db_;
    mapping_impl_state         *mapping_impl_db_;
    route_table_impl_state     *route_table_impl_db_;
    security_policy_impl_state *security_policy_impl_db_;
    mirror_impl_state          *mirror_impl_db_;
    nexthop_impl_state         *nexthop_impl_db_;
    nexthop_group_impl_state   *nexthop_group_impl_db_;
};
extern pds_impl_state g_pds_impl_state;

static inline apollo_impl_state *
apollo_impl_db (void)
{
    return  g_pds_impl_state.apollo_impl_db();
}

static inline tep_impl_state *
tep_impl_db (void)
{
    return g_pds_impl_state.tep_impl_db();
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

/// \@}

}    // namespace  impl
}    // namespace api

#endif    // __PDS_IMPL_STATE_HPP__
