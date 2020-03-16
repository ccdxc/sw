/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    pds_state.hpp
 *
 * @brief   This file captures all the state maintained in s/w
 */

#ifndef __PDS_STATE_HPP__
#define __PDS_STATE_HPP__

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/sdk/platform/utils/mpartition.hpp"
#include "nic/sdk/platform/utils/program.hpp"
#include "nic/sdk/lib/kvstore/kvstore.hpp"
#include "nic/apollo/api/device_state.hpp"
#include "nic/apollo/api/tep_state.hpp"
#include "nic/apollo/api/vpc_state.hpp"
#include "nic/apollo/api/subnet_state.hpp"
#include "nic/apollo/api/vnic_state.hpp"
#include "nic/apollo/api/mapping_state.hpp"
#include "nic/apollo/api/route_state.hpp"
#include "nic/apollo/api/policy_state.hpp"
#include "nic/apollo/api/if_state.hpp"
#include "nic/apollo/api/mirror_state.hpp"
#include "nic/apollo/api/meter_state.hpp"
#include "nic/apollo/api/tag_state.hpp"
#include "nic/apollo/api/lif_state.hpp"
#include "nic/apollo/api/service_state.hpp"
#include "nic/apollo/api/vpc_peer_state.hpp"
#include "nic/apollo/api/nexthop_state.hpp"
#include "nic/apollo/api/nexthop_group_state.hpp"
#include "nic/apollo/api/policer_state.hpp"
#include "nic/apollo/api/nat_state.hpp"
#include "nic/apollo/api/dhcp_state.hpp"
#include "nic/apollo/api/security_profile.hpp"
#include "nic/apollo/api/include/pds_event.hpp"
#include "nic/apollo/learn/learn_state.hpp"

using std::string;
using boost::property_tree::ptree;

namespace api {

/// \brief s/w state/db types
#define PDS_STATE(ENTRY)                                            \
    ENTRY(PDS_STATE_MIN,            0,             "min")           \
    ENTRY(PDS_STATE_DEVICE,         1,             "device")        \
    ENTRY(PDS_STATE_LIF,            2,             "lif")           \
    ENTRY(PDS_STATE_IF,             3,             "if")            \
    ENTRY(PDS_STATE_TEP,            4,             "tep")           \
    ENTRY(PDS_STATE_VPC,            5,             "vpc")           \
    ENTRY(PDS_STATE_SUBNET,         6,             "subnet")        \
    ENTRY(PDS_STATE_VNIC,           7,             "vnic")          \
    ENTRY(PDS_STATE_MAPPING,        8,             "mapping")       \
    ENTRY(PDS_STATE_ROUTE_TABLE,    9,             "route-table")   \
    ENTRY(PDS_STATE_POLICY,         10,            "policy")        \
    ENTRY(PDS_STATE_MIRROR,         11,            "mirror")        \
    ENTRY(PDS_STATE_METER,          12,            "meter")         \
    ENTRY(PDS_STATE_TAG,            13,            "tag")           \
    ENTRY(PDS_STATE_SVC_MAPPING,    14,            "svc-mapping")   \
    ENTRY(PDS_STATE_VPC_PEER,       15,            "vpc-peer")      \
    ENTRY(PDS_STATE_NEXTHOP,        16,            "nexthop")       \
    ENTRY(PDS_STATE_NEXTHOP_GROUP,  17,            "nexthop-group") \
    ENTRY(PDS_STATE_POLICER,        18,            "policer")       \
    ENTRY(PDS_STATE_NAT,            19,            "nat")           \
    ENTRY(PDS_STATE_DHCP,           20,            "dhcp")          \
    ENTRY(PDS_STATE_LEARN,          21,            "learn")         \
    ENTRY(PDS_STATE_MAX,            22,            "max")

SDK_DEFINE_ENUM(pds_state_t, PDS_STATE)
SDK_DEFINE_ENUM_TO_STR(pds_state_t, PDS_STATE)
#undef PDS_STATE

/// \brief ctxt passed as argument to state_walk cb
typedef struct state_walk_ctxt_s {
    std::string obj_state;
    state_base  *state;
} state_walk_ctxt_t;

/**
 * @defgroup PDS_STATE - Internal state
 * @{
 */

//--------------------------------------------------------------------------
// TODO:
// to speed up framework processing and avoid recursive walks during update
// processing we can maintain map of lists for the following:
// 1. policy key -> list of subnets
// 2. policy key -> list of vnics
// 3. route-table key -> list of vpcs
// 4. route-table key -> list of subnets
// 5. vpc key -> list of subnets
// 6. subnet key -> list of vnics
// with these auxiliary databases, we can avoid walks in add_deps()
//
// NOTE: while adding objs, add_to_db() needs to add to all relevant datbases
//       while deleteing objs, del_from_db() needs to delete from all relevant
//       databases
//       while updating db (with cloned obj), we need to both of the above
//--------------------------------------------------------------------------
class pds_state : public state_base {
public:
    pds_state();
    ~pds_state();
    sdk_ret_t init(string pipeline, string cfg_file);
    static void destroy(pds_state *ps);
    string cfg_path(void) const { return cfg_path_; }
    string pipeline(void) const { return pipeline_; }
    void set_catalog(catalog *catalog) { catalog_ = catalog; }
    catalog *catalogue(void) const { return catalog_; }
    void set_mempartition(mpartition *mpartition) { mpartition_ = mpartition; }
    mpartition *mempartition(void) const { return mpartition_; }
    void set_mempartition_cfg(string mpart_cfg) { mpart_cfg_ = mpart_cfg; }
    string mempartition_cfg(void) { return mpart_cfg_; }
    void set_prog_info(program_info *pginfo) { pginfo_ = pginfo; }
    program_info *prog_info(void) const { return pginfo_; }
    platform_type_t platform_type(void) const { return platform_type_; }
    sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;
    sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) override;
    void set_scale_profile(pds_scale_profile_t profile) {
        scale_profile_ = profile;
    }
    sdk::lib::kvstore *kvstore(void) const { return kvstore_; }
    void *port_metrics_handle(void) const { return port_metrics_hndl_; }
    void *mgmt_port_metrics_handle(void) const { return mgmt_port_metrics_hndl_; }
    void *hostif_metrics_handle(void) const { return hostif_metrics_hndl_; }
    pds_scale_profile_t scale_profile(void) const { return scale_profile_; }
    bool vpp_ipc_mock(void) const { return vpp_ipc_mock_; }
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
    device_state *device_db(void) {
        return (device_state *)state_[PDS_STATE_DEVICE];
    }
    lif_state *lif_db(void) {
        return (lif_state *)state_[PDS_STATE_LIF];
    }
    if_state *if_db(void) {
        return (if_state *)state_[PDS_STATE_IF];
    }
    tep_state *tep_db(void) {
        return (tep_state *)state_[PDS_STATE_TEP];
    }
    vpc_state *vpc_db(void) {
        return (vpc_state *)state_[PDS_STATE_VPC];
    }
    subnet_state *subnet_db(void) {
        return (subnet_state *)state_[PDS_STATE_SUBNET];
    }
    vnic_state *vnic_db(void) {
        return (vnic_state *)state_[PDS_STATE_VNIC];
    }
    mapping_state *mapping_db(void) {
        return (mapping_state *)state_[PDS_STATE_MAPPING];
    }
    route_table_state *route_table_db(void) {
        return (route_table_state *)state_[PDS_STATE_ROUTE_TABLE];
    }
    policy_state *policy_db(void) {
        return (policy_state *)state_[PDS_STATE_POLICY];
    }
    mirror_session_state *mirror_session_db(void) {
        return (mirror_session_state *)state_[PDS_STATE_MIRROR];
    }
    meter_state *meter_db(void) {
        return (meter_state *)state_[PDS_STATE_METER];
    }
    tag_state *tag_db(void) {
        return (tag_state *)state_[PDS_STATE_TAG];
    }
    svc_mapping_state *svc_mapping_db(void) {
        return (svc_mapping_state *)state_[PDS_STATE_SVC_MAPPING];
    }
    vpc_peer_state *vpc_peer_db(void) {
        return (vpc_peer_state *)state_[PDS_STATE_VPC_PEER];
    }
    nexthop_state *nexthop_db(void) {
        return (nexthop_state *)state_[PDS_STATE_NEXTHOP];
    }
    nexthop_group_state *nexthop_group_db(void) {
        return (nexthop_group_state *)state_[PDS_STATE_NEXTHOP_GROUP];
    }
    policer_state *policer_db(void) {
        return (policer_state *)state_[PDS_STATE_POLICER];
    }
    nat_state *nat_db(void) {
        return (nat_state *)state_[PDS_STATE_NAT];
    }
    dhcp_state *dhcp_db(void) {
        return (dhcp_state *)state_[PDS_STATE_DHCP];
    }
    pds_event_cb_t event_cb(void) const { return event_cb_; }
    void set_event_cb(pds_event_cb_t event_cb) { event_cb_ = event_cb; }
    void event_notify(const pds_event_t *event) {
        if (event_cb_) {
            event_cb_(event);
        }
    }
    const mac_addr_t& system_mac(void) const { return system_mac_; }
    void set_system_mac(mac_addr_t mac) {
        memcpy(system_mac_, mac, sizeof(system_mac_));
    }
    learn_state *learn_db(void) {
        return (learn_state *)state_[PDS_STATE_LEARN];
    }
    virtual sdk_ret_t transaction_begin(void) override;
    virtual sdk_ret_t transaction_end(bool abort) override;

    // bootype. TODO: returning true now. need to fix.
    bool cold_boot(void) { return true; }

private:
    sdk_ret_t parse_global_config_(string pipeline, string cfg_file);

private:
    string                  cfg_path_;
    string                  pipeline_;
    string                  mpart_cfg_;
    catalog                 *catalog_;
    mpartition              *mpartition_;
    platform_type_t         platform_type_;
    pds_scale_profile_t     scale_profile_;
    // TODO: remove this after we phase out apollo/artemis tests
    bool                    vpp_ipc_mock_;
    program_info            *pginfo_;
    uint64_t                control_cores_mask_;
    uint16_t                num_control_cores_;
    uint64_t                data_cores_mask_;
    uint16_t                num_data_cores_;
    state_base              *state_[PDS_STATE_MAX];
    pds_event_cb_t          event_cb_;
    mac_addr_t              system_mac_;
    sdk::lib::kvstore       *kvstore_;
    // handles for the metrics
    void                    *port_metrics_hndl_;
    void                    *mgmt_port_metrics_hndl_;
    void                    *hostif_metrics_hndl_;
};
extern pds_state g_pds_state;

/** * @} */    // end of PDS_STATE

}    // namespace api

using api::pds_state;

static inline device_state *
device_db (void)
{
    return api::g_pds_state.device_db();
}

static inline tep_state *
tep_db (void)
{
    return api::g_pds_state.tep_db();
}

static inline vpc_state *
vpc_db (void)
{
    return api::g_pds_state.vpc_db();
}

static inline subnet_state *
subnet_db (void)
{
    return api::g_pds_state.subnet_db();
}

static inline vnic_state *
vnic_db (void)
{
    return api::g_pds_state.vnic_db();
}

static inline mapping_state *
mapping_db (void)
{
    return api::g_pds_state.mapping_db();
}

static inline route_table_state *
route_table_db (void)
{
    return api::g_pds_state.route_table_db();
}

static inline policy_state *
policy_db (void)
{
    return api::g_pds_state.policy_db();
}

static inline if_state *
if_db (void)
{
    return api::g_pds_state.if_db();
}

static inline mirror_session_state *
mirror_session_db (void)
{
    return api::g_pds_state.mirror_session_db();
}

static inline meter_state *
meter_db (void)
{
    return api::g_pds_state.meter_db();
}

static inline tag_state *
tag_db (void)
{
    return api::g_pds_state.tag_db();
}

static inline lif_state *
lif_db (void)
{
    return api::g_pds_state.lif_db();
}

static inline svc_mapping_state *
svc_mapping_db (void)
{
    return api::g_pds_state.svc_mapping_db();
}

static inline vpc_peer_state *
vpc_peer_db (void)
{
    return api::g_pds_state.vpc_peer_db();
}

static inline nexthop_state *
nexthop_db (void)
{
    return api::g_pds_state.nexthop_db();
}

static inline nexthop_group_state *
nexthop_group_db (void)
{
    return api::g_pds_state.nexthop_group_db();
}

static inline policer_state *
policer_db (void)
{
    return api::g_pds_state.policer_db();
}

static inline nat_state *
nat_db (void)
{
    return api::g_pds_state.nat_db();
}

static inline dhcp_state *
dhcp_db (void)
{
    return api::g_pds_state.dhcp_db();
}

static inline learn_state *
learn_db (void)
{
    return api::g_pds_state.learn_db();
}

#endif    // __PDS_STATE_HPP__
