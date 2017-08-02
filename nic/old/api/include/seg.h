// --------------------------------------------------------------------------------
// Warning: This is an Auto-generated file.
// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------
// SEG HAL API related public data structures and APIs
// --------------------------------------------------------------------------------
#ifndef __SEG_H__
#define __SEG_H__

#include <base.h>
#include <eth.h>

// --------------------------------------------------------------------------------
// SEG key
// --------------------------------------------------------------------------------
typedef struct seg_key_s { 
    uint16_t id;
} __PACK__ seg_key_t; 

// --------------------------------------------------------------------------------
// SEG Valid bits for key/handle
// --------------------------------------------------------------------------------
typedef struct seg_key_handle_s { 
    seg_key_t key;
    hal_handle_t hndl;
    uint8_t __key_valid:1;
    uint8_t __hndl_valid:1;
} __PACK__ seg_key_handle_t; 

// --------------------------------------------------------------------------------
// SEG Multiple Key Handles 
// --------------------------------------------------------------------------------
typedef struct seg_key_handles_s { 
    uint32_t num_khs;
    seg_key_handle_t khs[0];
} __PACK__ seg_key_handles_t; 

// --------------------------------------------------------------------------------
// SEG data. Valid bits for each field 
// --------------------------------------------------------------------------------
typedef struct seg_data_s { 
    uint8_t type;
    uint8_t __type_valid:1;

    vlan_id_t underlay_encap;
    uint8_t __underlay_encap_valid:1;

    encap_t overlay_encap;
    uint8_t __overlay_encap_valid:1;

} __PACK__ seg_data_t; 

// --------------------------------------------------------------------------------
// SEG operational state 
// --------------------------------------------------------------------------------
typedef struct seg_oper_s { 
    hal_ret_t      reason_code;
} __PACK__ seg_oper_t; 

// --------------------------------------------------------------------------------
// SEG Full Entry 
// --------------------------------------------------------------------------------
typedef struct seg_s { 
    seg_key_handle_t kh;
    seg_data_t data;
    seg_oper_t oper;
} __PACK__ seg_t; 

// --------------------------------------------------------------------------------
// SEG Full Entries 
// --------------------------------------------------------------------------------
typedef struct segs_s { 
    uint32_t num_entries;
    seg_t entries[0];
} __PACK__ segs_t; 

// --------------------------------------------------------------------------------
// SEG Filter structure
// --------------------------------------------------------------------------------
typedef struct seg_filter_s { 
    uint16_t id;
    uint8_t __id_valid:1;

    uint8_t type;
    uint8_t __type_valid:1;

    vlan_id_t underlay_encap;
    uint8_t __underlay_encap_valid:1;

    encap_t overlay_encap;
    uint8_t __overlay_encap_valid:1;

} __PACK__ seg_filter_t; 

// --------------------------------------------------------------------------------
// SEG CRUD APIs
// --------------------------------------------------------------------------------
hal_ret_t hal_create_seg(seg_t *segs __INOUT__);
hal_ret_t hal_read_seg(seg_key_handles_t *khs __IN__,
    seg_filter_t *filter __IN__,
    segs_t **segs __OUT__);
hal_ret_t hal_update_seg(seg_t *segs __IN__);
hal_ret_t hal_update_seg_by_key(seg_key_handles_t *khs,
    seg_data_t *data __IN__);
hal_ret_t hal_delete_seg(seg_key_handles_t *khs __IN__);

// --------------------------------------------------------------------------------
// SEG Key Handle Init
// --------------------------------------------------------------------------------
static inline void
hal_init_seg_key_handle (seg_key_handle_t *kh __INOUT__)
{
    kh->__key_valid = FALSE;
    kh->__hndl_valid = FALSE;
}

// --------------------------------------------------------------------------------
// SEG Key Handles Init
// --------------------------------------------------------------------------------
static inline void
hal_init_seg_key_handles (seg_key_handles_t *kh __INOUT__)
{
    khs->num_khs = 0;
}

// --------------------------------------------------------------------------------
// SEG Data Fields Init
// --------------------------------------------------------------------------------
static inline void
hal_init_seg_data (seg_data_t *data __INOUT__)
{
    data->__type_valid = FALSE;
    data->__underlay_encap_valid = FALSE;
    data->__overlay_encap_valid = FALSE;
}

// --------------------------------------------------------------------------------
// SEG Filter Fields Init
// --------------------------------------------------------------------------------
static inline void
hal_init_seg_filter (seg_filter_t *filter __INOUT__)
{
    filter->__id_valid = FALSE;
    filter->__type_valid = FALSE;
    filter->__underlay_encap_valid = FALSE;
    filter->__overlay_encap_valid = FALSE;
}

// --------------------------------------------------------------------------------
// SEG Full Entries Init
// --------------------------------------------------------------------------------
static inline void
hal_init_segs (segs_t *entries __INOUT__)
{
    entries->num_entries = 0;
}

// --------------------------------------------------------------------------------
// SEG Setting Key
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_key_handle_key (seg_key_handle_t *kh __INOUT__,
    uint16_t id __IN__)
{
    kh->key.id = id;
    kh->__key_valid = TRUE;
}

// --------------------------------------------------------------------------------
// SEG Setting Key Handle
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_key_handle_handle (seg_key_handle_t *kh __INOUT__,
    hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

// --------------------------------------------------------------------------------
// SEG Setting Data Field type
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_data_type (seg_data_t *data __INOUT__,
    uint8_t type __IN__)
{
    data->type = type;
    data->__type_valid = TRUE;
}

// --------------------------------------------------------------------------------
// SEG Setting Data Field underlay_encap
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_data_underlay_encap (seg_data_t *data __INOUT__,
    vlan_id_t underlay_encap __IN__)
{
    data->underlay_encap = underlay_encap;
    data->__underlay_encap_valid = TRUE;
}

// --------------------------------------------------------------------------------
// SEG Setting Data Field overlay_encap
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_data_overlay_encap (seg_data_t *data __INOUT__,
    encap_t overlay_encap __IN__)
{
    data->overlay_encap = overlay_encap;
    data->__overlay_encap_valid = TRUE;
}

// --------------------------------------------------------------------------------
// SEG Setting Filter Field id
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_filter_id (seg_filter_t *filter __INOUT__,
    uint16_t id __IN__)
{
    filter->id = id;
    filter->__id_valid = TRUE;
}

// --------------------------------------------------------------------------------
// SEG Setting Filter Field type
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_filter_type (seg_filter_t *filter __INOUT__,
    uint8_t type __IN__)
{
    filter->type = type;
    filter->__type_valid = TRUE;
}

// --------------------------------------------------------------------------------
// SEG Setting Filter Field underlay_encap
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_filter_underlay_encap (seg_filter_t *filter __INOUT__,
    vlan_id_t underlay_encap __IN__)
{
    filter->underlay_encap = underlay_encap;
    filter->__underlay_encap_valid = TRUE;
}

// --------------------------------------------------------------------------------
// SEG Setting Filter Field overlay_encap
// --------------------------------------------------------------------------------
static inline void
hal_set_seg_filter_overlay_encap (seg_filter_t *filter __INOUT__,
    encap_t overlay_encap __IN__)
{
    filter->overlay_encap = overlay_encap;
    filter->__overlay_encap_valid = TRUE;
}

#endif // __SEG_H__
