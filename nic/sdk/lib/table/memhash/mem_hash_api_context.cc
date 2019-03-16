//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/p4/p4_api.hpp"

#include "mem_hash_api_context.hpp"
#include "mem_hash_p4pd.hpp"
#include "mem_hash.hpp"
#include "mem_hash_utils.hpp"
#include "mem_hash_stats.hpp"

//#define MEM_HASH_API_CONTEXT_CALLOC

using sdk::table::sdk_table_factory_params_t;
using sdk::table::mem_hash_properties_t;

mem_hash_api_context g_api_contexts[16];
uint8_t g_api_ctx_iter;

mem_hash_api_context* 
mem_hash_api_context::alloc_() {
    mem_hash_api_context *ctx = NULL;
    void *mem = NULL;

#ifdef MEM_HASH_API_CONTEXT_CALLOC
    mem = (mem_hash_api_context *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_API_CTX,
                                              sizeof(mem_hash_api_context));
    if (!mem) {
        MEMHASH_TRACE_ERR("failed to alloc api context.");
        return NULL;
    }
#else
    mem = &(g_api_contexts[g_api_ctx_iter++]);
    memset(mem, 0, sizeof(mem_hash_api_context));
#endif
    ctx = new (mem) mem_hash_api_context();

    numctx_++;
    MEMHASH_TRACE_VERBOSE("Number of API Contexts = %d", numctx_);
    return ctx;
}


char*
mem_hash_api_context::sw_data2str() {
    uint32_t len = 0;
    uint32_t i = 0;

    len += sprintf(str, "Valid=%d,More=%d,MoreHints=%d,",
                   mem_hash_p4pd_get_entry_valid(this),
                   mem_hash_p4pd_get_more_hashs(this),
                   mem_hash_p4pd_get_more_hints(this));
    for (i = 1; i <= num_hints; i++) {
        uint32_t hintX = mem_hash_p4pd_get_hint(this, i);
        uint32_t hash_msbitsX = mem_hash_p4pd_get_hash(this, i);
        if (hintX) {
            len += sprintf(str + len, "HashMsbits%d=%#x,Hint%d=%d,",
                           i, hash_msbitsX, i, hintX);
        } else {
            // if hint is not valid, then hash must be zero.
            SDK_ASSERT(hash_msbitsX == 0);
        }
    }
    return str;
}

mem_hash_api_context*
mem_hash_api_context::factory(mem_hash_api_context *pctx) {
    mem_hash_api_context *ctx = alloc_();
    if (!ctx) {
        return NULL;
    }

    ctx->sw_valid = false;
    ctx->op = pctx->op;

    // Copy the lengths
    ctx->sw_key_len = pctx->sw_key_len;
    ctx->sw_data_len = pctx->sw_data_len;
    ctx->sw_appdata_len = pctx->sw_appdata_len;
    ctx->hw_key_len = pctx->hw_key_len;
    ctx->hw_data_len = pctx->hw_data_len;

    // Copy the api params
    ctx->in_key = pctx->in_key;
    ctx->in_appdata = pctx->in_appdata;
    ctx->in_action_id = pctx->in_action_id;
    ctx->in_hash_valid = pctx->in_hash_valid;
    ctx->in_hash_32b = pctx->in_hash_32b;
    ctx->hash_msbits = pctx->hash_msbits;
    // Copy the required properties of the table to the context
    ctx->num_hints = pctx->num_hints;
    ctx->max_recircs = pctx->max_recircs;
    ctx->level = pctx->level + 1;
    ctx->key2str = pctx->key2str;
    ctx->appdata2str = pctx->appdata2str;
    ctx->itercb = pctx->itercb;
    ctx->cbdata = pctx->cbdata;
    ctx->props = pctx->props;

    // Use the handle from the parent context.
    ctx->handle = pctx->handle;

    // Save the table_stats pointer
    ctx->table_stats = pctx->table_stats;

    // Save the transaction
    ctx->txn = pctx->txn;

    // Save parent context
    ctx->pctx = pctx;
    return ctx;
}

mem_hash_api_context*
mem_hash_api_context::factory(uint32_t op, 
                              sdk_table_api_params_t *params,
                              mem_hash_properties_t *props,
                              mem_hash_table_stats *table_stats,
                              mem_hash_txn *txn) {
    uint32_t appdata_len = 0;

    appdata_len = p4pd_actiondata_appdata_size_get(props->main_table_id,
                                                   params->action_id);
    SDK_ASSERT(appdata_len && appdata_len <= MEMHASH_MAX_SW_DATA_LEN);

#ifndef MEM_HASH_API_CONTEXT_CALLOC
    g_api_ctx_iter = 0;
#endif

    mem_hash_api_context *ctx = alloc_();
    if (!ctx) {
        return NULL;
    }
    ctx->op = static_cast<sdk::table::sdk_table_api_op_t>(op);
    ctx->sw_valid = false;

    // Copy the lengths
    ctx->sw_key_len = props->key_len;
    ctx->sw_data_len = props->data_len;
    ctx->sw_appdata_len = appdata_len;
    ctx->hw_key_len = props->hw_key_len;
    ctx->hw_data_len = props->hw_data_len;

    // Copy the api params
    ctx->in_key = params->key;
    ctx->in_appdata = params->appdata;
    ctx->in_action_id = 0;
    ctx->in_hash_valid = params->hash_valid;
    ctx->in_hash_32b = params->hash_32b;
    ctx->hash_msbits = 0;

    // Copy the required properties of the table to the context
    ctx->num_hints = props->num_hints;
    ctx->max_recircs = props->max_recircs;
    ctx->level = 0;
    ctx->key2str = props->key2str;
    ctx->appdata2str = props->appdata2str;
    ctx->itercb = params->itercb;
    ctx->cbdata = params->cbdata;
    ctx->props = props;

    // Use the handle from the params.
    ctx->handle = reinterpret_cast<mem_hash_handle_t*>(&params->handle);

    // Save the table_stats pointer
    ctx->table_stats = table_stats;

    // Save the transaction
    ctx->txn = txn;
    //if (SDK_TABLE_API_OP_IS_CRUD(ctx->op) && txn->is_valid()) {
    //    SDK_ASSERT(ctx->is_handle_valid());
    //}

    return ctx;
}

void 
mem_hash_api_context::destroy(mem_hash_api_context* ctx) {
    MEMHASH_TRACE_VERBOSE("Destroying api context: %s", ctx->idstr());
#ifdef MEM_HASH_API_CONTEXT_CALLOC
    SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_API_CTX, ctx);
#endif

    numctx_--;
    MEMHASH_TRACE_VERBOSE("Number of API Contexts = %d", numctx_);
    return;
}

void
mem_hash_api_context::print_handle() {
    MEMHASH_TRACE_DEBUG("- Handle: IsHint=%d Index=%d Hint=%d Value:%d",
                        handle->is_hint, handle->index, handle->hint, handle->value);
}

void
mem_hash_api_context::print_input() {
    // Only crud api contexts have key.
    if (!in_key) {
        return;
    }

    if (in_key) {
        if (key2str) {
            MEMHASH_TRACE_DEBUG("- Key:[%s] Hash:[%#x]", key2str(in_key), in_hash_32b);
        } else {
            MEMHASH_TRACE_DEBUG("- RawSwKey:[%s]",
                                mem_hash_utils_rawstr(in_key, sw_key_len));
        }
    }

    if (in_appdata) {
        if (appdata2str) {
            MEMHASH_TRACE_DEBUG("- AppData:[%s]", appdata2str(in_appdata));
        } else {
            MEMHASH_TRACE_DEBUG("- RawSwData:[%s]",
                                mem_hash_utils_rawstr(in_appdata, sw_appdata_len));
        }
    }

    print_handle();
}
