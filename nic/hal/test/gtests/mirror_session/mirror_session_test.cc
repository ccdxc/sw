//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <assert.h>
#include <gtest/gtest.h>

#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"

#include "gen/proto/telemetry.grpc.pb.h"

using namespace hal;
using namespace hal::pd;
using namespace sdk::lib;


using telemetry::MirrorSessionSpec;
using telemetry::MirrorSessionStatus;
using telemetry::MirrorSessionResponse;
using telemetry::MirrorSessionDeleteRequest;
using telemetry::MirrorSessionDeleteResponse;
using telemetry::MirrorSessionGetRequest;
using telemetry::MirrorSessionGetResponse;
using telemetry::RuleAction;
using telemetry::ERSpanSpec;

hal_handle_t vrfh_, nwh_, l2segh_, eph1_, eph2_,  intfh1_, intfh2_, gre_, flowmonh_;

struct {
    uint32_t ip;
    uint64_t mac;
} g_eps[] = {
    { 0x0A000001, 0xAABB0A000001},
    { 0x0A000002, 0xAABB0A000002},
};

uint32_t session_id = 1234;
uint32_t mgmt_ip = 0x0A0A0A08;
uint32_t collector_ip = 0x0A000001;

static inline void
mirror_session_test_memleak_checks (void)
{
    EXPECT_EQ(g_hal_state->mirror_session_ht()->num_entries(), 0);
    EXPECT_EQ(g_hal_state->mirror_session_slab()->num_in_use(), 0);

    EXPECT_EQ(g_hal_state_pd->mirror_session_pd_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state_pd->mirror_session_idxr()->num_indices_allocated(), 0);
}

static hal_handle_t
add_gre_tunnel(uint8_t port_num, hal_handle_t vrfh,
               uint32_t sip, uint32_t dip, uint32_t ttl=100)
{
    InterfaceSpec gre_spec;
    InterfaceResponse gre_rsp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(vrfh);
    EXPECT_NE(vrf, nullptr);

    gre_spec.mutable_key_or_handle()->set_interface_id(port_num);
    gre_spec.set_type(::intf::IfType::IF_TYPE_TUNNEL);
    gre_spec.set_admin_status(::intf::IfStatus::IF_STATUS_UP);
    gre_spec.mutable_if_tunnel_info()->set_encap_type(::intf::IF_TUNNEL_ENCAP_TYPE_GRE);
    gre_spec.mutable_if_tunnel_info()->mutable_gre_info()->mutable_source()->set_ip_af(::types::IP_AF_INET);
    gre_spec.mutable_if_tunnel_info()->mutable_gre_info()->mutable_source()->set_v4_addr(sip);
    gre_spec.mutable_if_tunnel_info()->mutable_gre_info()->mutable_destination()->set_ip_af(::types::IP_AF_INET);
    gre_spec.mutable_if_tunnel_info()->mutable_gre_info()->mutable_destination()->set_v4_addr(dip);
    gre_spec.mutable_if_tunnel_info()->mutable_gre_info()->set_ttl(100);
    gre_spec.mutable_if_tunnel_info()->mutable_vrf_key_handle()->set_vrf_id(vrf->vrf_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    SDK_ASSERT(hal::interface_create(gre_spec, &gre_rsp) == HAL_RET_OK);
    hal::hal_cfg_db_close();

    return gre_rsp.mutable_status()->if_handle();
}

class mirror_session_test : public fte_base_test {
private:

protected:
    uint32_t vrf_id;
    mirror_session_test() { }
    virtual ~mirror_session_test() { }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
        vrf_id = get_vrf_id(vrfh_);
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
        mirror_session_test_memleak_checks();
    }

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        fte_base_test::SetUpTestCase();
        hal_test_utils_slab_disable_delete();

        // create_topology
        vrfh_ = add_vrf();
        nwh_ = add_network(vrfh_, 0x0A000000, 8, 0xAABB0A000000);
        l2segh_ = add_l2segment(nwh_, 100);
        intfh1_ =  add_uplink(PORT_NUM_1);
        if_port_oper_state_process_event(LOGICAL_PORT_NUM_1, port_event_t::PORT_EVENT_LINK_UP);
        intfh2_ =  add_uplink(PORT_NUM_2);
        if_port_oper_state_process_event(LOGICAL_PORT_NUM_2, port_event_t::PORT_EVENT_LINK_UP);
        eph1_ = add_endpoint(l2segh_, intfh1_, g_eps[0].ip, g_eps[0].mac, 0);
        eph2_ = add_endpoint(l2segh_, intfh2_, g_eps[1].ip, g_eps[1].mac, 0);
        gre_ = add_gre_tunnel(3, vrfh_, mgmt_ip, collector_ip);
    }

    MirrorSessionResponse create_erspan_session(uint32_t session_id, uint32_t vrf_id,
                                                uint32_t sip, uint32_t dip) {
        MirrorSessionSpec spec;
        MirrorSessionResponse rsp;

        spec.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        spec.mutable_key_or_handle()->set_mirrorsession_id(session_id);
        spec.mutable_erspan_spec()->mutable_src_ip()->set_ip_af(::types::IP_AF_INET);
        spec.mutable_erspan_spec()->mutable_src_ip()->set_v4_addr(sip);
        spec.mutable_erspan_spec()->mutable_dest_ip()->set_ip_af(::types::IP_AF_INET);
        spec.mutable_erspan_spec()->mutable_dest_ip()->set_v4_addr(dip);
        spec.mutable_erspan_spec()->set_span_id(session_id);

        mirror_session_create(spec, &rsp);
        return rsp;
    }

    MirrorSessionDeleteResponse delete_erspan_session(uint32_t session_id) {
        MirrorSessionDeleteRequest req;
        MirrorSessionDeleteResponse rsp;

        req.mutable_key_or_handle()->set_mirrorsession_id(session_id);
        mirror_session_delete(req, &rsp);
        return rsp;
    }

    MirrorSessionGetResponseMsg get_session(uint32_t session_id, bool all=false) {
        MirrorSessionGetRequest req;
        MirrorSessionGetResponseMsg rsp;

        if (all == false) {
            req.mutable_key_or_handle()->set_mirrorsession_id(session_id);
        }
        mirror_session_get(req, &rsp);
        return rsp;
    }

    FlowMonitorRuleResponse add_flowmon_policy(uint32_t vrf_id, uint32_t rule_id,
                                               uint32_t session_id) {
        hal_ret_t ret;
        FlowMonitorRuleSpec                      spec;
        FlowMonitorRuleResponse                  rsp;

        spec.mutable_key_or_handle()->set_flowmonitorrule_id(rule_id);
        spec.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        auto action = spec.mutable_action();
        action->add_action(RuleAction::MIRROR);
        auto mkh = action->add_ms_key_handle();
        mkh->set_mirrorsession_id(session_id);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::flow_monitor_rule_create(spec, &rsp);
        hal::hal_cfg_db_close();
        EXPECT_EQ(ret, HAL_RET_OK);
        return rsp;
    }

    FlowMonitorRuleGetResponseMsg get_flowmon_policy(uint32_t rule_id) {
        FlowMonitorRuleGetRequest req;
        FlowMonitorRuleGetResponseMsg rsp;

        auto key = req.mutable_key_or_handle();
        key->set_flowmonitorrule_id(rule_id);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        auto ret = hal::flow_monitor_rule_get(req, &rsp);
        hal::hal_cfg_db_close();
        EXPECT_EQ(ret, HAL_RET_OK);

        return rsp;
    }

    static uint32_t get_vrf_id(hal_handle_t handle) {
        hal::vrf_t *vrf = hal::vrf_lookup_by_handle(handle);
        EXPECT_NE(vrf, nullptr);
        return vrf->vrf_id;
    }
};

TEST_F (mirror_session_test, hw_id_indexer)
{
    uint32_t index;
    auto idxr = g_hal_state_pd->mirror_session_idxr();

    // allocate max mirror session hw index
    for (auto i = 0; i < MAX_MIRROR_SESSION_DEST; i++) {
        auto status = idxr->alloc(&index);
        EXPECT_EQ(status, indexer::SUCCESS);
        EXPECT_LT(index, MAX_MIRROR_SESSION_DEST);
    }

    // try to go over scale
    auto status = idxr->alloc(&index);
    EXPECT_NE(status, indexer::SUCCESS);

    // free mirror sesssion hw index
    for (auto i = 0; i < MAX_MIRROR_SESSION_DEST; i++) {
        auto status = idxr->free(i);
        EXPECT_EQ(status, indexer::SUCCESS);
    }
}

TEST_F (mirror_session_test, erspan_basic) {
    // create erspan session
    auto create_rsp = create_erspan_session(session_id, vrf_id, mgmt_ip, collector_ip);
    EXPECT_EQ(create_rsp.api_status(), types::API_STATUS_OK);
    EXPECT_LT(create_rsp.status().handle(), MAX_MIRROR_SESSION_DEST);

    // get  erspan session
    auto get_rsp = get_session(session_id);
    EXPECT_EQ(get_rsp.response_size(), 1);
    EXPECT_EQ(get_rsp.response(0).api_status(), types::API_STATUS_OK);
    auto spec = get_rsp.response(0).spec();
    EXPECT_EQ(spec.key_or_handle().mirrorsession_id(), session_id);
    auto erspan = spec.erspan_spec();
    EXPECT_EQ(erspan.src_ip().ip_af(), types::IPAddressFamily::IP_AF_INET);
    EXPECT_EQ(erspan.src_ip().v4_addr(), mgmt_ip);
    EXPECT_EQ(erspan.dest_ip().ip_af(), types::IPAddressFamily::IP_AF_INET);
    EXPECT_EQ(erspan.dest_ip().v4_addr(), collector_ip);

    // delete erspan session
    auto del_rsp = delete_erspan_session(session_id);
    EXPECT_EQ(del_rsp.api_status(), types::API_STATUS_OK);
}

TEST_F (mirror_session_test, erspan_scale) {
    MirrorSessionResponse create_rsp;
    MirrorSessionSpec spec;
    MirrorSessionDeleteResponse del_rsp;
    ERSpanSpec erspan;
    FlowMonitorRuleGetResponseMsg flowmon_rsp;
    mirror_session_id_t hw_id;
    std::unordered_set<uint32_t> mirror_session_id_set;

    // create erspan session
    for(auto i=0; i<MAX_MIRROR_SESSION_DEST; i++) {
        create_rsp = create_erspan_session(session_id+i, vrf_id, mgmt_ip, collector_ip);
        EXPECT_EQ(create_rsp.api_status(), types::API_STATUS_OK);
        EXPECT_LT(create_rsp.status().handle(), MAX_MIRROR_SESSION_DEST);
        EXPECT_EQ(mirror_session_get_hw_id(session_id+i, &hw_id), HAL_RET_OK);
        EXPECT_EQ(create_rsp.status().handle(), hw_id);
        add_flowmon_policy(vrf_id, session_id+i, session_id+i);
        flowmon_rsp = get_flowmon_policy(session_id+i);
        EXPECT_EQ(flowmon_rsp.response_size(), 1);
        EXPECT_EQ(flowmon_rsp.response(0).spec().action().ms_key_handle(0).mirrorsession_id(), session_id+i);
    }

    // try creating existing session again
    create_rsp = create_erspan_session(session_id, vrf_id, mgmt_ip,
                                       collector_ip);
    EXPECT_EQ(create_rsp.api_status(), types::API_STATUS_EXISTS_ALREADY);

    // try go over session scale
    create_rsp = create_erspan_session(session_id+MAX_MIRROR_SESSION_DEST,
                                       vrf_id, mgmt_ip, collector_ip);
    EXPECT_EQ(create_rsp.api_status(), types::API_STATUS_OUT_OF_RESOURCE);

    // get  erspan session
    auto get_rsp = get_session(session_id, true);
    EXPECT_EQ(get_rsp.response_size(), MAX_MIRROR_SESSION_DEST);
    for(auto i=0; i<MAX_MIRROR_SESSION_DEST; i++) {
        EXPECT_EQ(get_rsp.response(i).api_status(), types::API_STATUS_OK);
        spec = get_rsp.response(i).spec();
        ASSERT_TRUE(mirror_session_id_set.insert(spec.key_or_handle().mirrorsession_id()).second);
        erspan = spec.erspan_spec();
        EXPECT_EQ(erspan.src_ip().ip_af(), types::IPAddressFamily::IP_AF_INET);
        EXPECT_EQ(erspan.src_ip().v4_addr(), mgmt_ip);
        EXPECT_EQ(erspan.dest_ip().ip_af(), types::IPAddressFamily::IP_AF_INET);
        EXPECT_EQ(erspan.dest_ip().v4_addr(), collector_ip);
    }
    EXPECT_EQ(mirror_session_id_set.size(), MAX_MIRROR_SESSION_DEST);

    // delete non existing session
    del_rsp = delete_erspan_session(session_id+MAX_MIRROR_SESSION_DEST);
    EXPECT_NE(del_rsp.api_status(), types::API_STATUS_OK);

    // delete erspan session
    for(auto i=0; i<MAX_MIRROR_SESSION_DEST; i++) {
        del_rsp = delete_erspan_session(session_id+i);
        EXPECT_EQ(del_rsp.api_status(), types::API_STATUS_OK);
    }
}

TEST_F (mirror_session_test, repin) {
    MirrorSessionResponse create_rsp;
    MirrorSessionDeleteResponse del_rsp;
    hal_ret_t ret;
    auto ms_ht = g_hal_state->mirror_session_ht();

    // create mirror sessions
    for(auto i=0; i<MAX_MIRROR_SESSION_DEST; i++) {
        auto create_rsp = create_erspan_session(session_id+i, vrf_id, mgmt_ip, collector_ip);
        EXPECT_EQ(create_rsp.api_status(), types::API_STATUS_OK);
    }

    // bring down the port 1
    ret = if_port_oper_state_process_event(LOGICAL_PORT_NUM_1, port_event_t::PORT_EVENT_LINK_DOWN);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // invoke mirror session repin
    if_t *intf1 = find_if_by_handle(intfh1_);
    ASSERT_TRUE(intf1);
    if_t *intf2 = find_if_by_handle(intfh2_);
    ASSERT_TRUE(intf2);
    EXPECT_EQ(telemetry_mirror_session_handle_repin(), HAL_RET_OK);

#if 0
    auto walk_func = [](void *entry, void *ctxt) {
        mirror_session_t *session = (mirror_session_t *)entry;
        if_t *sess_if = (if_t *)ctxt;
        EXPECT_EQ(sess_if, session->dest_if);
        return false;
    };

    ms_ht->walk_safe(walk_func, intf2);
#endif
    // restore original state
    ret = if_port_oper_state_process_event(LOGICAL_PORT_NUM_1, port_event_t::PORT_EVENT_LINK_UP);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete erspan session
    for(auto i=0; i<MAX_MIRROR_SESSION_DEST; i++) {
        del_rsp = delete_erspan_session(session_id+i);
        EXPECT_EQ(del_rsp.api_status(), types::API_STATUS_OK);
    }
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
