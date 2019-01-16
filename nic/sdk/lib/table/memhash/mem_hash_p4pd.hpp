//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_P4PD_HPP__
#define __MEM_HASH_P4PD_HPP__

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "mem_hash_api_context.hpp"
using sdk::table::memhash::mem_hash_api_context;

p4pd_error_t
mem_hash_p4pd_set_entry_valid(mem_hash_api_context *ctx,
                              uint32_t entry_valid);
uint32_t
mem_hash_p4pd_get_entry_valid(mem_hash_api_context *ctx);

p4pd_error_t
mem_hash_p4pd_set_hint(mem_hash_api_context *ctx,
                       uint32_t hint, uint32_t slot);

uint32_t
mem_hash_p4pd_get_hint(mem_hash_api_context *ctx,
                       uint32_t slot);

p4pd_error_t
mem_hash_p4pd_set_more_hints(mem_hash_api_context *ctx,
                             uint32_t more_hints);

uint32_t
mem_hash_p4pd_get_more_hints(mem_hash_api_context *ctx);

p4pd_error_t
mem_hash_p4pd_set_hash(mem_hash_api_context *ctx,
                       uint32_t hash, uint32_t slot);

uint32_t
mem_hash_p4pd_get_hash(mem_hash_api_context *ctx,
                       uint32_t slot);

p4pd_error_t
mem_hash_p4pd_set_more_hashs(mem_hash_api_context *ctx,
                             uint32_t more_hashs);

uint32_t
mem_hash_p4pd_get_more_hashs(mem_hash_api_context *ctx);

p4pd_error_t
mem_hash_p4pd_appdata_set(mem_hash_api_context *ctx, void *appdata);

p4pd_error_t
mem_hash_p4pd_appdata_get(mem_hash_api_context *ctx, void *appdata);

#endif
