// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.hpp"
#include "gen/p4gen/gft/include/p4pd.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/gft/gft.hpp"
#include "nic/hal/pd/gft/gft_pd.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/p4/gft/include/defines.h"
#include "lib/table/tcam/tcam.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/common_p4plus/common_p4plus.hpp"

using sdk::table::tcam;

namespace hal {
namespace pd {

//------------------------------------------------------------------------------
// link GFT exact match profile's PI and PD states
//------------------------------------------------------------------------------
static inline void
gft_exact_match_profile_link_pi_pd (pd_gft_exact_match_profile_t *pd,
                                    gft_exact_match_profile_t *pi)
{
    pd->gft_exact_match_profile = pi;
    pi->pd = pd;
}

// TODO: Moved to emp_pd.cc. Cleanup
#if 0
//------------------------------------------------------------------------------
// create necessary state for the GFT exact match profile
//------------------------------------------------------------------------------
hal_ret_t
pd_gft_exact_match_profile_create (pd_gft_exact_match_profile_args_t *gft_args)
{
    hal_ret_t                              ret = HAL_RET_OK;
    sdk_ret_t                              sdk_ret;
    gft_hdr_group_exact_match_profile_t    *gft_emp;
    pd_gft_exact_match_profile_t           *gft_emp_pd;
    uint32_t                               num_profiles, tcam_idx = 0;
    rx_key1_swkey_t                        rx_key1 = { 0 };
    rx_key1_swkey_mask_t                   rx_key1_mask = { 0 };
    rx_key1_actiondata_t                     rx_key1_action = { 0 };
    rx_key2_swkey_t                        rx_key2 = { 0 };
    rx_key2_swkey_mask_t                   rx_key2_mask = { 0 };
    rx_key2_actiondata_t                     rx_key2_action = { 0 };
    rx_key3_swkey_t                        rx_key3 = { 0 };
    rx_key3_swkey_mask_t                   rx_key3_mask = { 0 };
    rx_key3_actiondata_t                     rx_key3_action = { 0 };
    rx_key4_swkey_t                        rx_key4 = { 0 };
    rx_key4_swkey_mask_t                   rx_key4_mask = { 0 };
    rx_key4_actiondata_t                     rx_key4_action = { 0 };
    tcam                                   *key_tbl;

    if (!gft_args || !gft_args->exact_match_profile) {
        return HAL_RET_INVALID_ARG;
    }

    if (gft_args->exact_match_profile->profile_id ==
            HAL_GFT_INVALID_PROFILE_ID) {
        return HAL_RET_INVALID_ARG;
    }

    num_profiles =
        gft_args->exact_match_profile->num_hdr_group_exact_match_profiles;
    if (!num_profiles || (num_profiles > 3)) {
        return HAL_RET_INVALID_ARG;
    }

    // create PD state for this exact match profile
    gft_emp_pd = gft_exact_match_profile_pd_alloc_init();
    if (gft_emp_pd == NULL) {
        return HAL_RET_OOM;
    }

    // link PI and PD states
    gft_exact_match_profile_link_pi_pd(gft_emp_pd,
                                       gft_args->exact_match_profile);

#if 0
    ret = gft_exact_match_profile_pd_add_to_db();
    if (ret != HAL_RET_OK) {
        gft_exact_match_profile_pd_cleanup(gft_emp_pd);
    }
#endif

    // set all the care bits in the key mask(s)
    memset(&rx_key1_mask, 0xFF, sizeof(rx_key1_mask));
    memset(&rx_key2_mask, 0xFF, sizeof(rx_key2_mask));
    memset(&rx_key3_mask, 0xFF, sizeof(rx_key3_mask));
    memset(&rx_key4_mask, 0xFF, sizeof(rx_key4_mask));

    gft_emp = &gft_args->exact_match_profile->hgem_profiles[0];
    if (gft_emp->headers & GFT_HEADER_ETHERNET) {
        rx_key1.ethernet_1_valid = 1;
        if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_ETHERNET_DST;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_ETHERNET_SRC;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_ETH_TYPE) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_ETHERNET_TYPE;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_CUSTOMER_VLAN_ID;
        }
    }
    if (gft_emp->headers & GFT_HEADER_IPV4) {
        rx_key1.ipv4_1_valid = 1;
        if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_SRC;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_DST;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_DSCP;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_PROTO;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_TTL) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_TTL;
        }
    } else if (gft_emp->headers & GFT_HEADER_IPV6) {
        rx_key1.ipv6_1_valid = 1;
        if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_SRC;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_DST;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_DSCP;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_PROTO;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_TTL) {
            rx_key1_action.rx_key1_action_u.rx_key1_rx_key1.match_fields |=
                MATCH_IP_TTL;
        }
    }
    if (gft_emp->headers & GFT_HEADER_ICMP) {
        rx_key1.icmp_1_valid = 1;
        if (gft_emp->match_fields & GFT_HEADER_FIELD_ICMP_TYPE) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_ICMP_TYPE_1;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_ICMP_CODE) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_ICMP_CODE_1;
        }
    } else if (gft_emp->headers & GFT_HEADER_TCP) {
        rx_key1.tcp_1_valid = 1;
        if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_TRANSPORT_SRC_PORT_1;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_TRANSPORT_DST_PORT_1;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_TCP_FLAGS) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_TCP_FLAGS_1;
        }
    } else if (gft_emp->headers & GFT_HEADER_UDP) {
        rx_key1.udp_1_valid = 1;
        if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_TRANSPORT_SRC_PORT_1;
        }
        if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_TRANSPORT_DST_PORT_1;
        }
    }
    if (gft_emp->headers & GFT_HEADER_IP_IN_IP_ENCAP) {
        rx_key1.tunnel_metadata_tunnel_type_1 = INGRESS_TUNNEL_TYPE_IP_IN_IP;
    }
    if (gft_emp->headers & GFT_HEADER_IP_IN_GRE_ENCAP) {
        rx_key1.tunnel_metadata_tunnel_type_1 = INGRESS_TUNNEL_TYPE_GRE;
        if (gft_emp->match_fields & GFT_HEADER_FIELD_GRE_PROTOCOL) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_GRE_PROTO_1;
        }
    }
    if (gft_emp->headers & GFT_HEADER_NVGRE_ENCAP) {
        rx_key1.tunnel_metadata_tunnel_type_1 = INGRESS_TUNNEL_TYPE_NVGRE;
    }
    if (gft_emp->headers & GFT_HEADER_VXLAN_ENCAP) {
        rx_key1.tunnel_metadata_tunnel_type_1 = INGRESS_TUNNEL_TYPE_VXLAN;
        if (gft_emp->match_fields & GFT_HEADER_FIELD_TENANT_ID) {
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                MATCH_TENANT_ID_1;
        }
    }

    if (num_profiles > 1) {
        gft_emp = &gft_args->exact_match_profile->hgem_profiles[1];
        if (gft_emp->headers & GFT_HEADER_ETHERNET) {
            rx_key1.ethernet_2_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_ETHERNET_DST;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_ETHERNET_SRC;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_ETH_TYPE) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_ETHERNET_TYPE;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_CUSTOMER_VLAN_ID;
            }
        }
        if (gft_emp->headers & GFT_HEADER_IPV4) {
            rx_key1.ipv4_2_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_SRC;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_DST;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_DSCP;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_PROTO;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TTL) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_TTL;
            }
        } else if (gft_emp->headers & GFT_HEADER_IPV6) {
            rx_key1.ipv6_2_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_SRC;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_DST;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_DSCP;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_PROTO;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TTL) {
                rx_key2_action.rx_key2_action_u.rx_key2_rx_key2.match_fields |=
                    MATCH_IP_TTL;
            }
        }
        if (gft_emp->headers & GFT_HEADER_ICMP) {
            rx_key1.icmp_2_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_ICMP_TYPE) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_ICMP_TYPE_2;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_ICMP_CODE) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_ICMP_CODE_2;
            }
        } else if (gft_emp->headers & GFT_HEADER_TCP) {
            rx_key1.tcp_2_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TRANSPORT_SRC_PORT_2;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TRANSPORT_DST_PORT_2;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TCP_FLAGS) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TCP_FLAGS_2;
            }
        } else if (gft_emp->headers & GFT_HEADER_UDP) {
            rx_key1.udp_2_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TRANSPORT_SRC_PORT_2;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TRANSPORT_DST_PORT_2;
            }
        }
        if (gft_emp->headers & GFT_HEADER_IP_IN_IP_ENCAP) {
            rx_key1.tunnel_metadata_tunnel_type_2 =
                INGRESS_TUNNEL_TYPE_IP_IN_IP;
        }
        if (gft_emp->headers & GFT_HEADER_IP_IN_GRE_ENCAP) {
            rx_key1.tunnel_metadata_tunnel_type_2 =
                INGRESS_TUNNEL_TYPE_GRE;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_GRE_PROTOCOL) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_GRE_PROTO_2;
            }
        }
        if (gft_emp->headers & GFT_HEADER_NVGRE_ENCAP) {
            rx_key1.tunnel_metadata_tunnel_type_2 =
                INGRESS_TUNNEL_TYPE_NVGRE;
        }
        if (gft_emp->headers & GFT_HEADER_VXLAN_ENCAP) {
            rx_key1.tunnel_metadata_tunnel_type_2 =
                INGRESS_TUNNEL_TYPE_VXLAN;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TENANT_ID) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TENANT_ID_2;
            }
        }
    }

    if (num_profiles > 2) {
        gft_emp = &gft_args->exact_match_profile->hgem_profiles[2];
        if (gft_emp->headers & GFT_HEADER_ETHERNET) {
            rx_key1.ethernet_3_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_ETHERNET_DST;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_ETHERNET_SRC;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_ETH_TYPE) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_ETHERNET_TYPE;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_CUSTOMER_VLAN_ID;
            }
        }
        if (gft_emp->headers & GFT_HEADER_IPV4) {
            rx_key1.ipv4_3_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_SRC;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_DST;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_DSCP;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_PROTO;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TTL) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_TTL;
            }
        } else if (gft_emp->headers & GFT_HEADER_IPV6) {
            rx_key1.ipv6_3_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_SRC;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_DST;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_DSCP;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_PROTO;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TTL) {
                rx_key3_action.rx_key3_action_u.rx_key3_rx_key3.match_fields |=
                    MATCH_IP_TTL;
            }
        }
        if (gft_emp->headers & GFT_HEADER_ICMP) {
            rx_key1.icmp_3_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_ICMP_TYPE) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_ICMP_TYPE_3;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_ICMP_CODE) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_ICMP_CODE_3;
            }
        } else if (gft_emp->headers & GFT_HEADER_TCP) {
            rx_key1.tcp_3_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TRANSPORT_SRC_PORT_3;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TRANSPORT_DST_PORT_3;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TCP_FLAGS) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TCP_FLAGS_3;
            }
        } else if (gft_emp->headers & GFT_HEADER_UDP) {
            rx_key1.udp_3_valid = 1;
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TRANSPORT_SRC_PORT_3;
            }
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TRANSPORT_DST_PORT_3;
            }
        }
        if (gft_emp->headers & GFT_HEADER_IP_IN_GRE_ENCAP) {
            if (gft_emp->match_fields & GFT_HEADER_FIELD_GRE_PROTOCOL) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_GRE_PROTO_3;
            }
        }
        if (gft_emp->headers & GFT_HEADER_VXLAN_ENCAP) {
            if (gft_emp->match_fields & GFT_HEADER_FIELD_TENANT_ID) {
                rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |=
                    MATCH_TENANT_ID_3;
            }
        }
    }

    // we are relying on the keys of all 4 tables are identical
    // if that changes, we will have to do field level assignments
    SDK_ASSERT(sizeof(rx_key1) == sizeof(rx_key2));
    SDK_ASSERT(sizeof(rx_key1) == sizeof(rx_key3));
    SDK_ASSERT(sizeof(rx_key1) == sizeof(rx_key4));

    // set up keys in other tables to be identical
    memcpy(&rx_key2, &rx_key1, sizeof(rx_key1));
    memcpy(&rx_key3, &rx_key1, sizeof(rx_key1));
    memcpy(&rx_key4, &rx_key1, sizeof(rx_key1));

    // set all action ids
    rx_key1_action.action_id = RX_KEY1_RX_KEY1_ID;
    rx_key2_action.action_id = RX_KEY2_RX_KEY2_ID;
    rx_key3_action.action_id = RX_KEY3_RX_KEY3_ID;
    rx_key4_action.action_id = RX_KEY4_RX_KEY4_ID;

    key_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_RX_KEY1);
    sdk_ret = key_tbl->insert(&rx_key1, &rx_key1_mask,
                              &rx_key1_action, &tcam_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_ENTRY_EXISTS) {
        goto end;
    } else if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to insert into rx key1 tcam table, err : {}",
                      ret);
        return ret;
    }

    key_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_RX_KEY2);
    sdk_ret = key_tbl->insert(&rx_key2, &rx_key2_mask,
                              &rx_key2_action, &tcam_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_ENTRY_EXISTS) {
    } else if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to insert into rx key2 tcam table, err : {}",
                      ret);
        return ret;
    }

    key_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_RX_KEY3);
    sdk_ret = key_tbl->insert(&rx_key3, &rx_key3_mask,
                              &rx_key3_action, &tcam_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_ENTRY_EXISTS) {
    } else if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to insert into rx key3 tcam table, err : {}",
                      ret);
        return ret;
    }

    key_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_RX_KEY4);
    sdk_ret = key_tbl->insert(&rx_key4, &rx_key4_mask,
                              &rx_key4_action, &tcam_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_ENTRY_EXISTS) {
    } else if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to insert into rx key4 tcam table, err : {}",
                      ret);
        return ret;
    }

end:

    return ret;
}
#endif

//------------------------------------------------------------------------------
// link GFT header transposition profile's PI and PD states
//------------------------------------------------------------------------------
static void
gft_hdr_group_xposition_link_pi_pd (pd_gft_hdr_xposition_profile_t *pd,
                                    gft_hdr_xposition_profile_t *pi)
{
    pd->gft_hdr_xposition_profile = pi;
    pi->pd = pd;
}

hal_ret_t
pd_gft_hdr_group_xposition_profile_create (pd_func_args_t *pd_func_args)
{
    pd_gft_hdr_group_xposition_profile_args_t *gft_args = pd_func_args->pd_gft_hdr_group_xposition_profile;
    pd_gft_hdr_xposition_profile_t    *gft_hdr_xposition_profile_pd = NULL;

    if (!gft_args || !gft_args->profile) {
        return HAL_RET_INVALID_ARG;
    }

    if (gft_args->profile->profile_id ==
            HAL_GFT_INVALID_PROFILE_ID) {
        return HAL_RET_INVALID_ARG;
    }

    if (!gft_args->profile->num_htp) {
        return HAL_RET_INVALID_ARG;
    }

    // create PD state for this exact match profile
    gft_hdr_xposition_profile_pd = gft_hdr_xposition_profile_pd_alloc_init();
    if (gft_hdr_xposition_profile_pd == NULL) {
        return HAL_RET_OOM;
    }

    // link PI and PD states
    gft_hdr_group_xposition_link_pi_pd(gft_hdr_xposition_profile_pd,
                                       gft_args->profile);

#if 0
    ret = gft_hdr_group_xposition_profile_pd_add_to_db();
    if (ret != HAL_RET_OK) {
        gft_hdr_group_xposition_profile_pd_cleanup(gft_hdr_xposition_profile_pd);
    }
#endif

    return HAL_RET_OK;
}

static inline void
gft_exact_match_flow_entry_link_pi_pd (pd_gft_exact_match_flow_entry_t *pd,
                                       gft_exact_match_flow_entry_t *pi)
{
    pd->gft_exact_match_flow_entry = pi;
    pi->pd = pd;
}

static inline hal_ret_t
gft_program_rx_xpositions (gft_hdr_group_xposition_t *xpositions,
                           uint32_t num_xpositions, uint32_t index)
{
#if 0
    rx_hdr_transpositions0_swkey_t       rx0_key = { 0 };
    rx_hdr_transpositions0_actiondata_t    rx0_action = { 0 };
    rx_hdr_transpositions1_swkey_t       rx1_key = { 0 };
    rx_hdr_transpositions1_actiondata_t    rx1_action = { 0 };
    rx_hdr_transpositions2_swkey_t       rx2_key = { 0 };
    rx_hdr_transpositions2_actiondata_t    rx2_action = { 0 };
    rx_hdr_transpositions3_swkey_t       rx3_key = { 0 };
    rx_hdr_transpositions3_actiondata_t    rx3_action = { 0 };
#endif

    return HAL_RET_OK;
}

#if 0
hal_ret_t
pd_gft_exact_match_flow_entry_create (pd_gft_exact_match_flow_entry_args_t *gft_args)
{
    pd_gft_exact_match_flow_entry_t    *gft_exact_match_flow_entry_pd = NULL;

    // expect both match profile and header xposition profile to be passed as
    // well all the time
    if (!gft_args->exact_match_profile   ||
        !gft_args->hdr_xposition_profile ||
        !gft_args->exact_match_flow_entry) {
        return HAL_RET_INVALID_ARG;
    }

    // create PD state for this exact match flow entry
    gft_exact_match_flow_entry_pd = gft_exact_match_flow_entry_pd_alloc_init();
    if (gft_exact_match_flow_entry_pd == NULL) {
        return HAL_RET_OOM;
    }

    // link PI and PD states
    gft_exact_match_flow_entry_link_pi_pd(gft_exact_match_flow_entry_pd,
                                          gft_args->exact_match_flow_entry);

    return HAL_RET_OK;
}
#endif

hal_ret_t
pd_asic_init (pd_func_args_t *pd_func_args)
{
    sdk_ret_t sdk_ret;
    pd_asic_init_args_t *args = pd_func_args->pd_asic_init;

    args->cfg->pgm_name = std::string("gft");

    args->cfg->num_pgm_cfgs = 1;
    args->cfg->pgm_cfg[0].path = std::string("pgm_bin");

    args->cfg->num_asm_cfgs = 2;
    args->cfg->asm_cfg[0].name = std::string("gft");
    args->cfg->asm_cfg[0].path = std::string("asm_bin");
    args->cfg->asm_cfg[0].symbols_func = NULL;
    args->cfg->asm_cfg[0].sort_func = NULL;
    args->cfg->asm_cfg[0].base_addr = std::string(JP4_PRGM);

    args->cfg->asm_cfg[1].name = std::string("p4plus");
    args->cfg->asm_cfg[1].path = std::string("p4plus_bin");
    args->cfg->asm_cfg[1].symbols_func = common_p4plus_symbols_init;
    args->cfg->asm_cfg[1].sort_func = NULL;
    args->cfg->asm_cfg[1].base_addr = std::string(JP4PLUS_PRGM);

    args->cfg->completion_func = asiccfg_init_completion_event;

    sdk_ret = sdk::asic::pd::asicpd_init(args->cfg);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
pd_lif_get_lport_id (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_acl_create (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_qos_class_set_global_pause_type (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_qos_class_init_tc_to_iq_map (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_qos_class_create (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_copp_create (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// TODO: this one also doesn't belong here .. we need to get the cpu tx/rx
// driver story right
hal_ret_t
pd_cpupkt_poll_receive (pd_func_args_t *pd_func_args)
{
    return HAL_RET_RETRY;
}

hal_ret_t
pd_cpupkt_poll_receive_new (pd_func_args_t *pd_func_args)
{
    return HAL_RET_RETRY;
}

}    // namespace pd
}    // namespace hal
