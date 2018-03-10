//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// tcam table management library
//
// - manages tcam tables
// - eventually calls P4 APIs to program.
//
// TODO:
//      - For INSERT_FAIL_DUPLICATE, instead of walking all entries better
//        store in some structure keyed on key, key_mask without replicating
//        the tcam_entry
//------------------------------------------------------------------------------

#ifndef __SDK_TCAM_HPP__
#define __SDK_TCAM_HPP__

#include "sdk/mem.hpp"
#include "sdk/base.hpp"
#include "sdk/indexer.hpp"
#include "sdk/ht.hpp"

using sdk::lib::indexer;
using sdk::lib::ht;

namespace sdk {
namespace table {

typedef struct tcam_entry_s tcam_entry_t;

typedef bool (*tcam_iterate_func_t)(const void *key,
                                    const void *key_mask,
                                    const void *data,
                                    uint32_t tcam_idx,
                                    const void *cb_data);
class tcam {
public:
    // NOTE : stats are mutually exclusive for every API. Only one stat will
    //        be incremented for an API call.
    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_FAIL_DUP_INS,
        STATS_INS_FAIL_NO_RES,
        STATS_INS_FAIL_HW,
        STATS_INS_WITHID_SUCCESS,
        STATS_INS_WITHID_FAIL_DUP_INS,
        STATS_INS_WITHID_FAIL_HW,
        STATS_INS_WITHID_FAIL_OOB,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_OOB,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_UPD_FAIL_HW,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_OOB,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_HW,
        STATS_RETR_SUCCESS,
        STATS_RETR_FAIL_OOB,
        STATS_RETR_FAIL_ENTRY_NOT_FOUND,
        STATS_RETR_FROM_HW_SUCCESS,
        STATS_RETR_FROM_HW_FAIL_OOB,
        STATS_RETR_FROM_HW_FAIL,
        STATS_MAX
    };

private:
    enum api {
        INSERT,
        INSERT_WITHID,
        UPDATE,
        REMOVE,
        RETRIEVE,
        RETRIEVE_FROM_HW,
        ITERATE
    };

    char            *name_;                 // table name
    uint32_t        id_;                    // table id
    uint32_t        capacity_;              // size of tcam table 
    uint32_t        swkey_len_;             // sw key len
    uint32_t        swdata_len_;            // sw data len
    bool            allow_dup_insert_;      // allow uplicate insert
    uint32_t        hwkey_len_;             // hw key len
    uint32_t        hwkeymask_len_;         // hw key mask len
    uint32_t        hwdata_len_;            // hw data len
    ht              *entry_ht_;             // hash table to store entries
    indexer         *indexer_;              // tcam entry indices
    uint64_t        *stats_;                // statistics
    bool            entry_trace_en_;        // enable entry tracing

    sdk_ret_t alloc_index_(uint32_t *idx, bool lowest);
    sdk_ret_t alloc_index_withid_(uint32_t idx);
    sdk_ret_t free_index_(uint32_t idx);
    bool entry_exists_(void *key, void *key_mask, uint32_t key_len,
                       tcam_entry_t **te);
    sdk_ret_t program_table_(tcam_entry_t *te);
    sdk_ret_t deprogram_table_(tcam_entry_t *te);
    void stats_incr_(stats stat);
    void stats_decr_(stats stat);
    void stats_update_(api ap, sdk_ret_t rs); 
    sdk_ret_t entry_trace_(tcam_entry_t *te);
    tcam(uint32_t id, uint32_t capacity,
         uint32_t swkey_len, uint32_t swdata_len, 
         bool allow_dup_insert = false, bool entry_trace_en = false);
    ~tcam();

public:
    // factory & destroy methods
    static tcam *factory(char *name, uint32_t id, 
                         uint32_t tcam_capacity, uint32_t swkey_len, 
                         uint32_t swdata_len, bool allow_dup_insert = false,
                         bool entry_trace_en = false);
    static void destroy(tcam *tcam);

    // get methods
    uint32_t id(void) const { return id_; }
    const char *name(void) const { return name_; }
    uint32_t capacity(void) const { return capacity_; }
    uint32_t num_entries_in_use(void) const;
    uint32_t num_inserts(void) const;
    uint32_t num_insert_errors(void) const;
    uint32_t num_deletes(void) const;
    uint32_t num_delete_errors(void) const;

    // operational methods
    sdk_ret_t insert(void *key, void *key_mask, void *data, uint32_t *index,
                     bool lowest=true);
    sdk_ret_t insert_withid(void *key, void *key_mask, void *data,
                            uint32_t index);
    sdk_ret_t update(uint32_t index, void *data);
    sdk_ret_t remove(uint32_t index);
    sdk_ret_t retrieve(uint32_t index, void *key, void *key_mask, 
                       void *data);
    sdk_ret_t retrieve_from_hw(uint32_t index, void *key, void *key_mask, 
                               void *data);
    sdk_ret_t iterate(tcam_iterate_func_t func, const void *cb_data);
    sdk_ret_t fetch_stats(const uint64_t **stats);
};

}    // namespace table
}    // namespace sdk

#endif    // __SDK_TCAM_HPP__

