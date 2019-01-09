//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_TYPES_HPP__
#define __MEM_HASH_TYPES_HPP__

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

namespace sdk {
namespace table {
namespace memhash {

class mem_hash_api_context {
public:
    enum match_type{
        MATCH_TYPE_NONE     = 0,
        MATCH_TYPE_EXM      = 1,
        MATCH_TYPE_HINT     = 2,
    };

private:
    static mem_hash_api_context* alloc_(uint32_t key_len, uint32_t data_len) {
        mem_hash_api_context   *ctx = NULL;
        void                *mem = NULL;

        mem = (mem_hash_api_context *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_API_CTX,
                                               sizeof(mem_hash_api_context));
        if (!mem) {
            SDK_TRACE_ERR("Failed to alloc api context.");
            return NULL;
        }

        ctx = new (mem) mem_hash_api_context();
        ctx->key_len = key_len;
        if (key_len) {
            ctx->swkey = (uint8_t *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_SWKEY, key_len);
        }

        if (!ctx->swkey) {
            SDK_TRACE_ERR("Failed to alloc swkey:%p", ctx->swkey);
            destroy(ctx);
            return NULL;
        }

        ctx->data_len = data_len;
        if (data_len) {
            ctx->swdata = (uint8_t *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_SWDATA, data_len);
            if (!ctx->swdata) {
                SDK_TRACE_ERR("Failed to alloc swdata:%p.", ctx->swdata);
                destroy(ctx);
                return NULL;
            }
        }
        return ctx;
    }

public:
    // Input params
    void        *key;
    uint32_t    key_len;
    void        *data;
    uint32_t    data_len;
    bool        crc32_valid;
    uint32_t    crc32;
    void        *cookie; // Callback cookie for iteratin
    //mem_hash_iterate_func_t iterfunc;

    // Derived fields from input
    uint32_t    hash;

    // Placeholders for HW read/write operations
    uint8_t     *swkey;
    uint8_t     *swdata;

    // NOTE NOTE NOTE:
    // Some of the below fields are re-used by main table and hint table
    // DO NOT USE these to pass info between tables.
    uint8_t     level;          // Chaining level
    uint8_t     max_recircs;    // Max recircs
    uint8_t     table_id;
    uint32_t    table_index;
    uint8_t     hint_slot;
    uint32_t    hint;
    bool        more_hashs;
    bool        write_pending;
    uint32_t    match_type;
    void        *bucket;
    char        str[128];
   
    // Parent API Context: used for context nesting. 
    // 1st level HintTable: pctx = MainTable context.
    // 2nd level HintTable: pctx = 1st level HintTable context.
    // and so on...
    mem_hash_api_context   *pctx;          

    // Constructor
    mem_hash_api_context() {
    }

    // Destructor
    ~mem_hash_api_context() {
    }

    char* inputstr() {
        sprintf(str, "key:%p,data:%p,crc32_valid:%d,crc32:%#x,cookie:%p",
                key, data, crc32_valid, crc32, cookie);
        return str;
    }

    // Debug string
    char* metastr() { 
        sprintf(str, "id:%d,idx:%d,slot:%d,hint:%d,"
                "more:%d,pending:%d,hash:%d",
                table_id, table_index, hint_slot,
                hint, more_hashs, write_pending, hash);
        return str;
    }

    bool ismain() { 
        return (level == 0); 
    }

    const char* idstr() {
        sprintf(str, "%s-L%d", ismain() ? "MainTable" : "HintTable", level);
        return str;
    }

    uint8_t* swkey_str() {
        return p4pd_mem_hash_entry_key_str(table_id, swkey);
    }

    uint8_t* swdata_str() {
        return p4pd_mem_hash_entry_data_str(table_id, swdata);
    }

    static mem_hash_api_context* factory(mem_hash_api_context *pctx) {
        mem_hash_api_context *ctx = alloc_(pctx->key_len, pctx->data_len);
        if (!ctx) {
            return NULL;
        }

        ctx->key = pctx->key;
        ctx->data = pctx->data;

        ctx->crc32_valid = pctx->crc32_valid;
        ctx->crc32 = pctx->crc32;
        ctx->hash = pctx->hash;
        ctx->pctx = pctx;
        ctx->max_recircs = pctx->max_recircs;
        ctx->level = pctx->level + 1;

        return ctx;
    }

    static mem_hash_api_context* factory(void *key, uint32_t key_len,
                                         void *data, uint32_t data_len,
                                         uint32_t max_recircs, uint32_t crc32) {
        mem_hash_api_context *ctx = alloc_(key_len, data_len);
        if (!ctx) {
            return NULL;
        }
        
        ctx->key = key;
        ctx->data = data;
        ctx->max_recircs = max_recircs;
        ctx->level = 0;

        ctx->crc32_valid = true;
        ctx->crc32 = crc32;
        return ctx;
    }

    static void destroy(mem_hash_api_context* ctx) {
        if (ctx->swkey) {
            SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_SWKEY, ctx->swkey);
        }
        if (ctx->swdata) {
            SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_SWDATA, ctx->swdata);
        }
        SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_API_CTX, ctx);
        return;
    }
};

} // namespace memhash
} // namespace table
} // namespace sdk

#endif // __MEM_HASH_TYPES_HPP__
