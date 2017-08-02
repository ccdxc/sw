#include <multicast_internal.h>


static tommy_hashtable multicast_table;
static tommy_hashtable multicast_entry_handle_table;
static bitmap_t *multicast_table_entry_handle;

static tommy_hashtable replication_hash_table;
static bitmap_t *replication_id_bmp;


hal_ret_t hal_multicast_init(void)
{
    // Multicast entry software table.
    tommy_hashtable_init(&multicast_table, MULTICAST_REPL_TBL_SIZE);
    tommy_hashtable_init(&multicast_entry_handle_table, MULTICAST_REPL_TBL_SIZE);
    multicast_table_entry_handle = bitmap_create(MULTICAST_REPL_TBL_SIZE);
    if (!multicast_table_entry_handle) {
        return HAL_RET_OOM;
    }
    bitmap_bit_set(multicast_table_entry_handle, 0);

    // Replication table.
    tommy_hashtable_init(&replication_hash_table, MULTICAST_REPL_TBL_SIZE);
    replication_id_bmp = bitmap_create(MULTICAST_REPL_TBL_SIZE);
    if (!replication_id_bmp) {
        // bitmap_destroy prev one, delete hashtables here.
        return HAL_RET_OOM;
    }
    bitmap_bit_set(replication_id_bmp, 0);
    
    return HAL_RET_OK;
}

uint32_t multicast_table_entry_id_alloc(void)
{
    uint32_t bit_loc = bitmap_get_first_free_bit(multicast_table_entry_handle);

    if (bit_loc != MULTICAST_INVALID_HANDLE_ID) {
        bitmap_bit_set(multicast_table_entry_handle, bit_loc);
    }
   
    return bit_loc;
}
    
void multicast_table_entry_id_dealloc(uint32_t multicast_entry_id)
{
    bitmap_bit_clear(multicast_table_entry_handle, multicast_entry_id);
}


multicast_table_entry_t* multicast_table_entry_alloc(uint32_t num_members)
{
    uint32_t mem_size = sizeof(multicast_table_entry_t) + (num_members * sizeof(replication_member_t)); 
    //zero out memory if required
    return( (multicast_table_entry_t*)malloc(mem_size));
}

multicast_handle_entry_t *multicast_handle_entry_alloc(uint32_t num_entries)
{
    //zero out memory
    return( (multicast_handle_entry_t*)malloc(sizeof(multicast_handle_entry_t) * num_entries));
}

static inline int multicast_table_hash_cmp(const void *key1, const void *key2)
{
    return (memcmp(key1, key2, MULTICAST_TBL_KEY_SIZE));
}

void mulitcast_internal_table_lookup_key_set(uint8_t *key, uint8_t *keylen, 
                                             multicast_group_entry_t *entry)
{
    uint8_t key_pos = 0;
    
    memcpy(key, &(entry->kh.key.vrf), sizeof(vrf_id_t));
    key_pos = sizeof(vrf_id_t);
    memcpy(key+key_pos, &(entry->kh.key.segid), sizeof(seg_id_t));
    key_pos += sizeof(seg_id_t);
    if (entry->kh.key.ip_addr.af == IP_AF_INET) {
        memcpy(key+key_pos, &(entry->kh.key.ip_addr.addr.v4_addr), sizeof(ipv4_addr_t));
        key_pos += sizeof(ipv4_addr_t);
        *keylen = key_pos;
    } else if (entry->kh.key.ip_addr.af == IP_AF_INET6) {
        memcpy(key+key_pos, &(entry->kh.key.ip_addr.addr.v4_addr), sizeof(ipv6_addr_t));
        key_pos += sizeof(ipv6_addr_t);
    }
    *keylen = key_pos;
}

multicast_table_entry_t *multicast_internal_group_table_lookup_find(uint8_t *key, 
                                                                    uint8_t keylen, 
                                                                    uint32_t *hash)
{
    multicast_table_entry_t *tbl_entry;
    
    *hash = (uint32_t)crc32(HAL_INTERNAL_MCAST_CRC32_HASH_SEED, key, keylen);

    tbl_entry = (multicast_table_entry_t*)tommy_hashtable_search(&multicast_table, 
                                                                 multicast_table_hash_cmp,
                                                                 key, *hash);
    if (tbl_entry) {
        // print entry content here
        return tbl_entry;
    }
    return NULL;
}


multicast_table_entry_t*
hal_internal_multicast_table_find(multicast_group_entry_t *multicast_entry)
{
    uint8_t key[MULTICAST_TBL_KEY_SIZE], keylen = 0;
    uint32_t hash = 0;
    
    mulitcast_internal_table_lookup_key_set(&key[0], &keylen, multicast_entry);
    return(multicast_internal_group_table_lookup_find(key, keylen, &hash));
}

hal_ret_t hal_internal_insert_multicast_table(multicast_table_entry_t *multicast_node,
                                              hal_handle_t *hndl)
{
    uint8_t key[MULTICAST_TBL_KEY_SIZE], keylen = 0, key_pos = 0;
    uint32_t hash = 0;
    uint8_t key_handle[MULTICAST_HDL_TBL_KEY_SIZE];

    multicast_handle_entry_t *handle_node = multicast_handle_entry_alloc(1);

    if (!handle_node) {
        return HAL_RET_OOM;
    }

    *hndl = (hal_handle_t)multicast_table_entry_id_alloc();    
    memcpy(key, &multicast_node->multicast_key.vrf, sizeof(vrf_id_t));
    key_pos += sizeof(vrf_id_t);
    memcpy(key+key_pos, &multicast_node->multicast_key.segid, sizeof(seg_id_t));
    key_pos += sizeof(seg_id_t);
    if (multicast_node->multicast_key.ip_addr.af == IP_AF_INET) {
        memcpy(key+key_pos, &multicast_node->multicast_key.ip_addr.addr.v4_addr, 
               sizeof(ipv4_addr_t));
        key_pos += sizeof(ipv4_addr_t);
    } else if (multicast_node->multicast_key.ip_addr.af == IP_AF_INET6) {
        memcpy(key+key_pos, &multicast_node->multicast_key.ip_addr.addr.v6_addr, 
               sizeof(ipv6_addr_t));
        key_pos += sizeof(ipv6_addr_t);
    } 
    keylen = key_pos;
    hash = (uint32_t)crc32(HAL_INTERNAL_MCAST_CRC32_HASH_SEED, key, keylen);

    memcpy(multicast_node->key, key, keylen);
    if (*hndl != MULTICAST_INVALID_HANDLE_ID) {
        multicast_node->hndl = *hndl;
    } else {
        return HAL_RET_NO_RESOURCE;
    }
  
    tommy_hashtable_insert(&multicast_table, &multicast_node->node, 
                           multicast_node, hash);

    //insert  into handle-table

    memcpy(key_handle, hndl, MULTICAST_HDL_TBL_KEY_SIZE);
    keylen = MULTICAST_HDL_TBL_KEY_SIZE;
    hash = (uint32_t)crc32(HAL_INTERNAL_MCAST_CRC32_HASH_SEED, key_handle, keylen);

    memcpy(&handle_node->multicast_key, &multicast_node->multicast_key, 
           sizeof(multicast_group_key_t));
    memcpy(&handle_node->multicast_data, &multicast_node->multicast_data, 
        sizeof(multicast_group_data_t));

    tommy_hashtable_insert(&multicast_entry_handle_table, &handle_node->node, 
                           handle_node, hash);
    
    return HAL_RET_OK;
}

hal_ret_t hal_process_one_multicast_entry(multicast_group_entry_t *multicast_entry, 
                                          hal_handle_t *hndl)
{
    multicast_table_entry_t *multicast_node;
    hal_ret_t err = HAL_RET_OK;
    uint32_t mem_size;  

    if (!multicast_entry || !hndl) {
        return HAL_RET_ERR;
    }

    if (multicast_entry->data.repl_id == HAL_MULTICAST_INVALID_REPL_ID) {
        return HAL_RET_INVALID_ARG;
    }

    multicast_node = hal_internal_multicast_table_find(multicast_entry);
    
    if (multicast_node) {
        return HAL_RET_ENTRY_EXISTS;
    }

    multicast_node = multicast_table_entry_alloc(multicast_entry->data.num_multicast_members);
    if (!multicast_node) {
        return HAL_RET_OOM;
    }
    memcpy(&multicast_node->multicast_key, &multicast_entry->kh.key, 
           sizeof(multicast_group_key_t));

    mem_size = sizeof(multicast_table_entry_t) + 
               (multicast_entry->data.num_multicast_members * sizeof(replication_member_t)); 

    memcpy(&multicast_node->multicast_data, &multicast_entry->data, mem_size);

    err = hal_internal_insert_multicast_table(multicast_node, hndl);      
    
    return err;
}

hal_ret_t hal_create_multicast_group_entry(multicast_group_entries_t 
                                           *multicast_group_entries __INOUT__)
{
    hal_ret_t err = HAL_RET_OK, batch_err = HAL_RET_OK;
    multicast_group_entry_t *curr_entry;
    hal_handle_t hndl;
    uint32_t num_batch_failures = 0, num_batch_success = 0;

    if (!multicast_group_entries) {
        return HAL_RET_INVALID_ARG;
    }

    if (multicast_group_entries->num_entries == 0) {
        return HAL_RET_INVALID_ARG;
    }

    for (uint32_t entry_id = 0; entry_id < multicast_group_entries->num_entries; 
         entry_id++) {
        curr_entry = &multicast_group_entries->entries[entry_id];

        if (curr_entry->kh.__key_valid != TRUE) {
        // if upperlayer is passing handle - should we be accepting for create or is this only for update ??
        }

        err = hal_process_one_multicast_entry(curr_entry, &hndl);
        if (err != HAL_RET_OK) {
            multicast_group_entries->entries[entry_id].oper.reason_code = err;
            num_batch_failures++;
            if (num_batch_success > 0) {
                batch_err = HAL_RET_BATCH_PARTIAL_FAIL;
            } else {
                batch_err = HAL_RET_BATCH_FAIL;
            }
            continue;
        } else {
            num_batch_success++;
        }
        curr_entry->kh.hndl = hndl;
    }
    //print num_batch_failures, num_batch_success
    return batch_err;
}


// Replication API

uint32_t replication_table_entry_id_alloc(void)
{
    uint32_t bit_loc = bitmap_get_first_free_bit(replication_id_bmp);

    if (bit_loc != REPLICATION_INVALID_HANDLE_ID) {
        bitmap_bit_set(replication_id_bmp, bit_loc);
    }
   
    return bit_loc;
}
    
void replication_table_entry_id_dealloc(uint32_t repl_id)
{
    bitmap_bit_clear(replication_id_bmp, repl_id);
}


replication_table_entry_t* replication_table_entry_alloc(uint32_t num_members)
{
    uint32_t mem_size = sizeof(replication_table_entry_t) * num_members; 
    //zero out memory if required
    return( (replication_table_entry_t*)malloc(mem_size));
}

static inline int replication_id_table_hash_cmp(const void *key1, const void *key2)
{
    return (memcmp(key1, key2, MULTICAST_TBL_KEY_SIZE));
}

hal_ret_t hal_internal_add_replication_one_member(hal_handle_t repl_id, 
                                                  replication_member_t *mbr)
{
    return HAL_RET_OK;
}

replication_table_entry_t*
replication_table_entry_find(replication_entry_t *entry)
{
    replication_table_entry_t *tbl_entry;
    uint8_t key[MULTICAST_TBL_KEY_SIZE], keylen = 0;
    uint32_t hash = 0;
   
    if (!entry)
        return NULL;
 
    keylen = sizeof(hal_handle_t);
    if (entry->kh.__key_valid) {
        memcpy(key, &entry->kh.key.repl_id, sizeof(multicast_repl_id_t));
    } else if (entry->kh.__hndl_valid) {
        memcpy(key, &entry->kh.hndl, sizeof(hal_handle_t));
    }

    hash = (uint32_t)crc32(HAL_INTERNAL_MCAST_CRC32_HASH_SEED, key, keylen);
    tbl_entry = (replication_table_entry_t*)tommy_hashtable_search(&replication_hash_table, 
                                                                   replication_id_table_hash_cmp,
                                                                   key, hash);
    if (tbl_entry) {
        return tbl_entry;
    }
    return NULL;
}

hal_ret_t replication_table_internal_entry_insert(replication_table_entry_t *repl_tbl_entry)
{
    uint32_t hash = 0;

    memcpy(repl_tbl_entry->key, &repl_tbl_entry->hndl, sizeof(hal_handle_t));  
    hash = (uint32_t)crc32(HAL_INTERNAL_MCAST_CRC32_HASH_SEED, 
                           repl_tbl_entry->key, sizeof(hal_handle_t));
    
    tommy_hashtable_insert(&replication_hash_table, 
                           &repl_tbl_entry->repl_id_node, 
                           repl_tbl_entry, hash);
    
    return HAL_RET_OK;
}

hal_ret_t 
hal_process_create_one_replication_entry(replication_entry_t *entry,
                                            hal_handle_t *hndl)
{
    hal_ret_t err = HAL_RET_OK;
    replication_table_entry_t *repl_tbl_entry;
    multicast_repl_id_t repl_id = 0;

    if (!entry || !hndl) {
        return HAL_RET_INVALID_ARG;
    }

    repl_tbl_entry = (replication_table_entry_t*)replication_table_entry_find(entry);

    if (repl_tbl_entry) {
        // entry with handle-id already exists - cannot create
        return HAL_RET_ENTRY_EXISTS;
    }
    repl_id = replication_table_entry_id_alloc();
    if (repl_id ==  MULTICAST_REPL_INVALID_HDL_ID) {
        return HAL_RET_NO_RESOURCE;
    }
    repl_tbl_entry = (replication_table_entry_t*)replication_table_entry_alloc(1);
    if (!repl_tbl_entry) {
        return HAL_RET_OOM;
    }
    repl_tbl_entry->hndl = repl_id;
    err= replication_table_internal_entry_insert(repl_tbl_entry);

    for (uint16_t i = 0; i < entry->data.num_repl_members; i++) {
        err = hal_internal_add_replication_one_member(repl_id, &entry->data.repl_mbrs[i]);
    }
 
    return err;
}
         

hal_ret_t hal_create_replication_entry(replication_entries_t *repl_entries __INOUT__)
{
    hal_ret_t err = HAL_RET_OK, batch_err = HAL_RET_OK;
    replication_entry_t *curr_entry;
    hal_handle_t hndl;
    uint32_t num_batch_failures = 0, num_batch_success = 0;
    
    if (!repl_entries) {
        return HAL_RET_INVALID_ARG;
    }

    if (repl_entries->num_entries == 0) {
        return HAL_RET_INVALID_ARG;
    }

    for (uint32_t entry_id = 0; entry_id < repl_entries->num_entries; entry_id++) {
        curr_entry = &repl_entries->entries[entry_id];

        if ((curr_entry->kh.__key_valid != TRUE) || 
            (curr_entry->kh.__hndl_valid != TRUE)) {
        // if upperlayer is passing handle - should we be accepting for create or is this only for update ??
        }

        err = hal_process_create_one_replication_entry(curr_entry, &hndl);
        if (err != HAL_RET_OK) {
            repl_entries->entries[entry_id].oper.reason_code = err;
            num_batch_failures++;
            if (num_batch_success > 0) {
                batch_err = HAL_RET_BATCH_PARTIAL_FAIL;
            } else {
                batch_err = HAL_RET_BATCH_FAIL;
            }
            continue;
        } else {
            num_batch_success++;
        }
        curr_entry->kh.hndl = hndl;
    }

     
   
    return batch_err;
}
 
