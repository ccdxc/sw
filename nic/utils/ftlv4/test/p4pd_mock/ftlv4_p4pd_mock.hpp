//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// This file contains Key and Data structures used to test the mem hash library.
// The variations mostly are because of the difference in key and data sizes which
// changes the number of hints.
//------------------------------------------------------------------------------
#ifndef __FTLV4_P4PD_MOCK_HPP__
#define __FTLV4_P4PD_MOCK_HPP__

#include <stdint.h>

typedef enum ftlv4_p4pd_table_ids_ {
    FTL_TBLID_NONE       = 0,
    FTL_TBLID_H5         = 1,
    FTL_TBLID_H5_OHASH   = 2,
    FTL_TBLID_MAX        = 3,
} ftlv4_p4_table_id;

typedef enum ftlv4_h5_action_ids {
    FTL_TBLID_H5_ACTION_ID_0    = 0,
} ftlv4_h5_action_ids_t;

void
ftlv4_mock_init ();

void
ftlv4_mock_cleanup ();

uint32_t
ftlv4_mock_get_valid_count (uint32_t table_id);

#endif
