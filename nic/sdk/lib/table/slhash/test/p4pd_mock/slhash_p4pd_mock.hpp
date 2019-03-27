//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __SLHASH_P4PD_MOCK_HPP__
#define __SLHASH_P4PD_MOCK_HPP__

#include <stdint.h>
#define KEY_SIZE sizeof(slhash_table_key_t)
#define DATA_SIZE sizeof(slhash_table_actiondata_t)
#define TABLE_SIZE 16*1024

typedef struct __attribute__((__packed__)) __slhash_table_key {
    uint32_t k1;
    uint32_t k2;
} slhash_table_key_t;

typedef struct __attribute__((__packed__)) __slhash_table_mask {
    uint32_t k1;
    uint32_t k2;
} slhash_table_mask_t;

typedef struct __attribute__((__packed__)) __slhash_table_info {
    uint16_t d1;
    uint16_t d2;
    uint32_t epoch1;
    uint32_t epoch2;
    uint8_t valid1;
    uint8_t valid2;
} slhash_table_info_t;

typedef union __slhash_table_action_union {
    slhash_table_info_t info;
} slhash_table_action_union_t;

typedef struct __attribute__((__packed__)) __slhash_table_actiondata {
    uint8_t action_id;
    slhash_table_action_union_t action_u;
} slhash_table_actiondata_t;

typedef enum slhash_table_action_ids {
    SLHASH_TABLE_ACTION_ID_0    = 0,
} slhash_table_action_ids_t;

typedef enum slhash_p4pd_table_ids_ {
    SLHASH_TABLE_ID_NONE = 0,
    SLHASH_TABLE_ID_MOCK = 1,
    SLHASH_TABLE_ID_OTCAM_MOCK = 2,
} slhash_p4_table_id;

typedef struct slhash_mock_table_entry_s {
    slhash_table_key_t key;
    slhash_table_mask_t mask;
    slhash_table_actiondata_t data;
} slhash_mock_table_entry_t;

typedef struct slhash_mock_table_s {
    slhash_mock_table_entry_t entries[TABLE_SIZE];
} slhash_mock_table_t;

int
slhash_mock_init ();

int
slhash_mock_cleanup ();

uint32_t
slhash_mock_get_valid_count (uint32_t table_id);

#endif
