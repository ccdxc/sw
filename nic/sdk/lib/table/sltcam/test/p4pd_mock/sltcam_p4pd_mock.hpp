//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __SLTCAM_P4PD_MOCK_HPP__
#define __SLTCAM_P4PD_MOCK_HPP__

#include <stdint.h>
#define KEY_SIZE sizeof(sltcam_table_key_t)
#define DATA_SIZE sizeof(sltcam_table_actiondata_t)
#define TABLE_SIZE 16*1024

typedef struct __attribute__((__packed__)) __sltcam_table_key {
    uint8_t entry_inactive;
    uint32_t k;
} sltcam_table_key_t;

typedef struct __attribute__((__packed__)) __sltcam_table_mask {
    uint8_t entry_inactive_mask;
    uint32_t m;
} sltcam_table_mask_t;

typedef struct __attribute__((__packed__)) __sltcam_table_info {
    uint32_t d;
} sltcam_table_info_t;

typedef union __sltcam_table_action_union {
    sltcam_table_info_t info;
} sltcam_table_action_union_t;

typedef struct __attribute__((__packed__)) __sltcam_table_actiondata {
    uint8_t action_id;
    sltcam_table_action_union_t action_u;
} sltcam_table_actiondata_t;

typedef enum sltcam_table_action_ids {
    SLTCAM_TABLE_ACTION_ID_0    = 0,
} sltcam_table_action_ids_t;

typedef enum sltcam_p4pd_table_ids_ {
    SLTCAM_TABLE_ID_NONE = 0,
    SLTCAM_TABLE_ID_MOCK = 1,
} sltcam_p4_table_id;

typedef struct sltcam_mock_table_entry_s {
    sltcam_table_key_t key;
    sltcam_table_mask_t mask;
    sltcam_table_actiondata_t data;
} sltcam_mock_table_entry_t;

typedef struct sltcam_mock_table_s {
    sltcam_mock_table_entry_t entries[TABLE_SIZE];
} sltcam_mock_table_t;

int
sltcam_mock_init ();

int
sltcam_mock_cleanup ();

uint32_t
sltcam_mock_get_valid_count (uint32_t table_id);

#endif
