//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// This file contains Key and Data structures used to test the mem hash library.
// The variations mostly are because of the difference in key and data sizes which
// changes the number of hints.
//------------------------------------------------------------------------------
#ifndef __FTL_P4PD_MOCK_HPP__
#define __FTL_P4PD_MOCK_HPP__

#include <stdint.h>

#ifdef IRIS
#define FLOW_HASH_ENTRY_T flow_hash_info_entry_t
#else
#define FLOW_HASH_ENTRY_T flow_hash_entry_t
#endif

typedef enum ftl_p4pd_table_ids_ {
    FTL_TBLID_NONE       = 0,
    FTL_TBLID_IPV6       = 1,
    FTL_TBLID_IPV6_OHASH = 2,
    FTL_TBLID_IPV4       = 3,
    FTL_TBLID_IPV4_OHASH = 4,
    FTL_TBLID_MAX        = 5,
} ftl_p4_table_id;

typedef enum ftl_action_ids {
    FTL_ACTION_ID_0    = 0,
} ftl_action_ids_t;

void
ftl_mock_init ();

void
ftl_mock_cleanup ();

uint32_t
ftl_mock_get_valid_count (uint32_t table_id);

#endif
