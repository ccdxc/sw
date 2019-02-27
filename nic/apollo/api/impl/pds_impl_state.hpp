/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    pds_impl_state.hpp
 *
 * @brief   This file captures all the state maintained in
 *          impl layer
 */

#if !defined (__PDS_IMPL_STATE_HPP__)
#define __PDS_IMPL_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/impl/apollo_impl_state.hpp"
#include "nic/apollo/api/impl/switchport_impl.hpp"
#include "nic/apollo/api/impl/tep_impl_state.hpp"
#include "nic/apollo/api/impl/vnic_impl_state.hpp"
#include "nic/apollo/api/impl/mapping_impl_state.hpp"
#include "nic/apollo/api/impl/route_impl_state.hpp"
#include "nic/apollo/api/impl/security_policy_impl_state.hpp"

namespace api {
namespace impl {

enum {
    PDS_MEM_ALLOC_IMPL_MIN = 16384,
    PDS_MEM_ALLOC_PDS_VNIC_IMPL = PDS_MEM_ALLOC_IMPL_MIN,
    PDS_MEM_ALLOC_PDS_TEP_IMPL,
    PDS_MEM_ALLOC_LOCAL_IP_MAPPING_TBL,
    PDS_MEM_ALLOC_REMOTE_VNIC_MAPPING_RX_TBL,
    PDS_MEM_ALLOC_REMOTE_VNIC_MAPPING_TX_TBL,
};

/**
 * @defgroup PDS_IMPL_STATE - Internal state
 * @{
 */

class pds_impl_state {
public:
    sdk_ret_t init(pds_state *state);
    pds_impl_state();
    ~pds_impl_state();
    apollo_impl_state *apollo_impl_db(void) { return apollo_impl_db_; }
    tep_impl_state *tep_impl_db(void) { return tep_impl_db_; }
    vnic_impl_state *vnic_impl_db(void) { return vnic_impl_db_; }
    mapping_impl_state *mapping_impl_db(void) { return mapping_impl_db_; }
    route_table_impl_state *route_table_impl_db(void) {
        return route_table_impl_db_;
    }
    security_policy_impl_state *security_policy_impl_db(void) {
        return security_policy_impl_db_;
    }

private:
    apollo_impl_state             *apollo_impl_db_;
    tep_impl_state                *tep_impl_db_;
    vnic_impl_state               *vnic_impl_db_;
    mapping_impl_state            *mapping_impl_db_;
    route_table_impl_state        *route_table_impl_db_;
    security_policy_impl_state    *security_policy_impl_db_;
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
security_policy_impl_db(void)
{
    return g_pds_impl_state.security_policy_impl_db();
}

/** * @} */    // end of PDS_IMPL_STATE

}    // namespace  impl
}    // namespace api

#endif    /** __PDS_IMPL_STATE_HPP__ */
