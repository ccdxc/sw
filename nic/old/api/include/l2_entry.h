//-----------------------------------------------------------------------------
// L2 Entry HAL API related public data structures and APIs
//-----------------------------------------------------------------------------
#ifndef __L2_ENTRY_H__
#define __L2_ENTRY_H__

#include <base.h>
#include <eth.h>

//------------------------------------------------------------------------------
// L2 entry key
//------------------------------------------------------------------------------
typedef struct l2_entry_key_s {
    uint32_t        segid;    // vlan/BD id or vnid id (aka. L2 segment id)
    mac_addr_t      mac;
} __PACK__ l2_entry_key_t;

//------------------------------------------------------------------------------
// TBD: valid bits for key/handle
//------------------------------------------------------------------------------
typedef struct l2_entry_key_handle_u {
    l2_entry_key_t        key;
    hal_handle_t          hndl;
    uint8_t               __key_valid:1;
    uint8_t               __hndl_valid:1;
} __PACK__ l2_entry_key_handle_t;

typedef struct l2_entry_key_handles_s {
    uint32_t                 num_khs;
    l2_entry_key_handle_t    khs[0];
} __PACK__ l2_entry_key_handles_t;

//------------------------------------------------------------------------------
// L2 entry data
// TBD: valid bits for each of the data fields
//------------------------------------------------------------------------------
typedef struct l2_entry_data_s {
    ifindex_t        ifindex;
    uint8_t          __ifindex_valid:1;
} __PACK__ l2_entry_data_t;

//------------------------------------------------------------------------------
// L2 entry operational state to be exposed to user or Ux
//     - should include counters, if any,
//     - any other events, timestamps etc.
//     - whatever helps in troubleshooting
//------------------------------------------------------------------------------
typedef struct l2_entry_oper_s {
    hal_ret_t      reason_code;
} __PACK__ l2_entry_oper_t;

//------------------------------------------------------------------------------
// Full L2 entry with key, data, operational state
//------------------------------------------------------------------------------
typedef struct l2_entry_s {
    l2_entry_key_handle_t    kh;
    l2_entry_data_t          data;
    l2_entry_oper_t          oper;
} __PACK__ l2_entry_t;

typedef struct l2_entries_s {
    uint32_t        num_entries;
    l2_entry_t      entries[0];
} __PACK__ l2_entries_t;

//------------------------------------------------------------------------------
// walk/query(s) filter for L2 entry Db
//------------------------------------------------------------------------------
typedef struct l2_entry_filter_s {
    uint32_t          segid;
    mac_addr_t        mac;
    ifindex_t         ifindex;
    vrf_id_t          vrf;       // <=== parent's parent !!
                                 // Can agent help here to breakdown per segid
                                 // here ??
    uint8_t           __segid_valid:1;
    uint8_t           __mac_valid:1;
    uint8_t           __ifindex_valid:1;
    uint8_t           __vrf_valid:1;
} __PACK__ l2_entry_filter_t;

//------------------------------------------------------------------------------
// L2 entry APIs                                                              //
//------------------------------------------------------------------------------

static inline void
hal_init_l2_entry_key_handle (l2_entry_key_handle_t *kh __INOUT__)
{
    // initialize L2 entry data with default values, if any
    kh->__key_valid = FALSE;
    kh->__hndl_valid = FALSE;
}

static inline void
hal_init_l2_entry_key_handles (l2_entry_key_handles_t *khs __INOUT__)
{
    khs->num_khs = 0;
}

static inline void
hal_init_l2_entry_data (l2_entry_data_t *data __INOUT__)
{
    data->__ifindex_valid = TRUE;
}

static inline void
hal_init_l2_entry_filter (l2_entry_filter_t *filter __INOUT__)
{
    filter->__segid_valid = FALSE;
    filter->__mac_valid = FALSE;
    filter->__ifindex_valid = FALSE;
    filter->__vrf_valid = FALSE;
}

static inline void
hal_init_l2_entries (l2_entries_t *entries __INOUT__)
{
    entries->num_entries = 0;
}

static inline void
hal_set_l2_entry_key_handle_key (l2_entry_key_handle_t *kh __INOUT__,
                                 uint32_t segid __IN__, mac_addr_t mac __IN__)
{
    kh->key.segid = segid;
    memcpy(kh->key.mac , mac, ETH_ADDR_LEN);
    kh->__key_valid = TRUE;
}

static inline void
hal_set_l2_entry_key_handle_handle (l2_entry_key_handle_t *kh __INOUT__,
                                    hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

static inline void
hal_set_l2_entry_data_ifindex (l2_entry_data_t *data __INOUT__,
                               ifindex_t ifindex __IN__)
{
    data->ifindex = ifindex;
    data->__ifindex_valid = TRUE;
}

static inline void
hal_set_l2_entry_filter_segid (l2_entry_filter_t *filter __INOUT__,
                               uint32_t segid __IN__)
{
    filter->segid = segid;
    filter->__segid_valid = TRUE;
}

static inline void
hal_set_l2_entry_filter_mac (l2_entry_filter_t *filter __INOUT__,
                             mac_addr_t mac __IN__)
{
    memcpy(filter->mac, mac, ETH_ADDR_LEN);
    filter->__mac_valid = TRUE;
}

static inline void
hal_set_l2_entry_filter_ifindex (l2_entry_filter_t *filter __INOUT__,
                                 uint32_t ifindex __IN__)
{
    filter->ifindex = ifindex;
    filter->__ifindex_valid = TRUE;
}

static inline void
hal_set_l2_entry_filter_vrf (l2_entry_filter_t *filter __INOUT__,
                             vrf_id_t vrf __IN__)
{
    filter->vrf = vrf;
    filter->__vrf_valid = TRUE;
}

static inline void
hal_free_l2_entries (l2_entries_t *entries __IN__)
{
    // TODO: free to respective slab
}

//------------------------------------------------------------------------------
// CRUD APIs
//------------------------------------------------------------------------------
hal_ret_t hal_create_l2_entry(l2_entries_t *l2_entries __INOUT__);    // handles filled by API
hal_ret_t hal_read_l2_entry(l2_entry_key_handles_t *khs __IN__,
                            l2_entry_filter_t *filter __IN__,
                            l2_entries_t **l2_entries __OUT__);       // memory freed by caller
                                                                      // using free_entries() API
hal_ret_t hal_update_l2_entry(l2_entries_t *l2_entries __IN__);       // 'oper' is ignored
hal_ret_t hal_update_l2_entry_by_key(l2_entry_key_handles_t *khs,
                              l2_entry_data_t *data __IN__);       // 'oper' is ignored
hal_ret_t hal_delete_l2_entry(l2_entry_key_handles_t *khs __IN__);

#endif    // __L2_ENTRY_H__

