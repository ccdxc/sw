//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// hash (with/without otcam) table management library
//
// - manages hash tables
// - eventually calls P4 APIs to program.
//
//------------------------------------------------------------------------------

#ifndef __SDK_HASH_HPP__
#define __SDK_HASH_HPP__

#include <boost/crc.hpp>
#include "sdk/mem.hpp"
#include "sdk/base.hpp"
#include "lib/table/hash/hash_entry.hpp"

using namespace std;

// global forward declarations
namespace sdk::table {
    class tcam;
}
using sdk::table::tcam;
using sdk::lib::ht;

namespace sdk {
namespace table {

typedef bool (*hash_iterate_func_t)(const void *key,
                                    const void *data,
                                    uint32_t hash_idx,
                                    const void *cb_data);

class hash {

public:
    enum HashPoly {
        // CRC_32,
        HASH_POLY0,
        HASH_POLY1,
        HASH_POLY2,
        HASH_POLY3
    };

    enum EntryType {
        DLEFT,
        OTCAM,
        BOTH
    };

    // Note: Stats are mutually exclusive for every API. Only one stat will
    //       be incremented for an API call.
    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_FAIL_HW,
        STATS_INS_FAIL_DUP_INS,
        STATS_INS_FAIL_NO_RES,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_OOB,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_UPD_FAIL_INV_ARG,
        STATS_UPD_FAIL_HW,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_OOB,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_INV_ARG,
        STATS_REM_FAIL_HW,
        STATS_RETR_SUCCESS,
        STATS_RETR_FAIL_OOB,
        STATS_RETR_FAIL_ENTRY_NOT_FOUND,
        STATS_RETR_FAIL_INV_ARG,
        STATS_NUM_HASH,
        STATS_NUM_TCAM,

        STATS_MAX

    };
private:

    char            *name_;             // table name
    uint32_t        id_;                // table id
    uint32_t        dleft_capacity_;    // size of dleft table
    uint32_t        swkey_len_;         // sw key len
    uint32_t        swdata_len_;        // sw data len
    HashPoly        hash_poly_;         // hash polynomial
    uint32_t        hwkey_len_;         // hw key len
    uint32_t        hwdata_len_;        // hw data len
    tcam            *otcam_;            // overflow tcam
    // HashEntryMap    hash_entry_map_;    // hash map
    ht              *entry_ht_;         // hash table to store entries
    bool            entry_trace_en_;    // entry trace enable

    // Static Declarations
    static const uint8_t otcam_bit_ = 28;    // Dleft or OTcam ?

    uint64_t        *stats_;                 // Statistics

    // overflow tcam presence
    bool has_otcam_();  
    // Generates Hash 
    uint32_t generate_hash_(void *key, uint32_t key_len);
    // hw program 
    sdk_ret_t program_table_(hash_entry_t *he, void *hwkey);
    sdk_ret_t deprogram_table_(hash_entry_t *he);


    static uint32_t form_hash_idx_from_dleft_id_(uint32_t dleft_id);
    static uint32_t form_hash_idx_from_otcam_id_(uint32_t otcam_id);
    typedef struct otcam_iterate_cb_s {
        hash_iterate_func_t cb;
        const void *cb_data;
    } otcam_iterate_cb_t;
    static bool otcam_iterate_(const void *key, 
                               const void *key_mask, 
                               const void *data, 
                               uint32_t tcam_idx, 
                               const void *cb_data);
    void stats_incr(stats stat);
    void stats_decr(stats stat);
    enum api {
        INSERT,
        UPDATE,
        REMOVE,
        RETRIEVE
    };
    void stats_update(api ap, sdk_ret_t rs); 
    // Entry Trace
    sdk_ret_t entry_trace_(hash_entry_t *he);

    hash(char *name, uint32_t dleft_table_id, 
         uint32_t otcam_table_id, uint32_t dleft_capacity,
         uint32_t otcam_capacity, uint32_t swkey_len, uint32_t swdata_len, 
         hash::HashPoly hash_poly = HASH_POLY0, bool entry_trace_en = false);
    ~hash();
public:
    static hash *factory(char *name, uint32_t dleft_table_id,
                         uint32_t otcam_table_id, uint32_t dleft_capacity,
                         uint32_t otcam_capacity, uint32_t swkey_len,
                         uint32_t swdata_len, hash::HashPoly hash_poly = HASH_POLY0,
                         bool entry_trace_en = false);
    static void destroy(hash *hash);

    // Debug Info
    uint32_t id(void) { return id_; }
    const char *name(void) { return name_; }
    uint32_t capacity(void) { return dleft_capacity_; }
    uint32_t oflow_capacity(void);
    uint32_t num_entries_in_use(void);
    uint32_t oflow_num_entries_in_use(void);
    uint32_t num_inserts(void);
    uint32_t num_insert_errors(void);
    uint32_t num_deletes(void);
    uint32_t num_delete_errors(void);

    sdk_ret_t insert(void *key, void *data, uint32_t *index, 
                     void *key_mask = NULL, bool direct_to_otcam = FALSE);
    sdk_ret_t update(uint32_t index, void *data);
    sdk_ret_t remove(uint32_t index);
    sdk_ret_t retrieve(uint32_t index, void *key, void *data);
    sdk_ret_t iterate(hash_iterate_func_t cb, const void *cb_data,
                      hash::EntryType type);

    // For Debugging
    static bool is_dleft(uint32_t hash_idx);
    static uint32_t get_dleft_id_from_hash_idx_(uint32_t hash_idx);
    static uint32_t get_otcam_id_from_hash_idx_(uint32_t hash_idx);
};

}    // namespace table
}    // namespace sdk

using sdk_hash = sdk::table::hash;

#endif    // __SDK_HASH_HPP__

