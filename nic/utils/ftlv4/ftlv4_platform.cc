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

#include "ftlv4_apictx.hpp"
#include "ftlv4_platform.hpp"
#include "ftlv4_utils.hpp"

using namespace sdk::lib;
using namespace sdk::platform::capri;

sdk_ret_t
ftlv4_platform_init(sdk::table::properties_t *props) {
    return SDK_RET_OK;
}

void
ftlv4_swap_bytes(uint8_t *entry) {
    // swap bytes. hash key is always 64 bytes.
    for(uint32_t i = 0; i < sizeof(ftlv4_entry_t) / 2; ++i) {
        uint8_t b = entry[i];
        entry[i] = entry[sizeof(ftlv4_entry_t) - 1 - i];
        entry[sizeof(ftlv4_entry_t) - 1 - i] = b;
    }
    return;
}

sdk_ret_t
ftlv4_platform_read(ftlv4_apictx *ctx) {
    if (ctx->props->stable_base_mem_va && ctx->props->ptable_base_mem_va) {
        auto baseaddr = ctx->level ? ctx->props->stable_base_mem_va
                                   : ctx->props->ptable_base_mem_va;
        memcpy(&ctx->entry, ((ftlv4_entry_t *)baseaddr)+ctx->table_index,
               sizeof(ftlv4_entry_t));
    } else if (ctx->props->stable_base_mem_pa && ctx->props->ptable_base_mem_pa) {
        auto baseaddr = ctx->level ? ctx->props->stable_base_mem_pa
                                   : ctx->props->ptable_base_mem_pa;
        pal_mem_read(baseaddr+ctx->table_index*sizeof(ftlv4_entry_t),
                     (uint8_t*)&ctx->entry, sizeof(ftlv4_entry_t));
    } else {
        assert(0);
    }

    ftlv4_swap_bytes((uint8_t *)&ctx->entry);
    return SDK_RET_OK;
}

sdk_ret_t
ftlv4_platform_write(ftlv4_apictx *ctx) {
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

    ftlv4_swap_bytes((uint8_t *)&ctx->entry);
    if (ctx->props->stable_base_mem_va && ctx->props->ptable_base_mem_va) {
        auto baseaddr = ctx->level ? ctx->props->stable_base_mem_va
                                   : ctx->props->ptable_base_mem_va;
        memcpy(((ftlv4_entry_t *)baseaddr)+ctx->table_index,
               &ctx->entry, sizeof(ftlv4_entry_t));
    } else if (ctx->props->stable_base_mem_pa && ctx->props->ptable_base_mem_pa) {
        auto baseaddr = ctx->level ? ctx->props->stable_base_mem_pa
                                   : ctx->props->ptable_base_mem_pa;
        pal_mem_write(baseaddr+ctx->table_index*sizeof(ftlv4_entry_t),
                      (uint8_t*)&ctx->entry, sizeof(ftlv4_entry_t));
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
        ctx->table_index * sizeof(ftlv4_entry_t), baseaddr);
#endif

    return SDK_RET_OK;
}
