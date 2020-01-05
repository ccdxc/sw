//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <cinttypes>

#include "lib/p4/p4_api.hpp"
#include "ftl_p4pd_mock.hpp"
#include "gen/p4gen/p4/include/ftl.h"
#include "nic/utils/ftl/ftl_base.hpp"

typedef struct ftl_mock_table_s {
    FLOW_HASH_ENTRY_T *entries;
} ftl_mock_table_t;

static ftl_mock_table_t mocktables[FTL_TBLID_MAX];

typedef int p4pd_error_t;

static uint32_t
table_size_get(uint32_t table_id)
{
    if (table_id == FTL_TBLID_IPV6 ||
        table_id == FTL_TBLID_IPV4) {
        return 8*1024*1024;
    } else if (table_id == FTL_TBLID_IPV6_OHASH ||
               table_id == FTL_TBLID_IPV4_OHASH) {
        return 4*1024*1024;
    }
    return 0;
}

void
ftl_mock_init ()
{
    mocktables[FTL_TBLID_IPV6].entries =
        (FLOW_HASH_ENTRY_T *)calloc(table_size_get(FTL_TBLID_IPV6), FLOW_HASH_ENTRY_T::entry_size());
    assert(mocktables[FTL_TBLID_IPV6].entries);
    mocktables[FTL_TBLID_IPV6_OHASH].entries =
        (FLOW_HASH_ENTRY_T *)calloc(table_size_get(FTL_TBLID_IPV6_OHASH), FLOW_HASH_ENTRY_T::entry_size());
    assert(mocktables[FTL_TBLID_IPV6_OHASH].entries);
    mocktables[FTL_TBLID_IPV4].entries =
        (FLOW_HASH_ENTRY_T *)calloc(table_size_get(FTL_TBLID_IPV4), FLOW_HASH_ENTRY_T::entry_size());
    assert(mocktables[FTL_TBLID_IPV4].entries);
    mocktables[FTL_TBLID_IPV4_OHASH].entries =
        (FLOW_HASH_ENTRY_T *)calloc(table_size_get(FTL_TBLID_IPV4_OHASH), FLOW_HASH_ENTRY_T::entry_size());
    assert(mocktables[FTL_TBLID_IPV4_OHASH].entries);
}

void
ftl_mock_cleanup ()
{
    free(mocktables[FTL_TBLID_IPV6].entries);
    free(mocktables[FTL_TBLID_IPV6_OHASH].entries);
    free(mocktables[FTL_TBLID_IPV4].entries);
    free(mocktables[FTL_TBLID_IPV4_OHASH].entries);
}

uint32_t
ftl_mock_get_valid_count (uint32_t table_id)
{
    uint32_t count = 0;
    uint32_t size = table_size_get(table_id);
    SDK_TRACE_VERBOSE("size of table id : %lu is %lu ", table_id, size);

    for (uint32_t i = 0; i < size; i++) {
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

    if (table_id == FTL_TBLID_IPV6) {
        props->tablename = (char *) "Ipv6FlowTable";
        props->has_oflow_table = 1;
        props->oflow_table_id = FTL_TBLID_IPV6_OHASH;
    } else if (table_id == FTL_TBLID_IPV4) {
        props->tablename = (char *) "Ipv4FlowTable";
        props->has_oflow_table = 1;
        props->oflow_table_id = FTL_TBLID_IPV4_OHASH;
    } else if (table_id == FTL_TBLID_IPV6_OHASH) {
        props->tablename = (char *) "Ipv6FlowOhashTable";
    } else if (table_id == FTL_TBLID_IPV4_OHASH) {
        props->tablename = (char *) "Ipv4FlowOhashTable";
    } else {
        assert(0);
    }

    return 0;
}
p4pd_error_t
p4pd_global_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    return p4pd_table_properties_get(table_id, props);
}
