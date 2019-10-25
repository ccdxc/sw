//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <cstring>
#include "sldirectmap.hpp"
#include "lib/p4/p4_api.hpp"
#include "lib/rte_indexer/rte_slab_op.hpp"

namespace sdk {
namespace table {

#define CHECK_INDEX_LIMIT(index, rv, label) { \
    if (index >= props_->ptable_size) {       \
        rv = SDK_RET_NO_RESOURCE;             \
        goto label;                           \
    }                                         \
}

#define CHECK_INDEX_STATUS(index, rv, status_err, label) { \
    CHECK_INDEX_LIMIT(index, rv, label)                    \
    if (indexer_->is_index_allocated(index)) {             \
        rv = SDK_RET_ENTRY_EXISTS;                         \
    } else {                                               \
        rv = SDK_RET_ENTRY_NOT_FOUND;                      \
    }                                                      \
    if (rv == status_err)                                  \
        goto label;                                        \
    else                                                   \
        rv = SDK_RET_OK;                                   \
}

#define SET_INDEX_TO_PARAM(params, index) params->handle.pindex(index);

#define GET_INDEX_FROM_PARAM(params, index, rv, label) { \
    SDK_ASSERT(params->handle.pvalid());                 \
    index = params->handle.pindex();                     \
    CHECK_INDEX_LIMIT(index, rv, label)                  \
}

//----------------------------------------------------------------------------
// directmap instance initialization
//----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::init_(sdk_table_factory_params_t *params) {
    p4pd_error_t p4pdret;
    p4pd_table_properties_t tinfo;

    props_ = (sdk::table::properties_t *)SDK_CALLOC(
        SDK_MEM_ALLOC_LIB_DIRECT_MAP_DATA, sizeof(sdk::table::properties_t));
    if (props_ == NULL) {
        return SDK_RET_OOM;
    }

    props_->ptable_id = params->table_id;
    //props_->health_monitor_func = params->health_monitor_func;
    props_->key2str = params->key2str;
    props_->data2str = params->appdata2str;
    props_->entry_trace_en = params->entry_trace_en;

    p4pdret = p4pd_global_table_properties_get(props_->ptable_id, &tinfo);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);
    props_->name = tinfo.tablename;
    props_->ptable_size = tinfo.tabledepth;
    props_->swdata_len = tinfo.actiondata_struct_size;
    SDK_ASSERT(props_->swdata_len);
    SDK_ASSERT(props_->swdata_len <= SDK_TABLE_MAX_SW_DATA_LEN);
    DIRECTMAP_TRACE_DEBUG("Init name %s swdata_len %dbits depth %u",
                          tinfo.tablename, props_->swdata_len,
                          props_->ptable_size);
    p4pd_hwentry_query(props_->ptable_id, &props_->hwkey_len,
                       NULL, &props_->hwdata_len);
    props_->hwkey_len = SDK_TABLE_BITS_TO_BYTES(props_->hwkey_len);
    SDK_ASSERT(props_->hwkey_len <= SDK_TABLE_MAX_HW_KEY_LEN);
    props_->hwdata_len = SDK_TABLE_BITS_TO_BYTES(props_->hwdata_len);
    SDK_ASSERT(props_->hwdata_len <= SDK_TABLE_MAX_HW_DATA_LEN);
    DIRECTMAP_TRACE_DEBUG("- hwkey_len %dBytes hwdata_len %dBytes ",
                          props_->hwkey_len, props_->hwdata_len);

    indexer_ = NULL;
    if (props_->ptable_size) {
        // RTE based indexer allocator
        indexer_ = rte_indexer::factory(props_->ptable_size);
        if (!indexer_)
            return SDK_RET_ERR;
    }

    return SDK_RET_OK;

}

//----------------------------------------------------------------------------
// factory method to instantiate the class
//----------------------------------------------------------------------------
sldirectmap *
sldirectmap::factory(sdk_table_factory_params_t *params) {
    void      *mem = NULL;
    sldirectmap *dm  = NULL;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_DIRECTMAP, sizeof(sldirectmap));
    if (!mem) {
        return NULL;
    }

    dm = new (mem) sldirectmap();
    if (dm->init_(params) != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to initialize sldirectmap");
        dm->~sldirectmap();
        SDK_FREE(SDK_MEM_ALLOC_LIB_DIRECTMAP, mem);
        return NULL;
    }

    SDK_TRACE_DEBUG("tableid %-4d table_size %-4d", dm->table_id(),
                    dm->capacity());
    return dm;
}

sldirectmap::sldirectmap() {
    indexer_ = NULL;
    props_ = NULL;
}

//----------------------------------------------------------------------------
// method to free & delete the object
//----------------------------------------------------------------------------
void
sldirectmap::destroy(sldirectmap *dm) {
    if (dm) {
        dm->~sldirectmap();
        SDK_FREE(SDK_MEM_ALLOC_LIB_DIRECTMAP, dm);
    }
}

sldirectmap::~sldirectmap() {
    if (indexer_) {
        indexer_->destroy(indexer_);
    }
}

//-----------------------------------------------------------------------------
// insert entry in HW
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::insert(sdk_table_api_params_t *params)
{
    sdk_ret_t           rs     = SDK_RET_OK;
    p4pd_error_t        pd_err = P4PD_SUCCESS;
    uint32_t            index;

    SDK_ASSERT(params->actiondata);
    rs = alloc_index_(&index);
    if (rs != SDK_RET_OK) {
        goto end;
    }

    // print entry
    PRINT_DATA(__func__, params->actiondata, index);

    // program P4
    pd_err = p4pd_global_entry_write_with_datamask(props_->ptable_id, index,
                                                   NULL, NULL,
                                                   params->actiondata, params->actiondata_mask);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
        SDK_ASSERT(0);
    }

    SET_INDEX_TO_PARAM(params, index);
    DIRECTMAP_TRACE_DEBUG("index %u", index);
    in_use_++;
end:
    api_stats_.insert(rs);
    return rs;
}

//-----------------------------------------------------------------------------
// insert entry in HW at index
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::insert_withid(sdk_table_api_params_t *params)
{
    sdk_ret_t rs = SDK_RET_OK;
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint32_t index;

    GET_INDEX_FROM_PARAM(params, index, rs, end);

    rs = alloc_index_withid_(index);
    if (rs != SDK_RET_OK) {
        goto end;
    }

    // print entry
    PRINT_DATA(__func__, params->actiondata, index);

    // program P4
    pd_err = p4pd_global_entry_write_with_datamask(props_->ptable_id, index, NULL, NULL,
                                                   params->actiondata, params->actiondata_mask);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
        SDK_ASSERT(0);
    }

    DIRECTMAP_TRACE_DEBUG("index %u", index);
    in_use_++;
end:
    api_stats_.insert_withid(rs);
    return rs;
}

//-----------------------------------------------------------------------------
// insert entry in HW at index
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::insert_atid(sdk_table_api_params_t *params)
{
    sdk_ret_t rs = SDK_RET_OK;
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint32_t index;

    // Here index should be allocated already
    GET_INDEX_FROM_PARAM(params, index, rs, end);
    CHECK_INDEX_STATUS(index, rs, SDK_RET_NO_RESOURCE, end);
    // print entry
    PRINT_DATA(__func__, params->actiondata, index);

    // program P4
    pd_err = p4pd_global_entry_write_with_datamask(props_->ptable_id, index, NULL, NULL,
                                                   params->actiondata, params->actiondata_mask);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
        SDK_ASSERT(0);
    }

    DIRECTMAP_TRACE_DEBUG("index %u", index);
    in_use_++;
end:
    api_stats_.insert_atid(rs);
    return rs;
}

//-----------------------------------------------------------------------------
// reserve a given id
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::reserve_index(sdk_table_api_params_t *params) {
    sdk_ret_t ret;
    uint32_t index;

    GET_INDEX_FROM_PARAM(params, index, ret, end);

    ret = alloc_index_withid_(index);
    if (ret != SDK_RET_OK) {
        goto end;
    }

    DIRECTMAP_TRACE_DEBUG("index %u", index);
    in_use_++;
end:
    api_stats_.reserve_index(ret);
    return ret;
}

//-----------------------------------------------------------------------------
// reserve a free id and return the allocated index
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::reserve(sdk_table_api_params_t *params) {
    sdk_ret_t ret;
    uint32_t index;

    ret = alloc_index_(&index);
    if (ret == SDK_RET_OK) {
        SET_INDEX_TO_PARAM(params, index);
    }

    DIRECTMAP_TRACE_DEBUG("index %u", index);
    in_use_++;
    api_stats_.reserve(ret);
    return ret;
}

//-----------------------------------------------------------------------------
// release a given index
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::release(sdk_table_api_params_t *params) {
    sdk_ret_t ret = SDK_RET_OK;
    uint32_t index;

    GET_INDEX_FROM_PARAM(params, index, ret, end);
    CHECK_INDEX_STATUS(index, ret, SDK_RET_ENTRY_NOT_FOUND, end);
    ret = free_index_(index);

    DIRECTMAP_TRACE_DEBUG("index %u", index);
    in_use_--;
end:
    api_stats_.release(ret);
    return ret;
}

//-----------------------------------------------------------------------------
// update entry in HW
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::update(sdk_table_api_params_t *params)
{
    sdk_ret_t           rs     = SDK_RET_OK;
    p4pd_error_t        pd_err = P4PD_SUCCESS;
    uint32_t index;

    GET_INDEX_FROM_PARAM(params, index, rs, end);
    CHECK_INDEX_STATUS(index, rs, SDK_RET_ENTRY_NOT_FOUND, end);
    // print entry
    PRINT_DATA(__func__, params->actiondata, index);

    // update p4 table entry
    pd_err = p4pd_global_entry_write_with_datamask(props_->ptable_id, index, NULL, NULL,
                                                   params->actiondata, params->actiondata_mask);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
        SDK_ASSERT(0);
    }

    DIRECTMAP_TRACE_DEBUG("index %u", index);
end:
    api_stats_.update(rs);
    return rs;
}

//-----------------------------------------------------------------------------
// remove entry from HW
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::remove(sdk_table_api_params_t *params)
{
    sdk_ret_t           rs        = SDK_RET_OK;
    p4pd_error_t        pd_err    = P4PD_SUCCESS;
    void                *tmp_data = NULL;
    uint32_t            index;

    GET_INDEX_FROM_PARAM(params, index, rs, end);
    CHECK_INDEX_STATUS(index, rs, SDK_RET_ENTRY_NOT_FOUND, end);
    // update P4 table by zeroing out
    tmp_data = SDK_CALLOC(SDK_MEM_ALLOC_LIB_DIRECT_MAP_SW_DATA, props_->swdata_len);
    pd_err = p4pd_global_entry_write(props_->ptable_id, index, NULL, NULL, tmp_data);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
        goto end;
    }

    rs = free_index_(index);
    if (rs != SDK_RET_OK) {
       goto end;
    }

    DIRECTMAP_TRACE_DEBUG("index %u", index);
    in_use_--;
end:
    if (tmp_data) {
        SDK_FREE(SDK_MEM_ALLOC_LIB_DIRECT_MAP_SW_DATA, tmp_data);
    }
    api_stats_.remove(rs);
    return rs;
}

//-----------------------------------------------------------------------------
// retrieve entry from HW
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::get(sdk_table_api_params_t *params)
{
    sdk_ret_t       rs = SDK_RET_OK;
    p4pd_error_t    pd_err = P4PD_SUCCESS;
    uint32_t        index;

    GET_INDEX_FROM_PARAM(params, index, rs, end);
    CHECK_INDEX_STATUS(index, rs, SDK_RET_ENTRY_NOT_FOUND, end);
    SDK_ASSERT(params->actiondata);

    // read the entry
    pd_err = p4pd_global_entry_read(props_->ptable_id, index, NULL, NULL, params->actiondata);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
        SDK_ASSERT(0);
    }

    DIRECTMAP_TRACE_DEBUG("index %u", index);
end:
    api_stats_.get(rs);
    return rs;
}

//-----------------------------------------------------------------------------
// iterate every entry and gives a call back
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::iterate(sdk_table_api_params_t *params)
{
    p4pd_error_t    pd_err = P4PD_SUCCESS;
    sdk_ret_t       rs = SDK_RET_OK;
    void            *tmp_data;
    sdk_table_api_params_t p = { 0 };

    SDK_ASSERT(params->itercb);
    tmp_data = SDK_CALLOC(SDK_MEM_ALLOC_LIB_DIRECT_MAP_HW_DATA, props_->swdata_len);
    for (uint32_t i = 0; i < props_->ptable_size; i++) {
        if (indexer_->is_index_allocated(i)) {
            pd_err = p4pd_global_entry_read(props_->ptable_id, i, NULL, NULL, tmp_data);
            SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);
            p.actiondata = tmp_data;
            p.handle.pindex(i);
            p.cbdata = params->cbdata;
            params->itercb(&p);
        }
    }
end:
    SDK_FREE(SDK_MEM_ALLOC_LIB_DIRECT_MAP_HW_DATA, tmp_data);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
        SDK_ASSERT(0);
    }
    return rs;
}

//---------------------------------------------------------------------------
// get stats
//---------------------------------------------------------------------------
sdk_ret_t
sldirectmap::stats_get(sdk_table_api_stats_t *api_stats) {
    api_stats_.get(api_stats);
    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// allocate an index
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::alloc_index_(uint32_t *idx)
{
    sdk_ret_t ret;

    ret = indexer_->alloc(idx);
    if (ret != SDK_RET_OK)
        DIRECTMAP_TRACE_DEBUG("Failed to allocate index with err: %d", ret);
    else
        DIRECTMAP_TRACE_DEBUG("Scan name %s idx %u\n", props_->name.c_str(), *idx);
    return ret;
}

//-----------------------------------------------------------------------------
// allocate an index with id
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::alloc_index_withid_(uint32_t idx)
{
    return indexer_->alloc(idx);
}

//-----------------------------------------------------------------------------
// free an index
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::free_index_(uint32_t idx)
{
    return indexer_->free(idx);
}

//-----------------------------------------------------------------------------
// returns string of the entry
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::entry_to_str(void *data, uint32_t index, char *buff,
                        uint32_t buff_size)
{
    p4pd_error_t    p4_err;

    p4_err = p4pd_global_table_ds_decoded_string_get(props_->ptable_id, index, NULL, NULL,
                                                     data, buff, sizeof(buff));
    SDK_ASSERT(p4_err == P4PD_SUCCESS);
    return SDK_RET_OK;
}

//-----------------------------------------------------------------------------
// prints the decoded entry
//-----------------------------------------------------------------------------
sdk_ret_t
sldirectmap::entry_trace_(const char *fname, void *data, uint32_t index)
{
    sdk_ret_t    ret;
    char         buff[4096] = {0};

    ret = entry_to_str(data, index, buff, sizeof(buff));
    SDK_ASSERT(ret == SDK_RET_OK);
    DIRECTMAP_TRACE_DEBUG("%s, TableId:%d, Index: %u\n%s", fname,
                          props_->ptable_id, index, buff);
    return SDK_RET_OK;
}

}    // namespace table
}    // namespace sdk
