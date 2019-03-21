//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// stateless tcam table management library
//------------------------------------------------------------------------------
#include <string.h>

#include "lib/p4/p4_api.hpp"
#include "include/sdk/table.hpp"
#include "include/sdk/base.hpp"

#include "sltcam_properties.hpp"
#include "sltcam_api_context.hpp"
#include "sltcam_utils.hpp"

namespace sdk {
namespace table {
namespace sltcam_internal {

char*
rawstr(void *data, uint32_t len) {
    static char str[512];
    uint32_t i = 0;
    uint32_t slen = 0;
    for (i = 0; i < len; i++) {
        slen += sprintf(str+slen, "%02x", ((uint8_t*)data)[i]);
    }
    return str;
}

int
sltctx::swcompare(uint32_t idx) {
    static uint8_t temp_swkey[SDK_TABLE_MAX_SW_KEY_LEN];
    static uint8_t temp_swkeymask[SDK_TABLE_MAX_SW_KEY_LEN];
    static uint8_t temp_swdata[SDK_TABLE_MAX_SW_DATA_LEN];
    p4pd_error_t p4pdret;
    p4pdret =  p4pd_entry_read(props->table_id, idx,
                               temp_swkey, temp_swkeymask, temp_swdata);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    return memcmp(params->key, temp_swkey, props->swkey_len);
}

int
sltctx::swcompare(uint32_t idx1, uint32_t idx2) {
    uint8_t swkey1[SDK_TABLE_MAX_SW_KEY_LEN];
    uint8_t swkeymask1[SDK_TABLE_MAX_SW_KEY_LEN];
    uint8_t swdata1[SDK_TABLE_MAX_SW_DATA_LEN];
    uint8_t swkey2[SDK_TABLE_MAX_SW_KEY_LEN];
    uint8_t swkeymask2[SDK_TABLE_MAX_SW_KEY_LEN];
    uint8_t swdata2[SDK_TABLE_MAX_SW_DATA_LEN];
    p4pd_error_t p4pdret;

    p4pdret =  p4pd_entry_read(props->table_id, idx1,
                               swkey1, swkeymask1, swdata1);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    p4pdret =  p4pd_entry_read(props->table_id, idx2,
                               swkey2, swkeymask2, swdata2);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    return memcmp(swkey1, swkey2, props->swkey_len);
}

void
sltctx::clearsw() {
    // KEY's default value is always FF
    memset(swkey, 0xFF, SDK_TABLE_MAX_SW_KEY_LEN);
    memset(swkeymask, 0xFF, SDK_TABLE_MAX_SW_KEY_LEN);
    // DATA's default value is always 0
    memset(swdata, 0, SDK_TABLE_MAX_SW_DATA_LEN);
    return;
}

void
sltctx::clearhw() {
    // KEY's default value is always FF
    memset(hwkey, 0xFF, SDK_TABLE_MAX_HW_KEY_LEN);
    memset(hwkeymask, 0xFF, SDK_TABLE_MAX_HW_KEY_LEN);
    // DATA's default value is always 0
    memset(hwdata, 0, SDK_TABLE_MAX_HW_DATA_LEN);
    return;
}

void
sltctx::copyin() {
    memcpy(swkey, params->key, props->swkey_len);
    memcpy(swkeymask, params->mask, props->swkey_len);
    memcpy(swdata, params->appdata, props->swdata_len);
    return;
}

void
sltctx::copyout() {
    if (params->key) {
        memcpy(params->key, swkey, props->swkey_len);
    }
    if (params->mask) {
        memcpy(params->mask, swkeymask, props->swkey_len);
    }
    if (params->appdata) {
        memcpy(params->appdata, swdata, props->swdata_len);
    }
    return;
}

static inline void
printbytes(const char *name, bytes2str_t b2s, void *b, uint32_t len) {
    if (b) {
        SLTCAM_TRACE_VERBOSE("- %s:[%s]", name, b2s ? b2s(b) : rawstr(b, len));
    }
}

void
sltctx::print_sw() {
    SLTCAM_TRACE_VERBOSE("SW Fields");
    printbytes("Key", props->key2str, swkey, props->swkey_len);
    printbytes("Mask", props->key2str, swkeymask, props->swkey_len);
    printbytes("Data", props->data2str, swdata, props->swdata_len);
}

void
sltctx::print_hw() {
    SLTCAM_TRACE_VERBOSE("HW Fields");
    printbytes("Key", props->key2str, hwkey, props->hwkey_len);
    printbytes("Mask", props->key2str, hwkeymask, props->hwkey_len);
    printbytes("Data", props->data2str, hwdata, props->hwdata_len);
}

void
sltctx::print_params() {
    if (params) {
        SLTCAM_TRACE_VERBOSE("Input Params");
        printbytes("Key", props->key2str, params->key, props->swkey_len);
        printbytes("Mask", props->key2str, params->mask, props->swkey_len);
        printbytes("Data", props->data2str, params->appdata, props->swdata_len);
    }
}

sdk_ret_t 
sltctx::init() {
    params = NULL;
    tcam_index = 0;
    dbslot = 0;
    tcam_index_valid = false;
    dbslot_valid = false;

    clearsw();
    clearhw();
    return sdk::SDK_RET_OK;
}

} // namespace sltcam_internal
} // namespace table
} // namespace sdk
