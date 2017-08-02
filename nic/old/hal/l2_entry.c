#include <l2_entry.h>
#include <crc32.h>
#include <bitmap.h>
#include <tommytypes.h>
#include <tommyhashtbl.h>

#define L2_MAC_TABLE_SIZE   (64 * 1024) 

#define L2_MAC_TABLE_KEY_SIZE 10
#define L2_HANDLE_TABLE_KEY_SIZE 4
#define L2_ENTRY_INVALID_HANDLE_ID 0xffffffff

static tommy_hashtable mac_table;
static tommy_hashtable l2_entry_handle_table;
static bitmap_t *l2_table_entry_handle;

//Move this to common file to be shared later.
// for now - add more later.
typedef struct l2_table_entry_s {
    uint8_t key[L2_MAC_TABLE_KEY_SIZE];
    hal_handle_t hndl;
    l2_entry_key_t l2_key;
    l2_entry_data_t l2_data;
    uint8_t    flags;
    tommy_hashtable_node node;
} l2_table_entry_t;

typedef struct l2_handle_entry_s {
    uint8_t key[L2_HANDLE_TABLE_KEY_SIZE];
    l2_entry_key_t l2_key;
    l2_entry_data_t l2_data;
    uint8_t    flags;
    tommy_hashtable_node node;
} l2_handle_entry_t;

// 1. Initialize/Create base datastructures.
// 2. Initialize any memories
// 3. Set resource limits
hal_ret_t hal_l2_entry_init(void)
{
    tommy_hashtable_init(&mac_table, L2_MAC_TABLE_SIZE);
    tommy_hashtable_init(&l2_entry_handle_table, L2_MAC_TABLE_SIZE);

    l2_table_entry_handle = bitmap_create(L2_MAC_TABLE_SIZE);

    if (!l2_table_entry_handle) {
        return HAL_RET_OOM;
    }
    // Set position zero as invalid. Set it upfront. 
    bitmap_bit_set(l2_table_entry_handle, 0);
    
    return HAL_RET_OK; 
}

uint32_t l2_table_entry_id_alloc(void) 
{
    uint32_t bit_loc = bitmap_get_first_free_bit(l2_table_entry_handle);

    if (bit_loc != L2_ENTRY_INVALID_HANDLE_ID) {
        bitmap_bit_set(l2_table_entry_handle, bit_loc);
    }

    return bit_loc;
}

void l2_table_entry_id_dealloc(uint32_t l2_entry_id)
{
    bitmap_bit_clear(l2_table_entry_handle, l2_entry_id);
}


l2_table_entry_t* l2_table_entry_alloc(uint32_t num_entries)
{
    //zero out memory if required
    return( (l2_table_entry_t*)malloc(sizeof(l2_table_entry_t) * num_entries));
}

l2_handle_entry_t *l2_handle_entry_alloc(uint32_t num_entries)
{
    //zero out memory
    return( (l2_handle_entry_t*)malloc(sizeof(l2_handle_entry_t) * num_entries));
} 
     
static inline hal_ret_t  hal_l2_entry_create_handle(hal_handle_t *hndl)
{
    return HAL_RET_OK;
}

static inline int mac_table_hash_cmp(const void *key1, const void* key2)
{
    return (memcmp(key1, key2, L2_MAC_TABLE_KEY_SIZE));
}

void l2_internal_mac_table_lookup_key_set(uint8_t *key, uint8_t *keylen, l2_entry_t *l2_entry)
{
   memcpy(key, &(l2_entry->kh.key.segid), sizeof(l2_entry->kh.key.segid));
   memcpy(key+sizeof(l2_entry->kh.key.segid), l2_entry->kh.key.mac, ETH_ADDR_LEN);
   *keylen = sizeof(l2_entry->kh.key.segid) + ETH_ADDR_LEN;
}

l2_table_entry_t *l2_internal_mac_table_lookup_find(uint8_t *key, uint8_t keylen, uint32_t *hash)
{
    l2_table_entry_t *tbl_entry;

    //some junk hash seed 33335555
    *hash = (uint32_t)crc32(0x33335555, key, keylen);
    tbl_entry = (l2_table_entry_t*) tommy_hashtable_search(&mac_table, 
                                      mac_table_hash_cmp, key, *hash);
    if (tbl_entry) {
        //add print here
        return tbl_entry;
    }
    return NULL; 
}


//wrapper function - in future we change tommyds - this will change.
l2_table_entry_t *hal_internal_mac_table_find(l2_entry_t *l2_entry) 
{
    
    uint8_t key[L2_MAC_TABLE_KEY_SIZE], keylen = 0;
    uint32_t hash = 0;

    l2_internal_mac_table_lookup_key_set(key, &keylen, l2_entry);
    return (l2_internal_mac_table_lookup_find(key, keylen, &hash)); 
}

//wrapper - for now use tommyds hash table
hal_ret_t hal_internal_insert_mac_table(l2_table_entry_t *mac_node, 
                                        hal_handle_t *hndl)
{
    uint8_t key[L2_MAC_TABLE_KEY_SIZE], keylen = 0;
    uint32_t hash = 0;
    uint8_t key_handle[L2_HANDLE_TABLE_KEY_SIZE];

    l2_handle_entry_t *handle_node = l2_handle_entry_alloc(1);

    if (!handle_node) {
        return HAL_RET_OOM;
    }
  
    // allocate handle - running number. Insert it into mac-table. 
    *hndl = (hal_handle_t)l2_table_entry_id_alloc();
    memcpy(key, &mac_node->l2_key.segid, sizeof(uint32_t)); 
    memcpy(key+sizeof(uint32_t), mac_node->l2_key.mac, ETH_ADDR_LEN);
    keylen = sizeof(uint32_t)+ETH_ADDR_LEN;  
    
    hash = (uint32_t)crc32(0x33335555, key, keylen);
    memcpy(mac_node->key, key, keylen);
    if (*hndl != L2_ENTRY_INVALID_HANDLE_ID) {
        mac_node->hndl  = *hndl;
    } else {
        return HAL_RET_NO_RESOURCE;
    }
    
    tommy_hashtable_insert(&mac_table, &mac_node->node, mac_node, hash); 
    
    // insert into handle-table - easy for handle based lookups. 
    // Opaque number passed upstream
    memcpy(key_handle, hndl, L2_HANDLE_TABLE_KEY_SIZE);
    keylen = L2_HANDLE_TABLE_KEY_SIZE;
    hash = (uint32_t)crc32(0x33335555, key_handle, keylen);
    memcpy(&handle_node->l2_key, &mac_node->l2_key, sizeof(l2_entry_key_t));
    memcpy(&handle_node->l2_data, &mac_node->l2_data, sizeof(l2_entry_data_t));

    tommy_hashtable_insert(&l2_entry_handle_table, &handle_node->node, handle_node, hash);
 
    return HAL_RET_OK;
}


hal_ret_t hal_process_one_l2_entry(l2_entry_t *l2_entry, hal_handle_t *hndl) 
{
    l2_table_entry_t *mac_node;
    hal_ret_t err = HAL_RET_OK;

    mac_node = hal_internal_mac_table_find(l2_entry);
    if (mac_node) {
        // or update the existing node's data ?? - will come back
        return HAL_RET_ENTRY_EXISTS;
    }
    mac_node = l2_table_entry_alloc(1);
    if (!mac_node) {
        return HAL_RET_OOM;
    }
    memcpy(mac_node->l2_key.mac, l2_entry->kh.key.mac, ETH_ADDR_LEN);
    mac_node->l2_key.segid = l2_entry->kh.key.segid;
    if (l2_entry->data.__ifindex_valid == TRUE) {
        
        mac_node->l2_data.ifindex = l2_entry->data.ifindex;
    } else {
        mac_node->l2_data.ifindex = IFINDEX_INVALID;
    }
    err = hal_internal_insert_mac_table(mac_node, hndl); 
    
    return err;
}

hal_ret_t hal_create_l2_entry(l2_entries_t *l2_entries __INOUT__)
{
    hal_ret_t err = HAL_RET_OK, batch_err = HAL_RET_OK;
    l2_entry_t *curr_entry;
    hal_handle_t hndl;
    uint32_t num_batch_failures = 0, num_batch_success = 0;
 
    if (!l2_entries) {
        return HAL_RET_INVALID_ARG;
    }

    if (l2_entries->num_entries == 0) {
        return HAL_RET_INVALID_ARG;
    }

    for (uint32_t entry_id = 0; entry_id < l2_entries->num_entries; entry_id++) {
        curr_entry = &l2_entries->entries[entry_id];

        if (curr_entry->kh.__key_valid != TRUE) {
        // if upperlayer is passing handle - should we be accepting for create or is this only for update ??
        }
        
        err = hal_process_one_l2_entry(curr_entry, &hndl);
        if (err != HAL_RET_OK) {
            l2_entries->entries[entry_id].oper.reason_code = err;
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

