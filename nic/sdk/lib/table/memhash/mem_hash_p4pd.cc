//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "mem_hash_api_context.hpp"

#define HWFIELD_SLOT_ID_ENTRY_VALID     0
#define HWFIELD_SLOT_ID_HASH(_slot) ((2*(_slot))-1)
#define HWFIELD_SLOT_ID_HINT(_slot) (2*(_slot))
#define HWFIELD_SLOT_ID_MORE_HASHS(_nhints) ((2*(_nhints))+1)
#define HWFIELD_SLOT_ID_MORE_HINTS(_nhints) ((2*(_nhints))+2)

using sdk::table::memhash::mem_hash_api_context;

p4pd_error_t
mem_hash_p4pd_set_entry_valid(mem_hash_api_context *ctx, uint32_t entry_valid) {
    return p4pd_actiondata_hwfield_set(ctx->table_id, ctx->in_action_id,
                           HWFIELD_SLOT_ID_ENTRY_VALID,
                           (uint8_t *)&entry_valid, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_entry_valid(mem_hash_api_context *ctx) {
    uint32_t entry_valid = 0;
    p4pd_error_t p4pdret = 0;
    p4pdret = p4pd_actiondata_hwfield_get(ctx->table_id, ctx->in_action_id,
                              HWFIELD_SLOT_ID_ENTRY_VALID,
                              (uint8_t *)&entry_valid, ctx->sw_data);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    return entry_valid;
}

p4pd_error_t
mem_hash_p4pd_set_hint(mem_hash_api_context *ctx,
                       uint32_t hint, uint32_t slot) {
    return p4pd_actiondata_hwfield_set(ctx->table_id, ctx->in_action_id,
                           HWFIELD_SLOT_ID_HINT(slot),
                           (uint8_t *)&hint, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_hint(mem_hash_api_context *ctx, uint32_t slot) {
    uint32_t hint = 0;
    p4pd_error_t p4pdret = 0;
    p4pdret = p4pd_actiondata_hwfield_get(ctx->table_id, ctx->in_action_id,
                              HWFIELD_SLOT_ID_HINT(slot),
                              (uint8_t *)&hint, ctx->sw_data);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    return hint;
}

p4pd_error_t
mem_hash_p4pd_set_more_hints(mem_hash_api_context *ctx, uint32_t more_hints) {
    return p4pd_actiondata_hwfield_set(ctx->table_id, ctx->in_action_id,
                           HWFIELD_SLOT_ID_MORE_HINTS(ctx->num_hints),
                           (uint8_t *)&more_hints, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_more_hints(mem_hash_api_context *ctx) {
    uint32_t more_hints = 0;
    p4pd_error_t p4pdret = 0;
    p4pdret = p4pd_actiondata_hwfield_get(ctx->table_id, ctx->in_action_id,
                              HWFIELD_SLOT_ID_MORE_HINTS(ctx->num_hints),
                              (uint8_t *)&more_hints, ctx->sw_data);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    return more_hints;
}

p4pd_error_t
mem_hash_p4pd_set_hash(mem_hash_api_context *ctx,
                       uint32_t hash, uint32_t slot) {
    return p4pd_actiondata_hwfield_set(ctx->table_id, ctx->in_action_id,
                           HWFIELD_SLOT_ID_HASH(slot),
                           (uint8_t *)&hash, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_hash(mem_hash_api_context *ctx, uint32_t slot) {
    uint32_t hash = 0;
    p4pd_error_t p4pdret = 0;
    p4pdret = p4pd_actiondata_hwfield_get(ctx->table_id, ctx->in_action_id,
                              HWFIELD_SLOT_ID_HASH(slot),
                              (uint8_t *)&hash, ctx->sw_data);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    return hash;
}

p4pd_error_t
mem_hash_p4pd_set_more_hashs(mem_hash_api_context *ctx,
                              uint32_t more_hashs) {
    return p4pd_actiondata_hwfield_set(ctx->table_id, ctx->in_action_id,
                           HWFIELD_SLOT_ID_MORE_HASHS(ctx->num_hints),
                           (uint8_t *)&more_hashs, ctx->sw_data);
}

uint32_t
mem_hash_p4pd_get_more_hashs(mem_hash_api_context *ctx) {
    uint32_t more_hashs = 0;
    p4pd_error_t p4pdret = 0;
    p4pdret = p4pd_actiondata_hwfield_get(ctx->table_id, ctx->in_action_id,
                              HWFIELD_SLOT_ID_MORE_HASHS(ctx->num_hints),
                              (uint8_t *)&more_hashs, ctx->sw_data);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    return more_hashs;
}

p4pd_error_t
mem_hash_p4pd_appdata_set(mem_hash_api_context *ctx, void *appdata) {
    return p4pd_actiondata_appdata_set(ctx->table_id, ctx->in_action_id,
                                       appdata, ctx->sw_data);
}

p4pd_error_t
mem_hash_p4pd_appdata_get(mem_hash_api_context *ctx, void *appdata) {
    return p4pd_actiondata_appdata_get(ctx->table_id, ctx->in_action_id,
                                       appdata, ctx->sw_data);
}
