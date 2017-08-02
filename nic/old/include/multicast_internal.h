#ifndef __MULTICAST_INTERNAL_H__
#define __MULTICAST_INTERNAL_H__

#include <crc32.h>
#include <bitmap.h>
#include <ip.h>
#include <multicast.h>
#include <tommytypes.h>
#include <tommyhashtbl.h>


#define MULTICAST_REPL_TBL_SIZE (64 * 1024)
#define MULTICAST_TBL_KEY_SIZE 22
#define MULTICAST_HDL_TBL_KEY_SIZE 4
#define MULTICAST_REPL_INVALID_HDL_ID 0

#define MULTICAST_INVALID_HANDLE_ID 0xffffffff
#define REPLICATION_INVALID_HANDLE_ID 0xffffffff

#define REPLICATION_KEY_SIZE 4

#define HAL_INTERNAL_MCAST_CRC32_HASH_SEED 0x33335555

typedef struct multicast_table_entry_s {
    uint8_t key[MULTICAST_TBL_KEY_SIZE];
    hal_handle_t hndl;
    tommy_hashtable_node node;
    multicast_group_key_t multicast_key;
    multicast_group_data_t multicast_data;
} __PACK__ multicast_table_entry_t;

typedef struct multicast_handle_entry_s {
    uint8_t key[MULTICAST_HDL_TBL_KEY_SIZE];
    tommy_hashtable_node node;
    multicast_group_key_t multicast_key;
    multicast_group_data_t multicast_data;
} __PACK__ multicast_handle_entry_t;

typedef struct hal_pd_replication_tbl_mbr_entry_s {
    uint64_t lif : 11;
    uint64_t encap_id : 24;
    uint64_t tunnel_rewrite_idx : 11;
    uint64_t l3_rewrite_index : 12;
    uint64_t pad : 6;
} hal_pd_replication_tbl_mbr_entry_t;

typedef struct hal_pd_replication_tbl_entry_s {
    uint64_t opq1;
    uint64_t opq2;
    uint64_t opq3;
    uint64_t opq4;
    uint64_t opq5;
    uint64_t opq6;
    uint64_t opq7;
    uint64_t pad : 32;
    uint64_t last : 1;
    uint64_t num_entries:4;
    uint64_t next_index:27;
} hal_pd_replication_tbl_entry_t;
 
typedef struct hal_pd_replication_node_s {
    uint32_t hw_repl_table_index;
    hal_pd_replication_tbl_entry_t hw_repl_tbl_entry;
} __PACK__ hal_pd_replication_node_t;
    
    
    
typedef struct replication_table_entry_s {
    uint8_t key[REPLICATION_KEY_SIZE];
    hal_handle_t hndl;
    tommy_hashtable_node repl_id_node;
    uint16_t repl_mbr_count;
    tommy_node repl_list_node;
    hal_pd_replication_node_t *pd_repl_list;
} __PACK__ replication_table_entry_t;


#endif //__MULTICAST_INTERNAL_H__
