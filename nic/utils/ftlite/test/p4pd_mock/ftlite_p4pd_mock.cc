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
#include "ftlite_p4pd_mock.hpp"
#include "nic/utils/ftlite/ftlite_ipv4_structs.hpp"
#include "nic/utils/ftlite/ftlite_ipv6_structs.hpp"

using namespace ftlite::internal;

typedef union ftlite_mock_table_s {
    ipv4_entry_t *v4entries;
    ipv6_entry_t *v6entries;
} ftlite_mock_table_t;

static ftlite_mock_table_t mocktables[FTLITE_TBLID_MAX];

typedef int p4pd_error_t;

static uint32_t
table_size_get(uint32_t table_id)
{
    if (table_id == FTLITE_TBLID_IPV4 ||
        table_id == FTLITE_TBLID_IPV6) {
        return 8*1024*1024;
    } else if (table_id == FTLITE_TBLID_IPV4_OHASH ||
               table_id == FTLITE_TBLID_IPV6_OHASH) {
        return 4*1024*1024;
    }
    return 0;
}

void
ftlite_mock_init ()
{
    mocktables[FTLITE_TBLID_IPV4].v4entries =
        (ipv4_entry_t *)calloc(table_size_get(FTLITE_TBLID_IPV4),
                                      sizeof(ipv4_entry_t));
    assert(mocktables[FTLITE_TBLID_IPV4].v4entries);

    mocktables[FTLITE_TBLID_IPV4_OHASH].v4entries =
        (ipv4_entry_t *)calloc(table_size_get(FTLITE_TBLID_IPV4_OHASH),
                                      sizeof(ipv4_entry_t));
    assert(mocktables[FTLITE_TBLID_IPV4_OHASH].v4entries);

    mocktables[FTLITE_TBLID_IPV6].v6entries =
        (ipv6_entry_t *)calloc(table_size_get(FTLITE_TBLID_IPV6),
                                      sizeof(ipv6_entry_t));
    assert(mocktables[FTLITE_TBLID_IPV6].v6entries);

    mocktables[FTLITE_TBLID_IPV6_OHASH].v6entries =
        (ipv6_entry_t *)calloc(table_size_get(FTLITE_TBLID_IPV6_OHASH),
                                      sizeof(ipv6_entry_t));
    assert(mocktables[FTLITE_TBLID_IPV6_OHASH].v6entries);

}

void
ftlite_mock_cleanup ()
{
    free(mocktables[FTLITE_TBLID_IPV4].v4entries);
    free(mocktables[FTLITE_TBLID_IPV4_OHASH].v4entries);
    free(mocktables[FTLITE_TBLID_IPV6].v6entries);
    free(mocktables[FTLITE_TBLID_IPV6_OHASH].v6entries);
}

uint32_t
ftlite_mock_get_valid_count (uint32_t table_id)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < table_size_get(table_id); i++) {
        if (table_id == FTLITE_TBLID_IPV4 ||
            table_id == FTLITE_TBLID_IPV4_OHASH) {
            if (mocktables[table_id].v4entries[i].entry_valid) {
                count++;
            }
        } else {
            if (mocktables[table_id].v6entries[i].entry_valid) {
                count++;
            }
        }
    }
    return count;
}

p4pd_error_t
p4pd_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    memset(props, 0, sizeof(p4pd_table_properties_t));
    props->hash_type = 0;
    props->tabledepth = table_size_get(table_id);

    if (table_id == FTLITE_TBLID_IPV4) {
        props->tablename = (char *) "Ipv4FlowTable";
        props->has_oflow_table = 1;
        props->oflow_table_id = FTLITE_TBLID_IPV4_OHASH;
        props->base_mem_va = (uint64_t)(mocktables[table_id].v4entries);
        props->hbm_layout.entry_width = 32;
    } else if (table_id == FTLITE_TBLID_IPV4_OHASH) {
        props->tablename = (char *) "Ipv4FlowOhashTable";
        props->base_mem_va = (uint64_t)(mocktables[table_id].v4entries);
    } else if (table_id == FTLITE_TBLID_IPV6) {
        props->tablename = (char *) "Ipv6FlowTable";
        props->has_oflow_table = 1;
        props->oflow_table_id = FTLITE_TBLID_IPV6_OHASH;
        props->base_mem_va = (uint64_t)(mocktables[table_id].v6entries);
        props->hbm_layout.entry_width = 64;
    } else {
        props->tablename = (char *) "Ipv6FlowOhashTable";
        props->base_mem_va = (uint64_t)(mocktables[table_id].v6entries);
    }

    return 0;
}
p4pd_error_t
p4pd_global_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    return p4pd_table_properties_get(table_id, props);
}

namespace sdk {
namespace platform {
namespace capri {

int capri_hbm_table_entry_cache_invalidate(bool ingress,
                                           uint64_t entry_addr,
                                           mem_addr_t base_mem_pa) {
    return 0;
}


int capri_hbm_table_entry_cache_invalidate(p4pd_table_cache_t cache,
                                           uint64_t entry_addr,
                                           uint16_t entry_width,
                                           mem_addr_t base_mem_pa) {
    return 0;
}


}
}
}

