//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
// This file contains Key and Data structures used to test the mem hash library.
// The variations mostly are because of the difference in key and data sizes which
// changes the number of hints.
//------------------------------------------------------------------------------
#ifndef __MEM_HASH_P4PD_MOCK_HPP__
#define __MEM_HASH_P4PD_MOCK_HPP__

#include <stdint.h>

typedef struct __attribute__((__packed__)) __mem_hash_h10_key {
    uint32_t k1;
} mem_hash_h10_key_t;

typedef struct __attribute__((__packed__)) __mem_hash_h10_data {
    uint8_t entry_valid;
    uint16_t d1;
    uint8_t d2;
    uint32_t d3;
    uint8_t d4;
    uint16_t hash1;
    uint16_t hint1;
    uint16_t hash2;
    uint16_t hint2;
    uint16_t hash3;
    uint16_t hint3;
    uint16_t hash4;
    uint16_t hint4;
    uint16_t hash5;
    uint16_t hint5;
    uint16_t hash6;
    uint16_t hint6;
    uint16_t hash7;
    uint16_t hint7;
    uint16_t hash8;
    uint16_t hint8;
    uint16_t hash9;
    uint16_t hint9;
    uint16_t hash10;
    uint16_t hint10;
    uint8_t more_hashes;
    uint16_t more_hints;
} mem_hash_h10_info_t;

typedef union __mem_hash_h10_action_union {
    mem_hash_h10_info_t info;
} mem_hash_h10_action_union_t;

typedef struct __attribute__((__packed__)) __mem_hash_h10_actiondata {
    uint8_t action_id;
    mem_hash_h10_action_union_t action_u;
} mem_hash_h10_actiondata_t;

typedef struct __attribute__((__packed__)) __mem_hash_h10_entry {
    mem_hash_h10_key_t key;
    mem_hash_h10_actiondata_t actiondata;
} mem_hash_h10_entry_t;

typedef struct __attribute__((__packed__)) __mem_hash_h5_key {
    uint8_t type;
    uint16_t sport;
    uint8_t proto;
    uint8_t src[16];
    uint16_t tag;;
    uint16_t dport;
    uint8_t dst[16];
} mem_hash_h5_key_t;

typedef struct __attribute__((__packed__)) __mem_hash_h5_data {
    uint8_t entry_valid;
    uint32_t d1;
    uint8_t d2;
    uint8_t hash1;
    uint32_t hint1;
    uint8_t hash2;
    uint32_t hint2;
    uint8_t hash3;
    uint32_t hint3;
    uint8_t hash4;
    uint32_t hint4;
    uint8_t hash5;
    uint32_t hint5;
    uint8_t more_hashes;
    uint32_t more_hints;
} mem_hash_h5_info_t;

typedef union __mem_hash_h5_action_union {
    mem_hash_h5_info_t info;
} mem_hash_h5_action_union_t;

typedef struct __attribute__((__packed__)) __mem_hash_h5_actiondata {
    uint8_t action_id;
    mem_hash_h5_action_union_t action_u;
} mem_hash_h5_actiondata_t;

typedef struct __attribute__((__packed__)) __mem_hash_h5_entry {
    mem_hash_h5_key_t key;
    mem_hash_h5_actiondata_t actiondata;
} mem_hash_h5_entry_t;

typedef enum mem_hash_p4pd_table_ids_ {
    MEM_HASH_P4TBL_ID_NONE       = 0,
    MEM_HASH_P4TBL_ID_H10        = 1,
    MEM_HASH_P4TBL_ID_H10_OHASH  = 2,
    MEM_HASH_P4TBL_ID_H5         = 3,
    MEM_HASH_P4TBL_ID_H5_OHASH   = 4,
    MEM_HASH_P4TBL_ID_MAX        = 5,
} mem_hash_p4_table_id;

int
mem_hash_mock_init ();


#endif
