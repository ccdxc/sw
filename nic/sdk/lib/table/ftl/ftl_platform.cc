//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "ftl_includes.hpp"

namespace sdk {
namespace table {
namespace internal {

static inline uint64_t
get_va (Apictx *ctx) {
    uint64_t baseva = ctx->level ? ctx->props->stable_base_mem_va
                             : ctx->props->ptable_base_mem_va;
    return baseva + (ctx->entry->entry_size() * ctx->table_index);
}

static inline uint64_t
get_pa (Apictx *ctx) {
    uint64_t basepa = ctx->level ? ctx->props->stable_base_mem_pa
                             : ctx->props->ptable_base_mem_pa;
    return basepa + (ctx->entry->entry_size() * ctx->table_index);
}

sdk_ret_t
memrd(Apictx *ctx) {
    if (ctx->props->stable_base_mem_va && ctx->props->ptable_base_mem_va) {
        ftl_memcpy(get_sw_entry_pointer(ctx->entry),
                   (uint8_t *)get_va(ctx),
                   ctx->entry->entry_size());
        // FTL_TRACE_ERR("Read from :0x%llx", get_va(ctx));
    } else if (ctx->props->stable_base_mem_pa && ctx->props->ptable_base_mem_pa) {
        pal_mem_read(get_pa(ctx),
                     (uint8_t*)get_sw_entry_pointer(ctx->entry),
                     ctx->entry->entry_size());
    }
    sdk::lib::swizzle(get_sw_entry_pointer(ctx->entry), ctx->entry->entry_size());
    return SDK_RET_OK;
}

sdk_ret_t
memwr(Apictx *ctx) {
    sdk::lib::swizzle(get_sw_entry_pointer(ctx->entry), ctx->entry->entry_size());
    if (ctx->props->stable_base_mem_va && ctx->props->ptable_base_mem_va) {
        ftl_memcpy((uint8_t *)get_va(ctx),
                   get_sw_entry_pointer(ctx->entry),
                   ctx->entry->entry_size());
        // FTL_TRACE_ERR("Wrote to :0x%llx", get_va(ctx));
    } else if (ctx->props->stable_base_mem_pa && ctx->props->ptable_base_mem_pa) {
        pal_mem_write(get_pa(ctx),
                      (uint8_t*)get_sw_entry_pointer(ctx->entry),
                      ctx->entry->entry_size());
        // FTL_TRACE_ERR("Wrote to :0x%llx", get_pa(ctx));
    }

#ifndef SIM
    // TODO: we need a asicpd API here instead of using capri
    // @hareesh, can you please check how we can make this work efficiently even
    // for elba ? also direction here is hard-coded, should we take this in
    // factory call ?
    auto basepa = ctx->level ? ctx->props->stable_base_mem_pa :
                               ctx->props->ptable_base_mem_pa;
    auto size = ctx->table_index * ctx->entry->entry_size();
    capri_hbm_table_entry_cache_invalidate(P4_TBL_CACHE_INGRESS, size, 1, basepa);
#endif

    return SDK_RET_OK;
}

sdk_ret_t
memclr(uint64_t memva, uint64_t mempa, uint32_t num_entries,
       uint32_t entry_size) {
    if (memva) {
        memset((void*)memva, 0, entry_size * num_entries);
    } else if (mempa) {
        pal_mem_set(mempa, 0, entry_size * num_entries, 0);
    }

    PAL_barrier();

#ifndef SIM
    for (uint32_t i = 0; i < num_entries; i++) {
        capri_hbm_table_entry_cache_invalidate(P4_TBL_CACHE_INGRESS,
                                               (i * entry_size),
                                               1, mempa);
    }
#endif

    return SDK_RET_OK;
}

} // namespace internal
} // namespace table
} // namespace sdk
