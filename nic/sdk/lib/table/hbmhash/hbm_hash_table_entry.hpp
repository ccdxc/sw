//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
/* ============================================================================
 |
 | HbmHashTableEntry
 | --------------
 |
 | - Represents an entry in HBM hash table.
 | - Identifies an entry whose first set of hash bits are same.
 |
 * ============================================================================
 */
#ifndef __HBM_HASH_TABLE_ENTRY_HPP__
#define __HBM_HASH_TABLE_ENTRY_HPP__

#include <map>
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"

#include "hbm_hash_mem_types.hpp"

namespace sdk {
namespace table {

class HbmHash;
class HbmHashEntry;
class HbmHashSpineEntry;
class HbmHashHintGroup;

typedef std::map<uint32_t, HbmHashHintGroup*> HGMap;
class HbmHashTableEntry {

private:

    uint32_t bucket_index_;
    HbmHash *hbm_hash_;

    HGMap  hint_groups_map_; // Hint (11) -> Group
    HbmHashSpineEntry *spine_entry_;
    uint32_t num_spine_entries_;

    HbmHashTableEntry(uint32_t bucket_index, HbmHash *hbm_hash);
    ~HbmHashTableEntry();
public:
    static HbmHashTableEntry *factory(uint32_t bucket_index,
                                      HbmHash *hbm_hash,
                                      uint32_t mtrack_id = SDK_MEM_ALLOC_HBM_HASH_TABLE_ENTRY);
    static void destroy(HbmHashTableEntry *fte,
                        uint32_t mtrack_id = SDK_MEM_ALLOC_HBM_HASH_TABLE_ENTRY);

    sdk_ret_t insert(HbmHashEntry *h_entry);
    sdk_ret_t remove(HbmHashEntry *h_entry);

    uint32_t get_num_hbm_hash_hgs();
    // HbmHashSpineEntry *get_spine_entry_for_new_hg(bool *is_new,
    sdk_ret_t get_spine_entry_for_new_hg(bool *is_new,
                                         HbmHashEntry *h_entry,
                                         HbmHashSpineEntry **spine_entry);
    sdk_ret_t get_spine_entry_for_hg(HbmHashHintGroup *hg,
                                     HbmHashEntry *h_entry,
                                     bool *is_new,
                                     HbmHashSpineEntry **spine_entry);
    HbmHashSpineEntry *get_last_spine_entry();
    HbmHashEntry *get_last_hbm_hash_entry();
    void add_hg(uint32_t hint_bits, HbmHashHintGroup *hg);
    void remove_hg(HbmHashHintGroup *hg);
    void print_hbm_hash_table_entries();
    void entry_to_str(char *buff, uint32_t buff_size);
    void inter_spine_str(HbmHashSpineEntry *eff_spine,
                         char *buff, uint32_t buff_size, uint32_t *num_recircs);
    // int find_pos_of_hg(HbmHashHintGroup *hg);
    // void form_ft_entry_from_hgs(uint32_t begin, uint32_t end, void *ft_entry);



    // Getters & Setters
    uint32_t get_bucket_index();
    HbmHash *get_hbm_hash();

    HbmHashSpineEntry *get_spine_entry();
    void set_spine_entry(HbmHashSpineEntry *sp_entry);
    void dec_num_spine_entries() { num_spine_entries_--; }
    void inc_num_spine_entries() { num_spine_entries_++; }
};

}   // namespace table
}   // namespace sdk
#endif // __HBM_HASH_TABLE_ENTRY_HPP__
