//-----------------------------------------------------------------------------
// VLAN HAL API related public data structures and APIs
//-----------------------------------------------------------------------------
#ifndef __VLAN_H__
#define __VLAN_H__

#include <eth.h>

#if 0
#include <ip.hpp>

// forward declaration
typedef struct ip_prefix_elem_s ip_prefix_elem_t;
typedef struct ip_prefix_elem_s {
    ip_prefix_t         ip_prefix;
    ip_prefix_elem_t    *next;
} __PACK__ ip_prefix_elem_t;
#endif

//------------------------------------------------------------------------------
// vlan information
// TBD - capture all the features that can be enabled/disabled on this
//------------------------------------------------------------------------------
//typedef struct vlan_key_t    vlan_id_t;    // TODO: shouldn't this be vnid ?

typedef uint32_t encap_t;
typedef uint32_t vlan_key_t;

typedef union vlan_key_handle_u {
    vlan_key_t          key;
    hal_handle_t        hndl;
    uint8_t             __key_valid:1;
    uint8_t             __hndl_valid:1;
} __PACK__ vlan_key_handle_t;

typedef struct vlan_key_handles_s {
    uint32_t                 num_khs;
    vlan_key_handle_t        khs[0];
} __PACK__ vlan_key_handles_t;

typedef struct vlan_data_s {
    uint8_t        vlan_type;       // tenant or infra etc.
    encap_t        wire_encap;
    encap_t        fabric_encap;
    uint8_t        __vlan_type_valid:1;
    uint8_t        __wire_encap_valid:1;
    uint8_t        __fabric_encap_valid:1;
} __PACK__ vlan_data_t;

//------------------------------------------------------------------------------
// user visible vlan operational data
//------------------------------------------------------------------------------
typedef struct vlan_oper_s {
    hal_ret_t      reason_code;
} vlan_oper_t;

typedef struct vlan_s {
    vlan_key_handles_t        kh;
    vlan_data_t               data;
    vlan_oper_t               oper;
} __PACK__ vlan_t;

typedef struct vlans_s {
    uint32_t        num_vlans;
    vlan_t          vlans[0];
} __PACK__ vlans_t;

//------------------------------------------------------------------------------
// walk/query(s) filter for vlan db
//------------------------------------------------------------------------------
typedef struct vlan_filter_s {
    uint8_t        type;
    // TODO: can add vrf, encaps too !, e.g., want to know all VLANs in a VRF
    uint8_t        __type_valid:1;
} __PACK__ vlan_filter_t;

//------------------------------------------------------------------------------
// VLAN APIs
//------------------------------------------------------------------------------
// helper functions
static inline void
hal_init_vlan_key_handle (vlan_key_handle_t *kh __INOUT__)
{
    kh->__key_valid = FALSE;
    kh->__hndl_valid = FALSE;
}

static inline void
hal_init_vlan_key_handles (vlan_key_handles_t *khs __INOUT__)
{
    khs->num_khs = 0;
}

static inline void
hal_init_vlan_data (vlan_data_t *data __INOUT__)
{
    data->__vlan_type_valid = FALSE;
    data->__wire_encap_valid = FALSE;
    data->__fabric_encap_valid = FALSE;
}

static inline void
hal_init_vlan_filter (vlan_filter_t *filter __INOUT__)
{
    filter->__type_valid = FALSE;
}

static inline void
hal_init_vlans (vlans_t *vlans __INOUT__)
{
    vlans->num_vlans = 0;
}

static inline void
hal_set_vlan_key_handle_key (vlan_key_handle_t *kh __INOUT__,
                             vlan_key_t vlan __IN__)
{
    kh->key = vlan;
    kh->__key_valid = TRUE;
}

static inline void
hal_set_vlan_key_handle_handle (vlan_key_handle_t *kh __INOUT__,
                                hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

static inline void
hal_set_vlan_data_vlan_type (vlan_data_t *data __INOUT__,
                             uint8_t vlan_type __IN__)
{
    data->vlan_type = vlan_type;
    data->__vlan_type_valid = TRUE;
}

static inline void
hal_set_vlan_data_wire_encap (vlan_data_t *data __INOUT__,
                              encap_t wire_encap __IN__)
{
    data->wire_encap = wire_encap;
    data->__wire_encap_valid = TRUE;
}

static inline void
hal_set_vlan_data_fabric_encap (vlan_data_t *data __INOUT__,
                                encap_t fabric_encap __IN__)
{
    data->fabric_encap = fabric_encap;
    data->__fabric_encap_valid = TRUE;
}

static inline void
hal_set_vlan_filter_vlan_type (vlan_filter_t *filter __INOUT__,
                               uint8_t type __IN__)
{
    filter->type = type;
    filter->__type_valid = TRUE;
}

static inline void
hal_free_vlans (vlans_t *entries __IN__)
{
    // TODO: free to respective slab
}

// CRUD APIs
hal_ret_t hal_create_vlan(vlans_t *vlans __INOUT__);    // handles filled by API
hal_ret_t hal_retrieve_vlan(vlan_key_handles_t *khs __IN__,
                            vlan_filter_t *filter __IN__,
                            vlans_t **vlans __OUT__);   // memory freed by caller
                                                        // using free_entries() API
hal_ret_t hal_update_vlan(vlans_t *vlans __IN__);       // 'oper' is ignored
hal_ret_t hal_delete_vlan(vlan_key_handles_t *khs __IN__);

#endif    // __VLAN_H__

