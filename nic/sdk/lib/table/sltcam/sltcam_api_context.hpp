//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// stateless tcam table management library
//------------------------------------------------------------------------------
#ifndef __SDK_SLTCAM_API_CONTEXT_HPP__
#define __SDK_SLTCAM_API_CONTEXT_HPP__
#include <string.h>

#include "lib/p4/p4_api.hpp"
#include "include/sdk/table.hpp"
#include "include/sdk/base.hpp"

#include "sltcam_properties.hpp"

namespace sdk {
namespace table {
namespace sltcam_internal {

class sltctx {
public:
    sdk::table::sltcam_internal::properties *props;
    sdk_table_api_params_t *params;
    sdk_table_api_op_t op;

    // TCAM Index
    bool tcam_index_valid;
    uint32_t tcam_index;

    // Tree Index
    bool dbslot_valid;
    uint32_t dbslot;

    // SW Data
    uint8_t swkey[SDK_TABLE_MAX_SW_KEY_LEN];
    uint8_t swkeymask[SDK_TABLE_MAX_SW_KEY_LEN];
    uint8_t swdata[SDK_TABLE_MAX_SW_DATA_LEN];

    // HW Data
    uint8_t hwkey[SDK_TABLE_MAX_HW_KEY_LEN];
    uint8_t hwkeymask[SDK_TABLE_MAX_HW_KEY_LEN];
    uint8_t hwdata[SDK_TABLE_MAX_HW_DATA_LEN];

public:
    sltctx() { init(); }
    ~sltctx() {}

    int swcompare(uint32_t idx);
    int swcompare(uint32_t idx1, uint32_t idx2);
    sdk_ret_t init();
    void clearsw();
    void clearhw();
    void copyin();
    void copyout();

    void print_sw();
    void print_hw();
    void print_params();
};

} // namespace sltcam_internal
} // namespace table
} // namespace sdk

#endif // __SDK_SLTCAM_API_CONTEXT_HPP__
