/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    state.hpp
 *
 * @brief   This file captures all the state maintained in s/w
 */

#if !defined (__OCI_STATE_HPP__)
#define __OCI_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/hal/apollo/api/switchport.hpp"
#include "nic/hal/apollo/api/tep.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/subnet.hpp"
#include "nic/hal/apollo/api/vnic.hpp"

namespace api {

/**
 * @defgroup OCI_STATE - Internal state
 * @{
 */

class oci_state {
public:
    oci_state();
    ~oci_state();
    switchport_state *switchport_db(void) { return &switchport_db_; }
    tep_state *tep_db(void) { return &tep_db_; }
    vcn_state *vcn_db(void) { return &vcn_db_; }
    subnet_state *subnet_db(void) { return &subnet_db_; }
    vnic_state *vnic_db(void) { return &vnic_db_ ; }

private:
    switchport_state    switchport_db_;
    tep_state           tep_db_;
    vcn_state           vcn_db_;
    subnet_state        subnet_db_;
    vnic_state          vnic_db_;
};
extern oci_state g_oci_state;

static inline switchport_state *
switchport_db (void)
{
    return g_oci_state.switchport_db();
}

static inline tep_state *
tep_db (void)
{
    return g_oci_state.tep_db();
}

static inline vcn_state *
vcn_db (void)
{
    return g_oci_state.vcn_db();
}

static inline subnet_state *
subnet_db (void)
{
    return g_oci_state.subnet_db();
}

static inline vnic_state *
vnic_db (void)
{
    return g_oci_state.vnic_db();
}

/** * @} */    // end of OCI_STATE

}    // namespace api

#endif    /** __STATE_HPP__ */
