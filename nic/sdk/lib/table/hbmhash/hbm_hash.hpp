//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

// ============================================================================
//
// Hbm Hash Table Managment
//
//
//  Hash(key) =
//  +------+---------+
//  |  11  |    21   |
//  +------+---------+
//              |        HBM Hash Table
//              |           +-----+
//              |           |     |
//              |           +-----+     +-----+----+----+----+------+----+
//              |-------->  |     | --> | 320 | 20 | 11 | 14 |  ... | 15 |
//                          +-----+     +-----+----+----+----+------+----+
//                          |     |                        |
//                          +-----+                        | HBM Hash Coll Tbl
//                          |     |                        |      +----+
//                          +-----+                        |      |    |
//                          |     |                        |      +----+
//                          +-----+                        -----> |    |
//                                                                +----+
//                                                                |    |
//                                                                +----+
//
//
//
// < ----------     Spine Entry     ---------->
// +-----+----+----+----+------+----+----+----+
// |  Anchor  |Hint Grp | ...  |Hint Grp | NS |
// +-----+----+----+----+------+----+----+----+
//
// Spine Entry: Entry which either goes into HBM Hash Table or HBM Hash Collision
//              Table and has Hint Groups.
// Anchor: Initial Few bits of spine entry which has the key and data of
//         a HBM Hash Entry.
// Hint Group: HBM Hash entries which has the same 21 bits and 11 bits.
//             6 HGs per spine.
// NS: Next Spine Pointer.
//
// ============================================================================
#ifndef __HBM_HASH_HPP__
#define __HBM_HASH_HPP__

#include "include/sdk/base.hpp"
#include <string>
#include <map>
#include <queue>
#include "lib/indexer/indexer.hpp"
#include "include/sdk/mem.hpp"
#include "lib/utils/crc_fast.hpp"
#include "lib/table/common/table.hpp"
#include "hbm_hash_mem_types.hpp"
#include "lib/slab/slab.hpp"

using namespace std;
using sdk::lib::indexer;
using sdk::utils::crcFast;
using sdk::table::table_health_state_t;
using sdk::table::table_health_monitor_func_t;

namespace sdk {
namespace table {

/* forward declarations */
class HbmHashTableEntry;
class HbmHashEntry;
class HbmHashHintGroup;

typedef bool (*hbm_hash_iterate_func_t)(uint32_t gl_index,
                                    const void *cb_data);

class HbmHash {
public:
    enum HashPoly {
        // CRC_32,
        HASH_POLY0,
        HASH_POLY1,
        HASH_POLY2,
        HASH_POLY3,
        HASH_POLY_MAX = HASH_POLY3
    };

    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_HBM_HASH_COLL, // STATS_INS_SUCCESS will not be incr.
        STATS_INS_FAIL_DUP_INS,
        STATS_INS_FAIL_NO_RES,
        STATS_INS_FAIL_HW,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_HW,
        STATS_MAX
    };


private:
    // Private Data
    std::string         table_name_;                    // table name
    uint32_t            table_id_;                      // table id
    uint32_t            collision_table_id_;            // collision table id
    uint32_t            hash_capacity_;                 // size of collision table
    uint32_t            coll_capacity_;                 // size of coll. table
    uint32_t            key_len_;                       // key len
    uint32_t            data_len_;                      // data len
    uint32_t            entire_data_len_;               // entire data len
    uint32_t            max_recircs_;                   // max. recircs
    HashPoly            hash_poly_;                     // hash polynomial
    crcFast             *crc_;                          // crc Table for fast comput.

    slab                *hbm_hash_entry_slab_;          // hbm_hash_entry slab
    slab                *hbm_sw_key_slab_;              // sw key slab
    slab                *hbm_hw_key_slab_;              // hw key slab
    slab                *hbm_sw_data_slab_;             // data slab
    slab                *hbm_hash_hint_group_slab_;     // hint group slab
    slab                *hbm_hash_table_entry_slab_;    // table entry slab
    slab                *hbm_hash_spine_entry_slab_;    // table entry slab

    uint32_t            hash_tbl_key_len_;              // hash table key len (21)
    uint32_t            hash_coll_tbl_key_len_;         // coll table key len (14)
    uint32_t            hint_len_;                      // hint len (11)
    uint32_t            hint_mem_len_B_;                // sw index into coll table
    uint32_t            num_hints_per_entry_;           // HGs per HBM Hash Entry (6)

    uint32_t            hwkey_len_;                     // Key len for HBM Hash Table
    uint32_t            hwdata_len_;                    // Data Len for HBM Hash Table

    bool                enable_delayed_del_;            // enable delayed del
    bool                entry_trace_en_;                // enable entry tracing


    table_health_state_t        health_state_;          // health state of collision table
    table_health_monitor_func_t health_monitor_func_;   // health mon. cb

    // Flat array with (21 bit key) => HBM Hash Table Entry
    HbmHashTableEntry   **hbm_hash_table_;
    uint32_t            hbm_hash_table_count_;

    // indexer for HBM Hash Coll. Table
    indexer             *coll_indexer_;

    // indexer and Map to store HBM Hash Entries
    indexer             *entry_indexer_;
    HbmHashEntry        **entry_map_;
    uint32_t            entry_count_;

    // Delayed Delete Queue
    std::queue<HbmHashEntry *> hbm_hash_entry_del_q_;
    std::queue<HbmHashHintGroup *> hbm_hash_hg_del_q_;

    uint64_t        *stats_;                // Statistics

    // Private Methods
    uint32_t get_num_bits_from_size_(uint32_t size);
    void pre_process_sizes_(uint32_t num_hash_entries,
                            uint32_t num_hash_coll_entries);
    uint32_t generate_hash_(void *key, uint32_t key_len, bool log = true);
    void stats_incr(stats stat);
    void stats_decr(stats stat);
    enum api {
        INSERT,
        UPDATE,
        REMOVE,
        RETRIEVE,
        RETRIEVE_FROM_HW,
        ITERATE
    };
    void stats_update(api ap, sdk_ret_t rs);
    // Public Methods
    HbmHash(std::string table_name, uint32_t table_id, uint32_t collision_table_id,
            uint32_t hash_capacity,
            uint32_t coll_capacity,
            uint32_t key_len,                  // 320
            uint32_t data_len,                 // 20
            // uint32_t hbm_hash_table_entry_len,          // 512
            uint32_t num_hints_per_entry = 6,
            uint32_t max_recircs = 8,
            HbmHash::HashPoly hash_poly = HASH_POLY0,
            bool entry_trace_en = false,
            table_health_monitor_func_t health_monitor_func = NULL);
    ~HbmHash();
    sdk_ret_t init();
    sdk_ret_t initialize_slabs();
    sdk_ret_t destroy_slabs();

    void insert_bucket(uint32_t index, HbmHashTableEntry* bucket);
    HbmHashTableEntry *remove_bucket(uint32_t index);
    HbmHashTableEntry *retrieve_bucket(uint32_t index) { return hbm_hash_table_[index]; }
    uint32_t bucket_count() { return hbm_hash_table_count_; }

    void insert_entry(uint32_t index, HbmHashEntry* bucket);
    HbmHashEntry *remove_entry(uint32_t index);
    HbmHashEntry *retrieve_entry(uint32_t index) { return entry_map_[index]; }
    uint32_t entry_count() { return entry_count_; }
    void trigger_health_monitor();
public:
    static HbmHash *factory(std::string table_name, uint32_t table_id,
                            uint32_t collision_table_id, uint32_t hash_capacity,
                            uint32_t coll_capacity, uint32_t key_len,
                            uint32_t data_len, uint32_t num_hints_per_entry = 6,
                            uint32_t max_recircs = 8,
                            HbmHash::HashPoly hash_poly = HASH_POLY0,
                            uint32_t mtrack_id = SDK_MEM_ALLOC_FLOW,
                            bool entry_trace_en = false,
                            table_health_monitor_func_t health_monitor_func = NULL);
    static void destroy(HbmHash *hbmhash,
                        uint32_t mtrack_id = SDK_MEM_ALLOC_FLOW);

    // Slab APIs
    void *hbm_hash_entry_alloc();
    void hbm_hash_entry_free(void *entry);
    void *hbm_sw_key_alloc();
    void hbm_sw_key_free(void *entry);
    void *hbm_hw_key_alloc();
    void hbm_hw_key_free(void *entry);
    void *hbm_sw_data_alloc();
    void hbm_sw_data_free(void *entry);
    void *hbm_hash_hint_group_alloc();
    void hbm_hash_hint_group_free(void *entry);
    void *hbm_hash_table_entry_alloc();
    void hbm_hash_table_entry_free(void *entry);
    void *hbm_hash_spine_entry_alloc();
    void hbm_hash_spine_entry_free(void *entry);


    // Debug Info
    uint32_t table_id(void) { return table_id_; }
    const char *table_name(void) { return table_name_.c_str(); }
    uint32_t table_capacity(void) { return hash_capacity_; }
    uint32_t coll_table_capacity(void) { return coll_capacity_; }
    uint32_t max_recircs(void) { return max_recircs_; }
    uint32_t table_num_entries_in_use(void);
    uint32_t coll_table_num_entries_in_use(void);
    uint32_t table_num_inserts(void);
    uint32_t table_num_insert_errors(void);
    uint32_t table_num_updates(void);
    uint32_t table_num_update_errors(void);
    uint32_t table_num_deletes(void);
    uint32_t table_num_delete_errors(void);

    sdk_ret_t insert(void *key, void *data, uint32_t *index);
    sdk_ret_t insert_with_hash(void *key, void *data, uint32_t *index, uint32_t hash_val);
    // calc_hash_ is a test only method used to generate hash collissions
    uint32_t calc_hash_(void *key, void *data);
    sdk_ret_t update(uint32_t index, void *data);
    sdk_ret_t remove(uint32_t index);


    void hbm_hash_entry_free_to_slab_(void *entry) {
        hbm_hash_entry_slab_->free(entry);
    }

    /*
    Hash::ReturnStatus retrieve(uint32_t index, void **key, uint32_t *key_len,
                                void **data, uint32_t *data_len);
    Hash::ReturnStatus iterate(
            boost::function<Hash::ReturnStatus (const void *key, uint32_t key_len,
                                                const void *data, uint32_t data_len,
                                                uint32_t hash_idx, const void *cb_data)> cb,
                                                const void *cb_data,
                                                Hash::EntryType type);
    */
    sdk_ret_t iterate(hbm_hash_iterate_func_t func, const void *cb_data);
    sdk_ret_t alloc_entry_index_(uint32_t *idx);
    sdk_ret_t free_hbm_hash_entry_index_(uint32_t idx);

    uint32_t fetch_hbm_hash_table_bits_(uint32_t hash_val);
    uint32_t fetch_hint_bits_(uint32_t hash_val);
    sdk_ret_t alloc_collision_index(uint32_t *index);
    sdk_ret_t free_collision_index(uint32_t index);
    void add_hbm_hash_entry_global_map(HbmHashEntry *fe, uint32_t index);
    sdk_ret_t print_hbm_hash();
    sdk_ret_t entry_to_str(uint32_t gl_index, char *buff, uint32_t buff_size);

    // Getters & Setters
    bool get_delayed_del_en();
    uint32_t get_table_id() { return table_id_; }
    uint32_t get_collision_table_id() { return collision_table_id_; }
    uint32_t get_data_len() { return data_len_; }
    uint32_t get_entire_data_len() { return entire_data_len_; }
    uint32_t get_key_len() { return key_len_; }
    uint32_t get_hwkey_len() { return hwkey_len_; }
    uint32_t get_hwdata_len() { return hwdata_len_; }
    uint32_t get_hint_mem_len_B(void) { return hint_mem_len_B_; }
    bool get_entry_trace_en() { return entry_trace_en_; }


    void set_delayed_del_en(bool en);

    uint32_t get_num_hints_per_entry();
    indexer *get_coll_indexer();
    void push_fe_delete_q(HbmHashEntry *fe);
    void push_hg_delete_q(HbmHashHintGroup *hg);
    sdk_ret_t hbm_hash_action_data_offsets(void *action_data,
                                       uint8_t **action_id,
                                       uint8_t **entry_valid,
                                       void **data,
                                       void **first_hash_hint,
                                       uint8_t **more_hashs,
                                       void **more_hints);

};

}   // namespace table
}   // namespace sdk

using sdk::table::HbmHash;

#endif // __HBM_HASH_HPP__
