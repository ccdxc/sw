/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_impl_state.hpp
 *
 * @brief   This file captures all the state maintained in
 *          impl layer
 */

#if !defined (__OCI_IMPL_STATE_HPP__)
#define __OCI_IMPL_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/hal/apollo/api/impl/switchport_impl.hpp"
#include "nic/hal/apollo/api/impl/tep_impl_state.hpp"
#include "nic/hal/apollo/api/impl/vnic_impl_state.hpp"
#include "nic/hal/apollo/api/impl/mapping_impl_state.hpp"

namespace impl {

/**
 * @defgroup OCI_IMPL_STATE - Internal state
 * @{
 */

class oci_impl_state {
public:
    oci_impl_state();
    ~oci_impl_state();
    tep_impl_state *tep_impl_db(void) { return &tep_impl_db_; }
    vnic_impl_state *vnic_impl_db(void) { return &vnic_impl_db_; }
    mapping_impl_state *mapping_impl_db(void) { return &mapping_impl_db_; }

private:
    tep_impl_state        tep_impl_db_;
    vnic_impl_state       vnic_impl_db_;
    mapping_impl_state    mapping_impl_db_;
};
extern oci_impl_state g_oci_impl_state;

static inline tep_impl_state *
tep_impl_db (void)
{
    return g_oci_impl_state.tep_impl_db();
}

static inline vnic_impl_state *
vnic_impl_db (void)
{
    return g_oci_impl_state.vnic_impl_db();
}

static inline mapping_impl_state *
mapping_impl_db (void)
{
    return g_oci_impl_state.mapping_impl_db();
}

/** * @} */    // end of OCI_IMPL_STATE

}    // namespace  impl

#endif    /** __OCI_IMPL_STATE_HPP__ */
