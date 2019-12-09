#include "nic/fte/test/fte_base_test.hpp"
#include "nic/fte/fte_flow.hpp"
#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec_group.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/endpoint.pb.h"
#include "gen/proto/nw.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "lib/utils/time_profile.hpp"

#define UPLINK_IF_ID_OFFSET 127
#define ENIC_IF_ID_OFFSET 150
#define LIF_ID_OFFSET     60

using namespace hal::plugins::sfw;

uint32_t fte_base_test::vrf_id_ = 0;
uint32_t fte_base_test::l2seg_id_ = 0;
uint32_t fte_base_test::intf_id_ = 0;
uint32_t fte_base_test::nwsec_id_ = 0;
uint32_t fte_base_test::nh_id_ = 0;
uint32_t fte_base_test::pool_id_ = 0;
uint64_t fte_base_test::flowmon_rule_id_ = 0;
uint32_t fte_base_test::lif_id_ = 0;
fte::ctx_t fte_base_test::ctx_ = {};
bool  fte_base_test::ipc_logging_disable_ = false;
std::vector<dev_handle_t> fte_base_test::handles;
std::map<hal_handle_t, ep_info_t> fte_base_test::eps;

static slab* v4_test_rule_slab_ =
    slab::factory("v4_test_rule", 0, sizeof(fte_base_test::v4_rule_t), 10*1024, true, true, true);

static fte_base_test::v4_rule_t *v4_test_rule_alloc() {
    fte_base_test::v4_rule_t *rule = (fte_base_test::v4_rule_t*)v4_test_rule_slab_->alloc();
    return rule;
}

hal_handle_t fte_base_test::add_vrf()
{
    hal_ret_t                   ret;
    vrf::VrfSpec                spec;
    vrf::VrfResponse            resp;
    nwsec::SecurityProfileSpec  sp_spec;
    nwsec::SecurityProfileResponse  sp_rsp;

    // Create security profile
    sp_spec.mutable_key_or_handle()->set_profile_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    spec.mutable_key_or_handle()->set_vrf_id(++fte_base_test::vrf_id_);
    spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(spec, &resp);
    hal::hal_cfg_db_close();

    EXPECT_EQ(ret, HAL_RET_OK);

    return resp.mutable_vrf_status()->key_or_handle().vrf_handle();
}

hal_handle_t fte_base_test::add_network(hal_handle_t vrfh, uint32_t v4_addr, uint8_t prefix_len, uint64_t rmac)
{
    hal_ret_t ret;
    nw::NetworkSpec spec;
    nw::NetworkResponse resp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(vrfh);
    EXPECT_NE(vrf, nullptr);

    spec.set_rmac(rmac);
    spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(vrf->vrf_id);
    spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(v4_addr);
    spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(prefix_len);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(spec, &resp);
    hal::hal_cfg_db_close();

    EXPECT_EQ(ret, HAL_RET_OK);

    return resp.mutable_status()->key_or_handle().nw_handle();
}

hal_handle_t fte_base_test::add_l2segment(hal_handle_t nwh, uint16_t vlan_id)
{
    hal_ret_t ret;
    l2segment::L2SegmentSpec spec;
    l2segment::L2SegmentResponse resp;

    hal::network_t *nw = hal::find_network_by_handle(nwh);
    EXPECT_NE(nw, nullptr);

    spec.mutable_vrf_key_handle()->set_vrf_id(nw->nw_key.vrf_id);
    spec.add_network_key_handle()->set_nw_handle(nwh);
    spec.mutable_key_or_handle()->set_segment_id(++l2seg_id_);
    spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    spec.mutable_wire_encap()->set_encap_value(vlan_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(spec, &resp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    return resp.mutable_l2segment_status()->key_or_handle().l2segment_handle();
}

hal_handle_t fte_base_test::add_enic(hal_handle_t l2segh, uint32_t useg, uint64_t mac, hal_handle_t uplinkh)
{
    hal_ret_t ret;
    intf::InterfaceSpec spec;
    intf::InterfaceResponse resp;
    intf::LifSpec           lif_spec;
    intf::LifResponse       lif_rsp;
    uint32_t                lif_id;

    // Create a lif
    lif_id = LIF_ID_OFFSET + ++lif_id_;
    lif_spec.mutable_key_or_handle()->set_lif_id(lif_id);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_if_handle(uplinkh);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    spec.set_type(intf::IF_TYPE_ENIC);
    spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_handle(lif_rsp.mutable_status()->lif_handle());
    spec.mutable_key_or_handle()->set_interface_id(ENIC_IF_ID_OFFSET + ++intf_id_);
    spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_l2segment_handle(l2segh);
    spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac);
    spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(useg);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &resp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    return resp.mutable_status()->if_handle();
}

hal_handle_t fte_base_test::add_uplink(uint8_t port_num)
{
    hal_ret_t ret;
    intf::InterfaceSpec spec;
    intf::InterfaceResponse resp;

    // Create an uplink
    spec.set_type(intf::IF_TYPE_UPLINK);
    spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + ++intf_id_);
    spec.mutable_if_uplink_info()->set_port_num(port_num);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &resp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    return resp.mutable_status()->if_handle();
}

hal_handle_t fte_base_test::add_endpoint(hal_handle_t l2segh, hal_handle_t intfh,
                                         uint32_t ip, uint64_t mac, uint16_t useg_vlan,
                                         bool enable_e2e, bool set_uplink, hal_handle_t uplink)
{
    hal_ret_t ret;
    endpoint::EndpointSpec spec;
    endpoint::EndpointResponse resp;
    ep_info_t  ep;

    hal::l2seg_t *l2seg = hal::l2seg_lookup_by_handle(l2segh);
    EXPECT_NE(l2seg, nullptr);

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(l2seg->vrf_handle);
    EXPECT_NE(vrf, nullptr);


    spec.mutable_vrf_key_handle()->set_vrf_id(vrf->vrf_id);
    spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()
        ->mutable_l2segment_key_handle()->set_l2segment_handle(l2segh);
    spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(intfh);
    spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac);
    if (ip != 0x0) {
        auto addr = spec.mutable_endpoint_attrs()->add_ip_address();
        addr->set_ip_af(types::IP_AF_INET);
        addr->set_v4_addr(ip);
    }
    spec.mutable_endpoint_attrs()->set_useg_vlan(useg_vlan);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(spec, &resp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);
    if (enable_e2e) {
        ep.ip = ip;
        ep.mac = mac;
        if (l2seg->wire_encap.type == types::ENCAP_TYPE_DOT1Q)
            ep.vlan = l2seg->wire_encap.val;
        else
            ep.vlan = 0;
        eps.insert(std::pair<hal_handle_t, ep_info_t>(resp.endpoint_status().key_or_handle().endpoint_handle(),ep));
    }

    return resp.endpoint_status().key_or_handle().endpoint_handle();
}

static inline void uuid_to_string(uint8_t *uuid, char *str)  {
    uint8_t offset = 0;
    char    c = '-';

    for (uint8_t idx=0; idx<16; idx++) {
        sprintf(&str[offset], "%02x", uuid[idx]);
        offset += 2;
        if (idx == 3 || idx == 5 || idx == 7 || idx == 9)
            sprintf(&str[offset++], "%c", c);
    }
}

hal_handle_t fte_base_test::add_nwsec_policy(hal_handle_t vrfh, std::vector<fte_base_test::v4_rule_t> &rules)
{
    hal_ret_t ret;
    nwsec::SecurityPolicySpec                      spec;
    nwsec::SecurityPolicyResponse                  resp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(vrfh);
    EXPECT_NE(vrf, nullptr);

    spec.mutable_key_or_handle()->mutable_security_policy_key()->
        set_security_policy_id(++nwsec_id_);
    spec.mutable_key_or_handle()->mutable_security_policy_key()->
        mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);

    uint32_t rule_id = 0;
    for (auto &rule: rules) {
        nwsec::SecurityRule *rule_spec = spec.add_rule();

        rule_spec->set_rule_id(++rule_id);
        rule_spec->mutable_action()->set_sec_action(rule.action);
        types::RuleMatch *match = rule_spec->mutable_match();

        if (rule.to.addr) {
            types::IPPrefix *prefix = match->add_dst_address()->
                mutable_address()->mutable_prefix()->mutable_ipv4_subnet();

            prefix->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            prefix->mutable_address()->set_v4_addr(rule.to.addr);
            prefix->set_prefix_len(rule.to.plen ?: 32);
        }

        if (rule.from.addr) {
            types::IPPrefix *prefix = match->add_src_address()->
                mutable_address()->mutable_prefix()->mutable_ipv4_subnet();
            prefix->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            prefix->mutable_address()->set_v4_addr(rule.from.addr);
            prefix->set_prefix_len(rule.from.plen ?: 32);
        }

        types::L4PortRange *port_range = match->mutable_app_match()->mutable_port_info()->add_dst_port_range();
        port_range->set_port_low(rule.app.dport_low);
        port_range->set_port_high(rule.app.dport_high);

        if (rule.app.proto) {
            match->set_protocol((types::IPProtocol)rule.app.proto);
        }

        if (rule.app.alg) {
            nwsec::AppData* app_data = rule_spec->mutable_action()->mutable_app_data();
            app_data->set_alg(rule.app.alg);
            app_data->set_idle_timeout(rule.app.idle_timeout);
            if (rule.app.has_alg_opts) {
                if (rule.app.alg == nwsec::APP_SVC_FTP) {
                    app_data->mutable_ftp_option_info()->set_allow_mismatch_ip_address(\
                                          rule.app.alg_opt.opt.ftp_opts.allow_mismatch_ip_address);
                } else if (rule.app.alg == nwsec::APP_SVC_SUN_RPC) {
                    for (uint8_t idx=0; idx<rule.app.alg_opt.opt.sunrpc_opts.programid_sz; idx++) {
                        nwsec::AppData_RPCData *rpc_data = app_data->mutable_sun_rpc_option_info()->add_data();
                        std::string str;

                        str = std::to_string(rule.app.alg_opt.opt.sunrpc_opts.program_ids[idx].program_id);
                        printf("Program id :%d str: %s\n", rule.app.alg_opt.opt.sunrpc_opts.program_ids[idx].program_id,
                                         str.c_str());
                        rpc_data->set_program_id(str);
                        rpc_data->set_idle_timeout(rule.app.alg_opt.opt.sunrpc_opts.program_ids[idx].timeout);
                    }
                } else if (rule.app.alg == nwsec::APP_SVC_MSFT_RPC) {
                    for (uint8_t idx=0; idx<rule.app.alg_opt.opt.msrpc_opts.uuid_sz; idx++) {
                        nwsec::AppData_RPCData *rpc_data = app_data->mutable_msrpc_option_info()->add_data();
                        char str[50];

                        uuid_to_string(rule.app.alg_opt.opt.msrpc_opts.uuids[idx].uuid, str);
                        printf("UUID: %s\n", str);
                        rpc_data->set_program_id(str);
                        rpc_data->set_idle_timeout(rule.app.alg_opt.opt.msrpc_opts.uuids[idx].timeout);
                    }
                } else if (rule.app.alg == nwsec::APP_SVC_DNS) {
                    app_data->mutable_dns_option_info()->set_drop_multi_question_packets(\
                                      rule.app.alg_opt.opt.dns_opts.drop_multi_question_packets);
                    app_data->mutable_dns_option_info()->set_drop_large_domain_name_packets(\
                                      rule.app.alg_opt.opt.dns_opts.drop_large_domain_name_packets);
                    app_data->mutable_dns_option_info()->set_drop_long_label_packets(\
                                      rule.app.alg_opt.opt.dns_opts.drop_long_label_packets);
                    app_data->mutable_dns_option_info()->set_max_msg_length(\
                                      rule.app.alg_opt.opt.dns_opts.max_msg_length);
                }
            }
        }
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(spec, &resp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    return resp.policy_status().key_or_handle().security_policy_handle();
}

hal_handle_t fte_base_test::add_flowmon_policy(hal_handle_t vrfh, std::vector<fte_base_test::v4_rule_t> &rules)
{
    hal_ret_t ret;
    telemetry::FlowMonitorRuleSpec                      spec;
    telemetry::FlowMonitorRuleResponse                  resp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(vrfh);
    EXPECT_NE(vrf, nullptr);

    spec.mutable_key_or_handle()->set_flowmonitorrule_id(++flowmon_rule_id_);
    spec.mutable_vrf_key_handle()->set_vrf_id(vrf->vrf_id);

    for (auto &rule: rules) {
        types::RuleMatch *match = spec.mutable_match();
        *(spec.mutable_action()) = rule.mon_action;
        if (rule.collect) {
            kh::CollectorKeyHandle *ckh = spec.add_collector_key_handle();
            ckh->set_collector_id(2);
        }

        if (rule.to.addr) {
            types::IPPrefix *prefix = match->add_dst_address()->
                mutable_address()->mutable_prefix()->mutable_ipv4_subnet();

            prefix->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            prefix->mutable_address()->set_v4_addr(rule.to.addr);
            prefix->set_prefix_len(rule.to.plen ?: 32);
        }

        if (rule.from.addr) {
            types::IPPrefix *prefix = match->add_src_address()->
                mutable_address()->mutable_prefix()->mutable_ipv4_subnet();
            prefix->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            prefix->mutable_address()->set_v4_addr(rule.from.addr);
            prefix->set_prefix_len(rule.from.plen ?: 32);
        }

        types::L4PortRange *dport_range = match->mutable_app_match()->mutable_port_info()->add_dst_port_range();
        dport_range->set_port_low(rule.app.dport_low);
        dport_range->set_port_high(rule.app.dport_high);

        if (rule.app.proto) {
            match->set_protocol((types::IPProtocol)rule.app.proto);
        }
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::flow_monitor_rule_create(spec, &resp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    return resp.status().handle();
}

hal_handle_t fte_base_test::add_route(hal_handle_t vrfh,
                                      uint32_t v4_addr, uint8_t prefix_len,
                                      hal_handle_t eph)
{
    hal_ret_t                          ret;
    nw::RouteSpec                   route_spec;
    nw::RouteResponse               route_rsp;
    nw::NexthopSpec                 nh_spec;
    nw::NexthopResponse             nh_rsp;

     // Create a nexthop with EP
    nh_spec.mutable_key_or_handle()->set_nexthop_id(++nh_id_);
    nh_spec.mutable_ep_key_or_handle()->set_endpoint_handle(eph);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nexthop_create(nh_spec, &nh_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    hal_handle_t nhh = nh_rsp.mutable_status()->nexthop_handle();

    // Create a route
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_vrf_key_handle()->set_vrf_handle(vrfh);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->set_prefix_len(prefix_len);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(v4_addr);
    route_spec.mutable_nh_key_or_handle()->set_nexthop_handle(nhh);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::route_create(route_spec, &route_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    return route_rsp.mutable_status()->route_handle();
}

hal_ret_t fte_base_test::inject_pkt(fte::cpu_rxhdr_t *cpu_rxhdr,
                                    std::vector<uint8_t *> &pkts, size_t pkt_len, bool copied_pkt_arg)
{
    struct fn_ctx_t {
        fte::ctx_t *ctx;
        fte::cpu_rxhdr_t *cpu_rxhdr;
        std::vector<uint8_t *> &pkts;
        size_t pkt_len;
        bool   copied_pkt;
        hal_ret_t ret;
    } fn_ctx = { &ctx_, cpu_rxhdr, pkts, pkt_len, copied_pkt_arg, HAL_RET_OK };

    fte::fte_execute(FTE_ID, [](void *data) {
        fn_ctx_t *fn_ctx = (fn_ctx_t *)data;
        auto pkt = fn_ctx->pkts[0];
        fte::ctx_t *ctx = fn_ctx->ctx;

        fte::flow_t iflow[fte::ctx_t::MAX_STAGES], rflow[fte::ctx_t::MAX_STAGES];
        uint16_t num_features;
        size_t fstate_size = fte::feature_state_size(&num_features);
        fte::feature_state_t *feature_state = (fte::feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);

        for (int i=0; i<fte::ctx_t::MAX_STAGES; i++) {
            iflow[i]  =  {};
            rflow[i]  =  {};
        }

        time_profile_begin(sdk::utils::time_profile::FTE_CTXT_INIT);
        for (uint32_t i=0; i<fn_ctx->pkts.size(); i++) {
            hal::hal_cfg_db_open(hal::CFG_OP_READ);
            fn_ctx->ret = ctx->init(fn_ctx->cpu_rxhdr, pkt, fn_ctx->pkt_len, fn_ctx->copied_pkt,
                                    iflow, rflow, feature_state, num_features);
            if (fn_ctx->ret == HAL_RET_OK) {
                fn_ctx->ret = ctx->process();
            }
            SDK_ASSERT(fn_ctx->ret == HAL_RET_OK);
            hal::hal_cfg_db_close();
        }
        time_profile_end(sdk::utils::time_profile::FTE_CTXT_INIT);
        HAL_FREE(hal::HAL_MEM_ALLOC_FTE, feature_state);
    }, &fn_ctx );

    return HAL_RET_OK;
}

hal_ret_t fte_base_test::inject_pkt(fte::cpu_rxhdr_t *cpu_rxhdr,
                                    uint8_t *pkt, size_t pkt_len, bool copied_pkt)
{
    std::vector<uint8_t *> pkts = { pkt };

    return inject_pkt(cpu_rxhdr, pkts, pkt_len, copied_pkt);
}

static inline ip_addr_t ep_ip(hal::ep_t *ep) {
    return dllist_entry(ep->ip_list_head.next, hal::ep_ip_entry_t, ep_ip_lentry)->ip_addr;
}

hal_ret_t
fte_base_test::inject_eth_pkt(const fte::lifqid_t &lifq,
                              hal_handle_t src_ifh, hal_handle_t src_l2segh,
                              std::vector<Tins::EthernetII> &pkts, bool add_padding)
{
    hal::pd::pd_if_get_hw_lif_id_args_t lif_args;
    hal::if_t *sif = hal::find_if_by_handle(src_ifh);
    hal::l2seg_t *l2seg = hal::l2seg_lookup_by_handle(src_l2segh);
    EXPECT_NE(l2seg, nullptr);

    // use first pkt to build cpu header
    Tins::EthernetII eth = pkts[0];

    uint8_t vlan_valid;
    uint16_t vlan_id;
    hal::if_l2seg_get_encap(sif, l2seg, &vlan_valid, &vlan_id);

    hal::pd::pd_l2seg_get_flow_lkupid_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    args.l2seg = l2seg;
    pd_func_args.pd_l2seg_get_flow_lkupid = &args;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, &pd_func_args);
    hal::pd::l2seg_hw_id_t hwid = args.hwid;

    if (sif->if_type == intf::IF_TYPE_ENIC) {
        hal::pd::pd_func_args_t pd_func_args = {0};
        lif_args.pi_if = sif;
        pd_func_args.pd_if_get_hw_lif_id = &lif_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_HW_LIF_ID, &pd_func_args);
    }

    fte::cpu_rxhdr_t cpu_rxhdr = {};
    cpu_rxhdr.src_lif = (sif->if_type == intf::IF_TYPE_ENIC)?lif_args.hw_lif_id:sif->if_id;
    cpu_rxhdr.lif = lifq.lif;
    cpu_rxhdr.qtype = lifq.qtype;
    cpu_rxhdr.qid = lifq.qid;
    cpu_rxhdr.lkp_vrf = hwid;
    cpu_rxhdr.lkp_dir = sif->if_type == intf::IF_TYPE_ENIC ? FLOW_DIR_FROM_DMA :
        FLOW_DIR_FROM_UPLINK;
    cpu_rxhdr.lkp_inst = 0;
    cpu_rxhdr.lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
    cpu_rxhdr.l2_offset = 0;

    cpu_rxhdr.l3_offset = eth.header_size() + eth.find_pdu<Tins::Dot1Q>()->header_size();
    cpu_rxhdr.l4_offset = cpu_rxhdr.l3_offset + eth.find_pdu<Tins::IP>()->header_size();
    cpu_rxhdr.payload_offset = cpu_rxhdr.l4_offset;

    auto l4pdu = eth.find_pdu<Tins::IP>()->inner_pdu();
    if (l4pdu) {
        cpu_rxhdr.payload_offset +=  l4pdu->header_size();
    }

    cpu_rxhdr.flags = 0;
    Tins::TCP *tcp = eth.find_pdu<Tins::TCP>();
    if (tcp) {
        cpu_rxhdr.tcp_seq_num = ntohl(tcp->seq());
        cpu_rxhdr.tcp_flags = ((tcp->get_flag(Tins::TCP::SYN) << 1) |
                               (tcp->get_flag(Tins::TCP::ACK) << 4));
    }

    std::vector<uint8_t *>buffs;
    size_t buff_size;
    bool copied_pkt = true;

    std::vector<uint8_t> buffer = pkts[0].serialize();
    buff_size = buffer.size();
    if (add_padding) {
        buff_size += 4;
        buffer.resize(buff_size, 0);
    }

    for (auto pkt: pkts) {
        //uint8_t *buffer = (uint8_t *)malloc(buff_size);
        //vector<uint8_t> raw = pkt.serialize();
        //memcpy(buffer, &raw[0], buff_size);
        buffs.push_back(&buffer[0]);
    }

    return inject_pkt(&cpu_rxhdr, buffs, buff_size, copied_pkt);
}

hal_ret_t
fte_base_test::inject_eth_pkt(const fte::lifqid_t &lifq,
                              hal_handle_t src_ifh, hal_handle_t src_l2segh,
                              Tins::EthernetII &eth, bool add_padding)
{
    ::vector<Tins::EthernetII> pkts = {eth};
    return inject_eth_pkt(lifq, src_ifh, src_l2segh, pkts, add_padding);
}


hal_ret_t
fte_base_test::inject_ipv4_pkt(const fte::lifqid_t &lifq,
                               hal_handle_t deph, hal_handle_t seph,
                               Tins::PDU &l4pdu, bool add_padding)
{
    hal::ep_t *sep = hal::find_ep_by_handle(seph);
    EXPECT_NE(sep, nullptr);
    ip_addr_t sip =  ep_ip(sep);
    hal::if_t *sif = hal::find_if_by_handle(sep->if_handle);

    hal::ep_t *dep = hal::find_ep_by_handle(deph);
    EXPECT_NE(dep, nullptr);
    ip_addr_t dip =  ep_ip(dep);

    hal::l2seg_t *l2seg = hal::l2seg_lookup_by_handle(sep->l2seg_handle);
    EXPECT_NE(l2seg, nullptr);
    uint8_t vlan_valid;
    uint16_t vlan_id;
    hal::if_l2seg_get_encap(sif, l2seg, &vlan_valid, &vlan_id);

    Tins::EthernetII eth = Tins::EthernetII(dep->l2_key.mac_addr, sep->l2_key.mac_addr) /
        Tins::Dot1Q(vlan_id) /
        Tins::IP(Tins::IPv4Address(htonl(dip.addr.v4_addr)),
                 Tins::IPv4Address(htonl(sip.addr.v4_addr))) /
        l4pdu;

    return inject_eth_pkt(lifq, sep->if_handle, sep->l2seg_handle, eth, add_padding);
}

static
int parse_v4_packet (uint8_t *pkt, int len, uint16_t *l3_offset,
                     uint16_t *l4_offset, uint16_t *payload_offset, uint8_t *proto)
{
    ether_header_t *eth;
    vlan_header_t *vlan;
    ipv4_header_t *ip;
    tcp_header_t  *tcp;
    uint16_t etype;
    eth = (ether_header_t *)pkt;
    if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
        vlan = (vlan_header_t*)pkt;
        etype = ntohs(vlan->etype);
        ip = (ipv4_header_t *)(vlan+1);
        *l3_offset = sizeof(vlan_header_t);
    } else {
        etype = ntohs(eth->etype);
        ip = (ipv4_header_t *)(eth+1);
        *l3_offset = sizeof(ether_header_t);
    }

    // TBD: parse options
    if (etype == ETHERTYPE_IP) {
        *l4_offset = *l3_offset + sizeof(ipv4_header_t);
        tcp = (tcp_header_t *)(ip+1);
        if (ip->protocol == IPPROTO_TCP) {
            *payload_offset = *l4_offset + (tcp->doff*4);
            *proto = IPPROTO_TCP;
        } else if (ip->protocol == IPPROTO_UDP) {
            *payload_offset = *l4_offset + sizeof(udp_header_t);
            *proto = IPPROTO_UDP;
        } else {
            return 1;
        }
    } else {
        return 1;
    }

    return 0;
}

static
void fix_checksum(uint8_t *pkt, int len)
{
    ether_header_t *eth;
    vlan_header_t *vlan;
    ipv4_header_t *ip;
    tcp_header_t  *tcp;
    udp_header_t  *udp;
    uint16_t etype;
    int offset=0;

    eth = (ether_header_t *)pkt;
    if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
        vlan = (vlan_header_t*)pkt;
        etype = ntohs(vlan->etype);
        ip = (ipv4_header_t *)(vlan+1);
        offset += sizeof(vlan_header_t);
    } else {
        etype = ntohs(eth->etype);
        ip = (ipv4_header_t *)(eth+1);
        offset += sizeof(ether_header_t);
    }

    // TBD: parse options
    if (etype == ETHERTYPE_IP) {
        ip->check = checksum((short unsigned int*)ip, sizeof(ipv4_header_t));
        offset += sizeof(ipv4_header_t);
        if (ip->protocol == IPPROTO_TCP) {
            tcp = (tcp_header_t*)(ip+1);
            tcp->check = get_tcp_checksum((void *)tcp, (len-offset), ip->saddr, ip->daddr);
        } else if (ip->protocol == IPPROTO_UDP) {
            udp = (udp_header_t*)(ip+1);
            udp->check = get_udp_checksum((void *)udp, (len-offset), ip->saddr, ip->daddr);
        }
    }
}

void
fte_base_test::process_e2e_packets (void)
{
    int       maxfd, ret;
    uint16_t  nread, nwrite;
    char      inpktbuf[PKTBUF_LEN];
    char      *inp = inpktbuf;
    char      outpktbuf[PKTBUF_LEN];
    char      *outp = outpktbuf;
    uint16_t  l3_offset=0, l4_offset=0, payload_offset=0;
    fte::lifqid_t lifq = fte::FLOW_MISS_LIFQ;
    hal_ret_t rc;
    uint8_t   proto = 0;
    int       pkt_len = 0;

    while (1) {
        fd_set rd_set;
        FD_ZERO(&rd_set);
        maxfd = -1;
        for (uint32_t i = 0 ; i < handles.size(); i++) {
           FD_SET(handles[i].fd, &rd_set);
           if (handles[i].fd > maxfd) {
               maxfd = handles[i].fd;
           }
        }

        ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);
        if (ret < 0 && errno == EINTR){
            continue;
        }

        for (uint32_t i = 0 ; i < handles.size(); i++) {
            if (FD_ISSET(handles[i].fd, &rd_set)) {
                dev_handle_t hdl = handles[i];
                if ((nread = read(hdl.fd, inp, PKTBUF_LEN)) < 0) {
                    continue;
                }

                /*---------- FTE PROCESSING ---------*/
                /*
                 * Get the EP information to fill the cpu header
                 * and parse the packet to get the offsets
                 */
                hal::ep_t *sep = hal::find_ep_by_handle(hdl.ep);
                EXPECT_NE(sep, nullptr);
                hal::if_t *sif = hal::find_if_by_handle(sep->if_handle);
                EXPECT_NE(sif, nullptr);

                hal::l2seg_t *l2seg = hal::l2seg_lookup_by_handle(sep->l2seg_handle);
                EXPECT_NE(l2seg, nullptr);
                hal::pd::pd_l2seg_get_flow_lkupid_args_t args;
                hal::pd::pd_func_args_t          pd_func_args = {0};
                args.l2seg = l2seg;
                pd_func_args.pd_l2seg_get_flow_lkupid = &args;
                hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, &pd_func_args);
                hal::pd::l2seg_hw_id_t hwid = args.hwid;

                if (!parse_v4_packet((uint8_t *)inp, nread, &l3_offset, &l4_offset, &payload_offset, &proto)) {
                    fte::cpu_rxhdr_t cpu_rxhdr = {};
                    cpu_rxhdr.src_lif = sif->if_id;
                    cpu_rxhdr.lif = lifq.lif;
                    cpu_rxhdr.qtype = lifq.qtype;
                    cpu_rxhdr.qid = lifq.qid;
                    cpu_rxhdr.lkp_vrf = hwid;
                    cpu_rxhdr.lkp_dir = sif->if_type == intf::IF_TYPE_ENIC ? FLOW_DIR_FROM_DMA :
                                                     FLOW_DIR_FROM_UPLINK;
                    cpu_rxhdr.lkp_inst = 0;
                    cpu_rxhdr.lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
                    cpu_rxhdr.l2_offset = 0;
                    cpu_rxhdr.l3_offset = l3_offset;
                    cpu_rxhdr.l4_offset = l4_offset;
                    cpu_rxhdr.payload_offset = payload_offset;
                    tcp_header_t  *tcp = (tcp_header_t *)((uint8_t *)inp + l4_offset);
                    cpu_rxhdr.tcp_seq_num = tcp->seq;
                    cpu_rxhdr.tcp_flags = ((tcp->syn << 1) | (tcp->ack << 4));
                    rc = inject_pkt(&cpu_rxhdr, (uint8_t *)inp, nread, true);
                    EXPECT_EQ(rc, HAL_RET_OK);
                    EXPECT_FALSE(ctx_.drop());
#ifdef DEBUG
                    printf("Received packet:");
                    for (int i = 0; i< nread; i++) {
                        if (i % 16 == 0) {
                            printf("\n");
                        }
                        printf(" 0x%02x", (unsigned char)inp[i]);
                    }
                    printf("\n");
#endif

                    fte::flow_t *iflow = ctx_.flow(hal::FLOW_ROLE_INITIATOR);
                    if (proto == IPPROTO_TCP) {
                        pkt_len = net_sfw_build_tcp_pkt(ctx_, (uint8_t *)outp, PKTBUF_LEN, iflow->header_rewrite_info(),
                                                        iflow->header_push_info());
                    } else {
                        pkt_len = net_sfw_build_udp_pkt(ctx_, (uint8_t *)outp, PKTBUF_LEN, iflow->header_rewrite_info(),
                                                        iflow->header_push_info());
                    }
                }

                fix_checksum((uint8_t *)outp, pkt_len);
#ifdef DEBUG
                printf("Received packet:");
                for (int i = 0; i< pkt_len; i++) {
                    if (i % 16 == 0) {
                        printf("\n");
                    }
                    printf(" 0x%02x", (unsigned char)outp[i]);
                }
                printf("\n");
#endif
                nwrite = write(handles[hdl.other_hdl].fd, outp, pkt_len);
                if (nwrite < 0) {
                    continue;
                }
            }
        }
    }
}

void
fte_base_test::run_service(hal_handle_t ep_h, std::string service)
{
    int idx = std::distance(eps.begin(), eps.find(ep_h));
    std::string prefix_cmd = "ip netns exec EP" + to_string(idx+1);

    std::string cmd = prefix_cmd + " " + service;
    std::system(cmd.c_str());
}

std::string
fte_base_test::prefix_cmd(hal_handle_t ep_h)
{
    int idx = std::distance(eps.begin(), eps.find(ep_h));
    std::string prefix_cmd = "ip netns exec EP" + to_string(idx+1);

    return prefix_cmd;
}


void
fte_base_test::gen_rules(uint32_t num_rules, uint32_t num_tenants,
                      vector<fte_base_test::v4_rule_t *> &rules)
{
    uint32_t num_ips_32 = num_rules/10,
        num_ips_24 = num_rules/10,
        num_ips_16 = num_rules/100,
        num_port_ranges = 100,
        num_exact_ports = 100;

    // generate ips (ip, plen)
    vector<pair<uint32_t, uint8_t>> ips;
    for (uint32_t i = 0; i < num_ips_32; i++) {
        uint32_t ip =fte_base_test::myrandom(0xFFFFFFFF);
        ips.push_back({ip, 32});
    }
    for (uint32_t i = 0; i < num_ips_24; i++) {
        uint32_t ip = fte_base_test::myrandom(0xFFFFFF);
        ips.push_back({ip << 8, 24});
    }
    for (uint32_t i = 0; i < num_ips_16; i++) {
        uint32_t ip = fte_base_test::myrandom(0xFFFF);
        ips.push_back({ip << 16, 16});
    }
    random_shuffle(ips.begin(), ips.end(), myrandom);

    // generate ports
    vector<pair<uint16_t, uint16_t>> ports;
    for (uint32_t i = 0; i < num_port_ranges; i++) {
        uint16_t port = fte_base_test::myrandom(0xFFFF);
        ports.push_back({port, port});
    }

    for (uint32_t i = 0; i < num_exact_ports; i++) {
        uint16_t low = fte_base_test::myrandom(0xFF00);
        uint16_t high = low + fte_base_test::myrandom(0xFF);
        ports.push_back({low, high});
    }
    random_shuffle(ports.begin(), ports.end(), fte_base_test::myrandom);

    // generate rules
    for (uint32_t i = 0; i < num_rules; i++) {
        uint32_t ip;
        uint8_t plen;
        uint16_t port_low, port_high;

        fte_base_test::v4_rule_t *rule = v4_test_rule_alloc();

        tie(ip, plen) = ips[ fte_base_test::myrandom(ips.size())];
		rule->from.addr = ip;
		rule->from.plen = plen;

        tie(ip, plen) = ips[fte_base_test::myrandom(ips.size())];
		rule->to.addr = ip;
		rule->to.plen = plen;

		rule->app.proto = fte_base_test::myrandom(1) == 0 ? 6 : 17;

        tie(port_low, port_high) = ports[fte_base_test::myrandom(ports.size())];
		rule->app.dport_low = port_low;
		rule->app.dport_high = port_high;

        rules.push_back(rule);

        //PRINT_RULE(rule);
        //PRINT_KEY(key);
     }

    random_shuffle(rules.begin(), rules.end(), fte_base_test::myrandom);
}
