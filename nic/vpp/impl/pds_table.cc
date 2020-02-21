//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
// Hardware table access interface

#include <nic/sdk/lib/p4/p4_utils.hpp>
#include <nic/sdk/lib/p4/p4_api.hpp>
#include "pds_table.h"

int
pds_table_read (uint32_t table_id, uint32_t key, void *action)
{
    p4pd_error_t p4pd_ret0;

    p4pd_ret0 = p4pd_global_entry_read(table_id, key,
                                       NULL, NULL, action);
    if (p4pd_ret0 != P4PD_SUCCESS) {
        return -1;
    }
    return 0;
}

int
pds_table_write (uint32_t table_id, uint32_t key, void *action)
{
    p4pd_error_t p4pd_ret0;

    p4pd_ret0 = p4pd_global_entry_write(table_id, key,
                                        NULL, NULL, action);
    if (p4pd_ret0 != P4PD_SUCCESS) {
        return -1;
    }
    return 0;
}

