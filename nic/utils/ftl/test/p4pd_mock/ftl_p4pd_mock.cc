//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "lib/p4/p4_api.hpp"
#include "ftl_p4pd_mock.hpp"
#include "nic/utils/ftl/ftl_structs.hpp"

typedef struct ftl_mock_table_s {
    ftl_entry_t *entries;
} ftl_mock_table_t;

static ftl_mock_table_t mocktables[FTL_TBLID_MAX];

typedef int p4pd_error_t;

static uint32_t
table_size_get(uint32_t table_id)
{
    if (table_id == FTL_TBLID_H5) {
        return 16*1024*1024;
    } else if (table_id == FTL_TBLID_H5_OHASH) {
        return 2*1024*1024;
    }
    return 0;
}

void
ftl_mock_init ()
{
    mocktables[FTL_TBLID_H5].entries =
        (ftl_entry_t *)calloc(table_size_get(FTL_TBLID_H5), sizeof(ftl_entry_t));
    assert(mocktables[FTL_TBLID_H5].entries);
    mocktables[FTL_TBLID_H5_OHASH].entries =
        (ftl_entry_t *)calloc(table_size_get(FTL_TBLID_H5_OHASH), sizeof(ftl_entry_t));
    assert(mocktables[FTL_TBLID_H5_OHASH].entries);
}

void
ftl_mock_cleanup ()
{
    free(mocktables[FTL_TBLID_H5].entries);
    free(mocktables[FTL_TBLID_H5_OHASH].entries);
}

uint32_t
ftl_mock_get_valid_count (uint32_t table_id)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < table_size_get(table_id); i++) {
        if (mocktables[table_id].entries[i].entry_valid) {
            count++;
        }
    }
    return count;
}

p4pd_error_t
p4pd_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    memset(props, 0, sizeof(p4pd_table_properties_t));
    props->hash_type = 0;
    props->hbm_layout.entry_width = 64;
    props->base_mem_pa = (uint64_t)(mocktables[table_id].entries);
    props->base_mem_va = (uint64_t)(mocktables[table_id].entries);
    props->tabledepth = table_size_get(table_id);

    if (table_id == FTL_TBLID_H5) {
        props->tablename = (char *) "FlowTable";
        props->has_oflow_table = 1;
        props->oflow_table_id = FTL_TBLID_H5_OHASH;
    } else {
        props->tablename = (char *) "FlowOhashTable";
    }

    return 0;
}
p4pd_error_t
p4pd_global_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    return p4pd_table_properties_get(table_id, props);
}
