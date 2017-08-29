// ============================================================================
//  
// Flow Table Managment
// 
//  
//  Hash(Flow_key) = 
//  +------+---------+
//  |  11  |    21   |
//  +------+---------+
//              |        Flow Hash Table
//              |           +-----+
//              |           |     |
//              |           +-----+     +-----+----+----+----+------+----+
//              |-------->  |     | --> | 320 | 20 | 11 | 14 |  ... | 15 |
//                          +-----+     +-----+----+----+----+------+----+
//                          |     |                        |
//                          +-----+                        | Flow Hash Coll Tbl
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
// Spine Entry: Entry which either goes into Flow Table or Flow Hash Collision 
//              Table and has Hint Groups.
// Anchor: Initial Few bits of spine entry which has the key and data of 
//         a Flow Entry.
// Hint Group: Flow entries which has the same 21 bits and 11 bits. 
//             6 HGs per spine.
// NS: Next Spine Pointer.
//
// ============================================================================


#ifndef __FLOW_HPP__
#define __FLOW_HPP__

#include <base.h>
#include <string>
#include <map>
#include <queue>
#include "indexer.hpp"
#include <boost/crc.hpp>

using namespace std;
using hal::utils::indexer;

namespace hal {
namespace pd {
namespace utils {

/* forward declarations */
class FlowTableEntry;
class FlowEntry;
class FlowHintGroup;


typedef std::map<uint32_t, FlowTableEntry*> FlowTableEntryMap;
typedef std::map<uint32_t, FlowEntry*> FlowEntryMap;

/** ---------------------------------------------------------------------------
  * 
  * class Flow
  *
  *     - Flow Management
  *
  * ---------------------------------------------------------------------------
*/
class Flow {
public:
    enum HashPoly {
        // CRC_32,
        HASH_POLY0,
        HASH_POLY1,
        HASH_POLY2,
        HASH_POLY3
    };

private:

    // Private Data
    std::string     table_name_;                // table name
    uint32_t        table_id_;                  // table id
    uint32_t        oflow_table_id_;            // oflow table id
    uint32_t        flow_hash_capacity_;        // size of flow table
    uint32_t        flow_coll_capacity_;        // size of coll. table
    uint32_t        key_len_;                   // key len
    uint32_t        data_len_;                  // data len
    HashPoly        hash_poly_;                 // hash polynomial


    uint32_t        hash_tbl_key_len_;          // hash table key len (21)
    uint32_t        hash_coll_tbl_key_len_;     // coll table key len (14)
    uint32_t        hint_len_;                  // hint len (11)
    uint32_t        num_hints_per_flow_entry_;  // HGs per Flow Entry (6)

    uint32_t        hwkey_len_;             // Key len for Flow Hash Table
    uint32_t        hwdata_len_;            // Data Len for Flow Hash Table

    bool            enable_delayed_del_;    // enable delayed el

    // Hash Value(21 bits) => Flow Table Entry
    std::map<uint32_t, FlowTableEntry*> flow_table_; 

    // indexer for Flow Coll. Table
    indexer         *flow_coll_indexer_;

    // indexer and Map to store Flow Entries
    indexer         *flow_entry_indexer_;
    std::map<uint32_t, FlowEntry*> flow_entry_map_;

	// Remove this once p4pd apis are implemented.
	bool hwkey_is_swkey_;

    // Delayed Delete Queue
    std::queue<FlowEntry *> flow_entry_del_q_;
    std::queue<FlowHintGroup *> flow_hg_del_q_;

    // Private Methods
    uint32_t get_num_bits_from_size_(uint32_t size);
    void pre_process_sizes_(uint32_t num_flow_hash_entries,
                            uint32_t num_flow_hash_coll_entries);
    uint32_t generate_hash_(void *key, uint32_t key_len, bool log = true);
public:


    // Public Methods
    Flow(std::string table_name, uint32_t table_id, uint32_t oflow_table_id,
            uint32_t flow_hash_capacity,         
            uint32_t oflow_capacity,    
            uint32_t flow_key_len,                  // 320
            uint32_t flow_data_len,                 // 20
            // uint32_t flow_table_entry_len,          // 512
            uint32_t num_hints_per_flow_entry = 6,
            Flow::HashPoly hash_poly = HASH_POLY0);
    ~Flow();


    hal_ret_t insert(void *key, void *data, uint32_t *index);
    // calc_hash_ is a test only method used to generate hash collissions
    uint32_t calc_hash_(void *key, void *data);
    hal_ret_t update(uint32_t index, void *data);
    hal_ret_t remove(uint32_t index);


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
	hal_ret_t alloc_flow_entry_index_(uint32_t *idx);
	hal_ret_t free_flow_entry_index_(uint32_t idx);

    uint32_t fetch_flow_table_bits_(uint32_t hash_val);
    uint32_t fetch_hint_bits_(uint32_t hash_val);
    hal_ret_t alloc_fhct_index(uint32_t *index);
    hal_ret_t free_fhct_index(uint32_t index);
    void add_flow_entry_global_map(FlowEntry *fe, uint32_t index);

    // Getters & Setters
    bool get_delayed_del_en();
    uint32_t get_table_id() { return table_id_; }
    uint32_t get_oflow_table_id() { return oflow_table_id_; }
    uint32_t get_flow_data_len() { return data_len_; }
    uint32_t get_key_len() { return key_len_; }
    uint32_t get_hwkey_len() { return hwkey_len_; }
    uint32_t get_hwdata_len() { return hwdata_len_; }


    void set_delayed_del_en(bool en);

    uint32_t get_num_hints_per_flow_entry();
    indexer *get_flow_coll_indexer();
    void push_fe_delete_q(FlowEntry *fe);
    void push_fhg_delete_q(FlowHintGroup *fhg);

};

}   // namespace utils
}   // namespace pd
}   // namespace hal
#endif // __FLOW_HPP__
