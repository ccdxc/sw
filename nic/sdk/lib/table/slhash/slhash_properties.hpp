//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// stateless tcam table management library
//------------------------------------------------------------------------------
#ifndef __SDK_SLHASH_PROPERTIES_HPP__
#define __SDK_SLHASH_PROPERTIES_HPP__

#include "lib/p4/p4_api.hpp"
#include "include/sdk/table.hpp"
#include "include/sdk/base.hpp"

#include "slhash_utils.hpp"

namespace sdk {
namespace table {
namespace slhash_internal {

class properties {
public:
    char *name; // table name
    uint32_t table_id; // table id
    uint32_t table_size; // size of tcam table
    uint32_t swkey_len; // sw key len
    uint32_t swdata_len; // sw data len
    uint32_t hwkey_len; // hw key len
    uint32_t hwkeymask_len; // hw key mask len
    uint32_t hwdata_len; // hw data len
    uint32_t hash_poly; // hash polynomial
    bool entry_trace_en; // enable entry tracing
    sdk::table::health_t health; // health of the table
    bytes2str_t key2str;
    bytes2str_t data2str;

    // Overflow Table ID
    bool has_oflow_table;
    uint32_t oflow_table_id;

public:
    properties() {
        table_id = 0;
        table_size = 0;
        swkey_len = 0;
        swdata_len = 0;
        hwkey_len = 0;
        hwkeymask_len = 0;
        hwdata_len = 0;
        hash_poly = 0;
        entry_trace_en = false;
        health = sdk::table::HEALTH_GREEN;
    }

    ~properties() {
    }

    sdk_ret_t init(sdk_table_factory_params_t *params) {
        p4pd_error_t p4pdret;
        p4pd_table_properties_t tinfo;

        p4pdret = p4pd_global_table_properties_get(params->table_id, &tinfo);
        SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

        name = tinfo.tablename;
        table_id = params->table_id;
        table_size = tinfo.tabledepth;
        swkey_len = tinfo.key_struct_size;
        SDK_ASSERT(swkey_len <= SDK_TABLE_MAX_SW_KEY_LEN);
        swdata_len = tinfo.actiondata_struct_size;
        SDK_ASSERT(swdata_len <= SDK_TABLE_MAX_SW_DATA_LEN);

        p4pd_global_hwentry_query(table_id, &hwkey_len,
                                  &hwkeymask_len, &hwdata_len);
        hwkey_len = SDK_TABLE_BITS_TO_BYTES(hwkey_len);
        SDK_ASSERT(hwkey_len <= SDK_TABLE_MAX_HW_KEY_LEN);
        hwkeymask_len = SDK_TABLE_BITS_TO_BYTES(hwkeymask_len);
        SDK_ASSERT(hwkeymask_len <= SDK_TABLE_MAX_HW_KEY_LEN);
        hwdata_len = SDK_TABLE_BITS_TO_BYTES(hwdata_len);
        SDK_ASSERT(hwdata_len <= SDK_TABLE_MAX_HW_DATA_LEN);

        hash_poly = tinfo.hash_type;

        key2str = params->key2str;
        data2str = params->appdata2str;
        entry_trace_en = params->entry_trace_en;

        has_oflow_table = tinfo.has_oflow_table;
        oflow_table_id = tinfo.oflow_table_id;

        SLHASH_TRACE_DEBUG("SLHASH Properties table=%s table_id=%d size=%d "
                        "swkey_len=%d swdata_len=%d hwkey_len=%d "
                        "hwdata_len=%d hwkeymask_len=%d entry_trace_en=%d"
                        "has_oflow_table:%d,oflow_table_id:%d",
                        name, table_id, table_size, swkey_len, swdata_len,
                        hwkey_len, hwdata_len, hwkeymask_len, entry_trace_en,
                        has_oflow_table, oflow_table_id);

        return SDK_RET_OK;
    }

};

} // namespace slhash_internal
} // namespace table
} // namespace sdk

#endif // __SDK_SLHASH_PROPERTIES_HPP__
