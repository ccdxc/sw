// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/src/lif/eth.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/gft/efe_pd.hpp"
#include "nic/hal/pd/gft/pd_utils.hpp"
#include "nic/hal/pd/gft/p4pd_defaults.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/p4/gft/include/defines.h"
#include "nic/include/interface_api.hpp"


namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
static pd_gft_efe_t *efe_pd_alloc ();
static pd_gft_efe_t *efe_pd_init (pd_gft_efe_t *efe);
static pd_gft_efe_t *efe_pd_alloc_init ();
static hal_ret_t efe_pd_free (pd_gft_efe_t *efe);
static hal_ret_t efe_pd_mem_free (pd_gft_efe_t *efe);
static hal_ret_t efe_pd_alloc_res(pd_gft_efe_t *pd_efe,
                                  pd_gft_exact_match_flow_entry_args_t *args);
static hal_ret_t efe_pd_program_hw(pd_gft_efe_t *pd_efe);
static hal_ret_t efe_pd_deprogram_hw (pd_gft_efe_t *pd_efe);
static hal_ret_t efe_pd_populate_response(pd_gft_efe_t *pd_efe,
                                          pd_gft_exact_match_flow_entry_args_t *args);

static void link_pi_pd(pd_gft_efe_t *pd_efe, gft_exact_match_flow_entry_t *pi_efe);
static void delink_pi_pd(pd_gft_efe_t *pd_efe, gft_exact_match_flow_entry_t *pi_efe);
static hal_ret_t efe_pd_cleanup(pd_gft_efe_t *efe_pd);
static inline hal_ret_t pd_gft_efe_make_clone(pd_func_args_t *pd_func_args);
static hal_ret_t efe_pd_program_transpositions(pd_gft_efe_t *pd_gft_efe);
static hal_ret_t efe_pd_program_flow(pd_gft_efe_t *pd_gft_efe);
static hal_ret_t efe_pd_program_tx_transpositions(pd_gft_efe_t *pd_gft_efe);
static hal_ret_t efe_pd_program_tx_flow(pd_gft_efe_t *pd_gft_efe);

//-----------------------------------------------------------------------------
// EFE Create in PD
//-----------------------------------------------------------------------------
hal_ret_t
pd_gft_exact_match_flow_entry_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t            ret = HAL_RET_OK;
    pd_gft_exact_match_flow_entry_args_t *args = pd_func_args->pd_gft_exact_match_flow_entry;
    pd_gft_efe_t         *pd_gft_efe = NULL;

    // Create efe PD
    pd_gft_efe = efe_pd_alloc_init();
    if (pd_gft_efe == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_gft_efe, args->exact_match_flow_entry);

    // Allocate Resources
    ret = efe_pd_alloc_res(pd_gft_efe, args);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        goto end;
    }

    // Program HW
    ret = efe_pd_program_hw(pd_gft_efe);

    // Populate response
    ret = efe_pd_populate_response(pd_gft_efe, args);

end:
    if (ret != HAL_RET_OK) {
        efe_pd_cleanup(pd_gft_efe);
    }
    return ret;
}

//-----------------------------------------------------------------------------
// PD EFE Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_gft_exact_match_flow_entry_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD EFE Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_gft_exact_match_flow_entry_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// Allocate resources for PD EFE
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_alloc_res(pd_gft_efe_t *pd_gft_efe, pd_gft_exact_match_flow_entry_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources.
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_dealloc_res(pd_gft_efe_t *efe_pd)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD efe Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD efe
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_cleanup(pd_gft_efe_t *efe_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!efe_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = efe_pd_dealloc_res(efe_pd);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // Delinking PI<->PD
    delink_pi_pd(efe_pd, (gft_exact_match_flow_entry_t *)efe_pd->pi_efe);

    // Freeing PD
    efe_pd_free(efe_pd);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate and Initialize EFE PD Instance
//-----------------------------------------------------------------------------
static inline pd_gft_efe_t *
efe_pd_alloc_init (void)
{
    return efe_pd_init(efe_pd_alloc());
}

//-----------------------------------------------------------------------------
// Allocate efe Instance
//-----------------------------------------------------------------------------
static inline pd_gft_efe_t *
efe_pd_alloc (void)
{
    pd_gft_efe_t    *efe;

    efe = (pd_gft_efe_t *)g_hal_state_pd->exact_match_flow_entry_pd_slab()->alloc();
    if (efe == NULL) {
        return NULL;
    }
    return efe;
}

//-----------------------------------------------------------------------------
// Initialize efe PD instance
//-----------------------------------------------------------------------------
static inline pd_gft_efe_t *
efe_pd_init (pd_gft_efe_t *efe)
{
    // Nothing to do currently
    if (!efe) {
        return NULL;
    }
    // Set here if you want to initialize any fields
    efe->flow_table_idx = INVALID_INDEXER_INDEX;
    efe->flow_idx = INVALID_INDEXER_INDEX;

    return efe;
}

//-----------------------------------------------------------------------------
// Program HW
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_program_hw (pd_gft_efe_t *pd_gft_efe)
{
    hal_ret_t                       ret = HAL_RET_OK;
    gft_exact_match_flow_entry_t    *gft_efe;

    gft_efe = (gft_exact_match_flow_entry_t *)pd_gft_efe->pi_efe;
    if (!gft_efe) {
        HAL_TRACE_ERR("pi is null");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (gft_match_prof_is_ingress(gft_efe->table_type)) {
        // Program transpositions
        ret = efe_pd_program_transpositions(pd_gft_efe);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program transpostions: ret:{}", ret);
            goto end;
        }
        // Program flow entry
        ret = efe_pd_program_flow(pd_gft_efe);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program flow : ret:{}", ret);
            goto end;
        }
    } else {
        // Program Tx transpositions
        ret = efe_pd_program_tx_transpositions(pd_gft_efe);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program transpostions: ret:{}", ret);
            goto end;
        }
        // Program Tx Flow entry
        ret = efe_pd_program_tx_flow(pd_gft_efe);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program flow : ret:{}", ret);
            goto end;
        }
    }

end:
    return ret;
}
// xpo ## TBL_ID ## _data.actionid = gft_hgxpos->action;

#define RX_XPOSITION_DATA(TBL_ID, LAYER)                                    \
    gft_hgxpos = &gft_efe->transpositions[TBL_ID];                          \
    hgem = &gft_efe->exact_matches[TBL_ID];                                 \
    if (gft_hgxpos->action == GFT_HDR_GROUP_XPOSITION_ACTION_POP) {         \
        if (gft_hgxpos->headers & GFT_HEADER_ETHERNET) {                    \
            xpo ## TBL_ID ## _data.                                         \
            rx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.      \
            hdr ## LAYER ## _bits |= TRANSPOSITIONS_POP_ETHERNET;           \
            xpo ## TBL_ID ## _data.                                         \
            rx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.      \
            hdr ## LAYER ## _bits |= TRANSPOSITIONS_POP_CTAG;               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_IPV4) {                        \
            xpo ## TBL_ID ## _data.                                         \
            rx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.      \
            hdr ## LAYER ## _bits |= TRANSPOSITIONS_POP_IPV4;               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_IPV6) {                        \
            xpo ## TBL_ID ## _data.                                         \
            rx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.      \
            hdr ## LAYER ## _bits |= TRANSPOSITIONS_POP_IPV6;               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_VXLAN_ENCAP) {                 \
            xpo3_data.                                                      \
            rx_hdr_transpositions3_action_u.                                \
            rx_hdr_transpositions3_rx_l4_hdr_transpositions.                \
            hdr_bits |= TRANSPOSITIONS_POP_VXLAN_1;                         \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_UDP) {                         \
            xpo3_data.                                                      \
            rx_hdr_transpositions3_action_u.                                \
            rx_hdr_transpositions3_rx_l4_hdr_transpositions.                \
            hdr_bits |= TRANSPOSITIONS_POP_UDP_1;                           \
        }                                                                   \
    } else if (gft_hgxpos->action ==                                        \
               GFT_HDR_GROUP_XPOSITION_ACTION_MODIFY) {                     \
        if (gft_hgxpos->headers & GFT_HEADER_ETHERNET) {                    \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {       \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_ETHERNET_DST;\
                memcpy(xpo ## TBL_ID ## _data.                              \
                       rx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                       ethernet_dst, gft_hgxpos->eth_fields.dmac,           \
                       ETH_ADDR_LEN);                                       \
                memrev(xpo ## TBL_ID ## _data.                              \
                       rx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                       ethernet_dst, ETH_ADDR_LEN);                         \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {       \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_ETHERNET_SRC;\
                memcpy(xpo ## TBL_ID ## _data.                              \
                       rx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                       ethernet_src, gft_hgxpos->eth_fields.smac,           \
                       ETH_ADDR_LEN);                                       \
                memrev(xpo ## TBL_ID ## _data.                              \
                       rx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                       ethernet_src, ETH_ADDR_LEN);                         \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_ETH_TYPE) {           \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_ETHERNET_TYPE;\
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                ethernet_type = gft_hgxpos->eth_fields.eth_type;            \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {   \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_POP_CTAG;           \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_CTAG;        \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                ctag = gft_hgxpos->eth_fields.customer_vlan_id;             \
                if (hgem->headers & GFT_HEADER_IPV4) {                      \
                    xpo ## TBL_ID ## _data.                                 \
                    rx_hdr_transpositions ## TBL_ID ## _action_u.           \
                    rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                    ethernet_type = ETHERTYPE_IPV4;                         \
                } else {                                                    \
                    xpo ## TBL_ID ## _data.                                 \
                    rx_hdr_transpositions ## TBL_ID ## _action_u.           \
                    rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                    ethernet_type = ETHERTYPE_IPV6;                         \
                }                                                           \
            }                                                               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_IPV4 ||                        \
            gft_hgxpos->headers & GFT_HEADER_IPV6) {                        \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {        \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_IP_SRC;      \
                memcpy(xpo ## TBL_ID ## _data.                              \
                       rx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                       ip_src, &gft_hgxpos->src_ip_addr.addr,               \
                       IP6_ADDR8_LEN);                                      \
                if (gft_hgxpos->src_ip_addr.af == IP_AF_IPV6) {             \
                    memrev(xpo ## TBL_ID ## _data.                          \
                           rx_hdr_transpositions ## TBL_ID ## _action_u.    \
                           rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                           ip_src, IP6_ADDR8_LEN);                          \
                }                                                           \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_DST_IP_ADDR) {        \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_IP_DST;      \
                memcpy(xpo ## TBL_ID ## _data.                              \
                       rx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                       ip_dst, &gft_hgxpos->dst_ip_addr.addr,               \
                       IP6_ADDR8_LEN);                                      \
                if (gft_hgxpos->dst_ip_addr.af == IP_AF_IPV6) {             \
                    memrev(xpo ## TBL_ID ## _data.                          \
                           rx_hdr_transpositions ## TBL_ID ## _action_u.    \
                           rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.\
                           ip_dst, IP6_ADDR8_LEN);                          \
                }                                                           \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_IP_DSCP) {            \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_IP_DSCP;     \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                ip_dscp = gft_hgxpos->dscp;                                 \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TTL) {                \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_IP_TTL;      \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                ip_ttl = gft_hgxpos->ttl;                                   \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_IP_PROTOCOL) {        \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                hdr ## LAYER ## _bits |= TRANSPOSITIONS_MODIFY_IP_PROTO;    \
                xpo ## TBL_ID ## _data.                                     \
                rx_hdr_transpositions ## TBL_ID ## _action_u.               \
                rx_hdr_transpositions ## TBL_ID ## _rx_hdr_transpositions.  \
                ip_proto = gft_hgxpos->ip_proto;                            \
            }                                                               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_TCP) {                         \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) { \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_TCP_SPORT_ ## LAYER;      \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                l4_sport_ ## LAYER = gft_hgxpos->encap_or_transport.tcp.sport;\
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) { \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_TCP_DPORT_ ## LAYER;      \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                l4_dport_ ## LAYER = gft_hgxpos->encap_or_transport.tcp.dport;\
            }                                                               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_UDP) {                         \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) { \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_UDP_SPORT_ ## LAYER;      \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                l4_sport_ ## LAYER = gft_hgxpos->encap_or_transport.udp.sport;\
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) { \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_UDP_DPORT_ ## LAYER;      \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                l4_dport_ ## LAYER = gft_hgxpos->encap_or_transport.udp.dport;\
            }                                                               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_ICMP) {                        \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_ICMP_TYPE) {          \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_ICMP_TYPE_ ## LAYER;      \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                l4_sport_ ## LAYER = gft_hgxpos->encap_or_transport.icmp.type;\
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_ICMP_CODE) {          \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_ICMP_CODE_ ## LAYER;      \
                xpo3_data.                                                  \
                rx_hdr_transpositions3_action_u.                            \
                rx_hdr_transpositions3_rx_l4_hdr_transpositions.            \
                l4_dport_ ## LAYER = gft_hgxpos->encap_or_transport.icmp.code;\
            }                                                               \
        }                                                                   \
    }


#define RX_XPOSITION_PGM(TBL_ID)                                            \
    rx_xpos_tbl = g_hal_state_pd->                                          \
                  dm_table(P4TBL_ID_RX_HDR_TRANSPOSITIONS ## TBL_ID);       \
    sdk_ret = rx_xpos_tbl->insert(&xpo ## TBL_ID ## _data,                  \
                                  &pd_gft_efe->flow_idx);                   \
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);                                  \
    if (ret != HAL_RET_OK) {                                                \
         HAL_TRACE_ERR(" Failed to insert into rx hdr xpos" #TBL_ID " table"\
                       "err : {}", ret);                                    \
        goto end;                                                           \
    }                                                                       \
    HAL_TRACE_DEBUG("Programmed rx_hdr_xpos" #TBL_ID " at {}",              \
                    pd_gft_efe->flow_idx);                                  \

#define RX_XPOSITION_PGM_WITHID(TBL_ID)                                     \
    rx_xpos_tbl = g_hal_state_pd->                                          \
                  dm_table(P4TBL_ID_RX_HDR_TRANSPOSITIONS ## TBL_ID);       \
    sdk_ret = rx_xpos_tbl->insert_withid(&xpo ## TBL_ID ## _data,           \
                                         pd_gft_efe->flow_idx);             \
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);                                  \
    if (ret != HAL_RET_OK) {                                                \
         HAL_TRACE_ERR(" Failed to insert into rx hdr xpos" #TBL_ID " table"\
                       "err : {}", ret);                                    \
        goto end;                                                           \
    }                                                                       \
    HAL_TRACE_DEBUG("Programmed rx_hdr_xpos" #TBL_ID " at {}",              \
                    pd_gft_efe->flow_idx);                                  \


//-----------------------------------------------------------------------------
// Program Rx transpostions
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_program_transpositions(pd_gft_efe_t *pd_gft_efe)
{
    hal_ret_t                               ret = HAL_RET_OK;
    sdk_ret_t                               sdk_ret;
    gft_exact_match_flow_entry_t            *gft_efe;
    gft_hdr_group_xposition_t               *gft_hgxpos;
    gft_hdr_group_exact_match_t             *hgem = NULL;
    uint32_t                                num_xpos = 0;
    rx_hdr_transpositions0_actiondata       xpo0_data = { 0 };
    rx_hdr_transpositions1_actiondata       xpo1_data = { 0 };
    rx_hdr_transpositions2_actiondata       xpo2_data = { 0 };
    rx_hdr_transpositions3_actiondata       xpo3_data = { 0 };
    directmap                               *rx_xpos_tbl = NULL;

    gft_efe = (gft_exact_match_flow_entry_t *)pd_gft_efe->pi_efe;
    if (!gft_efe) {
        HAL_TRACE_ERR("pi is null");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    num_xpos = gft_efe->num_transpositions;
    if (num_xpos > 3) {
        HAL_TRACE_ERR("Invalid no. of xpos: {}", num_xpos);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (num_xpos == 0) {
        pd_gft_efe->flow_idx = RX_TRANSPOSITION_NOP_ENTRY;
        goto end;
    }
    if (num_xpos > 0) {
        RX_XPOSITION_DATA(0, 1);
        RX_XPOSITION_PGM(0);
    }
    if (num_xpos > 1) {
        RX_XPOSITION_DATA(1, 2);
        RX_XPOSITION_PGM_WITHID(1);
    }
    if (num_xpos > 2) {
        RX_XPOSITION_DATA(2, 3);
        RX_XPOSITION_PGM_WITHID(2);
    }

    if (num_xpos) {
        RX_XPOSITION_PGM_WITHID(3);
    }

end:
    return ret;
}

#define GFT_KEY_FORM(LAYER)                                                 \
    gft_hgem = &gft_efe->exact_matches[LAYER - 1];                          \
    if (gft_hgem->headers & GFT_HEADER_ETHERNET) {                          \
        if (gft_hgem->fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {             \
            memcpy(gft_key.flow_lkp_metadata_ethernet_dst_ ## LAYER,        \
                   gft_hgem->eth_fields.dmac, ETH_ADDR_LEN);                \
            memrev(gft_key.flow_lkp_metadata_ethernet_dst_ ## LAYER,        \
                   ETH_ADDR_LEN);                                           \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {             \
            memcpy(gft_key.flow_lkp_metadata_ethernet_src_ ## LAYER,        \
                   gft_hgem->eth_fields.smac, ETH_ADDR_LEN);                \
            memrev(gft_key.flow_lkp_metadata_ethernet_src_ ## LAYER,        \
                   ETH_ADDR_LEN);                                           \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_ETH_TYPE) {                 \
            gft_key.flow_lkp_metadata_ethernet_type_ ## LAYER =             \
            gft_hgem->eth_fields.eth_type;                                  \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {         \
            gft_key.flow_lkp_metadata_ctag_ ## LAYER =                      \
            gft_hgem->eth_fields.customer_vlan_id;                          \
        }                                                                   \
    }                                                                       \
    if (gft_hgem->headers & GFT_HEADER_IPV4 ||                              \
        gft_hgem->headers & GFT_HEADER_IPV6) {                              \
        if (gft_hgem->fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {              \
            memcpy(gft_key.flow_lkp_metadata_ip_src_ ## LAYER,              \
                   &gft_hgem->src_ip_addr.addr, IP6_ADDR8_LEN);             \
            if (gft_hgem->headers & GFT_HEADER_IPV6) {                      \
                memrev(gft_key.flow_lkp_metadata_ip_src_ ## LAYER,          \
                       IP6_ADDR8_LEN);                                      \
            }                                                               \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_DST_IP_ADDR) {              \
            memcpy(gft_key.flow_lkp_metadata_ip_dst_ ## LAYER,              \
                   &gft_hgem->dst_ip_addr.addr, IP6_ADDR8_LEN);             \
            if (gft_hgem->headers & GFT_HEADER_IPV6) {                      \
                memrev(gft_key.flow_lkp_metadata_ip_dst_ ## LAYER,          \
                       IP6_ADDR8_LEN);                                      \
            }                                                               \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_IP_DSCP) {                  \
            gft_key.flow_lkp_metadata_ip_dscp_ ## LAYER =                   \
            gft_hgem->dscp;                                                 \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_IP_PROTOCOL) {              \
            gft_key.flow_lkp_metadata_ip_proto_ ## LAYER =                  \
            gft_hgem->ip_proto;                                             \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TTL) {                      \
            gft_key.flow_lkp_metadata_ip_ttl_ ## LAYER =                    \
            gft_hgem->ttl;                                                  \
        }                                                                   \
    }                                                                       \
    if (gft_hgem->headers & GFT_HEADER_ICMP) {                              \
        if (gft_hgem->fields & GFT_HEADER_FIELD_ICMP_TYPE) {                \
            gft_key.flow_lkp_metadata_l4_sport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.icmp.type;                         \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_ICMP_CODE) {                \
            gft_key.flow_lkp_metadata_l4_dport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.icmp.code;                         \
        }                                                                   \
    } else if (gft_hgem->headers & GFT_HEADER_TCP) {                        \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {       \
            gft_key.flow_lkp_metadata_l4_sport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.tcp.sport;                         \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {       \
            gft_key.flow_lkp_metadata_l4_dport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.tcp.dport;                         \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TCP_FLAGS) {                \
            gft_key.flow_lkp_metadata_tcp_flags_ ## LAYER =                 \
            gft_hgem->encap_or_transport.tcp.tcp_flags;                     \
        }                                                                   \
    } else if (gft_hgem->headers & GFT_HEADER_UDP) {                        \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {       \
            gft_key.flow_lkp_metadata_l4_sport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.udp.sport;                         \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {       \
            gft_key.flow_lkp_metadata_l4_dport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.udp.dport;                         \
        }                                                                   \
    }                                                                       \
    if (gft_hgem->headers & GFT_HEADER_IP_IN_GRE_ENCAP) {                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_GRE_PROTOCOL) {             \
            gft_key.flow_lkp_metadata_gre_proto_ ## LAYER =                 \
            gft_hgem->encap_or_transport.encap.gre_protocol;                \
        }                                                                   \
    }                                                                       \
    if (gft_hgem->headers & GFT_HEADER_VXLAN_ENCAP) {                       \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TENANT_ID) {                \
            gft_key.flow_lkp_metadata_tenant_id_ ## LAYER =                 \
            gft_hgem->encap_or_transport.encap.tenant_id;                   \
        }                                                                   \
    }


//-----------------------------------------------------------------------------
// Program flow
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_program_flow(pd_gft_efe_t *pd_gft_efe)
{
    hal_ret_t                               ret = HAL_RET_OK;
    gft_exact_match_flow_entry_t            *gft_efe;
    gft_hdr_group_exact_match_t             *gft_hgem = NULL;
    rx_gft_hash_swkey_t                     gft_key = { 0 };
    gft_flow_hash_data_t                    data = { 0 };
    uint32_t                                num_matches = 0;

    gft_efe = (gft_exact_match_flow_entry_t *)pd_gft_efe->pi_efe;

    if (!gft_efe) {
        HAL_TRACE_ERR("pi is null");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    num_matches = gft_efe->num_exact_matches;
    if (num_matches > 0) {
        GFT_KEY_FORM(1);
    }
    if (num_matches > 1) {
        GFT_KEY_FORM(2);
    }
    if (num_matches > 2) {
        GFT_KEY_FORM(3);
    }

    // Populate data
    data.flow_index = pd_gft_efe->flow_idx;
    data.policer_index = pd_gft_efe->policer_idx;

    ret = g_hal_state_pd->flow_table()->insert(&gft_key,
                                               &data,
                                               &pd_gft_efe->flow_table_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow table insert failed, err : {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Flow inserted at: {} and index {}", pd_gft_efe->flow_table_idx, data.flow_index);


end:
    return ret;
}

/*
 * TBL_ID: Tx Transposition table id
 *
 * TBL_ID:
 *      0, 1, 2, 3
 *
 * LAYER:
 *      _ID + 1
 *
 * P4_LAYER
 *      00, 01, 1
 *
 *  Valid:
 *
 *  Pop/Modify
 *      › Populate hdr1_bits
 *  Push:
 *      › Populate hdr0_bits
 *
 *  Modify: [ 1 ]
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS0: Modify MAC, Ctag, IP
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS4: Modify L4 Header
 *
 *  Push, Modify: [ 01, 1 ]
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS0: Push 01 MAC, Ctag, IP
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS1: Modify Layer MAC, Ctag, IP
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS4: Push 01 L4 Header. Modify 1 Layer L4 Header
 *
 *  Push, Push, Modify: [ 00, 01, 1 ]
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS0: Push 00 MAC, Ctag, IP
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS1: Push 01 MAC, Ctag, IP
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS2: Modify 1 MAC, Ctag, IP
 *      P4TBL_ID_RX_HDR_TRANSPOSITIONS3: Push 00, 01 L4 Header. Modify 1 Layer L4 Header
 */

#define TX_XPOSITION_DATA(TBL_ID, P4_LAYER)                                 \
    gft_hgxpos = &gft_efe->transpositions[TBL_ID];                          \
    hgem = &gft_efe->exact_matches[TBL_ID];                                 \
    if (gft_hgxpos->action == GFT_HDR_GROUP_XPOSITION_ACTION_POP) {         \
        if (gft_hgxpos->headers & GFT_HEADER_ETHERNET) {                    \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            hdr1_bits |= TRANSPOSITIONS_POP_ETHERNET;                       \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            hdr1_bits |= TRANSPOSITIONS_POP_CTAG;                           \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_IPV4) {                        \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            hdr1_bits |= TRANSPOSITIONS_POP_IPV4;                           \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_IPV6) {                        \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            hdr1_bits |= TRANSPOSITIONS_POP_IPV6;                           \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_VXLAN_ENCAP) {                 \
            xpo3_data.                                                      \
            tx_hdr_transpositions3_action_u.                                \
            tx_hdr_transpositions3_tx_l4_hdr_transpositions.                \
            hdr_bits |= TRANSPOSITIONS_POP_VXLAN_1;                         \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_UDP) {                         \
            xpo3_data.                                                      \
            tx_hdr_transpositions3_action_u.                                \
            tx_hdr_transpositions3_tx_l4_hdr_transpositions.                \
            hdr_bits |= TRANSPOSITIONS_POP_UDP_1;                           \
        }                                                                   \
    } else if (gft_hgxpos->action == GFT_HDR_GROUP_XPOSITION_ACTION_PUSH) { \
        if (gft_hgxpos->headers & GFT_HEADER_ETHERNET) {                    \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            hdr0_bits |= TRANSPOSITIONS_PUSH_ETHERNET_ ## P4_LAYER;         \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            hdr0_bits |= TRANSPOSITIONS_PUSH_CTAG_ ## P4_LAYER;             \
            /* Fields */                                                    \
            /* Dmac */                                                      \
            memcpy(xpo ## TBL_ID ## _data.                                  \
                   tx_hdr_transpositions ## TBL_ID ## _action_u.            \
                   tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                   ethernet_dst, gft_hgxpos->eth_fields.dmac,               \
                   ETH_ADDR_LEN);                                           \
            memrev(xpo ## TBL_ID ## _data.                                  \
                   tx_hdr_transpositions ## TBL_ID ## _action_u.            \
                   tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                   ethernet_dst, ETH_ADDR_LEN);                             \
            /* Smac */                                                      \
            memcpy(xpo ## TBL_ID ## _data.                                  \
                   tx_hdr_transpositions ## TBL_ID ## _action_u.            \
                   tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                   ethernet_src, gft_hgxpos->eth_fields.smac,               \
                   ETH_ADDR_LEN);                                           \
            memrev(xpo ## TBL_ID ## _data.                                  \
                   tx_hdr_transpositions ## TBL_ID ## _action_u.            \
                   tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                   ethernet_src, ETH_ADDR_LEN);                             \
            /* EtherType */                                                 \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            ethernet_type = gft_hgxpos->eth_fields.eth_type;                \
            /* CVlan */                                                     \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            ctag = gft_hgxpos->eth_fields.customer_vlan_id;                 \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_IPV4) {                        \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            hdr0_bits |= TRANSPOSITIONS_PUSH_IPV4_ ## P4_LAYER;             \
            /* Src IP */                                                    \
            memcpy(xpo ## TBL_ID ## _data.                                  \
                   tx_hdr_transpositions ## TBL_ID ## _action_u.            \
                   tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                   ip_src, &gft_hgxpos->src_ip_addr.addr,                   \
                   IP6_ADDR8_LEN);                                          \
            /* Dst IP */                                                    \
            memcpy(xpo ## TBL_ID ## _data.                                  \
                   tx_hdr_transpositions ## TBL_ID ## _action_u.            \
                   tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                   ip_dst, &gft_hgxpos->dst_ip_addr.addr,                   \
                   IP6_ADDR8_LEN);                                          \
            /* DSCP */                                                      \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            ip_dscp = gft_hgxpos->dscp;                                     \
            /* TTL */                                                       \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            ip_ttl = gft_hgxpos->ttl;                                       \
            /* Ip Proto */                                                  \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            ip_proto = gft_hgxpos->ip_proto;                                \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_IPV6) {                        \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            hdr0_bits |= TRANSPOSITIONS_PUSH_IPV6_ ## P4_LAYER;             \
            /* Src IP */                                                    \
            memcpy(xpo ## TBL_ID ## _data.                                  \
                   tx_hdr_transpositions ## TBL_ID ## _action_u.            \
                   tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                   ip_src, &gft_hgxpos->src_ip_addr.addr,                   \
                   IP6_ADDR8_LEN);                                          \
            if (gft_hgxpos->src_ip_addr.af == IP_AF_IPV6) {                 \
                memrev(xpo ## TBL_ID ## _data.                              \
                       tx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                       ip_src, IP6_ADDR8_LEN);                              \
            }                                                               \
            /* Dst IP */                                                    \
            memcpy(xpo ## TBL_ID ## _data.                                  \
                   tx_hdr_transpositions ## TBL_ID ## _action_u.            \
                   tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                   ip_dst, &gft_hgxpos->dst_ip_addr.addr,                   \
                   IP6_ADDR8_LEN);                                          \
            if (gft_hgxpos->dst_ip_addr.af == IP_AF_IPV6) {                 \
                memrev(xpo ## TBL_ID ## _data.                              \
                       tx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                       ip_dst, IP6_ADDR8_LEN);                              \
            }                                                               \
            /* DSCP */                                                      \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            ip_dscp = gft_hgxpos->dscp;                                     \
            /* TTL */                                                       \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            ip_ttl = gft_hgxpos->ttl;                                       \
            /* Ip Proto */                                                  \
            xpo ## TBL_ID ## _data.                                         \
            tx_hdr_transpositions ## TBL_ID ## _action_u.                   \
            tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.      \
            ip_proto = gft_hgxpos->ip_proto;                                \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_VXLAN_ENCAP) {                 \
            xpo3_data.                                                      \
            tx_hdr_transpositions3_action_u.                                \
            tx_hdr_transpositions3_tx_l4_hdr_transpositions.                \
            hdr0_bits |= TRANSPOSITIONS_PUSH_VXLAN_ ## P4_LAYER;          \
            /* Tenant Id */                                                 \
            xpo3_data.                                                      \
            tx_hdr_transpositions3_action_u.                                \
            tx_hdr_transpositions3_tx_l4_hdr_transpositions.                \
            tenant_id_ ## P4_LAYER = gft_hgxpos->encap_or_transport.encap.tenant_id;\
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_UDP) {                         \
            xpo3_data.                                                      \
            tx_hdr_transpositions3_action_u.                                \
            tx_hdr_transpositions3_tx_l4_hdr_transpositions.                \
            hdr0_bits |= TRANSPOSITIONS_PUSH_UDP_ ## P4_LAYER;              \
            /* L4 Sport */                                                  \
            xpo3_data.                                                      \
            tx_hdr_transpositions3_action_u.                                \
            tx_hdr_transpositions3_tx_l4_hdr_transpositions.                \
            l4_sport_ ## P4_LAYER = gft_hgxpos->encap_or_transport.udp.sport;\
            /* L4 Dport */                                                  \
            xpo3_data.                                                      \
            tx_hdr_transpositions3_action_u.                                \
            tx_hdr_transpositions3_tx_l4_hdr_transpositions.                \
            l4_dport_ ## P4_LAYER = gft_hgxpos->encap_or_transport.udp.dport;\
        }                                                                   \
    } else if (gft_hgxpos->action ==                                        \
               GFT_HDR_GROUP_XPOSITION_ACTION_MODIFY) {                     \
        if (gft_hgxpos->headers & GFT_HEADER_ETHERNET) {                    \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {       \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_ETHERNET_DST;            \
                memcpy(xpo ## TBL_ID ## _data.                              \
                       tx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                       ethernet_dst, gft_hgxpos->eth_fields.dmac,           \
                       ETH_ADDR_LEN);                                       \
                memrev(xpo ## TBL_ID ## _data.                              \
                       tx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                       ethernet_dst, ETH_ADDR_LEN);                         \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {       \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_ETHERNET_SRC;            \
                memcpy(xpo ## TBL_ID ## _data.                              \
                       tx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                       ethernet_src, gft_hgxpos->eth_fields.smac,           \
                       ETH_ADDR_LEN);                                       \
                memrev(xpo ## TBL_ID ## _data.                              \
                       tx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                       ethernet_src, ETH_ADDR_LEN);                         \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_ETH_TYPE) {           \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_ETHERNET_TYPE;\
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                ethernet_type = gft_hgxpos->eth_fields.eth_type;            \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {   \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_POP_CTAG;           \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_CTAG;        \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                ctag = gft_hgxpos->eth_fields.customer_vlan_id;             \
                if (hgem->headers & GFT_HEADER_IPV4) {                      \
                    xpo ## TBL_ID ## _data.                                 \
                    tx_hdr_transpositions ## TBL_ID ## _action_u.           \
                    tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                    ethernet_type = ETHERTYPE_IPV4;                         \
                } else {                                                    \
                    xpo ## TBL_ID ## _data.                                 \
                    tx_hdr_transpositions ## TBL_ID ## _action_u.           \
                    tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                    ethernet_type = ETHERTYPE_IPV6;                         \
                }                                                           \
            }                                                               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_IPV4 ||                        \
            gft_hgxpos->headers & GFT_HEADER_IPV6) {                        \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {        \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_IP_SRC;      \
                memcpy(xpo ## TBL_ID ## _data.                              \
                       tx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                       ip_src, &gft_hgxpos->src_ip_addr.addr,               \
                       IP6_ADDR8_LEN);                                      \
                if (gft_hgxpos->src_ip_addr.af == IP_AF_IPV6) {             \
                    memrev(xpo ## TBL_ID ## _data.                          \
                           tx_hdr_transpositions ## TBL_ID ## _action_u.    \
                           tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                           ip_src, IP6_ADDR8_LEN);                          \
                }                                                           \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_DST_IP_ADDR) {        \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_IP_DST;      \
                memcpy(xpo ## TBL_ID ## _data.                              \
                       tx_hdr_transpositions ## TBL_ID ## _action_u.        \
                       tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                       ip_dst, &gft_hgxpos->dst_ip_addr.addr,               \
                       IP6_ADDR8_LEN);                                      \
                if (gft_hgxpos->dst_ip_addr.af == IP_AF_IPV6) {             \
                    memrev(xpo ## TBL_ID ## _data.                          \
                           tx_hdr_transpositions ## TBL_ID ## _action_u.    \
                           tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.\
                           ip_dst, IP6_ADDR8_LEN);                          \
                }                                                           \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_IP_DSCP) {            \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_IP_DSCP;     \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                ip_dscp = gft_hgxpos->dscp;                                 \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TTL) {                \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_IP_TTL;      \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                ip_ttl = gft_hgxpos->ttl;                                   \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_IP_PROTOCOL) {        \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                hdr1_bits |= TRANSPOSITIONS_MODIFY_IP_PROTO;    \
                xpo ## TBL_ID ## _data.                                     \
                tx_hdr_transpositions ## TBL_ID ## _action_u.               \
                tx_hdr_transpositions ## TBL_ID ## _tx_hdr_transpositions.  \
                ip_proto = gft_hgxpos->ip_proto;                            \
            }                                                               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_TCP) {                         \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) { \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_TCP_SPORT_1;              \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                l4_sport_1 = gft_hgxpos->encap_or_transport.tcp.sport;      \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) { \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_TCP_DPORT_1;              \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                l4_dport_1 = gft_hgxpos->encap_or_transport.tcp.dport;      \
            }                                                               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_UDP) {                         \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) { \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_UDP_SPORT_1;              \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                l4_sport_1 = gft_hgxpos->encap_or_transport.udp.sport;      \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) { \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_UDP_DPORT_1;              \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                l4_dport_1 = gft_hgxpos->encap_or_transport.udp.dport;      \
            }                                                               \
        }                                                                   \
        if (gft_hgxpos->headers & GFT_HEADER_ICMP) {                        \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_ICMP_TYPE) {          \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_ICMP_TYPE_1;              \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                l4_sport_1 = gft_hgxpos->encap_or_transport.icmp.type;      \
            }                                                               \
            if (gft_hgxpos->fields & GFT_HEADER_FIELD_ICMP_CODE) {          \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                hdr_bits |= TRANSPOSITIONS_MODIFY_ICMP_CODE_1;              \
                xpo3_data.                                                  \
                tx_hdr_transpositions3_action_u.                            \
                tx_hdr_transpositions3_tx_l4_hdr_transpositions.            \
                l4_dport_1 = gft_hgxpos->encap_or_transport.icmp.code;      \
            }                                                               \
        }                                                                   \
    }

#define TX_XPOSITION_PGM(TBL_ID)                                            \
    tx_xpos_tbl = g_hal_state_pd->                                          \
                  dm_table(P4TBL_ID_TX_HDR_TRANSPOSITIONS ## TBL_ID);       \
    sdk_ret = tx_xpos_tbl->insert(&xpo ## TBL_ID ## _data,                  \
                                  &pd_gft_efe->flow_idx);                   \
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);                                  \
    if (ret != HAL_RET_OK) {                                                \
         HAL_TRACE_ERR(" Failed to insert into tx hdr xpos" #TBL_ID " table"\
                       "err : {}", ret);                                    \
        goto end;                                                           \
    }                                                                       \
    HAL_TRACE_DEBUG("Programmed tx_hdr_xpos" #TBL_ID " at {}",              \
                    pd_gft_efe->flow_idx);                                  \

#define TX_XPOSITION_PGM_WITHID(TBL_ID)                                     \
    tx_xpos_tbl = g_hal_state_pd->                                          \
                  dm_table(P4TBL_ID_TX_HDR_TRANSPOSITIONS ## TBL_ID);       \
    sdk_ret = tx_xpos_tbl->insert_withid(&xpo ## TBL_ID ## _data,           \
                                         pd_gft_efe->flow_idx);             \
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);                                  \
    if (ret != HAL_RET_OK) {                                                \
         HAL_TRACE_ERR(" Failed to insert into tx hdr xpos" #TBL_ID " table"\
                       "err : {}", ret);                                    \
        goto end;                                                           \
    }                                                                       \
    HAL_TRACE_DEBUG("Programmed tx_hdr_xpos" #TBL_ID " at {}",              \
                    pd_gft_efe->flow_table_idx);                            \




//-----------------------------------------------------------------------------
// Program Tx transpostions
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_program_tx_transpositions(pd_gft_efe_t *pd_gft_efe)
{
    hal_ret_t                               ret = HAL_RET_OK;
    sdk_ret_t                               sdk_ret;
    gft_exact_match_flow_entry_t            *gft_efe;
    gft_hdr_group_xposition_t               *gft_hgxpos;
    gft_hdr_group_exact_match_t             *hgem = NULL;
    uint32_t                                num_xpos = 0;
    tx_hdr_transpositions0_actiondata       xpo0_data = { 0 };
    tx_hdr_transpositions1_actiondata       xpo1_data = { 0 };
    tx_hdr_transpositions2_actiondata       xpo2_data = { 0 };
    tx_hdr_transpositions3_actiondata       xpo3_data = { 0 };
    directmap                               *tx_xpos_tbl = NULL;

    gft_efe = (gft_exact_match_flow_entry_t *)pd_gft_efe->pi_efe;
    if (!gft_efe) {
        HAL_TRACE_ERR("pi is null");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    num_xpos = gft_efe->num_transpositions;
    if (num_xpos > 3) {
        HAL_TRACE_ERR("Invalid no. of xpos: {}", num_xpos);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    /*
     * P4 Layers
     *
     * 00, 01, 1, ‹Payload›
     */
    if (num_xpos == 0) {
        pd_gft_efe->flow_idx = TX_TRANSPOSITION_NOP_ENTRY;
        goto end;
    }

    if (num_xpos > 0) {
        if (num_xpos == 1) {
            // Modify layer 1
            TX_XPOSITION_DATA(0, 00);
        } else if (num_xpos == 2) {
            // Pushing layer 01
            TX_XPOSITION_DATA(0, 01);
        } else if (num_xpos == 3) {
            // Pushing layer 00
            TX_XPOSITION_DATA(0, 00);
        }
        TX_XPOSITION_PGM(0);
    }

    if (num_xpos > 1) {
        if (num_xpos == 2) {
            // Modify layer 1
            TX_XPOSITION_DATA(1, 00);
        } else if (num_xpos == 3) {
            // Pushing layer 01
            TX_XPOSITION_DATA(1, 01);
        }
        TX_XPOSITION_PGM_WITHID(1);
    }

    if (num_xpos > 2) {
        if (num_xpos == 3) {
            // Modify layer 1
            TX_XPOSITION_DATA(2, 00);
        }
        TX_XPOSITION_PGM_WITHID(2);
    }

    if (num_xpos) {
        TX_XPOSITION_PGM_WITHID(3);
    }

end:
    return ret;
}

#define TX_GFT_KEY_FORM(LAYER)                                              \
    gft_hgem = &gft_efe->exact_matches[LAYER - 1];                          \
    if (gft_hgem->headers & GFT_HEADER_ETHERNET) {                          \
        if (gft_hgem->fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {             \
            memcpy(gft_key.flow_lkp_metadata_ethernet_dst_ ## LAYER,        \
                   gft_hgem->eth_fields.dmac, ETH_ADDR_LEN);                \
            memrev(gft_key.flow_lkp_metadata_ethernet_dst_ ## LAYER,        \
                   ETH_ADDR_LEN);                                           \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {             \
            memcpy(gft_key.flow_lkp_metadata_ethernet_src_ ## LAYER,        \
                   gft_hgem->eth_fields.smac, ETH_ADDR_LEN);                \
            memrev(gft_key.flow_lkp_metadata_ethernet_src_ ## LAYER,        \
                   ETH_ADDR_LEN);                                           \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_ETH_TYPE) {                 \
            gft_key.flow_lkp_metadata_ethernet_type_ ## LAYER =             \
            gft_hgem->eth_fields.eth_type;                                  \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {         \
            gft_key.flow_lkp_metadata_ctag_ ## LAYER =                      \
            gft_hgem->eth_fields.customer_vlan_id;                          \
        }                                                                   \
    }                                                                       \
    if (gft_hgem->headers & GFT_HEADER_IPV4 ||                              \
        gft_hgem->headers & GFT_HEADER_IPV6) {                              \
        if (gft_hgem->fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {              \
            memcpy(gft_key.flow_lkp_metadata_ip_src_ ## LAYER,              \
                   &gft_hgem->src_ip_addr.addr, IP6_ADDR8_LEN);             \
            if (gft_hgem->headers & GFT_HEADER_IPV6) {                      \
                memrev(gft_key.flow_lkp_metadata_ip_src_ ## LAYER,          \
                       IP6_ADDR8_LEN);                                      \
            }                                                               \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_DST_IP_ADDR) {              \
            memcpy(gft_key.flow_lkp_metadata_ip_dst_ ## LAYER,              \
                   &gft_hgem->dst_ip_addr.addr, IP6_ADDR8_LEN);             \
            if (gft_hgem->headers & GFT_HEADER_IPV6) {                      \
                memrev(gft_key.flow_lkp_metadata_ip_dst_ ## LAYER,          \
                       IP6_ADDR8_LEN);                                      \
            }                                                               \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_IP_DSCP) {                  \
            gft_key.flow_lkp_metadata_ip_dscp_ ## LAYER =                   \
            gft_hgem->dscp;                                                 \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_IP_PROTOCOL) {              \
            gft_key.flow_lkp_metadata_ip_proto_ ## LAYER =                  \
            gft_hgem->ip_proto;                                             \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TTL) {                      \
            gft_key.flow_lkp_metadata_ip_ttl_ ## LAYER =                    \
            gft_hgem->ttl;                                                  \
        }                                                                   \
    }                                                                       \
    if (gft_hgem->headers & GFT_HEADER_ICMP) {                              \
        if (gft_hgem->fields & GFT_HEADER_FIELD_ICMP_TYPE) {                \
            gft_key.flow_lkp_metadata_l4_sport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.icmp.type;                         \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_ICMP_CODE) {                \
            gft_key.flow_lkp_metadata_l4_dport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.icmp.code;                         \
        }                                                                   \
    } else if (gft_hgem->headers & GFT_HEADER_TCP) {                        \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {       \
            gft_key.flow_lkp_metadata_l4_sport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.tcp.sport;                         \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {       \
            gft_key.flow_lkp_metadata_l4_dport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.tcp.dport;                         \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TCP_FLAGS) {                \
            gft_key.flow_lkp_metadata_tcp_flags_ ## LAYER =                 \
            gft_hgem->encap_or_transport.tcp.tcp_flags;                     \
        }                                                                   \
    } else if (gft_hgem->headers & GFT_HEADER_UDP) {                        \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {       \
            gft_key.flow_lkp_metadata_l4_sport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.udp.sport;                         \
        }                                                                   \
        if (gft_hgem->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {       \
            gft_key.flow_lkp_metadata_l4_dport_ ## LAYER =                  \
            gft_hgem->encap_or_transport.udp.dport;                         \
        }                                                                   \
    }                                                                       \

//-----------------------------------------------------------------------------
// Program Tx flow
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_program_tx_flow(pd_gft_efe_t *pd_gft_efe)
{
    hal_ret_t                               ret = HAL_RET_OK;
    gft_exact_match_flow_entry_t            *gft_efe;
    gft_hdr_group_exact_match_t             *gft_hgem = NULL;
    tx_gft_hash_swkey_t                     gft_key = { 0 };
    gft_flow_hash_data_t                    data = { 0 };
    uint32_t                                num_matches = 0;

    gft_efe = (gft_exact_match_flow_entry_t *)pd_gft_efe->pi_efe;

    if (!gft_efe) {
        HAL_TRACE_ERR("pi is null");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    num_matches = gft_efe->num_exact_matches;
    if (num_matches != 1) {
        HAL_TRACE_ERR("In TX: No: of matches have to be 1");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    TX_GFT_KEY_FORM(1);

    // Populate data
    data.flow_index = pd_gft_efe->flow_idx;
    data.policer_index = pd_gft_efe->policer_idx;

    ret = g_hal_state_pd->tx_flow_table()->insert(&gft_key,
                                               &data,
                                               &pd_gft_efe->flow_table_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow table insert failed, err : {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Flow inserted at: {} and index {}", pd_gft_efe->flow_table_idx, data.flow_index);


end:
    return ret;
}

//-----------------------------------------------------------------------------
// Populate response
//-----------------------------------------------------------------------------
hal_ret_t
efe_pd_populate_response(pd_gft_efe_t *pd_efe,
                         pd_gft_exact_match_flow_entry_args_t *args)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (!args->rsp) {
        goto end;
    }

    // Populating flow index tables
    args->rsp->mutable_status()->set_flow_index(pd_efe->flow_table_idx);
    args->rsp->mutable_status()->set_flow_info_index(pd_efe->flow_idx);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// DeProgram HW
//-----------------------------------------------------------------------------
static inline hal_ret_t
efe_pd_deprogram_hw (pd_gft_efe_t *pd_gft_efe)
{
    hal_ret_t                               ret = HAL_RET_OK;
    return ret;
}

//-----------------------------------------------------------------------------
// Freeing efe PD. Frees PD memory and all other memories allocated for PD.
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_free (pd_gft_efe_t *efe)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_GFT_EFE_PD, efe);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Freeing efe PD memory. Just frees the memory of PD structure.
//-----------------------------------------------------------------------------
static hal_ret_t
efe_pd_mem_free (pd_gft_efe_t *efe)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_GFT_EFE_PD, efe);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Linking PI <-> PD
//-----------------------------------------------------------------------------
static void
link_pi_pd (pd_gft_efe_t *pd_gft_efe, gft_exact_match_flow_entry_t *pi_efe)
{
    pd_gft_efe->pi_efe = pi_efe;
    pi_efe->pd = pd_gft_efe;
}

//-----------------------------------------------------------------------------
// Un-Linking PI <-> PD
//-----------------------------------------------------------------------------
static void
delink_pi_pd (pd_gft_efe_t *pd_gft_efe, gft_exact_match_flow_entry_t *pi_efe)
{
    pd_gft_efe->pi_efe = NULL;
    pi_efe->pd = NULL;
}

//-----------------------------------------------------------------------------
// Makes a clone
//-----------------------------------------------------------------------------
static inline hal_ret_t
pd_gft_efe_make_clone (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_gft_exact_match_flow_entry_make_clone_args_t *args =
        pd_func_args->pd_gft_exact_match_flow_entry_make_clone;
    pd_gft_efe_t        *pd_gft_efe_clone = NULL;

    gft_exact_match_flow_entry_t *efe = args->exact_match_flow_entry;
    gft_exact_match_flow_entry_t *clone = args->clone;

    pd_gft_efe_clone = efe_pd_alloc_init();
    if (pd_gft_efe_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_gft_efe_clone, efe->pd, sizeof(pd_gft_efe_t));

    link_pi_pd(pd_gft_efe_clone, clone);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Frees PD memory without indexer free.
//-----------------------------------------------------------------------------
static inline hal_ret_t
pd_gft_efe_mem_free (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_gft_exact_match_flow_entry_args_t *args = pd_func_args->pd_gft_exact_match_flow_entry;
    pd_gft_efe_t    *efe_pd;

    efe_pd = (pd_gft_efe_t *)args->exact_match_flow_entry->pd;
    efe_pd_mem_free(efe_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
