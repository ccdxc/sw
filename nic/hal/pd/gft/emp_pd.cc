// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/src/eth.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/gft/emp_pd.hpp"
#include "nic/hal/pd/gft/pd_utils.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/p4/gft/include/defines.h"
#include "nic/include/interface_api.hpp"


namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
static pd_gft_emp_t *emp_pd_alloc();
static pd_gft_emp_t *emp_pd_init(pd_gft_emp_t *emp);
static pd_gft_emp_t *emp_pd_alloc_init();
static hal_ret_t emp_pd_free(pd_gft_emp_t *emp);
static hal_ret_t emp_pd_mem_free(pd_gft_emp_t *emp);
static hal_ret_t emp_pd_alloc_res(pd_gft_emp_t *pd_emp, 
                                  pd_gft_exact_match_profile_args_t *args);
static hal_ret_t emp_pd_program_hw(pd_gft_emp_t *pd_emp);
static hal_ret_t emp_pd_deprogram_hw (pd_gft_emp_t *pd_emp);

static void link_pi_pd(pd_gft_emp_t *pd_emp, 
                       gft_exact_match_profile_t *pi_emp);
static void delink_pi_pd(pd_gft_emp_t *pd_emp, 
                         gft_exact_match_profile_t *pi_emp);
static hal_ret_t emp_pd_cleanup(pd_gft_emp_t *emp_pd);
static hal_ret_t pd_emp_make_clone(gft_exact_match_profile_t *emp, 
                                   gft_exact_match_profile_t *clone);
static hal_ret_t emp_pd_rx_keys_program_hw (pd_gft_emp_t *pd_gft_emp);
static hal_ret_t emp_pd_tx_keys_program_hw (pd_gft_emp_t *pd_gft_emp);

//-----------------------------------------------------------------------------
// EMP Create in PD
//-----------------------------------------------------------------------------
hal_ret_t
pd_gft_exact_match_profile_create (pd_gft_exact_match_profile_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;
    pd_gft_emp_t         *pd_gft_emp = NULL;

    // Alloc memory
    pd_gft_emp = emp_pd_alloc_init();
    if (pd_gft_emp == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_gft_emp, args->exact_match_profile);

    // Allocate Resources
    ret = emp_pd_alloc_res(pd_gft_emp, args);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        goto end;
    }

    // Program HW
    ret = emp_pd_program_hw(pd_gft_emp);

end:
    if (ret != HAL_RET_OK) {
        emp_pd_cleanup(pd_gft_emp);
    }
    return ret;
}

//-----------------------------------------------------------------------------
// PD EMP Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_gft_exact_match_profile_update (pd_gft_exact_match_profile_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD EMP Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_gft_exact_match_profile_delete (pd_gft_exact_match_profile_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// Allocate resources for PD EMP
//-----------------------------------------------------------------------------
static hal_ret_t 
emp_pd_alloc_res (pd_gft_emp_t *pd_gft_emp, 
                  pd_gft_exact_match_profile_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;
    
    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources. 
//-----------------------------------------------------------------------------
static hal_ret_t
emp_pd_dealloc_res (pd_gft_emp_t *emp_pd)
{
    hal_ret_t           ret = HAL_RET_OK;

    return ret;
}

//-----------------------------------------------------------------------------
// PD emp Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD emp
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
static hal_ret_t
emp_pd_cleanup (pd_gft_emp_t *emp_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!emp_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = emp_pd_dealloc_res(emp_pd);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // Delinking PI<->PD
    delink_pi_pd(emp_pd, (gft_exact_match_profile_t *)emp_pd->pi_emp);

    // Freeing PD
    emp_pd_free(emp_pd);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate and Initialize EMP PD Instance
//-----------------------------------------------------------------------------
static inline pd_gft_emp_t *
emp_pd_alloc_init (void)
{
    return emp_pd_init(emp_pd_alloc());
}

//-----------------------------------------------------------------------------
// Allocate emp Instance
//-----------------------------------------------------------------------------
static inline pd_gft_emp_t *
emp_pd_alloc (void)
{
    pd_gft_emp_t    *emp;

    emp = (pd_gft_emp_t *)g_hal_state_pd->exact_match_profile_pd_slab()->alloc();
    if (emp == NULL) {
        return NULL;
    }
    return emp;
}

//-----------------------------------------------------------------------------
// Initialize emp PD instance
//-----------------------------------------------------------------------------
static inline pd_gft_emp_t *
emp_pd_init (pd_gft_emp_t *emp)
{
    // Nothing to do currently
    if (!emp) {
        return NULL;
    }
    // Set here if you want to initialize any fields
    emp->rx_key1_idx = INVALID_INDEXER_INDEX;
    emp->rx_key2_idx = INVALID_INDEXER_INDEX;
    emp->rx_key3_idx = INVALID_INDEXER_INDEX;
    emp->rx_key4_idx = INVALID_INDEXER_INDEX;
    emp->tx_key1_idx = INVALID_INDEXER_INDEX;

    return emp;
}

//-----------------------------------------------------------------------------
// Program HW
//-----------------------------------------------------------------------------
static hal_ret_t
emp_pd_program_hw (pd_gft_emp_t *pd_gft_emp)
{
    hal_ret_t                 ret = HAL_RET_OK;
    gft_exact_match_profile_t *gft_emp = NULL;

    gft_emp = (gft_exact_match_profile_t *)pd_gft_emp->pi_emp;

    if (!gft_emp) {
        HAL_TRACE_ERR("pi is null");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (gft_match_prof_is_ingress(gft_emp->table_type)) {
        // Program the Rx Keys
        ret = emp_pd_rx_keys_program_hw(pd_gft_emp);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program hw for rx keys: ret:{}", ret);
            goto end;
        }
    } else {
        // Program Tx Key
        ret = emp_pd_tx_keys_program_hw(pd_gft_emp);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program hw for tx keys: ret:{}", ret);
            goto end;
        }
    }
    
end:
    return ret;
}

#define RX_KEY_LAYER_FORM_KEY_DATA(LAYER)                                   \
    gft_hgmp = &gft_emp->hgem_profiles[LAYER - 1];                          \
    if (gft_hgmp->headers & GFT_HEADER_ETHERNET) {                          \
        rx_key1.ethernet_ ## LAYER ## _valid = 1;                           \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {       \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_ETHERNET_DST;                                         \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {       \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_ETHERNET_SRC;                                         \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_ETH_TYPE) {           \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_ETHERNET_TYPE;                                        \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {   \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_CUSTOMER_VLAN_ID;                                     \
        }                                                                   \
    }                                                                       \
    if (gft_hgmp->headers & GFT_HEADER_IPV4) {                              \
        rx_key1.ipv4_ ## LAYER ## _valid = 1;                               \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {        \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_SRC;                                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {        \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_DST;                                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {            \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_DSCP;                                              \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {        \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_PROTO;                                             \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TTL) {                \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_TTL;                                               \
        }                                                                   \
    } else if (gft_hgmp->headers & GFT_HEADER_IPV6) {                       \
        rx_key1.ipv6_ ## LAYER ## _valid = 1;                               \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {        \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_SRC;                                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {        \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_DST;                                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {            \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_DSCP;                                              \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {        \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_PROTO;                                             \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TTL) {                \
            rx_key ## LAYER ## _action.rx_key ## LAYER ##                   \
            _action_u.rx_key ## LAYER ## _rx_key ## LAYER.match_fields |=   \
                MATCH_IP_TTL;                                               \
        }                                                                   \
    }                                                                       \
    if (gft_hgmp->headers & GFT_HEADER_ICMP) {                              \
        rx_key1.icmp_ ## LAYER ## _valid = 1;                               \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_ICMP_TYPE) {          \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_ICMP_TYPE_ ## LAYER ;                                     \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_ICMP_CODE) {          \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_ICMP_CODE_ ## LAYER ;                                     \
        }                                                                   \
    } else if (gft_hgmp->headers & GFT_HEADER_TCP) {                        \
        rx_key1.tcp_ ## LAYER ## _valid = 1;                                \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) { \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_TRANSPORT_SRC_PORT_ ## LAYER ;                            \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) { \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_TRANSPORT_DST_PORT_ ## LAYER ;                            \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TCP_FLAGS) {          \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_TCP_FLAGS_ ## LAYER ;                                     \
        }                                                                   \
    } else if (gft_hgmp->headers & GFT_HEADER_UDP) {                        \
        rx_key1.udp_ ## LAYER ## _valid = 1;                                \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) { \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_TRANSPORT_SRC_PORT_ ## LAYER ;                            \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) { \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_TRANSPORT_DST_PORT_ ## LAYER ;                            \
        }                                                                   \
    }                                                                       

#define RX_KEY_LAYER_FORM_L1L2_TUNNEL_KEY_DATA(LAYER)                       \
    if (gft_hgmp->headers & GFT_HEADER_IP_IN_IP_ENCAP) {                    \
        rx_key1.tunnel_metadata_tunnel_type_ ## LAYER = INGRESS_TUNNEL_TYPE_IP_IN_IP; \
    }                                                                       \
    if (gft_hgmp->headers & GFT_HEADER_IP_IN_GRE_ENCAP) {                   \
        rx_key1.tunnel_metadata_tunnel_type_ ## LAYER = INGRESS_TUNNEL_TYPE_GRE; \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_GRE_PROTOCOL) {       \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_GRE_PROTO_ ## LAYER ;                                     \
        }                                                                   \
    }                                                                       \
    if (gft_hgmp->headers & GFT_HEADER_NVGRE_ENCAP) {                       \
        rx_key1.tunnel_metadata_tunnel_type_ ## LAYER = INGRESS_TUNNEL_TYPE_NVGRE; \
    }                                                                       \
    if (gft_hgmp->headers & GFT_HEADER_VXLAN_ENCAP) {                       \
        rx_key1.tunnel_metadata_tunnel_type_ ## LAYER = INGRESS_TUNNEL_TYPE_VXLAN; \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TENANT_ID) {          \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_TENANT_ID_ ## LAYER ;                                     \
        }                                                                   \
    }

#define RX_KEY_LAYER_FORM_L3_TUNNEL_KEY_DATA(LAYER)                         \
    if (gft_hgmp->headers & GFT_HEADER_IP_IN_GRE_ENCAP) {                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_GRE_PROTOCOL) {       \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_GRE_PROTO_ ## LAYER ;                                     \
        }                                                                   \
    }                                                                       \
    if (gft_hgmp->headers & GFT_HEADER_VXLAN_ENCAP) {                       \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TENANT_ID) {          \
            rx_key4_action.rx_key4_action_u.rx_key4_rx_key4.match_fields |= \
            MATCH_TENANT_ID_ ## LAYER ;                                     \
        }                                                                   \
    }


#define RX_KEY_MEMCPY(DST, SRC)                                             \
    memcpy(&rx_key ## DST, &rx_key ## SRC, sizeof(rx_key1));    

#define RX_KEY_ACTIONID(ACT)                                                \
    rx_key ## ACT ## _action.actionid = RX_KEY ## ACT ## _RX_KEY ## ACT ## _ID;

#define RX_KEY_PGM(TBL_ID)                                                  \
    key_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_RX_KEY ## TBL_ID);        \
    sdk_ret = key_tbl->insert(&rx_key ## TBL_ID, &rx_key ## TBL_ID ## _mask,\
                              &rx_key ## TBL_ID ##_action, &tcam_idx);      \
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);                                  \
    if (ret == HAL_RET_DUP_INS_FAIL) {                                      \
        HAL_TRACE_ERR("Duplicate insert failure for rx_key ## TBL_ID ## "); \
        goto end;                                                           \
    } else if (ret != HAL_RET_OK) {                                         \
        HAL_TRACE_ERR("Failed to insert into rx key" #TBL_ID " tcam table"  \
                      "err : {}", ret);                                     \
        return ret;                                                         \
    }                                                                       \
    pd_gft_emp->rx_key ## TBL_ID ## _idx = tcam_idx;                        \
    HAL_TRACE_DEBUG("Programmed rx_key" #TBL_ID " at {}", tcam_idx);        \

//-----------------------------------------------------------------------------
// Program RX Keys
//-----------------------------------------------------------------------------
static hal_ret_t
emp_pd_rx_keys_program_hw (pd_gft_emp_t *pd_gft_emp)
{
    hal_ret_t                              ret = HAL_RET_OK;
    sdk_ret_t                              sdk_ret;
    gft_exact_match_profile_t              *gft_emp = NULL;
    gft_hdr_group_exact_match_profile_t    *gft_hgmp = NULL;
    uint32_t                               num_profiles = 0;
    uint32_t                               tcam_idx = 0;
    rx_key1_swkey_t                        rx_key1 = { 0 };
    rx_key1_swkey_mask_t                   rx_key1_mask = { 0 };
    rx_key1_actiondata                     rx_key1_action = { 0 };
    rx_key2_swkey_t                        rx_key2 = { 0 };
    rx_key2_swkey_mask_t                   rx_key2_mask = { 0 };
    rx_key2_actiondata                     rx_key2_action = { 0 };
    rx_key3_swkey_t                        rx_key3 = { 0 };
    rx_key3_swkey_mask_t                   rx_key3_mask = { 0 };
    rx_key3_actiondata                     rx_key3_action = { 0 };
    rx_key4_swkey_t                        rx_key4 = { 0 };
    rx_key4_swkey_mask_t                   rx_key4_mask = { 0 };
    rx_key4_actiondata                     rx_key4_action = { 0 };
    tcam                                   *key_tbl;

    gft_emp = (gft_exact_match_profile_t *)pd_gft_emp->pi_emp;

    if (!gft_emp) {
        HAL_TRACE_ERR("pi is null");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    num_profiles = gft_emp->num_hdr_group_exact_match_profiles;
    if (!num_profiles || num_profiles > 3) {
        HAL_TRACE_ERR("Invalid no. of profiles: {}", num_profiles);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Setting masks
    memset(&rx_key1_mask, ~0, sizeof(rx_key1_swkey_mask_t));
    memset(&rx_key2_mask, ~0, sizeof(rx_key2_swkey_mask_t));
    memset(&rx_key3_mask, ~0, sizeof(rx_key3_swkey_mask_t));
    memset(&rx_key4_mask, ~0, sizeof(rx_key4_swkey_mask_t));

    // Forming Key1 for layer 1
    RX_KEY_LAYER_FORM_KEY_DATA(1);
    RX_KEY_LAYER_FORM_L1L2_TUNNEL_KEY_DATA(1);
    if (num_profiles > 1) {
        // Forming Key1 for layer 2
        RX_KEY_LAYER_FORM_KEY_DATA(2);
        RX_KEY_LAYER_FORM_L1L2_TUNNEL_KEY_DATA(2);
    }
    if (num_profiles > 2) {
        // Forming Key1 for layer 3
        RX_KEY_LAYER_FORM_KEY_DATA(3);
        RX_KEY_LAYER_FORM_L3_TUNNEL_KEY_DATA(3);
    }

    // Copying Key1 to Key2, Key3 and Key4
    RX_KEY_MEMCPY(2, 1);
    RX_KEY_MEMCPY(3, 1);
    RX_KEY_MEMCPY(4, 1);

    // Assigning action id
    RX_KEY_ACTIONID(1);
    RX_KEY_ACTIONID(2);
    RX_KEY_ACTIONID(3);
    RX_KEY_ACTIONID(4);

    // Programming Rx Keys
    RX_KEY_PGM(1);
    RX_KEY_PGM(2);
    RX_KEY_PGM(3);
    RX_KEY_PGM(4);

end:
    return ret;
}

#define TX_KEY_LAYER_FORM_KEY_DATA(LAYER)                                   \
    gft_hgmp = &gft_emp->hgem_profiles[LAYER - 1];                          \
    if (gft_hgmp->headers & GFT_HEADER_ETHERNET) {                          \
        tx_key1.ethernet_ ## LAYER ## _valid = 1;                           \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {       \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_ETHERNET_DST;                                         \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {       \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_ETHERNET_SRC;                                         \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_ETH_TYPE) {           \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_ETHERNET_TYPE;                                        \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {   \
            tx_key## LAYER ## _action.tx_key_action_u.tx_key_tx_key.        \
            match_fields |=                                                 \
                MATCH_CUSTOMER_VLAN_ID;                                     \
        }                                                                   \
    }                                                                       \
    if (gft_hgmp->headers & GFT_HEADER_IPV4) {                              \
        tx_key1.ipv4_ ## LAYER ## _valid = 1;                               \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {        \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_SRC;                                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {        \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_DST;                                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {            \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_DSCP;                                              \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {        \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_PROTO;                                             \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TTL) {                \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_TTL;                                               \
        }                                                                   \
    } else if (gft_hgmp->headers & GFT_HEADER_IPV6) {                       \
        tx_key1.ipv6_ ## LAYER ## _valid = 1;                               \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {        \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_SRC;                                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {        \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_DST;                                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_IP_DSCP) {            \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_DSCP;                                              \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {        \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_PROTO;                                             \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TTL) {                \
            tx_key ## LAYER ## _action.tx_key_action_u.tx_key_tx_key.       \
            match_fields |=                                                 \
                MATCH_IP_TTL;                                               \
        }                                                                   \
    }                                                                       \
    if (gft_hgmp->headers & GFT_HEADER_ICMP) {                              \
        tx_key1.icmp_ ## LAYER ## _valid = 1;                               \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_ICMP_TYPE) {          \
            tx_key1_action.tx_key_action_u.tx_key_tx_key.match_fields |=    \
            MATCH_ICMP_TYPE_1 << 16;                                        \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_ICMP_CODE) {          \
            tx_key1_action.tx_key_action_u.tx_key_tx_key.match_fields |=    \
            MATCH_ICMP_CODE_1 << 16;                                        \
        }                                                                   \
    } else if (gft_hgmp->headers & GFT_HEADER_TCP) {                        \
        tx_key1.tcp_ ## LAYER ## _valid = 1;                                \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) { \
            tx_key1_action.tx_key_action_u.tx_key_tx_key.match_fields |=    \
            MATCH_TRANSPORT_SRC_PORT_1 << 16;                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) { \
            tx_key1_action.tx_key_action_u.tx_key_tx_key.match_fields |=    \
            MATCH_TRANSPORT_DST_PORT_1 << 16;                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TCP_FLAGS) {          \
            tx_key1_action.tx_key_action_u.tx_key_tx_key.match_fields |=    \
            MATCH_TCP_FLAGS_1 << 16;                                        \
        }                                                                   \
    } else if (gft_hgmp->headers & GFT_HEADER_UDP) {                        \
        tx_key1.udp_ ## LAYER ## _valid = 1;                                \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) { \
            tx_key1_action.tx_key_action_u.tx_key_tx_key.match_fields |=    \
            MATCH_TRANSPORT_SRC_PORT_1 << 16;                               \
        }                                                                   \
        if (gft_hgmp->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) { \
            tx_key1_action.tx_key_action_u.tx_key_tx_key.match_fields |=    \
            MATCH_TRANSPORT_DST_PORT_1 << 16;                               \
        }                                                                   \
    }                                                                       

#define TX_KEY_PGM(TBL_ID)                                                  \
    key_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_TX_KEY);                  \
    sdk_ret = key_tbl->insert(&tx_key ## TBL_ID, &tx_key ## TBL_ID ## _mask,\
                              &tx_key ## TBL_ID ##_action, &tcam_idx);      \
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);                                  \
    if (ret == HAL_RET_DUP_INS_FAIL) {                                      \
        HAL_TRACE_ERR("Duplicate insert failure for tx_key ## TBL_ID ## "); \
        goto end;                                                           \
    } else if (ret != HAL_RET_OK) {                                         \
        HAL_TRACE_ERR("Failed to insert into tx key" #TBL_ID " tcam table"  \
                      "err : {}", ret);                                     \
        return ret;                                                         \
    }                                                                       \
    pd_gft_emp->tx_key ## TBL_ID ## _idx = tcam_idx;                        \
    HAL_TRACE_DEBUG("Programmed tx_key" #TBL_ID " at {}", tcam_idx);        \

//-----------------------------------------------------------------------------
// Program TX Key
//-----------------------------------------------------------------------------
static hal_ret_t
emp_pd_tx_keys_program_hw (pd_gft_emp_t *pd_gft_emp)
{
    hal_ret_t                              ret = HAL_RET_OK;
    sdk_ret_t                              sdk_ret;
    gft_exact_match_profile_t              *gft_emp = NULL;
    gft_hdr_group_exact_match_profile_t    *gft_hgmp = NULL;
    uint32_t                               num_profiles = 0;
    uint32_t                               tcam_idx = 0;
    tx_key_swkey_t                         tx_key1 = { 0 };
    tx_key_swkey_mask_t                    tx_key1_mask = { 0 };
    tx_key_actiondata                      tx_key1_action = { 0 };
    tcam                                   *key_tbl;

    gft_emp = (gft_exact_match_profile_t *)pd_gft_emp->pi_emp;

    if (!gft_emp) {
        HAL_TRACE_ERR("pi is null");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    num_profiles = gft_emp->num_hdr_group_exact_match_profiles;
    if (!num_profiles || num_profiles > 1) {
        HAL_TRACE_ERR("Invalid no. of profiles: {}", num_profiles);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Setting masks
    memset(&tx_key1_mask, ~0, sizeof(tx_key_swkey_mask_t));

    // Forming Key1 for layer 1
    TX_KEY_LAYER_FORM_KEY_DATA(1);
    // Assign action id
    tx_key1_action.actionid = TX_KEY_TX_KEY_ID;

    // Program Tx Keys
    TX_KEY_PGM(1);

end: 
    return ret;
}


//-----------------------------------------------------------------------------
// DeProgram HW
//-----------------------------------------------------------------------------
static hal_ret_t
emp_pd_deprogram_hw (pd_gft_emp_t *pd_gft_emp)
{
    hal_ret_t                               ret = HAL_RET_OK;
    return ret;
}

//-----------------------------------------------------------------------------
// Freeing emp PD. Frees PD memory and all other memories allocated for PD.
//-----------------------------------------------------------------------------
static hal_ret_t
emp_pd_free (pd_gft_emp_t *emp)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_GFT_EMP_PD, emp);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Freeing emp PD memory. Just frees the memory of PD structure.
//-----------------------------------------------------------------------------
static hal_ret_t
emp_pd_mem_free (pd_gft_emp_t *emp)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_GFT_EMP_PD, emp);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Linking PI <-> PD
//-----------------------------------------------------------------------------
static void 
link_pi_pd (pd_gft_emp_t *pd_gft_emp, gft_exact_match_profile_t *pi_emp)
{
    pd_gft_emp->pi_emp = pi_emp;
    pi_emp->pd = pd_gft_emp;
}

//-----------------------------------------------------------------------------
// Un-Linking PI <-> PD
//-----------------------------------------------------------------------------
static void 
delink_pi_pd (pd_gft_emp_t *pd_gft_emp, gft_exact_match_profile_t *pi_emp)
{
    pd_gft_emp->pi_emp = NULL;
    pi_emp->pd = NULL;
}

//-----------------------------------------------------------------------------
// Makes a clone
//-----------------------------------------------------------------------------
static hal_ret_t
pd_gft_emp_make_clone (pd_gft_exact_match_profile_make_clone_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_gft_emp_t        *pd_gft_emp_clone = NULL;

    gft_exact_match_profile_t *emp = args->exact_match_profile;
    gft_exact_match_profile_t *clone = args->clone;

    pd_gft_emp_clone = emp_pd_alloc_init();
    if (pd_gft_emp_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_gft_emp_clone, emp->pd, sizeof(pd_gft_emp_t));

    link_pi_pd(pd_gft_emp_clone, clone);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Frees PD memory without indexer free.
//-----------------------------------------------------------------------------
static hal_ret_t
pd_gft_emp_mem_free (pd_gft_exact_match_profile_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_gft_emp_t    *emp_pd;

    emp_pd = (pd_gft_emp_t *)args->exact_match_profile->pd;
    emp_pd_mem_free(emp_pd);

    return ret;
}

}    // namespace pd
}    // namespace hal
