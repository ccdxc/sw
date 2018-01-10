//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// directmap (index) table management library
//
// - manages directmap tables
// - eventually calls P4 APIs to program.
//
//------------------------------------------------------------------------------
#ifndef __SDK_DIRECT_MAP_HPP__
#define __SDK_DIRECT_MAP_HPP__

#include "sdk/mem.hpp"
#include "sdk/base.hpp"
#include "lib/indexer/indexer.hpp"
#include "lib/table/directmap/directmap_entry.hpp"

using sdk::lib::ht;
using sdk::lib::indexer;

namespace sdk {
namespace table {

typedef bool (*direct_map_iterate_func_t)(uint32_t index, void *data, 
                                          const void *cb_data);

class directmap {

public:
    // Note: Stats are mutually exclusive for every API. Only one stat will
    //       be incremented for an API call.
    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_FAIL_HW,
        STATS_INS_FAIL_NO_RES,
        STATS_INS_WITHID_SUCCESS,
        STATS_INS_WITHID_FAIL_DUP_INS,
        STATS_INS_WITHID_FAIL_OOB,
        STATS_INS_WITHID_FAIL_HW,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_INV_ARG,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_UPD_FAIL_HW,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_INV_ARG,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_HW,
        STATS_RETR_SUCCESS,
        STATS_RETR_FAIL_INV_ARG,
        STATS_RETR_FAIL_ENTRY_NOT_FOUND,
        STATS_RETR_FAIL_HW,
        STATS_ITER_SUCCESS,
        STATS_ITER_FAIL_HW,
        STATS_MAX
    };
private:

    enum api {
        INSERT,
        INSERT_WITHID,
        UPDATE,
        REMOVE,
        RETRIEVE,
        ITERATE
    };

    char            *name_;          // table name
    uint32_t        id_;            // table id
    uint32_t        capacity_;      // size of table
    indexer         *indexer_;      // entry indices
    uint32_t        swdata_len_;    // sw data len
    uint32_t        hwdata_len_;    // hw data len
    ht              *entry_ht_;     // hash table to store entries
    uint64_t        *stats_;        // statistics
    bool            sharing_en_;    // enable sharing

    // private methods
    sdk_ret_t alloc_index_(uint32_t *idx);  
    sdk_ret_t alloc_index_withid_(uint32_t idx);    
    sdk_ret_t free_index_(uint32_t idx);
    void stats_incr(stats stat);
    void stats_decr(stats stat);
    void stats_update(api ap, sdk_ret_t rs); 
    sdk_ret_t entry_trace_(void *data, uint32_t index);

    static void * dm_entry_get_key_func(void *entry);
    static uint32_t dm_entry_compute_hash_func(void *key, uint32_t ht_size);
    static bool dm_entry_compare_key_func(void *key1, void *key2);

    sdk_ret_t add_directmap_entry_to_db(directmap_entry_t *dme);
    void *del_directmap_entry_from_db(directmap_entry_t *dme);
    directmap_entry_t *find_directmap_entry(directmap_entry_t *key);

    directmap(char *name, uint32_t id, uint32_t capacity, 
              uint32_t swdata_len, bool sharing_en = false);
    ~directmap();
public:
    // factory & destroy methods
    static directmap *factory(char *name, uint32_t id, 
                              uint32_t capacity, uint32_t swdata_len, 
                              bool sharing_en = false);
    static void destroy(directmap *dm);

    // debug methods
    uint32_t id(void) { return id_; }
    const char *name(void) { return name_;}
    uint32_t capacity(void) { return capacity_; }
    uint32_t num_entries_in_use(void) const;
    uint32_t num_inserts(void) const;
    uint32_t num_insert_errors(void) const;
    uint32_t num_deletes(void) const;
    uint32_t num_delete_errors(void) const;


    // operational methods
    sdk_ret_t insert(void *data, uint32_t *index);
    sdk_ret_t insert_withid(void *data, uint32_t index);
    sdk_ret_t update(uint32_t index, void *data);
    sdk_ret_t remove(uint32_t index, void *data = NULL);
    sdk_ret_t retrieve(uint32_t index, void *data);
    sdk_ret_t iterate(direct_map_iterate_func_t iterate_func, 
                      const void *cb_data);
    sdk_ret_t fetch_stats(const uint64_t **stats);
};

}    // namespace table
}    // namespace sdk

#endif // __SDK_DIRECT_MAP_HPP__

