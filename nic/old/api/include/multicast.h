#ifndef __MULTICAST_H__
#define __MULTICAST_H__

#include <base.h>

#define MULTICAST_GROUP_ID_INVALID 0

#define HAL_MULTICAST_INVALID_REPL_ID 0

typedef uint32_t multicast_group_t;
typedef uint32_t nexthop_t;
typedef uint32_t multicast_repl_id_t;


typedef enum {
    HAL_REPL_ENCAP_TYPE_VLAN = 1,
    HAL_REPL_ENCAP_TYPE_VXLAN,
    HAL_REPL_ENCAP_TYPE_QPID
} encap_type_t;
   
typedef struct multicast_member_s {
    ifindex_t phy_ifindex;
    ifindex_t if_index;
    // if_encap_t if_encap; //encap type and value ??
    encap_type_t encap_type;
    encap_id_t encap_id;
    // in case L3 mcast is required - will need nexthop
    //nexthop_t nexthop_id;
} __PACK__ replication_member_t;

typedef struct multicast_group_key_s {
    vrf_id_t vrf;
    seg_id_t segid;
    //group-ip
    ip_addr_t ip_addr; 
    // should I add SIP also for igmpv3 ??
} __PACK__ multicast_group_key_t;

typedef struct multicast_group_key_handle_u {
    multicast_group_key_t key;
    hal_handle_t hndl;
    uint8_t               __key_valid:1;
    uint8_t               __hndl_valid:1;
} __PACK__ multicast_group_key_handle_t;

typedef struct multicast_group_key_handles_s {
    uint32_t                 num_khs;
    multicast_group_key_handle_t    khs[0];
} __PACK__ multicast_group_key_handles_t;

typedef struct multicast_group_data_s {
    multicast_group_t        multicast_group_id;
    multicast_repl_id_t      repl_id;
    uint32_t            num_multicast_members;
    replication_member_t       multicast_members[0];
    uint8_t          __multicast_group_id_valid:1;
} __PACK__ multicast_group_data_t;

typedef struct multicast_group_oper_s {
    hal_ret_t      reason_code;
} __PACK__ multicast_group_oper_t;


typedef struct multicast_group_entry_s {
    multicast_group_key_handle_t    kh;
    multicast_group_data_t          data;
    multicast_group_oper_t          oper;
} __PACK__ multicast_group_entry_t;

typedef struct multicast_group_entries_s {
    uint32_t        num_entries;
    multicast_group_entry_t      entries[0];
} __PACK__ multicast_group_entries_t;

typedef struct multicast_group_filter_s {
    uint16_t multicast_group_id;

    uint8_t __multicast_group_id_valid:1;
} __PACK__ multicast_group_filter_t;

static inline void
hal_init_multicast_group_key_handle (multicast_group_key_handle_t *kh __INOUT__)
{
    kh->__hndl_valid = FALSE;
}

static inline void 
hal_init_multicast_group_key (multicast_group_key_handle_t *kh __INOUT__)
{
    kh->__key_valid = FALSE;
    kh->key.vrf = 0;
    kh->key.segid = 0;
    memset(&kh->key.ip_addr, 0, sizeof(ip_addr_t));
}

static inline void
hal_init_multicast_group_key_handles (multicast_group_key_handles_t *khs __INOUT__)
{
    khs->num_khs = 0;
}

static inline void
hal_init_multicast_group_data (multicast_group_data_t *data __INOUT__)
{
    data->multicast_group_id = MULTICAST_GROUP_ID_INVALID;
    data->num_multicast_members = 0;
}

static inline void
hal_init_multicast_group_filter (multicast_group_filter_t *filter __INOUT__)
{
    filter->multicast_group_id = MULTICAST_GROUP_ID_INVALID;
}

static inline void
hal_init_multicast_group_entries (multicast_group_entries_t *entries __INOUT__)
{
    entries->num_entries = 0;
}

static inline void
hal_set_multicast_group_key_handle_handle (multicast_group_key_handle_t *kh __INOUT__,
                                    hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

static inline void
hal_set_multicast_group_data(multicast_group_data_t *data __INOUT__,
                               multicast_group_t multicast_group_id __IN__,
                               uint32_t num_multicast_members __IN__,
                               replication_member_t *multicast_members __IN__)
{
    data->multicast_group_id = multicast_group_id;
    data->__multicast_group_id_valid = TRUE;
    data->num_multicast_members = num_multicast_members;
    memcpy(data->multicast_members, multicast_members, sizeof(replication_member_t) * num_multicast_members);
}
// CRUD
hal_ret_t hal_create_multicast_group_entry(multicast_group_entries_t *multicast_group_entries __INOUT__);    // handles filled by API
hal_ret_t hal_read_multicast_group_entry(multicast_group_key_handles_t *khs __IN__,
                            multicast_group_filter_t *filter __IN__,
                            multicast_group_entries_t **multicast_group_entries __OUT__);       // memory freed by caller
                                                                      // using free_entries() API
hal_ret_t hal_update_multicast_group_entry(multicast_group_entries_t *multicast_group_entries __IN__);       // 'oper' is ignored
hal_ret_t hal_update_multicast_group_entry_by_key(multicast_group_key_handles_t *khs,
                              multicast_group_data_t *data __IN__);       // 'oper' is ignored
hal_ret_t hal_delete_multicast_group_entry(multicast_group_key_handles_t *khs __IN__);

//MULTICAST group member add and delete will be added only later if its really required.
//For now the assumption is that upper layer will do a get, do an add/delete and then update.

typedef struct replication_key_s {
    multicast_repl_id_t repl_id;
} __PACK__ replication_key_t;

typedef struct replication_key_handle_s {
    replication_key_t key;
    hal_handle_t      hndl;
    uint8_t           __key_valid:1;
    uint8_t               __hndl_valid:1;
} __PACK__ replication_key_handle_t;

typedef struct replication_key_handles_s {
    uint32_t                 num_khs;
    replication_key_handle_t khs[0];
} __PACK__ replication_key_handles_t;

typedef struct replication_data_s {
    multicast_repl_id_t repl_id;
    uint16_t num_repl_members;
    replication_member_t repl_mbrs[0];
} __PACK__ replication_data_t;

typedef struct replication_oper_s {
    hal_ret_t      reason_code;
} __PACK__ replication_oper_t;

typedef struct replication_filter_s {
    multicast_repl_id_t repl_id;
    uint8_t             __repl_id_valid:1;
} replication_filter_t;

typedef struct replication_entry_s {
    replication_key_handle_t  kh;
    replication_data_t        data;
    replication_oper_t        oper;
} __PACK__ replication_entry_t;

typedef struct replication_entries_s {
    uint32_t        num_entries;
    replication_entry_t entries[0];
} __PACK__ replication_entries_t;

hal_ret_t hal_create_replication_entry(replication_entries_t *repl_entries __INOUT__);
hal_ret_t hal_read_replication_entry(replication_key_handles_t *khs __IN__,
                                     replication_filter_t *filter __IN__,
                                     replication_entries_t **replication_entries __OUT__);
hal_ret_t hal_update_replication_entry(replication_entries_t *repl_entries __IN__);
hal_ret_t hal_delete_replication_entry(replication_key_handles_t *khs __IN__);
hal_ret_t hal_add_replication_member(replication_key_handle_t *kh __IN__, replication_member_t *mbr __IN__);
hal_ret_t hal_delete_replication_member(replication_key_handle_t *kh __IN__, replication_member_t *mbr __IN__);

 
#endif // __MULTICAST_H__
