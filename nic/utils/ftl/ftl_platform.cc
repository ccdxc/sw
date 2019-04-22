//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/include/sdk/platform.hpp"

#include "ftl_apictx.hpp"
#include "ftl_platform.hpp"
#include "ftl_utils.hpp"

using namespace sdk::lib;

sdk_ret_t
ftl_platform_init(sdk::table::properties_t *props) {
    return SDK_RET_OK;
}

void
ftl_swap_bytes(uint8_t *entry) {
    // swap bytes. hash key is always 64 bytes.
    for(auto i = 0; i < 32; ++i) {
        uint8_t b = entry[i];
        entry[i] = entry[63 - i];
        entry[63 - i] = b;
    }
    return;
}

sdk_ret_t
ftl_platform_read(ftl_apictx *ctx) {
    auto baseaddr = ctx->level ? ctx->props->stable_base_mem_va
                               : ctx->props->ptable_base_mem_va;
    memcpy(&ctx->entry, ((ftl_entry_t *)baseaddr)+ctx->table_index,
           sizeof(ftl_entry_t));
    ftl_swap_bytes((uint8_t *)&ctx->entry);
    return SDK_RET_OK;
}

sdk_ret_t
ftl_platform_write(ftl_apictx *ctx) {
#if 0
    static bool mmap_done = false;
    static void *base = NULL;
    if(mmap_done == false) {
        sdk::lib::pal_init(sdk::platform::platform_type_t::PLATFORM_TYPE_HW);
        base = sdk::lib::pal_mem_map(0x141201000, 1073741824);
        printf("BASE %p\n", base);
        mmap_done = true;
    }
    auto baseaddr = base;
#endif
    auto baseaddr = ctx->level ? ctx->props->stable_base_mem_va
                               : ctx->props->ptable_base_mem_va;
    ftl_swap_bytes((uint8_t *)&ctx->entry);
    memcpy(((ftl_entry_t *)baseaddr)+ctx->table_index,
           &ctx->entry, sizeof(ftl_entry_t));
    return SDK_RET_OK;
}
