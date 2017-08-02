#ifndef __FLOW_INTERNAL_H__
#define __FLOW_INTERNAL_H__

#include <bitmap.h>

#define HAL_FIRST_HASH_BITS 21
#define HAL_SECOND_HASH_BITS (32 - HAL_FIRST_HASH_BITS)

#define HAL_FLOW_TABLE_SIZE (1<< HAL_FIRST_HASH_BITS)
#define HASH_FLOW_OCAM_SIZE (16 * 1024)

#define HAL_FLOW_LOOKUP_KEY_TYPE_MASK 0xf
#define HAL_OFLOW_FLOW_TABLE_SIZE (16 * 1024)

#define FLOW_ENTRY_U64_SIZE 8

// ASIC always sends full 512 bits for CRC hash.
// Our flow-key size is 316 bits and the rest will be padded with zeros
// Also the the fields used in the key for metadata could be in different
// order than as mentioned below in the struct.
// Because of this - the build_key kind of abstraction provides how the 
// keymaker is building the key - which could juggle the fields in between.
// This specific part is put as wrapper and will be revisited.

#define HASH_FLOW_KEY_SIZE 64 //512 bits

// This is as per p4 program definition and not necessarily the way its in hardware
// This part will be revisited once things are clear based on how compiler is creating it. 
typedef struct flow_table_internal_entry_s {
    // 316 bits of key
    uint64_t flow_lkp_type:4;
    uint64_t flow_lkp_vrf:16;
    uint64_t flow_lkp_src_1:44;
    uint64_t flow_lkp_src_2:64;
    uint64_t flow_lkp_src_3:20;
    uint64_t flow_lkp_dst_1:44;
    uint64_t flow_lkp_dst_2:64;
    uint64_t flow_lkp_dst_3:20;
    uint64_t flow_lkp_proto:8;
    uint64_t flow_lkp_sport:16;
    uint64_t flow_lkp_dport:16;
    uint64_t flow_data_type1:4; //was told this can be variable and not fixed 9 bits ??
    //
    uint64_t flow_data_type2:5;
    uint64_t flow_index:20;
    uint64_t flow_hash_1:11;
    uint64_t flow_hint_1:16;
    uint64_t flow_hash_2:11;
    uint64_t flow_hint_2_1:1;
    //
    uint64_t flow_hint_2_2:15;
    uint64_t flow_hash_3:11;
    uint64_t flow_hint_3:16;
    uint64_t flow_hash_4:11;
    uint64_t flow_hint_4_1:11;
    //
    uint64_t flow_hint_4_2:5;
    uint64_t flow_hash_5:11;
    uint64_t flow_hint_5:16;
    uint64_t flow_hash_6:11;
    uint64_t flow_hint_6:16;
    uint64_t flow_entry_valid:1;
    uint64_t pad:4;
} __PACK__ flow_table_internal_entry_t;

//use the same for both entry types
#if 0
typedef struct flow_table_internal_overflow_entry_s {
    // 316 bits of key
    uint64_t flow_lkp_type:4;
    uint64_t flow_lkp_vrf:16;
    uint64_t flow_lkp_src_1:44;
    uint64_t flow_lkp_src_2:64;
    uint64_t flow_lkp_src_3:20;
    uint64_t flow_lkp_dst_1:44;
    uint64_t flow_lkp_dst_2:64;
    uint64_t flow_lkp_dst_3:20;
    uint64_t flow_lkp_proto:8;
    uint64_t flow_lkp_sport:16;
    uint64_t flow_lkp_dport:16;
    uint64_t flow_data_type1:4; //was told this can be variable and not fixed 9 bits ??
    //
    uint64_t flow_data_type2:5;
    uint64_t flow_index:20;
    uint64_t flow_entry_valid:1;
    uint64_t pad:38;
} flow_table_internal_overflow_entry_t;
#endif

typedef struct flow_table_internal_s {
    uint32_t num_entries;
    uint32_t flow_hash_bits;
    flow_table_internal_entry_t entries[0];
} flow_table_internal_t;

typedef struct hal_flow_table_key_s {
    uint8_t flow_type;
    uint16_t vrf;
    uint64_t src[2];
    uint64_t dst[2];
    uint8_t protocol;
    uint16_t sport;
    uint16_t dport;
} hal_flow_table_key_t;

typedef struct hal_flow_table_data_s {
    uint8_t flow_result;
} hal_flow_table_data_t;

typedef struct hal_internal_flow_handle_table_entry_s {
    uint32_t primary_index;
    uint32_t oflow_index;
} hal_internal_flow_handle_table_entry_t;


#endif //__FLOW_INTERNAL_H__
