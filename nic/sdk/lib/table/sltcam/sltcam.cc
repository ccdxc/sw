//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "lib/p4/p4_api.hpp"
#include "include/sdk/table.hpp"
#include "include/sdk/base.hpp"

#include "sltcam.hpp"
#include "sltcam_utils.hpp"
#include "sltcam_txn.hpp"

using sltctx = sdk::table::sltcam_internal::sltctx;

namespace sdk {
namespace table {

#define SLTCAM_API_BEGIN(_name) {                            \
        SLTCAM_TRACE_DEBUG("%s sltcam begin: %s %s",         \
                            "--", _name, "--");              \
}

#define SLTCAM_API_END(_name, _status) {                     \
        SLTCAM_TRACE_DEBUG("%s sltcam end: %s (r:%d) %s",    \
                            "--", _name, _status, "--");     \
}

#define SLTCAM_API_BEGIN_() {                                \
        SLTCAM_API_BEGIN(props_.name);                       \
        SDK_SPINLOCK_LOCK(&slock_);                          \
}

#define SLTCAM_API_END_(_status) {                           \
        SLTCAM_API_END(props_.name, (_status));              \
        SDK_SPINLOCK_UNLOCK(&slock_);                        \
}

//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
sltcam *
sltcam::factory(sdk::table::sdk_table_factory_params_t *params) {
    void *mem  = NULL;
    sltcam *table = NULL;
    sdk_ret_t ret = sdk::SDK_RET_OK;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_ID_TCAM, sizeof(sltcam));
    if (!mem) {
        return NULL;
    }

    table = (sltcam *) new (mem) sltcam();
    ret = table->init_(params);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_DEBUG("init_, r:%d", ret);
        destroy(table);
        return NULL;
    }

    return table;
}

sdk_ret_t
sltcam::init_(sdk::table::sdk_table_factory_params_t *params) {
    sdk_ret_t ret = sdk::SDK_RET_OK;
    ret = props_.init(params);
    if (ret != sdk::SDK_RET_OK) {
        return ret;
    }

    SDK_ASSERT(props_.table_size);
    indexer_ = indexer::factory(props_.table_size, false, false);
    if (indexer_ == NULL) {
        SLTCAM_TRACE_ERR("Failed to create indexer_");
        return sdk::SDK_RET_OOM;
    }

    ret = txn_.init(props_.table_size);
    if (ret != sdk::SDK_RET_OK) {
        return ret;
    }

    ret = db_.init(props_.table_size);
    if (ret != sdk::SDK_RET_OK) {
        return ret;
    }

    return sdk::SDK_RET_OK;
}

//---------------------------------------------------------------------------
// factory method to free & delete the object
//---------------------------------------------------------------------------
void
sltcam::destroy(sltcam *table)
{
    if (table) {
        indexer::destroy(table->indexer_);
        table->~sltcam();
        SDK_FREE(SDK_MEM_ALLOC_ID_TCAM, table);
    }
}

//----------------------------------------------------------------------------
// Write entry to HW
//----------------------------------------------------------------------------
sdk_ret_t
sltcam::write_(sltctx *ctx) {
    p4pd_error_t p4pdret;
    static char buff[4096] = {0};

    if (ctx->props->entry_trace_en) {
        p4pd_global_table_ds_decoded_string_get(ctx->props->table_id,
                                                ctx->tcam_index,
                                                ctx->swkey, ctx->swkeymask,
                                                ctx->swdata, buff, sizeof(buff));
        SLTCAM_TRACE_DEBUG("Table: %s, EntryIndex:%u\n%s",
                           ctx->props->name, ctx->tcam_index, buff);
    }

    SLTCAM_TRACE_DEBUG("Table: %s, EntryIndex:%u",
                       ctx->props->name, ctx->tcam_index);
    ctx->print_sw();

    p4pdret = p4pd_entry_install(ctx->props->table_id, ctx->tcam_index,
                                 ctx->swkey, ctx->swkeymask, ctx->swdata);
    return p4pdret == P4PD_SUCCESS ? sdk::SDK_RET_OK : sdk::SDK_RET_ERR;
}

//----------------------------------------------------------------------------
// Read entry from HW
//----------------------------------------------------------------------------
sdk_ret_t
sltcam::read_(sltctx *ctx) {
    p4pd_error_t p4pdret;
    SDK_ASSERT(ctx->tcam_index < props_.table_size);
    SLTCAM_TRACE_DEBUG("hw index:%d", ctx->tcam_index);
    p4pdret =  p4pd_entry_read(ctx->props->table_id, ctx->tcam_index,
                               ctx->swkey, ctx->swkeymask, ctx->swdata);
    ctx->print_sw();
    return p4pdret == P4PD_SUCCESS ? sdk::SDK_RET_OK : sdk::SDK_RET_ERR;
}

//----------------------------------------------------------------------------
// Allocate a TCAM index
//----------------------------------------------------------------------------
sdk_ret_t
sltcam::alloc_(sltctx *ctx) {
    indexer::status irs;

    SDK_ASSERT(ctx->tcam_index_valid == false);
    if (ctx->params->num_handles) {
        irs = indexer_->alloc(&ctx->tcam_index, !ctx->params->highest,
                              ctx->params->num_handles);
    } else {
        irs = indexer_->alloc(&ctx->tcam_index, !ctx->params->highest);
    }
    if (irs != indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    ctx->tcam_index_valid = true;
    SLTCAM_TRACE_DEBUG("hw index:%u", ctx->tcam_index);
    return sdk::SDK_RET_OK;
}

//----------------------------------------------------------------------------
// free given index
//----------------------------------------------------------------------------
sdk_ret_t
sltcam::dealloc_(sltctx *ctx) {
    indexer::status irs;
    SDK_ASSERT(ctx->tcam_index_valid);

    if (ctx->params->num_handles) {
        for (uint32_t i = 0; i < ctx->params->num_handles; i++) {
            irs = indexer_->free(ctx->tcam_index + i);
            if (irs != indexer::SUCCESS) {
                SLTCAM_TRACE_ERR("Failed to free index %u", ctx->tcam_index + i);
                // continue to free other indices as much as possible
            }
        }
    } else {
        irs = indexer_->free(ctx->tcam_index);
    }
    if (irs != indexer::SUCCESS) {
        return sdk::SDK_RET_ERR;
    }
    ctx->tcam_index_valid = false;
    return sdk::SDK_RET_OK;
}

static sltctx*
create_sltctx(sdk::table::sdk_table_api_op_t op,
              sdk::table::sdk_table_api_params_t *params,
              sdk::table::sltcam_internal::properties *props) {
    static sltctx ctx;
    ctx.init();
    ctx.params = params;
    ctx.props = props;
    ctx.op = op;

    ctx.tcam_index = params ? params->handle.pindex() : 0;
    ctx.tcam_index_valid = params ? params->handle.pvalid() : false;

    ctx.print_params();
    return &ctx;
}

sdk_ret_t
sltcam::find_(sltctx *ctx) {
    return db_.find(ctx);
}

//---------------------------------------------------------------------------
// Insert tcam entry by key or handle
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::insert(sdk::table::sdk_table_api_params_t *params) {
__label__ done;
    sdk_ret_t ret = sdk::SDK_RET_OK;
    SLTCAM_API_BEGIN_();

    auto ctx = create_sltctx(sdk::table::SDK_TABLE_API_INSERT, params, &props_);
    SDK_ASSERT_RETURN(ctx, sdk::SDK_RET_OOM);

    // Validate this ctx (& api params) with the transaction
    ret = txn_.validate(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "txn validate, r:%d", ret);
    }

    // Check if an entry with same key exists already.
    ret = find_(ctx);
    if (ret == sdk::SDK_RET_OK) {
        ret = sdk::SDK_RET_ENTRY_EXISTS;
        // Return the handle of the existing entry
        goto handle_set;
    } else if (ret != sdk::SDK_RET_ENTRY_NOT_FOUND) {
        SLTCAM_TRACE_ERR_GOTO(done, "find, r:%d", ret);
    }

    // Allocate a tcam entry if not already reserved.
    if (ctx->params->handle.valid() == false) {
        ctx->params->num_handles = 1;
        ret = alloc_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            SLTCAM_TRACE_ERR_GOTO(done, "alloc, r:%d", ret);
        }
    }

    // Copy the params to sw fields
    ctx->copyin();

    // Write to HW
    ret = write_(ctx);
    if (ret != sdk::SDK_RET_OK) {
        dealloc_(ctx);
        SLTCAM_TRACE_ERR_GOTO(done, "write, r:%d", ret);
    }

    // Insert to sw db
    ret = db_.insert(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "db insert, r:%d", ret);
    }

    // Release this handle
    txn_.release(ctx);

handle_set:
    // Save the handle
    params->handle.pindex(ctx->tcam_index);

done:
    //db_.sanitize(ctx);
    SLTCAM_API_END_(ret);
    stats_.insert(ret);
    return ret;
}

//---------------------------------------------------------------------------
// Update tcam entry given key or handle
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::update(sdk::table::sdk_table_api_params_t *params) {
__label__ done;
    SLTCAM_API_BEGIN_();

    auto ctx = create_sltctx(sdk::table::SDK_TABLE_API_UPDATE, params, &props_);
    SDK_ASSERT_RETURN(ctx, sdk::SDK_RET_OOM);

    // Validate this ctx (& api params) with the transaction
    auto ret = txn_.validate(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "txn validate, r:%d", ret);
    }

    // Find the tcam_index, if handle is not provided.
    if (ctx->params->handle.valid() == false) {
        ret = find_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            SLTCAM_TRACE_ERR_GOTO(done, "find, r:%d", ret);
        }
    }

    // Copy the params to sw fields
    ctx->copyin();

    // Write to HW
    ret = write_(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "write, r:%d", ret);
    }

done:
    //db_.sanitize(ctx);
    SLTCAM_API_END_(ret);
    stats_.update(ret);
    return ret;
}

//---------------------------------------------------------------------------
// Remove tcam entry given key or handle
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::remove(sdk::table::sdk_table_api_params_t *params) {
__label__ done;
    SLTCAM_API_BEGIN_();

    auto ctx = create_sltctx(sdk::table::SDK_TABLE_API_REMOVE, params, &props_);
    SDK_ASSERT_RETURN(ctx, sdk::SDK_RET_OOM);

    // Validate this ctx (& api params) with the transaction
    auto ret = txn_.validate(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "txn validate, r:%d", ret);
    }

    // remove using handle
    if (ctx->tcam_index_valid) {
        // use tcam_index to read swkey
        ret = read_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            SLTCAM_TRACE_ERR_GOTO(done, "read, r:%d", ret);
        }
        // Copy the swkey into params->key
        ctx->copyout();
    }

    // find dbslot
    ret = find_(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "find, r:%d", ret);
    }

    // Remove from HW using swkey
    ctx->clearsw();
    ret = write_(ctx);
    if (ret != sdk::SDK_RET_OK) {
        // At this point, not much can be done, print err and proceed
        SLTCAM_TRACE_ERR("write, r:%d", ret);
    }

    // Remove the entry from the DB
    ret = db_.remove(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "db remove, r:%d", ret);
    }

    // Release this handle
    txn_.release(ctx);

    // Free the tcam entry
    ctx->params->num_handles = 1;
    ret = dealloc_(ctx);
    if (ret != sdk::SDK_RET_OK) {
        // At this point, not much can be done, print err and proceed
        SLTCAM_TRACE_ERR("dealloc, r:%d", ret);
    }

done:
    //db_.sanitize(ctx);
    SLTCAM_API_END_(ret);
    stats_.remove(ret);
    return ret;
}

//---------------------------------------------------------------------------
// Get TCAM entry by key or handle
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::get(sdk::table::sdk_table_api_params_t *params) {
__label__ done;
    auto ret = SDK_RET_OK;
    SLTCAM_API_BEGIN_();

    auto ctx = create_sltctx(sdk::table::SDK_TABLE_API_GET, params, &props_);
    SDK_ASSERT_RETURN(ctx, sdk::SDK_RET_OOM);

    if (ctx->tcam_index_valid == false) {
        ret = find_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            SLTCAM_TRACE_ERR_GOTO(done, "find, r:%d", ret);
        }
    }

    ret = read_(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "read, r:%d", ret);
    }

    // Copy the data out
    ctx->copyout();

    // Save the handle
    params->handle.pindex(ctx->tcam_index);

done:
    SLTCAM_API_END_(ret);
    stats_.get(ret);
    return ret;
}

//---------------------------------------------------------------------------
// Iterate over all tcam entries
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::iterate(sdk::table::sdk_table_api_params_t *params) {
    sdk_table_api_params_t iterparams = { 0 };
    sdk_ret_t ret = sdk::SDK_RET_OK;
    SLTCAM_API_BEGIN_();
    auto ctx = create_sltctx(sdk::table::SDK_TABLE_API_GET, params, &props_);
    SDK_ASSERT_RETURN(ctx, sdk::SDK_RET_OOM);

    for(auto it = db_.begin(); it != db_.end(); it++) {
        ctx->tcam_index = db_.element(it);
        ctx->tcam_index_valid = true;

        ret = read_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            SLTCAM_TRACE_ERR_GOTO(done, "read, r:%d", ret);
        }

        iterparams.key = ctx->swkey;
        iterparams.mask = ctx->swkeymask;
        iterparams.appdata = ctx->swdata;
        iterparams.cbdata = params->cbdata;
        params->itercb(&iterparams);
    }

done:
    SLTCAM_API_END_(ret);
    return ret;
}

//----------------------------------------------------------------------------
// Get TCAM statistics
//----------------------------------------------------------------------------
sdk_ret_t
sltcam::stats_get(sdk::table::sdk_table_api_stats_t *api_stats,
                  sdk::table::sdk_table_stats_t *table_stats) {
    sdk_ret_t ret = sdk::SDK_RET_OK;
    SLTCAM_API_BEGIN_();
    ret = stats_.get(api_stats, table_stats);
    SLTCAM_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// Reserve an entry
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::reserve(sdk::table::sdk_table_api_params_t *params) {
__label__ done;
    SLTCAM_API_BEGIN_();

    auto ctx = create_sltctx(sdk::table::SDK_TABLE_API_RELEASE,
                             params, &props_);
    SDK_ASSERT_RETURN(ctx, sdk::SDK_RET_OOM);

    // Allocate the entry
    auto ret = alloc_(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "alloc, r:%d", ret);
    }

    // Reserve the entry in the transaction
    ret = txn_.reserve(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "txn reserve, r:%d", ret);
    }

    // Save the handle
    params->handle.pindex(ctx->tcam_index);

done:
    //db_.sanitize(ctx);
    SLTCAM_API_END_(ret);
    stats_.reserve(ret);
    return ret;
}

//---------------------------------------------------------------------------
// Release an entry
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::release(sdk::table::sdk_table_api_params_t *params) {
    SLTCAM_API_BEGIN_();

    auto ctx = create_sltctx(sdk::table::SDK_TABLE_API_RELEASE,
                                 params, &props_);
    SDK_ASSERT_RETURN(ctx, sdk::SDK_RET_OOM);

    // Release this entry from the transaction
    auto ret = txn_.release(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR_GOTO(done, "txn release, r:%d", ret);
    }

    // Free the entry
    ret = dealloc_(ctx);
    if (ret != sdk::SDK_RET_OK) {
        SLTCAM_TRACE_ERR("dealloc, r:%d", ret);
    }

done:
    SLTCAM_API_END_(ret);
    stats_.release(ret);
    return ret;
}

//---------------------------------------------------------------------------
// Start the transaction
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::txn_start() {
    SLTCAM_API_BEGIN_();
    auto ret = txn_.start();
    SLTCAM_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// End the transaction
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::txn_end() {
    SLTCAM_API_BEGIN_();
    auto ret = txn_.end();
    SLTCAM_API_END_(ret);
    return ret;
}

//---------------------------------------------------------------------------
// Sanitize internal data structures
//---------------------------------------------------------------------------
sdk_ret_t
sltcam::sanitize() {
    auto ctx = create_sltctx(sdk::table::SDK_TABLE_API_UPDATE, NULL, &props_);
    SDK_ASSERT_RETURN(ctx, sdk::SDK_RET_OOM);
    db_.sanitize(ctx);
    return sdk::SDK_RET_OK;
}

} // namespace table
} // namespace sdk
