#include <crc32.h>
#include <bitmap.h>
//#include <flow.h>
#include <flow_internal.h>

flow_table_internal_t *flow_table_internal;
flow_table_internal_entry_t *oflow_flow_table_internal;
bitmap_t *flow_table_oflow_entry_alloc_bmp;

hal_internal_flow_handle_table_entry_t *flow_handle_table;
bitmap_t *flow_handle_entry_alloc_bmp;

static inline uint32_t flow_table_internal_get_flow_table_size(void)
{
    if (flow_table_internal) {
        return (flow_table_internal->num_entries);
    }
    return 0;
}

static inline uint32_t flow_table_internal_get_first_hash_bits(void)
{
    if (flow_table_internal) {
        return (flow_table_internal->flow_hash_bits);
    }
    return 0;
}

//Always pass power of 2 please.
int flow_table_internal_create_flow_table(uint32_t size)
{
    int i = 0;
    uint8_t flow_hash_bits = 0;

    //Round off to nearest power of 2
    while (size) {
        size = size >> 1;
        i++;
    }
    //Double the size for internal creation
    flow_hash_bits = i+1;
   
    // Allocate table fixed - double the size.
    // If we have to use a different allocator. 
    uint32_t memsize = sizeof(flow_table_internal_t) + 
                       ((1<<flow_hash_bits) * sizeof(flow_table_internal_entry_t));
    flow_table_internal = (flow_table_internal_t*)malloc(memsize);
    if (!flow_table_internal) {
        return HAL_RET_OOM;
    }
    // set table size
    flow_table_internal->num_entries = (1<<flow_hash_bits); 
    // For use on how many bits of crc to take - no use otherwise.
    flow_table_internal->flow_hash_bits = flow_hash_bits;
 
    memsize = sizeof(flow_table_internal_entry_t) * HAL_OFLOW_FLOW_TABLE_SIZE;
    oflow_flow_table_internal = (flow_table_internal_entry_t*) malloc(memsize); 
    if (!oflow_flow_table_internal) {
        free(flow_table_internal);
        return HAL_RET_OOM;
    }
    // Overflow table - capped for now at 16k size. Is this really enough - why not 64k ?
    // If we need a bigger number - we will burn bits for hints and may bring down number of
    // hints in the flow table.
    flow_table_oflow_entry_alloc_bmp = bitmap_create(HAL_OFLOW_FLOW_TABLE_SIZE);
    if (!flow_table_oflow_entry_alloc_bmp) {
        free(flow_table_internal);
        free(oflow_flow_table_internal);
        return HAL_RET_OOM;
    }
    flow_handle_entry_alloc_bmp = bitmap_create(1 << flow_hash_bits);
    if (!flow_handle_entry_alloc_bmp) {
        free(flow_table_internal);
        free(oflow_flow_table_internal);
        free(flow_table_oflow_entry_alloc_bmp);
        return HAL_RET_OOM;
    }
    // set index 0 as used - treat it as invalid
    bitmap_bit_set(flow_table_oflow_entry_alloc_bmp, 0);
    bitmap_bit_set(flow_handle_entry_alloc_bmp, 0);

    return HAL_RET_OK; 
}

// Overflow flat table index allocator
int flow_table_internal_alloc_oflow_index(void)
{
    uint32_t bit_loc = bitmap_get_first_free_bit(flow_table_oflow_entry_alloc_bmp);
    if ((bit_loc >= 0) && (bit_loc < HAL_OFLOW_FLOW_TABLE_SIZE)) {
        bitmap_bit_set(flow_table_oflow_entry_alloc_bmp, bit_loc);
        return bit_loc; 
    }
    return -1;
}

uint32_t flow_table_internal_alloc_flow_handle(void) 
{
    uint32_t bit_loc = bitmap_get_first_free_bit(flow_handle_entry_alloc_bmp);
    if ((bit_loc >= 0) && (bit_loc < (flow_handle_entry_alloc_bmp->num_words << 5))) {
        bitmap_bit_set(flow_handle_entry_alloc_bmp, bit_loc);
        return bit_loc;
    }
    return 0;
}

flow_table_internal_entry_t *flow_table_internal_get_entry_by_hash(uint32_t hash)
{
    if (hash > flow_table_internal_get_flow_table_size()) {
        return NULL;
    }
    return (&flow_table_internal->entries[hash]);
} 

flow_table_internal_entry_t *flow_table_internal_get_overflow_entry_by_index(uint32_t oflow_index)
{
    if (oflow_index >= HAL_OFLOW_FLOW_TABLE_SIZE) {
        return NULL;
    }
    return (&oflow_flow_table_internal[oflow_index]);
}

void flow_table_internal_get_key_from_entry(flow_table_internal_entry_t *entry, uint64_t *key)
{
    uint64_t temp;

    if (!entry || !key)
        return;

    key[0] = entry->flow_lkp_type;
    key[0] |= (entry->flow_lkp_vrf) << 4;
    key[0] |= (entry->flow_lkp_src_1) << 20;
    key[1] = entry->flow_lkp_src_2; 
    key[2] = entry->flow_lkp_src_3;
    key[2] |= entry->flow_lkp_dst_1 << 20;
    key[3] = entry->flow_lkp_dst_2;
    key[4] = entry->flow_lkp_dst_3;
    temp = entry->flow_lkp_proto;
    key[4] |= temp << 20;
    temp = entry->flow_lkp_sport;
    key[4] |= temp << 28;
    temp = entry->flow_lkp_dport;
    key[4] |= temp << 44;
}

int hal_internal_compare_flow_key_with_entry_key(uint64_t *key, uint32_t hash1, uint32_t hash2, uint32_t *oflow_index)
{
    uint64_t entry_key[FLOW_ENTRY_U64_SIZE];
    flow_table_internal_entry_t *entry = NULL;
    
    entry = flow_table_internal_get_entry_by_hash(hash1);

    if (entry->flow_entry_valid == 0)
        return -1;

    flow_table_internal_get_key_from_entry(entry, &entry_key[0]);

    if (memcmp(key, entry_key, sizeof(uint64_t) * FLOW_ENTRY_U64_SIZE) == 0) {
        return 0;
    }
   
   
    if (entry->flow_hash_1 == hash2) {
        *oflow_index = entry->flow_hint_1;
    } else if (entry->flow_hash_2 == hash2) {
        *oflow_index = (entry->flow_hint_2_1) | ((entry->flow_hint_2_2) << 1);
    } else if (entry->flow_hash_3 == hash2) {
        *oflow_index = entry->flow_hint_3;
    } else if (entry->flow_hash_4 == hash2) {
        *oflow_index = (entry->flow_hint_2_1) | ((entry->flow_hint_2_2) << 11);
    } else if (entry->flow_hash_5 == hash2) {
        *oflow_index = entry->flow_hint_5;
    } else if (entry->flow_hash_6 == hash2) {
        *oflow_index = entry->flow_hint_6;
    } else {
        entry = NULL;
    }
    if (*oflow_index != 0) {
        entry = flow_table_internal_get_overflow_entry_by_index(*oflow_index);
        flow_table_internal_get_key_from_entry(entry, &entry_key[0]);
        if (memcmp(key, entry_key, sizeof(uint64_t) * FLOW_ENTRY_U64_SIZE) == 0) {
            return 0;
        }
    }

    return -1;
} 
        
     
    
         
// The HW key may not be built exactly the below way and it depends on 
// which order the key-members are in the flits. 
// The below part assumes that it could be like the way its in p4-program
// and if its not true - will need to be revisited. 
// Need to look at the K generation in asm.
static void hal_internal_flow_build_flow_key(hal_flow_table_key_t *flow_key,
                                                    uint64_t *key)
{
    uint64_t temp;
    // 64-bits 4,16,128,128,8,16,16
    key[0] = flow_key->flow_type & 0xf;
    key[0] |= (flow_key->vrf) << 4;
    // fill source 
    key[0] |= (flow_key->src[0] & 0xfffffffffff) << 20; //1
    key[1] |= (flow_key->src[0] & 0xfffff00000000000) >> 44;
    key[1] |= (flow_key->src[1] & 0xfffffffffff) << 20;
    key[2] |= (flow_key->src[1] & 0xfffff00000000000) >> 44;
    key[2] |= (flow_key->dst[0] & 0xfffffffffff) << 20; 
    key[3] |= (flow_key->dst[0] & 0xfffff00000000000) >> 44;
    key[3] |= (flow_key->dst[1] & 0xfffffffffff) << 20;
    key[4] |= (flow_key->dst[1] & 0xfffff00000000000) >> 44;
    temp = flow_key->protocol;
    key[4] |= temp << 20;
    temp = flow_key->sport;
    key[4] |= temp << 28;
    temp = flow_key->dport; 
    key[4] |= temp << 44;
}

hal_ret_t flow_table_internal_insert_primary(uint32_t hash, flow_table_internal_entry_t *entry, 
                                             hal_flow_table_key_t *flow_key,
                                             hal_flow_table_data_t *flow_data)
{
    uint64_t temp;
    if (!entry || !flow_key || !flow_data) {
        return HAL_RET_ERR;
    }

    entry->flow_lkp_type = flow_key->flow_type & 0xf;
    entry->flow_lkp_vrf = flow_key->vrf;
    
    temp = flow_key->src[1]; 
    entry->flow_lkp_src_1 = flow_key->src[0]  | 
                            ((temp & 0xfff ) << 32);
    entry->flow_lkp_src_2 = ((flow_key->src[1] & 0xffff000) >> 12) |
                            (flow_key->src[2] << 20) | 
                            ((flow_key->src[3] & 0xfff) << 52);
    entry->flow_lkp_src_3 = (flow_key->src[3] & 0xffff000) >> 12; 
    entry->flow_lkp_dst_1 = flow_key->dst[0] |
                            ((flow_key->dst[1] & 0xfff ) << 32);
    entry->flow_lkp_dst_2 = ((flow_key->dst[1] & 0xffff000) >> 12) |
                            (flow_key->dst[2] << 20) | 
                            ((flow_key->dst[3] & 0xfff) << 52);
    entry->flow_lkp_dst_3 = (flow_key->dst[3] & 0xffff000) >> 12; 
                             
    entry->flow_lkp_proto = flow_key->protocol;
    entry->flow_lkp_sport = flow_key->sport;
    entry->flow_lkp_dport = flow_key->dport;    
    entry->flow_data_type1 = flow_data->flow_result & 0xf;
    entry->flow_data_type2 = 0;
     
    entry->flow_hash_1 = 0;
    entry->flow_hint_1 = 0;
    entry->flow_hash_2 = 0;
    entry->flow_hint_2_1 = 0;
    //
    entry->flow_hint_2_2 = 0;
    entry->flow_hash_3 = 0;
    entry->flow_hint_3 = 0;
    entry->flow_hash_4 = 0;
    entry->flow_hint_4_1 = 0;
    //
    entry->flow_hint_4_2 = 0;
    entry->flow_hash_5 = 0;
    entry->flow_hint_5 = 0;
    entry->flow_hash_6 = 0;
    entry->flow_hint_6 = 0; 
    entry->flow_entry_valid = 1;
    entry->pad = 0;
    
    return HAL_RET_OK; 
}

void flow_table_internal_fill_overflow_entry(flow_table_internal_entry_t *entry,
                                              hal_flow_table_key_t *flow_key,
                                              hal_flow_table_data_t *flow_data)
{
    if (!entry || !flow_key || !flow_data) {
        return;
    }

    entry->flow_lkp_type = flow_key->flow_type & 0xf;
    entry->flow_lkp_vrf = flow_key->vrf;
    entry->flow_lkp_src_1 = flow_key->src[0]  | 
                            ((flow_key->src[1] & 0xfff ) << 32);
    entry->flow_lkp_src_2 = ((flow_key->src[1] & 0xffff000) >> 12) |
                            (flow_key->src[2] << 20) | 
                            ((flow_key->src[3] & 0xfff) << 52);
    entry->flow_lkp_src_3 = (flow_key->src[3] & 0xffff000) >> 12; 
    entry->flow_lkp_dst_1 = flow_key->dst[0] |
                            ((flow_key->dst[1] & 0xfff ) << 32);
    entry->flow_lkp_dst_2 = ((flow_key->dst[1] & 0xffff000) >> 12) |
                            (flow_key->dst[2] << 20) | 
                            ((flow_key->dst[3] & 0xfff) << 52);
    entry->flow_lkp_dst_3 = (flow_key->dst[3] & 0xffff000) >> 12; 
                             
    entry->flow_lkp_proto = flow_key->protocol;
    entry->flow_lkp_sport = flow_key->sport;
    entry->flow_lkp_dport = flow_key->dport;    
    entry->flow_data_type1 = flow_data->flow_result & 0xf;
    entry->flow_data_type2 = 0;
    entry->pad = 0;
}

hal_ret_t flow_table_internal_insert_overflow (uint32_t hash2, 
                                              flow_table_internal_entry_t *primary_entry,
                                              hal_flow_table_key_t *flow_key,
                                              hal_flow_table_data_t *flow_data,
                                              uint32_t *oflow_tbl_index)
{
    uint8_t secondary_index = 0; 
    uint32_t primary_collision_index = 0;
    int oflow_index = -1;
    flow_table_internal_entry_t *oflow_entry = NULL;   


    if (!primary_entry || !flow_key || !flow_data) {
        return HAL_RET_ERR;
    }

    // Check if the upper 11 bits are matching.
    // If they match - recirc has to happen with hints coming from overflow
    // which actually means another overflow.
    if (primary_entry->flow_hash_1 == hash2) {
        primary_collision_index = primary_entry->flow_hint_1;
    } else if (primary_entry->flow_hash_2 == hash2) {
        primary_collision_index = primary_entry->flow_hint_2_1 | (primary_entry->flow_hint_2_2 << 1);
    } else if (primary_entry->flow_hash_3 == hash2) {
        primary_collision_index = primary_entry->flow_hint_3;
    } else if (primary_entry->flow_hash_4 == hash2) {
        primary_collision_index = primary_entry->flow_hint_4_1 | (primary_entry->flow_hint_4_2 << 11);
    } else if (primary_entry->flow_hash_5 == hash2) {
        primary_collision_index = primary_entry->flow_hint_5;
    } else if (primary_entry->flow_hash_6 == hash2) {
        primary_collision_index = primary_entry->flow_hint_6;
    }

    if (primary_collision_index != 0) {
        // second level recirc needed. for now bail out
        return HAL_RET_NO_RESOURCE;
    }

    // See which of the hints are "free"  
    if ((primary_entry->flow_hash_1 == 0) && 
        (primary_entry->flow_hint_1 == 0)) {
        secondary_index = 1;
    } else if ((primary_entry->flow_hash_2 == 0) &&
               (primary_entry->flow_hint_2_1 == 0) &&
               (primary_entry->flow_hint_2_2 == 0)) {
        secondary_index = 2;
    } else if ((primary_entry->flow_hash_3 == 0) && 
        (primary_entry->flow_hint_3 == 0)) {
        secondary_index = 3;
    } else if ((primary_entry->flow_hash_4 == 0) &&
               (primary_entry->flow_hint_4_1 == 0) &&
               (primary_entry->flow_hint_4_2 == 0)) {
        secondary_index = 4;
    } else if ((primary_entry->flow_hash_5 == 0) && 
        (primary_entry->flow_hint_5 == 0)) {
        secondary_index = 5;
    } else if ((primary_entry->flow_hash_6 == 0) && 
        (primary_entry->flow_hint_6 == 0)) {
        secondary_index = 6;
    }
    
    //allocate an overflow entry and fillup the corresponding hint and hash
    if (secondary_index != 0) {
        oflow_index = flow_table_internal_alloc_oflow_index();
        if ((oflow_index <= 0) || (oflow_index >= HAL_OFLOW_FLOW_TABLE_SIZE) ) {
            return HAL_RET_NO_RESOURCE;
        }
    }
    // fill up the hint and  overflow index
    switch(secondary_index) {
        case 1: 
            primary_entry->flow_hash_1 = hash2 & 0x7ff; // 11 bits
            primary_entry->flow_hint_1 = oflow_index; 
            break;
        case 2:            
            primary_entry->flow_hash_2 = hash2 & 0x7ff; // 11 bits
            primary_entry->flow_hint_2_1 = oflow_index & 0x1; //1 bit
            primary_entry->flow_hint_2_2 = (oflow_index & 0xfffe) >> 1; //15 bits 
            break;
        case 3: 
            primary_entry->flow_hash_3 = hash2 & 0x7ff; // 11 bits
            primary_entry->flow_hint_3 = oflow_index; 
            break;
        case 4:            
            primary_entry->flow_hash_4 = hash2 & 0x7ff; // 11 bits
            primary_entry->flow_hint_4_1 = oflow_index & 0x7ff; //11 bit
            primary_entry->flow_hint_4_2 = (oflow_index & 0xf800) >> 11; //5 bits 
            break;
        case 5: 
            primary_entry->flow_hash_5 = hash2 & 0x7ff; // 11 bits
            primary_entry->flow_hint_5 = oflow_index; 
            break;
        case 6: 
            primary_entry->flow_hash_6 = hash2 & 0x7ff; // 11 bits
            primary_entry->flow_hint_6 = oflow_index; 
            break;
        default:
            break;
    }
    oflow_entry = &oflow_flow_table_internal[oflow_index]; 
    flow_table_internal_fill_overflow_entry(oflow_entry, flow_key, flow_data);    
    *oflow_tbl_index = oflow_index;  

    return HAL_RET_OK;
}

void flow_table_internal_add_flow_handle_entry(hal_handle_t hal_flow_handle,
                                               uint32_t hash1, uint32_t hash2,
                                               uint32_t oflow_index)
{
    flow_handle_table[hal_flow_handle].primary_index = hash1;
    flow_handle_table[hal_flow_handle].oflow_index = oflow_index;
}
                                                              
hal_ret_t hal_internal_flow_table_insert_one_entry (
              hal_flow_table_key_t *flow_key,
              hal_flow_table_data_t *flow_data)
{
    hal_ret_t err = HAL_RET_OK;
    uint64_t key[FLOW_ENTRY_U64_SIZE];
    uint32_t hash;
    uint32_t hash1, hash2;
    uint32_t oflow_index;
    flow_table_internal_entry_t *entry;
    hal_handle_t hal_flow_handle;

    memset(&key[0], 0, sizeof(uint64_t) * FLOW_ENTRY_U64_SIZE);

    hal_internal_flow_build_flow_key(flow_key, &key[0]);
    hash = (uint32_t)crc32(0x33335555, key, 64);
    
    uint8_t flow_hash_bits = (uint8_t)flow_table_internal_get_first_hash_bits(); 
    hash1 = hash & ((1 << flow_hash_bits) -1);
    //hash2 = (hash & (( 1 << (32 - HAL_FIRST_HASH_BITS)) - 1)) >> HAL_FIRST_HASH_BITS;
    //p4 hash it fixed for now 11 bits and 21 bits
    // we can always use upper 11 bits irrespective of hash primary hash table size
    hash2 = (hash & 0xffe00000) >> 21; 

    // check if entry exists

    if (hal_internal_compare_flow_key_with_entry_key(&key[0], hash1, 
                                                     hash2, &oflow_index) == 0) {
        return HAL_RET_ENTRY_EXISTS;
    }
    
    entry = flow_table_internal_get_entry_by_hash(hash1);

    if (entry->flow_entry_valid == 0) {
        err = flow_table_internal_insert_primary(hash, entry, flow_key, flow_data);
    } else {
        err = flow_table_internal_insert_overflow(hash2, entry, flow_key, 
                                                  flow_data, &oflow_index);
    }
    
    if (err != HAL_RET_OK) {
        return err;
    }

    hal_flow_handle = flow_table_internal_alloc_flow_handle();

    if (hal_flow_handle != 0 ) {
        flow_table_internal_add_flow_handle_entry(hal_flow_handle, 
                                                  hash1, hash2, oflow_index);
    }

    return err; 
           
} 
