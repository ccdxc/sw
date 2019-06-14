//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// This file contains Key and Data structures used to test the mem hash library.
// The variations mostly are because of the difference in key and data sizes which
// changes the number of hints.
//------------------------------------------------------------------------------
#ifndef __FTLITE_P4PD_MOCK_HPP__
#define __FTLITE_P4PD_MOCK_HPP__

#include <stdint.h>

typedef enum ftlite_p4pd_table_ids_ {
    FTLITE_TBLID_NONE         = 0,
    FTLITE_TBLID_IPV4         = 1,
    FTLITE_TBLID_IPV4_OHASH   = 2,
    FTLITE_TBLID_IPV6         = 3,
    FTLITE_TBLID_IPV6_OHASH   = 4,
    FTLITE_TBLID_MAX          = 5,
} ftlite_p4_table_id;

void
ftlite_mock_init ();

void
ftlite_mock_cleanup ();

uint32_t
ftlite_mock_get_valid_count (uint32_t table_id);

#endif
