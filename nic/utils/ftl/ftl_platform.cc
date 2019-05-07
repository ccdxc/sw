//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/platform/capri/capri_tbl_rw.hpp"

#include "ftl_apictx.hpp"
#include "ftl_platform.hpp"
#include "ftl_utils.hpp"

using namespace sdk::lib;
using namespace sdk::platform::capri;

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
    if (ctx->props->stable_base_mem_va && ctx->props->ptable_base_mem_va) {
        auto baseaddr = ctx->level ? ctx->props->stable_base_mem_va
                                   : ctx->props->ptable_base_mem_va;
        memcpy(&ctx->entry, ((ftl_entry_t *)baseaddr)+ctx->table_index,
               sizeof(ftl_entry_t));
    } else if (ctx->props->stable_base_mem_pa && ctx->props->ptable_base_mem_pa) {
        auto baseaddr = ctx->level ? ctx->props->stable_base_mem_pa
                                   : ctx->props->ptable_base_mem_pa;
        pal_mem_read(baseaddr+ctx->table_index*sizeof(ftl_entry_t),
                     (uint8_t*)&ctx->entry, sizeof(ftl_entry_t));
    } else {
        assert(0);
    }

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

    ftl_swap_bytes((uint8_t *)&ctx->entry);
    if (ctx->props->stable_base_mem_va && ctx->props->ptable_base_mem_va) {
        auto baseaddr = ctx->level ? ctx->props->stable_base_mem_va
                                   : ctx->props->ptable_base_mem_va;
        memcpy(((ftl_entry_t *)baseaddr)+ctx->table_index,
               &ctx->entry, sizeof(ftl_entry_t));
    } else if (ctx->props->stable_base_mem_pa && ctx->props->ptable_base_mem_pa) {
        auto baseaddr = ctx->level ? ctx->props->stable_base_mem_pa
                                   : ctx->props->ptable_base_mem_pa;
        pal_mem_write(baseaddr+ctx->table_index*sizeof(ftl_entry_t),
                      (uint8_t*)&ctx->entry, sizeof(ftl_entry_t));
    } else {
        assert(0);
    }
#ifndef SIM
    // TODO: we need a asicpd API here instead of using capri
    // @hareesh, can you please check how we can make this work efficiently even
    // for elba ? also direction here is hard-coded, should we take this in
    // factory call ?
    auto baseaddr = ctx->level ? ctx->props->stable_base_mem_pa :
                                 ctx->props->ptable_base_mem_pa;
    capri_hbm_table_entry_cache_invalidate(
        true,    // ctx->props->gress == P4_GRESS_INGRESS,
        ctx->table_index * sizeof(ftl_entry_t), baseaddr);
#endif

    return SDK_RET_OK;
}
