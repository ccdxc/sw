#ifndef __L3_H__
#define __L3_H__

#include <base.h>
#include <ip.h>

#define MAX_VRF_NAME_LEN                             32
#define ECMP_GROUP_ID_INVALID 0
#define NEXTHOP_ID_INVALID    0
#define VRF_ID_INVALID        0
#define PREFIX_LEN_INVALID    255
#define EGRESS_PORT_INVALID   0
#define IFINDEX_INVALID       0


typedef uint32_t nexthop_t;
typedef uint32_t ecmp_group_t;

typedef enum {
    HAL_NEXT_HOP_TYPE_NONE = 0,
    HAL_NEXT_HOP_TYPE_SIMPLE = 1,
    HAL_NEXT_HOP_TYPE_ECMP = 2
} nexthop_type_t;

//VRF
typedef struct l3_vrf_key_s {
    uint8_t vrf_name[MAX_VRF_NAME_LEN];
} __PACK__ l3_vrf_key_t;

typedef struct l3_vrf_key_handle_u {
    l3_vrf_key_t key;
    hal_handle_t hndl;
    uint8_t               __key_valid:1;
    uint8_t               __hndl_valid:1;
} __PACK__ l3_vrf_key_handle_t;

typedef struct l3_vrf_key_handles_s {
    uint32_t                 num_khs;
    l3_vrf_key_handle_t    khs[0];
} __PACK__ l3_vrf_key_handles_t;

typedef struct l3_vrf_data_s {
} __PACK__ l3_vrf_data_t;

typedef struct l3_vrf_oper_s {
    hal_ret_t      reason_code;
} __PACK__ l3_vrf_oper_t;

typedef struct l3_vrf_entry_s {
    l3_vrf_key_handle_t kh;
    l3_vrf_data_t       data;
    l3_vrf_oper_t       oper;
} __PACK__ l3_vrf_entry_t;

typedef struct l3_vrf_entries_s {
    uint32_t        num_entries;
    l3_vrf_entry_t  entries[0];
} __PACK__ l3_vrf_entries_t;

typedef struct l3_vrf_filter_s {
    uint8_t vrf_name[MAX_VRF_NAME_LEN];
    uint16_t vrf_id;

    uint8_t __vrf_name_valid:1;
    uint8_t __vrf_id_valid:1;
} __PACK__ l3_vrf_filter_t;


//L3 VRF entry APIs
static inline void
hal_init_l3_vrf_key_handle (l3_vrf_key_handle_t *kh __INOUT__)
{
    kh->__key_valid = FALSE;
    kh->__hndl_valid = FALSE;
}

static inline void
hal_init_l3_vrf_key_handles (l3_vrf_key_handles_t *khs __INOUT__)
{
    khs->num_khs = 0;
}

static inline void
hal_init_l3_vrf_data (l3_vrf_data_t *data __INOUT__)
{
}

static inline void
hal_init_l3_vrf_filter (l3_vrf_filter_t *filter __INOUT__)
{
    filter->vrf_id = VRF_ID_INVALID;
    memset(filter->vrf_name, 0, MAX_VRF_NAME_LEN);
}

static inline void
hal_init_l3_vrf_entries (l3_vrf_entries_t *entries __INOUT__)
{
    entries->num_entries = 0;
}

static inline void
hal_set_l3_vrf_key_handle_key (l3_vrf_key_handle_t *kh __INOUT__,
                                 uint8_t *vrf_name __IN__)
{
    memcpy(kh->key.vrf_name, vrf_name, MAX_VRF_NAME_LEN);
    kh->__key_valid = TRUE;
}

static inline void
hal_set_l3_vrf_key_handle_handle (l3_vrf_key_handle_t *kh __INOUT__,
                                    hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

static inline void
hal_set_l3_vrf_data_none(l3_vrf_data_t *data __INOUT__)
{

}

hal_ret_t hal_create_l3_vrf_entry(l3_vrf_entries_t *l3_vrf_entries __INOUT__);    // handles filled by API
hal_ret_t hal_read_l3_vrf_entry(l3_vrf_key_handles_t *khs __IN__,
                            l3_vrf_filter_t *filter __IN__,
                            l3_vrf_entries_t **l3_vrf_entries __OUT__);       // memory freed by caller
                                                                      // using free_entries() API
hal_ret_t hal_update_l3_vrf_entry(l3_vrf_entries_t *l3_vrf_entries __IN__);       // 'oper' is ignored
hal_ret_t hal_update_l3_vrf_entry_by_key(l3_vrf_key_handles_t *khs,
                              l3_vrf_data_t *data __IN__);       // 'oper' is ignored
hal_ret_t hal_delete_l3_vrf_entry(l3_vrf_key_handles_t *khs __IN__);

//Route 
typedef struct l3_route_entry_key_s {
    uint16_t vrf_id;
    ip_prefix_t ip_prefix; // has prefix-length inside.
} __PACK__ l3_route_entry_key_t;

typedef struct l3_route_entry_key_handle_u {
    l3_route_entry_key_t key;
    hal_handle_t hndl;
    uint8_t               __key_valid:1;
    uint8_t               __hndl_valid:1;
} __PACK__ l3_route_entry_key_handle_t;

typedef struct l3_entry_route_key_handles_s {
    uint32_t                 num_khs;
    l3_route_entry_key_handle_t    khs[0];
} __PACK__ l3_route_entry_key_handles_t;

typedef struct l3_route_entry_data_s {
    nexthop_type_t   nexthop_type;
    nexthop_t        nexthop_id;
    uint8_t          __nexthop_id_valid:1;
} __PACK__ l3_route_entry_data_t;

typedef struct l3_route_entry_oper_s {
    hal_ret_t      reason_code;
} __PACK__ l3_route_entry_oper_t;


typedef struct l3_route_entry_s {
    l3_route_entry_key_handle_t    kh;
    l3_route_entry_data_t          data;
    l3_route_entry_oper_t          oper;
} __PACK__ l3_route_entry_t;

typedef struct l3_route_entries_s {
    uint32_t        num_entries;
    l3_route_entry_t      entries[0];
} __PACK__ l3_route_entries_t;

#if 0
typedef struct l3_route_entry_khd_s {
    l3_route_entry_key_handle_t    kh;
    l3_route_entry_data_t          data;
    l3_route_entry_oper_t          oper;
} __PACK__ l3_route_entry_khd_t;

typedef struct l3_route_entry_khds_s {
    uint32_t              num_khds;
    l3_route_entry_khd_t        khds[0];
} __PACK__ l3_route_entry_khds_t;
#endif

typedef struct l3_route_entry_filter_s {
    uint16_t vrf_id;
    uint16_t nexthop_id; //routes pointing to this next-hop
    uint8_t prefix_len; //routes with this prefix-length

    uint8_t __vrf_id_valid:1;
    uint8_t __nexthop_id_valid:1;
    uint8_t __prefix_len_valid:1;
} __PACK__ l3_route_entry_filter_t;


//L3 Route entry APIs
static inline void
hal_init_l3_route_entry_key_handle (l3_route_entry_key_handle_t *kh __INOUT__)
{
    kh->__key_valid = FALSE;
    kh->__hndl_valid = FALSE;
}

static inline void
hal_init_l3_route_entry_key_handles (l3_route_entry_key_handles_t *khs __INOUT__)
{
    khs->num_khs = 0;
}

static inline void
hal_init_l3_route_entry_data (l3_route_entry_data_t *data __INOUT__)
{
    data->nexthop_id = NEXTHOP_ID_INVALID;
}

static inline void
hal_init_l3_route_entry_filter (l3_route_entry_filter_t *filter __INOUT__)
{
    filter->vrf_id = VRF_ID_INVALID;
    filter->nexthop_id = NEXTHOP_ID_INVALID;
    filter->prefix_len = PREFIX_LEN_INVALID;
}

static inline void
hal_init_l3_route_entries (l3_route_entries_t *entries __INOUT__)
{
    entries->num_entries = 0;
}

static inline void
hal_set_l3_route_entry_key_handle_key (l3_route_entry_key_handle_t *kh __INOUT__,
                                 uint32_t vrf_id __IN__, ip_prefix_t ip_prefix __IN__)
{
    kh->key.vrf_id = vrf_id;
    kh->key.ip_prefix = ip_prefix;
    kh->__key_valid = TRUE;
}

static inline void
hal_set_l3_route_entry_key_handle_handle (l3_route_entry_key_handle_t *kh __INOUT__,
                                    hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

static inline void
hal_set_l3_route_entry_data_nexthop(l3_route_entry_data_t *data __INOUT__,
                               nexthop_t nexthop_id __IN__)
{
    data->nexthop_id = nexthop_id;
    data->__nexthop_id_valid = TRUE;
}

hal_ret_t hal_create_l3_route_entry(l3_route_entries_t *l3_route_entries __INOUT__);    // handles filled by API
hal_ret_t hal_read_l3_route_entry(l3_route_entry_key_handles_t *khs __IN__,
                            l3_route_entry_filter_t *filter __IN__,
                            l3_route_entries_t **l3_route_entries __OUT__);       // memory freed by caller
                                                                      // using free_entries() API
hal_ret_t hal_update_l3_route_entry(l3_route_entries_t *l3_route_entries __IN__);       // 'oper' is ignored
hal_ret_t hal_update_l3_route_entry_by_key(l3_route_entry_key_handles_t *khs,
                              l3_route_entry_data_t *data __IN__);       // 'oper' is ignored
hal_ret_t hal_delete_l3_route_entry(l3_route_entry_key_handles_t *khs __IN__);

//Nexthop
typedef struct l3_nexthop_key_s {
    uint16_t vrf_id;
    ip_addr_t ip_addr;
} __PACK__ l3_nexthop_key_t;

typedef struct l3_nexthop_key_handle_u {
    uint16_t vrf_id;
    ip_addr_t ip_addr;
    hal_handle_t hndl;
    uint8_t               __key_valid:1;
    uint8_t               __hndl_valid:1;
} __PACK__ l3_nexthop_key_handle_t;

typedef struct l3_nexthop_key_handles_s {
    uint32_t                 num_khs;
    l3_nexthop_key_handle_t    khs[0];
} __PACK__ l3_nexthop_key_handles_t;

typedef struct l3_nexthop_data_s {
    ifindex_t      ifindex; //includes tunnels
    uint32_t       egress_port; //can it be derived from above ??
    mac_addr_t     dst_mac_addr;
} __PACK__ l3_nexthop_data_t;

typedef struct l3_nexthop_oper_s {
    hal_ret_t      reason_code;
} __PACK__ l3_nexthop_oper_t;


typedef struct l3_nexthop_entry_s {
    l3_nexthop_key_handle_t    kh;
    l3_nexthop_data_t          data;
    l3_nexthop_oper_t          oper;
} __PACK__ l3_nexthop_entry_t;

typedef struct l3_nexthop_entries_s {
    uint32_t        num_entries;
    l3_nexthop_entry_t      entries[0];
} __PACK__ l3_nexthop_entries_t;
 
typedef struct l3_nexthop_filter_s {
    uint16_t nexthop_id;

    uint8_t __nexthop_id_valid:1;
} __PACK__ l3_nexthop_filter_t;

static inline void
hal_init_l3_nexthop_key_handle (l3_nexthop_key_handle_t *kh __INOUT__)
{
    kh->__key_valid = FALSE;
    kh->__hndl_valid = FALSE;
}

static inline void
hal_init_l3_nexthop_key_handles (l3_nexthop_key_handles_t *khs __INOUT__)
{
    khs->num_khs = 0;
}

static inline void
hal_init_l3_nexthop_data (l3_nexthop_data_t *data __INOUT__)
{
    data->ifindex = IFINDEX_INVALID;
    data->egress_port = EGRESS_PORT_INVALID;
}

static inline void
hal_init_l3_nexthop_filter (l3_nexthop_filter_t *filter __INOUT__)
{
    filter->nexthop_id = NEXTHOP_ID_INVALID;
}

static inline void
hal_init_l3_nexthop_entries (l3_nexthop_entries_t *entries __INOUT__)
{
    entries->num_entries = 0;
}

static inline void
hal_set_l3_nexthop_key_handle_handle (l3_nexthop_key_handle_t *kh __INOUT__,
                                    hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

static inline void
hal_set_l3_nexthop_data_nexthop(l3_nexthop_data_t *data __INOUT__,
                               ifindex_t ifindex __IN__, 
                               uint32_t egress_port __IN__,
                               mac_addr_t dstmac __IN__)
{
    data->ifindex = ifindex;
    data->egress_port = egress_port;
    memcpy(data->dst_mac_addr, dstmac, ETH_ADDR_LEN);
}

hal_ret_t hal_create_nexthop_entry(l3_nexthop_entries_t *l3_nexthop_entries __INOUT__);    // handles filled by API
hal_ret_t hal_read_l3_nexthop_entry(l3_nexthop_key_handles_t *khs __IN__,
                            l3_nexthop_filter_t *filter __IN__,
                            l3_nexthop_entries_t **l3_nexthop_entries __OUT__);       // memory freed by caller
                                                                      // using free_entries() API
hal_ret_t hal_update_l3_nexthop_entry(l3_nexthop_entries_t *l3_nexthop_entries __IN__);       // 'oper' is ignored
hal_ret_t hal_update_l3_nexthop_entry_by_key(l3_nexthop_key_handles_t *khs,
                              l3_nexthop_data_t *data __IN__);       // 'oper' is ignored
hal_ret_t hal_delete_l3_nexthop_entry(l3_nexthop_key_handles_t *khs __IN__);


//ECMP

typedef struct ecmp_member_s {
    l3_nexthop_entry_t nexthop;
} __PACK__ ecmp_member_t;

typedef struct l3_ecmp_group_key_s {
    hal_handle_t hndl;
} __PACK__ l3_ecmp_group_key_t;

typedef struct l3_ecmp_group_key_handle_u {
    hal_handle_t hndl;
    uint8_t               __hndl_valid:1;
} __PACK__ l3_ecmp_group_key_handle_t;

typedef struct l3_ecmp_group_key_handles_s {
    uint32_t                 num_khs;
    l3_ecmp_group_key_handle_t    khs[0];
} __PACK__ l3_ecmp_group_key_handles_t;

typedef struct l3_ecmp_group_data_s {
    ecmp_group_t        ecmp_group_id;
    uint32_t            num_ecmp_members;
    ecmp_member_t       ecmp_members[0];
    uint8_t          __ecmp_group_id_valid:1;
} __PACK__ l3_ecmp_group_data_t;

typedef struct l3_ecmp_group_oper_s {
    hal_ret_t      reason_code;
} __PACK__ l3_ecmp_group_oper_t;


typedef struct l3_ecmp_group_entry_s {
    l3_ecmp_group_key_handle_t    kh;
    l3_ecmp_group_data_t          data;
    l3_ecmp_group_oper_t          oper;
} __PACK__ l3_ecmp_group_entry_t;

typedef struct l3_ecmp_group_entries_s {
    uint32_t        num_entries;
    l3_ecmp_group_entry_t      entries[0];
} __PACK__ l3_ecmp_group_entries_t;
 
typedef struct l3_ecmp_group_filter_s {
    uint16_t ecmp_group_id;

    uint8_t __ecmp_group_id_valid:1;
} __PACK__ l3_ecmp_group_filter_t;

static inline void
hal_init_l3_ecmp_group_key_handle (l3_ecmp_group_key_handle_t *kh __INOUT__)
{
    kh->__hndl_valid = FALSE;
}

static inline void
hal_init_l3_ecmp_group_key_handles (l3_ecmp_group_key_handles_t *khs __INOUT__)
{
    khs->num_khs = 0;
}

static inline void
hal_init_l3_ecmp_group_data (l3_ecmp_group_data_t *data __INOUT__)
{
    data->ecmp_group_id = ECMP_GROUP_ID_INVALID;
    data->num_ecmp_members = 0;
}

static inline void
hal_init_l3_ecmp_group_filter (l3_ecmp_group_filter_t *filter __INOUT__)
{
    filter->ecmp_group_id = ECMP_GROUP_ID_INVALID;
}

static inline void
hal_init_l3_ecmp_group_entries (l3_ecmp_group_entries_t *entries __INOUT__)
{
    entries->num_entries = 0;
}

static inline void
hal_set_l3_ecmp_group_key_handle_handle (l3_ecmp_group_key_handle_t *kh __INOUT__,
                                    hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

static inline void
hal_set_l3_ecmp_group_data(l3_ecmp_group_data_t *data __INOUT__,
                               ecmp_group_t ecmp_group_id __IN__,
                               uint32_t num_ecmp_members __IN__,
                               ecmp_member_t *ecmp_members __IN__)
{
    data->ecmp_group_id = ecmp_group_id;
    data->__ecmp_group_id_valid = TRUE;
    data->num_ecmp_members = num_ecmp_members;
    memcpy(data->ecmp_members, ecmp_members, sizeof(ecmp_member_t) * num_ecmp_members);
}
// CRUD
hal_ret_t hal_create_ecmp_group_entry(l3_ecmp_group_entries_t *l3_ecmp_group_entries __INOUT__);    // handles filled by API
hal_ret_t hal_read_l3_ecmp_group_entry(l3_ecmp_group_key_handles_t *khs __IN__,
                            l3_ecmp_group_filter_t *filter __IN__,
                            l3_ecmp_group_entries_t **l3_ecmp_group_entries __OUT__);       // memory freed by caller
                                                                      // using free_entries() API
hal_ret_t hal_update_l3_ecmp_group_entry(l3_ecmp_group_entries_t *l3_ecmp_group_entries __IN__);       // 'oper' is ignored
hal_ret_t hal_update_l3_ecmp_group_entry_by_key(l3_ecmp_group_key_handles_t *khs,
                              l3_ecmp_group_data_t *data __IN__);       // 'oper' is ignored
hal_ret_t hal_delete_l3_ecmp_group_entry(l3_ecmp_group_key_handles_t *khs __IN__);

//ECMP group member add and delete will be added only later if its really required.
//For now the assumption is that upper layer will do a get, do an add/delete and then update.

#endif    // __L3_H__
