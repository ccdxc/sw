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
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/sdk/platform/utils/mpartition.hpp"
#include "nic/hal/apollo/api/switchport.hpp"
#include "nic/hal/apollo/api/tep.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/subnet.hpp"
#include "nic/hal/apollo/api/vnic.hpp"
#include "nic/hal/apollo/api/mapping.hpp"
#include "nic/hal/apollo/api/route.hpp"

using std::string;

namespace api {

/**
 * @defgroup OCI_STATE - Internal state
 * @{
 */

class oci_state {
public:
    oci_state();
    ~oci_state();
    void set_cfg_path(string cfg_path) { cfg_path_ = cfg_path; }
    string cfg_path(void) const { return cfg_path_; }
    void set_catalog(catalog *catalog) { catalog_ = catalog; }
    catalog *catalogue(void) const { return catalog_; }
    void set_mpartition(mpartition *mpartition) { mpartition_ = mpartition; }
    mpartition *mempartition(void) const { return mpartition_; }
    void set_platform_type(platform_type_t type) { platform_type_ = type; }
    platform_type_t platform_type(void) const { return platform_type_; }
    void set_control_cores_mask(uint64_t control_cores_mask) {
        control_cores_mask_ = control_cores_mask;
    }
    uint64_t control_cores_mask(void) const { return control_cores_mask_; }
    void set_num_control_cores(uint16_t num_cores) {
        num_control_cores_ = num_cores;
    }
    uint16_t num_control_cores(void) const { return num_control_cores_; }
    void set_data_cores_mask(uint64_t data_cores_mask) {
        data_cores_mask_ = data_cores_mask;
    }
    uint64_t data_cores_mask(void) const { return data_cores_mask_; }
    void set_num_data_cores(uint16_t num_cores) {
        num_data_cores_ = num_cores;
    }
    uint16_t num_data_cores(void) const { return num_data_cores_; }
    switchport_state *switchport_db(void) { return &switchport_db_; }
    tep_state *tep_db(void) { return &tep_db_; }
    vcn_state *vcn_db(void) { return &vcn_db_; }
    subnet_state *subnet_db(void) { return &subnet_db_; }
    vnic_state *vnic_db(void) { return &vnic_db_ ; }
    mapping_state *mapping_db(void) { return &mapping_db_; }
    route_table_state *route_table_db(void) { return &route_table_db_; }

private:
    string               cfg_path_;
    catalog              *catalog_;
    mpartition           *mpartition_;
    platform_type_t      platform_type_;
    uint64_t             control_cores_mask_;
    uint16_t             num_control_cores_;
    uint64_t             data_cores_mask_;
    uint16_t             num_data_cores_;
    switchport_state     switchport_db_;
    tep_state            tep_db_;
    vcn_state            vcn_db_;
    subnet_state         subnet_db_;
    vnic_state           vnic_db_;
    mapping_state        mapping_db_;
    route_table_state    route_table_db_;
};
extern oci_state g_oci_state;

/** * @} */    // end of OCI_STATE

}    // namespace api

using api::oci_state;

static inline switchport_state *
switchport_db (void)
{
    return api::g_oci_state.switchport_db();
}

static inline tep_state *
tep_db (void)
{
    return api::g_oci_state.tep_db();
}

static inline vcn_state *
vcn_db (void)
{
    return api::g_oci_state.vcn_db();
}

static inline subnet_state *
subnet_db (void)
{
    return api::g_oci_state.subnet_db();
}

static inline vnic_state *
vnic_db (void)
{
    return api::g_oci_state.vnic_db();
}

static inline mapping_state *
mapping_db (void)
{
    return api::g_oci_state.mapping_db();
}

static inline route_table_state *
route_table_db (void)
{
    return api::g_oci_state.route_table_db();
}

#endif    /** __OCI_STATE_HPP__ */
