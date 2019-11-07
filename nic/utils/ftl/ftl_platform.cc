//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "ftl_includes.hpp"
namespace sdk {
namespace table {
namespace FTL_MAKE_AFTYPE(internal) {

sdk_ret_t
memrd(FTL_MAKE_AFTYPE(apictx) *ctx) {
    if (ctx->props->stable_base_mem_va && ctx->props->ptable_base_mem_va) {
        auto baseva = ctx->level ? ctx->props->stable_base_mem_va
                                 : ctx->props->ptable_base_mem_va;
        auto baseentry = (FTL_MAKE_AFTYPE(entry_t) *)baseva;
        FTL_MAKE_AFTYPE(memcpy)(&ctx->entry, baseentry + ctx->table_index);
    } else if (ctx->props->stable_base_mem_pa && ctx->props->ptable_base_mem_pa) {
        auto basepa = ctx->level ? ctx->props->stable_base_mem_pa
                                 : ctx->props->ptable_base_mem_pa;
        auto baseentry = (FTL_MAKE_AFTYPE(entry_t) *)basepa;
        pal_mem_read((uint64_t)(baseentry + ctx->table_index),
                     (uint8_t*)&ctx->entry, sizeof(FTL_MAKE_AFTYPE(entry_t)));
    }
    FTL_MAKE_AFTYPE(swizzle)(&ctx->entry);
    return SDK_RET_OK;
}

sdk_ret_t
memwr(FTL_MAKE_AFTYPE(apictx) *ctx) {
    FTL_MAKE_AFTYPE(swizzle)(&ctx->entry);
    if (ctx->props->stable_base_mem_va && ctx->props->ptable_base_mem_va) {
        auto baseva = ctx->level ? ctx->props->stable_base_mem_va
                                 : ctx->props->ptable_base_mem_va;
        auto baseentry = (FTL_MAKE_AFTYPE(entry_t) *)baseva;
        FTL_MAKE_AFTYPE(memcpy)(baseentry + ctx->table_index, &ctx->entry);
        //FTL_TRACE_ERR("Wrote to :0x%llx", baseentry + ctx->table_index);
    } else if (ctx->props->stable_base_mem_pa && ctx->props->ptable_base_mem_pa) {
        auto basepa = ctx->level ? ctx->props->stable_base_mem_pa
                                 : ctx->props->ptable_base_mem_pa;
        auto baseentry = (FTL_MAKE_AFTYPE(entry_t) *)basepa;
        pal_mem_write((uint64_t)(baseentry + ctx->table_index),
                      (uint8_t*)&ctx->entry, sizeof(FTL_MAKE_AFTYPE(entry_t)));
        //FTL_TRACE_ERR("Wrote to :0x%llx", baseentry + ctx->table_index);
    }

#ifndef SIM
    // TODO: we need a asicpd API here instead of using capri
    // @hareesh, can you please check how we can make this work efficiently even
    // for elba ? also direction here is hard-coded, should we take this in
    // factory call ?
    auto basepa = ctx->level ? ctx->props->stable_base_mem_pa :
                               ctx->props->ptable_base_mem_pa;
    auto size = ctx->table_index * sizeof(FTL_MAKE_AFTYPE(entry_t));
    capri_hbm_table_entry_cache_invalidate(P4_TBL_CACHE_INGRESS, size, 1, basepa);
#endif

    return SDK_RET_OK;
}

sdk_ret_t
memclr(uint64_t memva, uint64_t mempa, uint32_t num_entries) {
    memset((void*)memva, 0, sizeof(FTL_MAKE_AFTYPE(entry_t))*num_entries);

    PAL_barrier();

    for (uint32_t i = 0; i < num_entries; i++) {
        capri_hbm_table_entry_cache_invalidate(P4_TBL_CACHE_INGRESS,
                                               (i * sizeof(FTL_MAKE_AFTYPE(entry_t))),
                                               1, mempa);
    }

    return SDK_RET_OK;
}

} // namespace FTL_MAKE_AFTYPE(internal)
} // namespace table
} // namespace sdk

