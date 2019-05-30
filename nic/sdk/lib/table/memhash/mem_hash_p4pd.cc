//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "mem_hash_api_context.hpp"
#include "mem_hash_p4pd.hpp"
#include "mem_hash_utils.hpp"

#ifdef MEMHASH_PERF_DBG_ENABLE
#define MEMHASH_P4PD_SET_DISABLE true
#define MEMHASH_P4PD_GET_DISABLE true
#define MEMHASH_P4PD_KEY_BUILD_DISABLE true
#define MEMHASH_P4PD_READ_DISABLE true
#define MEMHASH_P4PD_INSTALL_DISABLE true
#else
#define MEMHASH_P4PD_SET_DISABLE false
#define MEMHASH_P4PD_GET_DISABLE false
#define MEMHASH_P4PD_KEY_BUILD_DISABLE false
#define MEMHASH_P4PD_READ_DISABLE false
#define MEMHASH_P4PD_INSTALL_DISABLE false
#endif

#define HWFIELD_SLOT_ID_ENTRY_VALID     0
#define HWFIELD_SLOT_ID_HASH(_slot) ((2*(_slot))-1)
#define HWFIELD_SLOT_ID_HINT(_slot) (2*(_slot))
#define HWFIELD_SLOT_ID_MORE_HASHS(_nhints) ((2*(_nhints))+1)
#define HWFIELD_SLOT_ID_MORE_HINTS(_nhints) ((2*(_nhints))+2)

using sdk::table::memhash::mem_hash_api_context;

static struct memhash_p4pd_api_stats_s {
    uint32_t s_hwfield_set;
    uint32_t s_hwfield_get;
    uint32_t s_appdata_get;
    uint32_t s_appdata_set;
    uint32_t s_entry_read;
    uint32_t s_entry_install;
    uint32_t s_hwkey_build;
} memhash_p4pd_api_stats;

#ifdef MEMHASH_P4PD_STATS_ENABLE
#define MEMHASH_P4PD_STATS_INCR(_name) (memhash_p4pd_api_stats.s_ ## _name)++
#else
#define MEMHASH_P4PD_STATS_INCR(_name)
#endif

void
mem_hash_p4pd_stats_print() {
    MEMHASH_TRACE_VERBOSE("hwfield set:%d get:%d "
                          "appdata set:%d get:%d "
                          "entry read:%d install:%d "
                          "hwkey build:%d",
                          memhash_p4pd_api_stats.s_appdata_set,
                          memhash_p4pd_api_stats.s_appdata_get,
                          memhash_p4pd_api_stats.s_hwfield_set,
                          memhash_p4pd_api_stats.s_hwfield_get,
                          memhash_p4pd_api_stats.s_entry_read,
                          memhash_p4pd_api_stats.s_entry_install,
                          memhash_p4pd_api_stats.s_hwkey_build);
    return;
}

void
mem_hash_p4pd_stats_reset() {
    memset(&memhash_p4pd_api_stats, 0, sizeof(memhash_p4pd_api_stats));
}

p4pd_error_t
mem_hash_p4pd_hwkey_hwmask_build(uint32_t tableid, void *swkey,
                                 void *swkey_mask, uint8_t *hw_key,
                                 uint8_t *hw_key_y) {
    if (MEMHASH_P4PD_KEY_BUILD_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(hwkey_build);
    return p4pd_global_hwkey_hwmask_build(tableid, swkey, swkey_mask,
                                          hw_key, hw_key_y);
}

p4pd_error_t
mem_hash_p4pd_set_entry_valid(mem_hash_api_context *ctx, uint32_t entry_valid) {
    if (MEMHASH_P4PD_SET_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_set);
    return p4pd_global_actiondata_hwfield_set(ctx->table_id,
                                              ctx->params->action_id,
                                              HWFIELD_SLOT_ID_ENTRY_VALID,
                                              (uint8_t *)&entry_valid,
                                              ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_entry_valid(mem_hash_api_context *ctx) {
    uint32_t entry_valid = 0;
    p4pd_error_t ret = 0;
    if (MEMHASH_P4PD_GET_DISABLE) {
        return 0;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_get);
    ret = p4pd_global_actiondata_hwfield_get(ctx->table_id,
                                             ctx->params->action_id,
                                             HWFIELD_SLOT_ID_ENTRY_VALID,
                                             (uint8_t *)&entry_valid,
                                             ctx->sw_data);
    SDK_ASSERT(ret == P4PD_SUCCESS);
    return entry_valid;
}

p4pd_error_t
mem_hash_p4pd_set_hint(mem_hash_api_context *ctx,
                       uint32_t hint, uint32_t slot) {
    if (MEMHASH_P4PD_SET_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_set);
    return p4pd_global_actiondata_hwfield_set(ctx->table_id,
                                              ctx->params->action_id,
                                              HWFIELD_SLOT_ID_HINT(slot),
                                              (uint8_t *)&hint, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_hint(mem_hash_api_context *ctx, uint32_t slot) {
    uint32_t hint = 0;
    p4pd_error_t ret = 0;
    if (MEMHASH_P4PD_GET_DISABLE) {
        return 0;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_get);
    ret = p4pd_global_actiondata_hwfield_get(ctx->table_id,
                                             ctx->params->action_id,
                                             HWFIELD_SLOT_ID_HINT(slot),
                                             (uint8_t *)&hint, ctx->sw_data);
    SDK_ASSERT(ret == P4PD_SUCCESS);
    return hint;
}

p4pd_error_t
mem_hash_p4pd_set_more_hints(mem_hash_api_context *ctx, uint32_t more_hints) {
    if (MEMHASH_P4PD_SET_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_set);
    return p4pd_global_actiondata_hwfield_set(ctx->table_id,
               ctx->params->action_id,
               HWFIELD_SLOT_ID_MORE_HINTS(ctx->props->num_hints),
               (uint8_t *)&more_hints, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_more_hints(mem_hash_api_context *ctx) {
    uint32_t more_hints = 0;
    p4pd_error_t ret = 0;
    if (MEMHASH_P4PD_GET_DISABLE) {
        return 0;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_get);
    ret = p4pd_global_actiondata_hwfield_get(ctx->table_id,
              ctx->params->action_id,
              HWFIELD_SLOT_ID_MORE_HINTS(ctx->props->num_hints),
              (uint8_t *)&more_hints, ctx->sw_data);
    SDK_ASSERT(ret == P4PD_SUCCESS);
    return more_hints;
}

p4pd_error_t
mem_hash_p4pd_set_hash(mem_hash_api_context *ctx,
                       uint32_t hash, uint32_t slot) {
    if (MEMHASH_P4PD_SET_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_set);
    return p4pd_global_actiondata_hwfield_set(ctx->table_id,
                                              ctx->params->action_id,
                                              HWFIELD_SLOT_ID_HASH(slot),
                                              (uint8_t *)&hash, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_hash(mem_hash_api_context *ctx, uint32_t slot) {
    uint32_t hash = 0;
    p4pd_error_t ret = 0;
    if (MEMHASH_P4PD_GET_DISABLE) {
        return 0;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_get);
    ret = p4pd_global_actiondata_hwfield_get(ctx->table_id,
                                             ctx->params->action_id,
                                             HWFIELD_SLOT_ID_HASH(slot),
                                             (uint8_t *)&hash, ctx->sw_data);
    SDK_ASSERT(ret == P4PD_SUCCESS);
    return hash;
}

p4pd_error_t
mem_hash_p4pd_set_more_hashs(mem_hash_api_context *ctx,
                              uint32_t more_hashs) {
    if (MEMHASH_P4PD_SET_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_set);
    return p4pd_global_actiondata_hwfield_set(ctx->table_id,
               ctx->params->action_id,
               HWFIELD_SLOT_ID_MORE_HASHS(ctx->props->num_hints),
               (uint8_t *)&more_hashs, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_more_hashs(mem_hash_api_context *ctx) {
    uint32_t more_hashs = 0;
    p4pd_error_t ret = 0;
    if (MEMHASH_P4PD_GET_DISABLE) {
        return 0;
    }
    MEMHASH_P4PD_STATS_INCR(hwfield_get);
    ret = p4pd_global_actiondata_hwfield_get(ctx->table_id,
              ctx->params->action_id,
              HWFIELD_SLOT_ID_MORE_HASHS(ctx->props->num_hints),
              (uint8_t *)&more_hashs, ctx->sw_data);
    SDK_ASSERT(ret == P4PD_SUCCESS);
    return more_hashs;
}

p4pd_error_t
mem_hash_p4pd_appdata_set(mem_hash_api_context *ctx, void *appdata) {
    if (MEMHASH_P4PD_SET_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(appdata_set);
    return p4pd_global_actiondata_appdata_set(ctx->table_id,
                                              ctx->params->action_id,
                                              appdata, ctx->sw_data);
}

p4pd_error_t
mem_hash_p4pd_appdata_get(mem_hash_api_context *ctx, void *appdata) {
    if (MEMHASH_P4PD_GET_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(appdata_get);
    return p4pd_global_actiondata_appdata_set(ctx->table_id,
                                              ctx->params->action_id,
                                              appdata, ctx->sw_data);
}

p4pd_error_t
mem_hash_p4pd_entry_read(uint32_t tableid, uint32_t index,
                         void *swkey, void *swkey_mask,
                         void *actiondata) {
    if (MEMHASH_P4PD_READ_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(entry_read);
    return p4pd_global_entry_read(tableid, index, swkey,
                                  swkey_mask, actiondata);
}

p4pd_error_t
mem_hash_p4pd_entry_install(uint32_t tableid, uint32_t index,
                            void *swkey, void *swkey_mask,
                            void *actiondata) {
    if (MEMHASH_P4PD_INSTALL_DISABLE) {
        return P4PD_SUCCESS;
    }
    MEMHASH_P4PD_STATS_INCR(entry_install);
    return p4pd_global_entry_install(tableid, index, swkey,
                                     swkey_mask, actiondata);
}
