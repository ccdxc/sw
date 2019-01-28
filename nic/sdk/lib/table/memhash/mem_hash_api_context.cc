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

using sdk::table::mem_hash_factory_params_t;
using sdk::table::mem_hash_properties_t;

mem_hash_api_context* 
mem_hash_api_context::alloc_(uint32_t sw_key_len, uint32_t sw_data_len,
                             uint32_t sw_appdata_len,
                             uint32_t hw_key_len, uint32_t hw_data_len) {
    mem_hash_api_context *ctx = NULL;
    void *mem = NULL;

    mem = (mem_hash_api_context *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_API_CTX,
                                              sizeof(mem_hash_api_context));
    if (!mem) {
        SDK_TRACE_ERR("failed to alloc api context.");
        return NULL;
    }

    ctx = new (mem) mem_hash_api_context();
    ctx->sw_key_len = sw_key_len;
    if (ctx->sw_key_len) {
        ctx->sw_key = (uint8_t *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_SW_KEY, sw_key_len);
        if (!ctx->sw_key) {
            SDK_TRACE_ERR("failed to alloc sw_key:%p", ctx->sw_key);
            destroy(ctx);
            return NULL;
        }
    }

    ctx->sw_data_len = sw_data_len;
    if (ctx->sw_data_len) {
        ctx->sw_data = (uint8_t *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_SW_DATA, sw_data_len);
        if (!ctx->sw_data) {
            SDK_TRACE_ERR("failed to alloc sw_data:%p.", ctx->sw_data);
            destroy(ctx);
            return NULL;
        }
    }

    ctx->sw_appdata_len = sw_appdata_len;
    if (ctx->sw_appdata_len) {
        ctx->sw_appdata = (uint8_t *)SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_SW_APPDATA,
                                               sw_appdata_len);
        if (!ctx->sw_data) {
            SDK_TRACE_ERR("failed to alloc sw_appdata:%p.", ctx->sw_appdata);
            destroy(ctx);
            return NULL;
        }
    }

    ctx->hw_key_len = hw_key_len;
    if (ctx->hw_key_len) {
        ctx->hw_key = (uint8_t *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_HW_KEY, hw_key_len);
        if (!ctx->hw_key) {
            SDK_TRACE_ERR("failed to alloc hw_key:%p", ctx->hw_key);
            destroy(ctx);
            return NULL;
        }
    }

    ctx->hw_data_len = hw_data_len;
    if (ctx->hw_data_len) {
        ctx->hw_data = (uint8_t *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_HW_DATA, hw_data_len);
        if (!ctx->hw_data) {
            SDK_TRACE_ERR("failed to alloc sw_data:%p.", ctx->hw_data);
            destroy(ctx);
            return NULL;
        }
    }

    numctx_++;
    SDK_TRACE_DEBUG("Number of API Contexts = %d", numctx_);
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
    mem_hash_api_context *ctx = alloc_(pctx->sw_key_len, pctx->sw_data_len,
                                       pctx->sw_appdata_len,
                                       pctx->hw_key_len, pctx->hw_data_len);
    if (!ctx) {
        return NULL;
    }
    ctx->op = pctx->op;

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
    mem_hash_api_context *ctx = alloc_(props->key_len, props->data_len,
                                       appdata_len,
                                       props->hw_key_len, props->hw_data_len);
    if (!ctx) {
        return NULL;
    }
    ctx->op = static_cast<mem_hash_api_context::api_op>(op);

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

    // Use the handle from the params.
    ctx->handle = reinterpret_cast<mem_hash_handle_t*>(&params->handle);

    // Save the table_stats pointer
    ctx->table_stats = table_stats;

    // Save the transaction
    ctx->txn = txn;
    return ctx;
}

void 
mem_hash_api_context::destroy(mem_hash_api_context* ctx) {
    SDK_TRACE_DEBUG("Destroying api context: %s", ctx->idstr());
    if (ctx->sw_key) {
        SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_SW_KEY, ctx->sw_key);
    }
    if (ctx->sw_data) {
        SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_SW_DATA, ctx->sw_data);
    }
    if (ctx->sw_appdata) {
        SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_SW_APPDATA, ctx->sw_appdata);
    }
    if (ctx->hw_key) {
        SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_SW_KEY, ctx->hw_key);
    }
    if (ctx->hw_data) {
        SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_SW_DATA, ctx->hw_data);
    }

    SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_API_CTX, ctx);

    numctx_--;
    SDK_TRACE_DEBUG("Number of API Contexts = %d", numctx_);
    return;
}

void
mem_hash_api_context::print_handle() {
    SDK_TRACE_DEBUG("- Handle: IsHint=%d Index=%d Hint=%d",
                    handle->is_hint, handle->index, handle->hint);
}

void
mem_hash_api_context::print_input() {
    SDK_TRACE_DEBUG("- Key:[%s] Hash:[%#x]",
        key2str ? key2str(in_key) :
                  mem_hash_utils_rawstr(in_key, sw_key_len), in_hash_32b);
    SDK_TRACE_DEBUG("- RawKey:[%s]",
                    mem_hash_utils_rawstr(in_key, sw_key_len));
    SDK_TRACE_DEBUG("- AppData:[%s]",
        appdata2str ? appdata2str(in_appdata) :
                      mem_hash_utils_rawstr(in_appdata, sw_appdata_len));
    SDK_TRACE_DEBUG("- RawData:[%s]",
                    mem_hash_utils_rawstr(in_appdata, sw_appdata_len));
    print_handle();
}
