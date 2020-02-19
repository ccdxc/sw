//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <gtest/gtest.h>
#include "gen/proto/interface.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/endpoint.pb.h"
#include "gen/proto/gft.pb.h"
#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/cfg/gft/gft.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

using intf::InterfaceSpec;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using vrf::VrfSpec;
using vrf::VrfResponse;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using endpoint::EndpointSpec;
using endpoint::EndpointResponse;
using endpoint::EndpointUpdateRequest;
using gft::GftExactMatchProfileSpec;
using gft::GftExactMatchProfileResponse;
using gft::GftHeaderGroupExactMatchProfile;
using gft::GftHeaders;
using gft::GftHeaderFields;
using gft::GftExactMatchFlowEntrySpec;
using gft::GftExactMatchFlowEntryResponse;
using gft::GftHeaderGroupTransposition;
using gft::GftHeaderGroupExactMatch;

class scale_test : public hal_base_test {
protected:
  scale_test() {
  }

  virtual ~scale_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_base_test::SetUpTestCase("hal_hw.json");
    hal_test_utils_slab_disable_delete();
  }
};

static sdk_ret_t
create_port (uint32_t ifindex, port_args_t *port_args)
{
    void *port_info;

    HAL_TRACE_DEBUG("Creating port {}", port_args->port_num);
    /**
     * store user configured admin_state in another variable to be used
     * during xcvr insert/remove events
     */
    port_args->user_admin_state = port_args->admin_state;

    /**
     * store user configured AN in another variable to be used
     * during xcvr insert/remove events
     */
    port_args->auto_neg_cfg = port_args->auto_neg_enable;

    /**
     * store user configured num_lanes in another variable to be used
     * during xcvr insert/remove events
     */
    port_args->num_lanes_cfg = port_args->num_lanes;

    sdk::linkmgr::port_args_set_by_xcvr_state(port_args);
    port_info = sdk::linkmgr::port_create(port_args);
    if (port_info == NULL) {
        HAL_TRACE_ERR("port {} create failed", port_args->port_num);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static sdk_ret_t
populate_port_info (uint32_t ifindex, uint32_t phy_port,
                    port_args_t *port_args)
{
    uint32_t    logical_port;

    logical_port = port_args->port_num =
        sdk::lib::catalog::ifindex_to_logical_port(ifindex);
    port_args->port_type = hal::g_hal_state->catalog()->port_type_fp(phy_port);
    if (port_args->port_type == port_type_t::PORT_TYPE_ETH) {
        port_args->port_speed = port_speed_t::PORT_SPEED_100G;
        port_args->fec_type = port_fec_type_t::PORT_FEC_TYPE_RS;
    } else if (port_args->port_type == port_type_t::PORT_TYPE_MGMT) {
        port_args->port_speed = port_speed_t::PORT_SPEED_1G;
        port_args->fec_type = port_fec_type_t::PORT_FEC_TYPE_NONE;
    }
    port_args->admin_state = port_admin_state_t::PORT_ADMIN_STATE_UP;
    port_args->num_lanes = hal::g_hal_state->catalog()->num_lanes_fp(phy_port);
    port_args->mac_id = hal::g_hal_state->catalog()->mac_id(logical_port, 0);
    port_args->mac_ch = hal::g_hal_state->catalog()->mac_ch(logical_port, 0);
    if (port_args->port_type != port_type_t::PORT_TYPE_MGMT) {
        port_args->auto_neg_enable = true;
    }
    port_args->debounce_time = 0;
    port_args->mtu = 0;    /**< default will be set to max mtu */
    port_args->pause = port_pause_type_t::PORT_PAUSE_TYPE_NONE;
    port_args->loopback_mode = port_loopback_mode_t::PORT_LOOPBACK_MODE_NONE;

    for (uint32_t i = 0; i < port_args->num_lanes; i++) {
        port_args->sbus_addr[i] =
            hal::g_hal_state->catalog()->sbus_addr(logical_port, i);
    }
    port_args->breakout_modes =
        hal::g_hal_state->catalog()->breakout_modes(phy_port);

    return SDK_RET_OK;
}

static inline sdk_ret_t
create_ports (void)
{
    uint32_t num_phy_ports;
    port_args_t port_args;
    uint32_t ifindex;

    num_phy_ports = hal::g_hal_state->catalog()->num_fp_ports();
    for (uint32_t phy_port = 1; phy_port <= num_phy_ports; phy_port++) {
        ifindex = ETH_IFINDEX(hal::g_hal_state->catalog()->slot(),
                              phy_port, ETH_IF_DEFAULT_CHILD_PORT);
        memset(&port_args, 0, sizeof(port_args));
        populate_port_info(ifindex, phy_port, &port_args);
        create_port(ifindex, &port_args);
    }
    return SDK_RET_OK;
}

static inline hal_ret_t
create_uplinks (uint32_t num_uplinks)
{
    hal_ret_t            ret;
    InterfaceSpec        if_spec;
    InterfaceResponse    if_rsp;
    uint32_t             ifindex;

    for (uint32_t i = 0; i < num_uplinks; i++) {
        if_spec.set_type(intf::IF_TYPE_UPLINK);
        if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + i);
        ifindex = ETH_IFINDEX(hal::g_hal_state->catalog()->slot(),
                              i + 1, 1);
#if 0
        if (i == 0) {
            ifindex = 1;
        } else if (i == 1) {
            ifindex = 5;
        }
#endif
        if_spec.mutable_if_uplink_info()->set_port_num(ifindex);
        if_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(if_spec, &if_rsp);
        hal::hal_cfg_db_close();
        assert(ret == HAL_RET_OK);
    }
    return HAL_RET_OK;
}

#define VRF_ID_OFFSET 1
static inline hal_ret_t
create_vrfs (uint32_t num_vrfs)
{
    hal_ret_t   ret;
    VrfSpec     vrf_spec;
    VrfResponse vrf_rsp;

    for (uint32_t i = 0; i < num_vrfs; i++) {
        vrf_spec.Clear();
        vrf_rsp.Clear();
        vrf_spec.mutable_key_or_handle()->set_vrf_id(VRF_ID_OFFSET + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::vrf_create(vrf_spec, &vrf_rsp);
        hal::hal_cfg_db_close();
        assert(ret == HAL_RET_OK);
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
create_l2segs_per_vrf (uint32_t vrf_id, uint32_t num_l2segs, uint32_t encap)
{
    hal_ret_t           ret;
    L2SegmentSpec       l2seg_spec;
    L2SegmentResponse   l2seg_rsp;

    for (uint32_t i = 1; i <= num_l2segs; i++) {
        l2seg_spec.Clear();
        l2seg_rsp.Clear();
        l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
        l2seg_spec.mutable_key_or_handle()->set_segment_id((vrf_id - 1) * num_l2segs + i);
        l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        l2seg_spec.mutable_wire_encap()->set_encap_value(encap + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        assert(ret == HAL_RET_OK);
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
create_l2segs (uint32_t num_l2segs, uint32_t encap, uint32_t num_vrfs)
{
    for (uint32_t i = 0; i < num_vrfs; i++) {
        create_l2segs_per_vrf(VRF_ID_OFFSET + i, num_l2segs, encap);
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
create_eps (uint32_t num_vrfs, uint32_t num_l2segs_per_vrf,
            uint32_t num_eps_per_l2seg, uint32_t encap,
            uint32_t if_id, uint64_t base_mac)
{
    hal_ret_t           ret;
    EndpointSpec        ep_spec;
    EndpointResponse    ep_rsp;

    for (uint32_t i = VRF_ID_OFFSET; i <= num_vrfs; i++) {
        for (uint32_t j = 1; j <= num_l2segs_per_vrf; j++) {
            for (uint32_t k = 0; k < num_eps_per_l2seg; k++) {
                ep_spec.Clear();
                ep_rsp.Clear();
                ep_spec.mutable_vrf_key_handle()->set_vrf_id(i);
                ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->
                    mutable_l2segment_key_handle()->set_segment_id((i - 1) * num_l2segs_per_vrf + j);
                ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(if_id);
                ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(base_mac++);
                hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
                ret = hal::endpoint_create(ep_spec, &ep_rsp);
                hal::hal_cfg_db_close();
                assert(ret == HAL_RET_OK);
            }
        }
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
create_exact_match_profiles (void)
{
    hal_ret_t                           ret;
    GftExactMatchProfileSpec            emp_spec;
    GftExactMatchProfileResponse        mp_rsp;
    GftHeaderGroupExactMatchProfile     *emp;
    GftHeaders                          *mp_headers;
    GftHeaderFields                     *mp_hdr_fields;

    // exact match profile (emp) spec for ingress
    emp_spec.set_table_type(gft::GFT_TABLE_TYPE_EXACT_MATCH_INGRESS);
    emp_spec.mutable_key_or_handle()->set_profile_id(1);
    emp = emp_spec.add_exact_match_profiles();
    mp_headers = emp->mutable_headers();
    mp_headers->set_ethernet_header(true);
    mp_headers->set_ipv4_header(true);
    mp_headers->set_udp_header(true);
    mp_hdr_fields = emp->mutable_match_fields();
    mp_hdr_fields->set_dst_mac_addr(true);
    mp_hdr_fields->set_src_mac_addr(true);
    mp_hdr_fields->set_src_ip_addr(true);
    mp_hdr_fields->set_dst_ip_addr(true);
    mp_hdr_fields->set_src_port(true);
    mp_hdr_fields->set_dst_port(true);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_profile_create(emp_spec, &mp_rsp);
    hal::hal_cfg_db_close();
    assert(ret == HAL_RET_OK);

    // exact match profile (emp) spec for ingress
    emp_spec.Clear();
    emp_spec.set_table_type(gft::GFT_TABLE_TYPE_EXACT_MATCH_EGRESS);
    emp_spec.mutable_key_or_handle()->set_profile_id(2);
    emp = emp_spec.add_exact_match_profiles();
    mp_headers = emp->mutable_headers();
    mp_headers->set_ethernet_header(true);
    mp_headers->set_ipv4_header(true);
    mp_headers->set_udp_header(true);
    mp_hdr_fields = emp->mutable_match_fields();
    mp_hdr_fields->set_dst_mac_addr(true);
    mp_hdr_fields->set_src_mac_addr(true);
    mp_hdr_fields->set_src_ip_addr(true);
    mp_hdr_fields->set_dst_ip_addr(true);
    mp_hdr_fields->set_src_port(true);
    mp_hdr_fields->set_dst_port(true);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_profile_create(emp_spec, &mp_rsp);
    hal::hal_cfg_db_close();
    assert(ret == HAL_RET_OK);

    return HAL_RET_OK;
}

static inline hal_ret_t
create_exact_match_flow_entries (bool ingress, uint32_t num_entries)
{
    hal_ret_t                         ret;
    GftExactMatchFlowEntrySpec        efe_spec;
    GftExactMatchFlowEntryResponse    efe_rsp;
    GftHeaderGroupExactMatch          *em;
    GftHeaderGroupTransposition       *xposn;
    GftHeaders                        *headers, *xpos_hdrs;
    GftHeaderFields                   *hdr_fields, *xpos_fields;

    for (uint32_t i = 0; i < num_entries; i++) {
        efe_spec.set_table_type(ingress ? gft::GFT_TABLE_TYPE_EXACT_MATCH_INGRESS :
                                          gft::GFT_TABLE_TYPE_EXACT_MATCH_EGRESS);
        efe_spec.mutable_key_or_handle()->set_flow_entry_id(i);
        efe_spec.mutable_exact_match_profile()->set_profile_id(ingress ? 1 : 2);
        em = efe_spec.add_exact_matches();
        headers = em->mutable_headers();
        hdr_fields = em->mutable_match_fields();
        headers->set_ethernet_header(true);
        hdr_fields->set_dst_mac_addr(true);
        hdr_fields->set_src_mac_addr(true);
        headers->set_ipv4_header(true);
        hdr_fields->set_src_ip_addr(true);
        hdr_fields->set_dst_ip_addr(true);
        headers->set_udp_header(true);
        hdr_fields->set_src_port(true);
        hdr_fields->set_dst_port(true);
        em->mutable_eth_fields()->set_dst_mac_addr(0xF1D0D1D0);
        em->mutable_eth_fields()->set_src_mac_addr(0xDEADBEEF);
        em->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET);
        em->mutable_src_ip_addr()->set_v4_addr(0x0a000001 + i);
        em->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET);
        em->mutable_dst_ip_addr()->set_v4_addr(0x0b000001 + i);
        em->mutable_encap_or_transport()->mutable_udp_fields()->set_sport(80 + (i%65535));
        em->mutable_encap_or_transport()->mutable_udp_fields()->set_dport(8080 + (i%65535));

        xposn = efe_spec.add_transpositions();
        xposn->set_action(gft::TRANSPOSITION_ACTION_MODIFY);
        xpos_hdrs = xposn->mutable_headers();
        xpos_fields = xposn->mutable_header_fields();
        xpos_hdrs->set_ethernet_header(true);
        xpos_fields->set_dst_mac_addr(true);
        xpos_fields->set_src_mac_addr(true);
        xpos_hdrs->set_ipv4_header(true);
        xpos_fields->set_src_ip_addr(true);
        xpos_fields->set_dst_ip_addr(true);
        xpos_hdrs->set_udp_header(true);
        xpos_fields->set_src_port(true);
        xpos_fields->set_dst_port(true);
        xposn->mutable_eth_fields()->set_dst_mac_addr(0xDEADBADE);
        xposn->mutable_eth_fields()->set_src_mac_addr(0xFEEDBABE);
        xposn->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET);
        xposn->mutable_src_ip_addr()->set_v4_addr(0x0c000001 + i);
        xposn->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET);
        xposn->mutable_dst_ip_addr()->set_v4_addr(0x0d000001 + i);
        xposn->mutable_encap_or_transport()->mutable_udp_fields()->set_sport(90 + (i%65535));
        xposn->mutable_encap_or_transport()->mutable_udp_fields()->set_dport(9090 + (i%65535));

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::gft_exact_match_flow_entry_create(efe_spec, &efe_rsp);
        hal::hal_cfg_db_close();
        assert(ret == HAL_RET_OK);

        HAL_TRACE_DEBUG("flow_index: {}, flow_info_index:{}",
                        efe_rsp.mutable_status()->flow_index(),
                        efe_rsp.mutable_status()->flow_info_index());
        efe_spec.Clear();
    }
}

typedef struct headers_valid_s {
    bool eth_v;
    bool ip4_v;
    bool ip6_v;
    bool tcp_v;
    bool udp_v;
    bool vxlan_v;
} headers_valid_t;

typedef struct header_fields_valid_s {
    bool smac_v;
    bool dmac_v;
    bool sip_v;
    bool dip_v;
    bool sport_v;
    bool dport_v;
} header_fields_valid_t;

static inline hal_ret_t
create_emp (bool is_ingress,
            headers_valid_t *hdr_v[3],
            header_fields_valid_t *hdr_fval_v[3],
            uint32_t num_layers,
            uint32_t prof_id)
{
    hal_ret_t                           ret;
    GftExactMatchProfileSpec            emp_spec;
    GftExactMatchProfileResponse        mp_rsp;
    GftHeaderGroupExactMatchProfile     *emp;
    GftHeaders                          *mp_headers;
    GftHeaderFields                     *mp_hdr_fields;
    headers_valid_t                     *hv;
    header_fields_valid_t               *hfv;

    emp_spec.set_table_type(is_ingress ?
                            gft::GFT_TABLE_TYPE_EXACT_MATCH_INGRESS :
                            gft::GFT_TABLE_TYPE_EXACT_MATCH_EGRESS);
    emp_spec.mutable_key_or_handle()->set_profile_id(prof_id);
    for (uint32_t i = 0; i < num_layers; i++) {
        hv = hdr_v[i];
        hfv = hdr_fval_v[i];

        emp = emp_spec.add_exact_match_profiles();

        mp_headers = emp->mutable_headers();
        mp_headers->set_ethernet_header(hv->eth_v);
        mp_headers->set_ipv4_header(hv->ip4_v);
        mp_headers->set_ipv6_header(hv->ip6_v);
        mp_headers->set_udp_header(hv->udp_v);
        mp_headers->set_tcp_header(hv->tcp_v);
        mp_headers->set_vxlan_encap(hv->vxlan_v);

        mp_hdr_fields = emp->mutable_match_fields();
        mp_hdr_fields->set_dst_mac_addr(hfv->dmac_v);
        mp_hdr_fields->set_src_mac_addr(hfv->smac_v);
        mp_hdr_fields->set_src_ip_addr(hfv->sip_v);
        mp_hdr_fields->set_dst_ip_addr(hfv->dip_v);
        mp_hdr_fields->set_src_port(hfv->sport_v);
        mp_hdr_fields->set_dst_port(hfv->dport_v);
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_profile_create(emp_spec, &mp_rsp);
    hal::hal_cfg_db_close();
    assert(ret == HAL_RET_OK);
    return HAL_RET_OK;
}

typedef struct header_fields_s {
    uint32_t sip_v4;
    uint32_t dip_v4;
    uint8_t sip_v6[16];
    uint8_t dip_v6[16];
    bool is_tcp;
    uint32_t sport;
    uint32_t dport;
} header_fields_t;

static inline std::string
form_ipv6_str (uint8_t ip_v6[16])
{
    return std::string(reinterpret_cast<const char*>(ip_v6), 16);
#if 0
    std::string ipv6_str = "";
    for (uint32_t i = 0; i < 16; i++) {
        ipv6_str += std::to_string(ip_v6[i]);
    }
    return ipv6_str;
#endif
}

static inline hal_ret_t
create_flow (bool is_ingress,
             headers_valid_t *hdr_v[3],
             header_fields_valid_t *hdr_fval_v[3],
             uint32_t num_layers,
             uint32_t prof_id,
             // Flow
             uint32_t flow_idx,
             // Xpositions
             headers_valid_t *xhdr_v[3],
             header_fields_valid_t *xhdr_fval_v[3],
             header_fields_t *hdr_flds[3],
             uint32_t num_xpos_layers)
{
    hal_ret_t                           ret;
    GftExactMatchFlowEntrySpec          efe_spec;
    GftExactMatchFlowEntryResponse      efe_rsp;
    GftHeaders                          *mp_headers, *xpos_hdrs;
    GftHeaderFields                     *mp_hdr_fields, *xpos_fields;
    GftHeaderGroupExactMatch            *em;
    GftHeaderGroupTransposition         *xposn;
    headers_valid_t                     *hv;
    header_fields_valid_t               *hfv;
    header_fields_t                     *hdr_fld;

    efe_spec.set_table_type(is_ingress ?
                            gft::GFT_TABLE_TYPE_EXACT_MATCH_INGRESS :
                            gft::GFT_TABLE_TYPE_EXACT_MATCH_EGRESS);
    efe_spec.mutable_key_or_handle()->set_flow_entry_id(flow_idx);
    efe_spec.mutable_exact_match_profile()->set_profile_id(prof_id);

    printf("Flow: Idx: %u\n", flow_idx);
    printf("---------------\n");

    for (uint32_t i = 0; i < num_layers; i++) {
        hv = hdr_v[i];
        hfv = hdr_fval_v[i];
        hdr_fld = hdr_flds[i];

        em = efe_spec.add_exact_matches();

        mp_headers = em->mutable_headers();
        mp_headers->set_ethernet_header(hv->eth_v);
        mp_headers->set_ipv4_header(hv->ip4_v);
        mp_headers->set_ipv6_header(hv->ip6_v);
        mp_headers->set_udp_header(hv->udp_v);
        mp_headers->set_tcp_header(hv->tcp_v);
        mp_headers->set_vxlan_encap(hv->vxlan_v);

        mp_hdr_fields = em->mutable_match_fields();
        mp_hdr_fields->set_dst_mac_addr(hfv->dmac_v);
        mp_hdr_fields->set_src_mac_addr(hfv->smac_v);
        mp_hdr_fields->set_src_ip_addr(hfv->sip_v);
        mp_hdr_fields->set_dst_ip_addr(hfv->dip_v);
        mp_hdr_fields->set_src_port(hfv->sport_v);
        mp_hdr_fields->set_dst_port(hfv->dport_v);

        printf("\tLayer : %d ", i);

        if (hv->ip4_v) {
            printf("IPv4 : ");
            if (hfv->sip_v) {
                em->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET);
                em->mutable_src_ip_addr()->set_v4_addr(hdr_fld->sip_v4);
                printf(" SIP: %s ", ipv4addr2str(hdr_fld->sip_v4));
            }
            if (hfv->dip_v) {
                em->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET);
                em->mutable_dst_ip_addr()->set_v4_addr(hdr_fld->dip_v4);
                printf(" DIP: %s ", ipv4addr2str(hdr_fld->dip_v4));
            }
        }
        if (hv->ip6_v) {
            printf("IPv6 : ");
            if (hfv->sip_v) {
                em->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET6);
                em->mutable_src_ip_addr()->set_v6_addr(form_ipv6_str(hdr_fld->sip_v6));
                printf(" SIP: %s ", form_ipv6_str(hdr_fld->sip_v6).c_str());
            }
            if (hfv->dip_v) {
                em->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET6);
                em->mutable_dst_ip_addr()->set_v6_addr(form_ipv6_str(hdr_fld->dip_v6));
                printf(" DIP: %s ", form_ipv6_str(hdr_fld->dip_v6).c_str());
            }
        }
        if (hv->tcp_v) {
            if (hfv->sport_v) {
                printf("TCP : ");
                em->mutable_encap_or_transport()->mutable_tcp_fields()->set_sport(hdr_fld->sport);
                printf("sport: %d ", hdr_fld->sport);
            }
            if (hfv->dport_v) {
                em->mutable_encap_or_transport()->mutable_tcp_fields()->set_dport(hdr_fld->dport);
                printf("dport: %d ", hdr_fld->dport);
            }
        } else {
            if (hfv->sport_v) {
                printf("UDP : ");
                em->mutable_encap_or_transport()->mutable_udp_fields()->set_sport(hdr_fld->sport);
                printf("sport: %d ", hdr_fld->sport);
            }
            if (hfv->dport_v) {
                em->mutable_encap_or_transport()->mutable_udp_fields()->set_dport(hdr_fld->dport);
                printf("dport: %d ", hdr_fld->dport);
            }
        }
        printf("\n");
    }

    printf("Transpositions\n");
    printf("---------------\n");
    for (uint32_t i = 0; i < num_xpos_layers; i++) {
        hv = xhdr_v[i];
        hfv = xhdr_fval_v[i];
        hdr_fld = hdr_flds[i];

        xposn = efe_spec.add_transpositions();

        xposn->set_action(gft::TRANSPOSITION_ACTION_MODIFY);

        xpos_hdrs = xposn->mutable_headers();
        xpos_hdrs->set_ethernet_header(hv->eth_v);
        xpos_hdrs->set_ipv4_header(hv->ip4_v);
        xpos_hdrs->set_ipv6_header(hv->ip6_v);
        xpos_hdrs->set_udp_header(hv->udp_v);
        xpos_hdrs->set_tcp_header(hv->tcp_v);

        xpos_fields = xposn->mutable_header_fields();
        xpos_fields->set_dst_mac_addr(hfv->dmac_v);
        xpos_fields->set_src_mac_addr(hfv->smac_v);
        xpos_fields->set_src_ip_addr(hfv->sip_v);
        xpos_fields->set_dst_ip_addr(hfv->dip_v);
        xpos_fields->set_src_port(hfv->sport_v);
        xpos_fields->set_dst_port(hfv->dport_v);

        printf("\tLayer : %d ", i);

        if (hv->ip4_v) {
            printf("IPv4 : ");
            if (hfv->sip_v) {
                xposn->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET);
                xposn->mutable_src_ip_addr()->set_v4_addr(hdr_fld->sip_v4 + (63 << 24));
                printf(" SIP: %s ", ipv4addr2str(hdr_fld->sip_v4 + (63 << 24)));
            }
            if (hfv->dip_v) {
                xposn->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET);
                xposn->mutable_dst_ip_addr()->set_v4_addr(hdr_fld->dip_v4 + (63 << 24));
                printf(" DIP: %s ", ipv4addr2str(hdr_fld->dip_v4 + (63 << 24)));
            }
        }
        if (hv->ip6_v) {
            printf("IPv6 : ");
            if (hfv->sip_v) {
                xposn->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET6);
                hdr_fld->sip_v6[12] += 1;
                xposn->mutable_src_ip_addr()->set_v6_addr(form_ipv6_str(hdr_fld->sip_v6));
                printf(" SIP: %s ", form_ipv6_str(hdr_fld->sip_v6).c_str());
            }
            if (hfv->dip_v) {
                xposn->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET6);
                hdr_fld->dip_v6[12] += 1;
                xposn->mutable_dst_ip_addr()->set_v6_addr(form_ipv6_str(hdr_fld->dip_v6));
                printf(" DIP: %s ", form_ipv6_str(hdr_fld->dip_v6).c_str());
            }
        }
        printf("\n");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_flow_entry_create(efe_spec, &efe_rsp);
    hal::hal_cfg_db_close();
    assert(ret == HAL_RET_OK);

    HAL_TRACE_DEBUG("flow_index: {}, flow_info_index:{}",
                    efe_rsp.mutable_status()->flow_index(),
                    efe_rsp.mutable_status()->flow_info_index());
    return HAL_RET_OK;
}

#if 0
#define NUM_OUTER_IPS (1 << 2)
#define NUM_MIDDLE_IPS (1 << 2)
#define NUM_INNER_IPS (1 << 2)
#define NUM_INNER_PORTS (1 << 4)

#define NUM_OUTER_IPS (1 << 0)
#define NUM_MIDDLE_IPS (1 << 0)
#define NUM_INNER_IPS (1 << 0)
#define NUM_INNER_PORTS (1 << 0)
#endif

#define NUM_OUTER_IPS (1 << 1)
#define NUM_MIDDLE_IPS (1 << 1)
#define NUM_INNER_IPS (1 << 1)
#define NUM_INNER_PORTS (1 << 3)

#define OUTER_SIP_BASE 0x1000001    // 1.0.0.1
#define OUTER_DIP_BASE 0x1010001    // 1.1.0.1
#define MIDDLE_SIP_BASE 0x2000001   // 2.0.0.1
#define MIDDLE_DIP_BASE 0x2010001   // 2.1.0.1
#define INNER_SIP_BASE 0x3000001    // 3.0.0.1
#define INNER_DIP_BASE 0x3010001    // 3.1.0.1
#define INNER_SPORT_BASE 1
#define INNER_DPORT_BASE 1

static void
create_flows (bool v4_flows)
{
    headers_valid_t hdr_valid1 = { 0 };
    headers_valid_t hdr_valid2 = { 0 };
    headers_valid_t hdr_valid3 = { 0 };
    header_fields_valid_t hdr_fields_valid1 = { 0 };
    header_fields_valid_t hdr_fields_valid2 = { 0 };
    header_fields_valid_t hdr_fields_valid3 = { 0 };
    headers_valid_t xhdr_valid1 = { 0 };
    headers_valid_t xhdr_valid2 = { 0 };
    headers_valid_t xhdr_valid3 = { 0 };
    headers_valid_t *xhdr_valid[3] = {
        &xhdr_valid1,
        &xhdr_valid2,
        &xhdr_valid3
    };
    headers_valid_t *hdrs_valid[3];
    header_fields_valid_t *hdr_fields_valid[3];
    header_fields_valid_t xhdr_fval_valid1 = { 0 };
    header_fields_valid_t xhdr_fval_valid2 = { 0 };
    header_fields_valid_t xhdr_fval_valid3 = { 0 };
    header_fields_valid_t *xhdr_fval_valid[3]  = {
        &xhdr_fval_valid1,
        &xhdr_fval_valid2,
        &xhdr_fval_valid3
    };
    header_fields_t hdr_flds1 = { 0 };
    header_fields_t hdr_flds2 = { 0 };
    header_fields_t hdr_flds3 = { 0 };
    header_fields_t *hdr_flds[3] = {
        &hdr_flds1,
        &hdr_flds2,
        &hdr_flds3
    };
    static uint32_t prof_id = 1;

    printf("Creating Exact Match Profiles ...\n");
    // Layer 1
    hdr_valid1.eth_v = true;
    hdr_valid1.ip4_v = v4_flows;
    hdr_valid1.ip6_v = !v4_flows;
    hdr_valid1.udp_v = true;
    hdr_valid1.vxlan_v = true;
    // Layer 2
    hdr_valid2.eth_v = true;
    hdr_valid2.ip4_v = v4_flows;
    hdr_valid2.ip6_v = !v4_flows;
    hdr_valid2.udp_v = true;
    hdr_valid2.vxlan_v = true;
    // Layer 3
    hdr_valid3.eth_v = true;
    hdr_valid3.ip4_v = v4_flows;
    hdr_valid3.ip6_v = !v4_flows;
    hdr_valid3.tcp_v = true;
    hdrs_valid[0] = &hdr_valid1;
    hdrs_valid[1] = &hdr_valid2;
    hdrs_valid[2] = &hdr_valid3;
    hdr_fields_valid1.sip_v = true;
    hdr_fields_valid1.dip_v = true;
    hdr_fields_valid2.sip_v = true;
    hdr_fields_valid2.dip_v = true;
    hdr_fields_valid3.sip_v = true;
    hdr_fields_valid3.dip_v = true;
    hdr_fields_valid3.sport_v = true;
    hdr_fields_valid3.dport_v = true;
    hdr_fields_valid[0] = &hdr_fields_valid1;
    hdr_fields_valid[1] = &hdr_fields_valid2;
    hdr_fields_valid[2] = &hdr_fields_valid3;
    create_emp(true, hdrs_valid, hdr_fields_valid, 3, prof_id);

    xhdr_valid1.ip4_v = v4_flows;
    xhdr_valid1.ip6_v = !v4_flows;
    xhdr_valid2.ip4_v = v4_flows;
    xhdr_valid2.ip6_v = !v4_flows;
    xhdr_valid3.ip4_v = v4_flows;
    xhdr_valid3.ip6_v = !v4_flows;

    xhdr_fval_valid1.sip_v = true;
    xhdr_fval_valid1.dip_v = true;
    xhdr_fval_valid2.sip_v = true;
    xhdr_fval_valid2.dip_v = true;
    xhdr_fval_valid3.sip_v = true;
    xhdr_fval_valid3.dip_v = true;

    if (v4_flows) {
        uint32_t outer_sip = OUTER_SIP_BASE, outer_dip = OUTER_DIP_BASE;
        uint32_t middle_sip = MIDDLE_SIP_BASE, middle_dip = MIDDLE_DIP_BASE;
        uint32_t inner_sip = INNER_SIP_BASE, inner_dip = INNER_DIP_BASE;
        uint16_t inner_sport = INNER_SPORT_BASE, inner_dport = INNER_DPORT_BASE;
        uint32_t count = 0;
        printf("Creating IPv4 flow entries ...\n");
        for (uint32_t i = 0; i < NUM_OUTER_IPS; i++) { // Outer IPs
            outer_sip = OUTER_SIP_BASE + i;
            for (uint32_t j = 0; j < NUM_OUTER_IPS; j++) { // Outer IPs
                outer_dip = OUTER_DIP_BASE + j;
                for (uint32_t k = 0; k < NUM_MIDDLE_IPS; k++) { // Middle IPs
                    middle_sip = MIDDLE_SIP_BASE + k;
                    for (uint32_t l = 0; l < NUM_MIDDLE_IPS; l++) { // Middle IPs
                        middle_dip = MIDDLE_DIP_BASE + l;
                        for (uint32_t m = 0; m < NUM_INNER_IPS; m++) { // Inner most IPs
                            inner_sip = INNER_SIP_BASE + m;
                            for (uint32_t n = 0; n < NUM_INNER_IPS; n++) { // Inner most IPs
                                inner_dip = INNER_DIP_BASE + n;
                                for (uint32_t o = 0; o < NUM_INNER_PORTS; o++) { // Inner most ports
                                    inner_sport = INNER_SPORT_BASE + o;
                                    for (uint32_t p = 0; p < NUM_INNER_PORTS; p++) { // Inner most ports
                                        inner_dport = INNER_DPORT_BASE + p;
                                        hdr_flds1.sip_v4 = outer_sip;
                                        hdr_flds1.dip_v4 = outer_dip;
                                        hdr_flds2.sip_v4 = middle_sip;
                                        hdr_flds2.dip_v4 = middle_dip;
                                        hdr_flds3.sip_v4 = inner_sip;
                                        hdr_flds3.dip_v4 = inner_dip;
                                        hdr_flds3.sport = inner_sport;
                                        hdr_flds3.dport = inner_dport;
                                        create_flow(true, hdrs_valid, hdr_fields_valid, 3, prof_id,
                                                    count++ /* flow_idx */,
                                                    xhdr_valid, xhdr_fval_valid, hdr_flds,
                                                    3 /* xpos layers */);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        printf("Creating IPv6 flow entries ...\n");
        uint8_t outer_sip_base[16] = { 0 }, outer_dip_base[16] = { 0 };
        uint8_t middle_sip_base[16] = { 0 }, middle_dip_base[16] = { 0 };
        uint8_t inner_sip_base[16] = { 0 }, inner_dip_base[16] = { 0 };
        uint8_t outer_sip[16] = { 0 }, outer_dip[16] = { 0 };
        uint8_t middle_sip[16] = { 0 }, middle_dip[16] = { 0 }, inner_sip[16] = { 0 };
        uint8_t inner_dip[16] = { 0 };
        uint16_t inner_sport_base = INNER_SPORT_BASE, inner_dport_base = INNER_DPORT_BASE;
        uint16_t inner_sport, inner_dport;
        uint32_t count = 0;

        // outer_sip_base 2019::0100:0001
        outer_sip_base[0] = 0x20;
        outer_sip_base[1] = 0x19;
        outer_sip_base[12] = 0x1;
        outer_sip_base[15] = 0x1;

        // outer_dip_base 2019::0101:0001
        outer_dip_base[0] = 0x20;
        outer_dip_base[1] = 0x19;
        outer_dip_base[12] = 0x1;
        outer_dip_base[13] = 0x1;
        outer_dip_base[15] = 0x1;

        // middle_sip_base 2020::1000:0001
        middle_sip_base[0] = 0x20;
        middle_sip_base[1] = 0x20;
        middle_sip_base[12] = 0x10;
        middle_sip_base[15] = 0x1;

        // middle_dip_base 2020::1001:0001
        middle_dip_base[0] = 0x20;
        middle_dip_base[1] = 0x20;
        middle_dip_base[12] = 0x10;
        middle_dip_base[13] = 0x1;
        middle_dip_base[15] = 0x1;

        // inner_sip_base 2021::2000:0001
        inner_sip_base[0] = 0x20;
        inner_sip_base[1] = 0x21;
        inner_sip_base[12] = 0x20;
        inner_sip_base[15] = 0x1;

        // inner_dip_base 2021::2001:0001
        inner_dip_base[0] = 0x20;
        inner_dip_base[1] = 0x21;
        inner_dip_base[12] = 0x20;
        inner_dip_base[13] = 0x1;
        inner_dip_base[15] = 0x1;

        printf("Creating flow entries ...\n");
        memcpy(outer_sip, outer_sip_base, sizeof(outer_sip));
        for (uint32_t i = 0; i < NUM_OUTER_IPS; i++,outer_sip[15]++) { // Outer IPs
            memcpy(outer_dip, outer_dip_base, sizeof(outer_dip));
            for (uint32_t j = 0; j < NUM_OUTER_IPS; j++,outer_dip[15]++) { // Outer IPs
                memcpy(middle_sip, middle_sip_base, sizeof(middle_sip));
                for (uint32_t k = 0; k < NUM_MIDDLE_IPS; k++,middle_sip[15]++) { // Middle IPs
                    memcpy(middle_dip, middle_dip_base, sizeof(middle_dip));
                    for (uint32_t l = 0; l < NUM_MIDDLE_IPS; l++,middle_dip[15]++) { // Middle IPs
                        memcpy(inner_sip, inner_sip_base, sizeof(inner_sip));
                        for (uint32_t m = 0; m < NUM_INNER_IPS; m++,inner_sip[15]++) { // Inner most IPs
                            memcpy(inner_dip, inner_dip_base, sizeof(inner_dip));
                            for (uint32_t n = 0; n < NUM_INNER_IPS; n++,inner_dip[15]++) { // Inner most IPs
                                inner_sport = inner_sport_base;
                                for (uint32_t o = 0; o < NUM_INNER_PORTS; o++,inner_sport++) { // Inner most ports
                                    inner_dport = inner_dport_base;
                                    for (uint32_t p = 0; p < NUM_INNER_PORTS; p++, inner_dport++) { // Inner most ports
                                        memcpy(hdr_flds1.sip_v6, outer_sip, sizeof(outer_sip));
                                        memcpy(hdr_flds1.dip_v6, outer_dip, sizeof(outer_dip));
                                        memcpy(hdr_flds2.sip_v6, middle_sip, sizeof(middle_sip));
                                        memcpy(hdr_flds2.dip_v6, middle_dip, sizeof(middle_dip));
                                        memcpy(hdr_flds3.sip_v6, inner_sip, sizeof(inner_sip));
                                        memcpy(hdr_flds3.dip_v6, inner_dip, sizeof(inner_dip));
                                        hdr_flds3.sport = inner_sport;
                                        hdr_flds3.dport = inner_dport;
                                        create_flow(true, hdrs_valid, hdr_fields_valid, 3, prof_id,
                                                    count++ /* flow_idx */,
                                                    xhdr_valid, xhdr_fval_valid, hdr_flds,
                                                    3 /* xpos layers */);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    prof_id++;
}

TEST_F(scale_test, test1)
{
    printf("Creating ports ...\n");
    create_ports();

    printf("Creating uplinks ...\n");
    create_uplinks(2);

    printf("Creating VRFs ...\n");
    create_vrfs(1);

    printf("Creating L2 segments ...\n");
    create_l2segs(4, 0, 1);

    printf("Creating Endpoints ...\n");
    create_eps(1, 4, 16, 0, UPLINK_IF_ID_OFFSET, 0xFEED000);
    create_eps(1, 4, 16, 0, UPLINK_IF_ID_OFFSET + 1, 0xBABE000);

    printf("Creating flows ...\n");
    //create_flows(true);
    create_flows(false);
    printf("Configuration done ...\n");
    fflush(stdout);
    while (1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
