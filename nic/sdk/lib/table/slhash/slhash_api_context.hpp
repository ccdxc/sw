//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// stateless tcam table management library
//------------------------------------------------------------------------------
#ifndef __SDK_SLHASH_API_CONTEXT_HPP__
#define __SDK_SLHASH_API_CONTEXT_HPP__
#include <string.h>

#include "lib/p4/p4_api.hpp"
#include "lib/utils/crc_fast.hpp"
#include "include/sdk/table.hpp"
#include "include/sdk/base.hpp"

#include "slhash_properties.hpp"

namespace sdk {
namespace table {
namespace slhash_internal {

class slhctx {
public:
    sdk::table::slhash_internal::properties *props;
    sdk_table_api_params_t *params;
    sdk_table_api_op_t op;

    // Full 32bit hash value
    bool hash_valid;
    uint32_t hash_32b;
    
    // Index
    bool index_valid;
    uint32_t index;

    // SW Data
    uint8_t swkey[SDK_TABLE_MAX_SW_KEY_LEN];
    uint8_t swkeymask[SDK_TABLE_MAX_SW_KEY_LEN];
    uint8_t swdata[SDK_TABLE_MAX_SW_DATA_LEN];

    // HW Key for Hash Calculation
    uint8_t hwkey[SDK_TABLE_MAX_HW_KEY_LEN];

    // Output Handle
    sdk::table::handle_t ohandle;

    // Params for tcam apis
    bool tcam_params_valid;
    sdk_table_api_params_t tcam_params;

public:
    slhctx() { init(sdk::table::SDK_TABLE_API_NONE, NULL, NULL); }
        
    int keycompare();
    sdk_ret_t init(sdk_table_api_op_t op,
                   sdk_table_api_params_t *params,
                   sdk::table::slhash_internal::properties *props);
    
    sdk_ret_t calchash();
    sdk_ret_t read();
    sdk_ret_t write();

    void clear_();
    void copyin_();
    void copyout();

    void print_sw();
    void print_hw();
    void print_params();

    sdk::table::handle_t inhandle();
    sdk::table::handle_t outhandle();
};

} // namespace slhash_internal
} // namespace table
} // namespace sdk

#endif // __SDK_SLHASH_API_CONTEXT_HPP__
