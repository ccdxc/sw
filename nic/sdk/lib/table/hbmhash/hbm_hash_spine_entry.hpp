//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __HBM_HASH_SPINE_ENTRY_HPP__
#define __HBM_HASH_SPINE_ENTRY_HPP__

#include <list>
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "include/sdk/mem.hpp"

#include "hbm_hash_mem_types.hpp"

namespace sdk {
namespace table {

class HbmHash;
class HbmHashEntry;
class HbmHashHintGroup;
class HbmHashTableEntry;

typedef std::list<HbmHashHintGroup*> HbmHashHintGroupList;

class HbmHashSpineEntry {
private:
    HbmHashTableEntry *ht_entry_;      // Back Pointer to FT Entry
    HbmHashEntry *anchor_entry_; 		// Anchor entry if is_in_ht_:TRUE
    HbmHashHintGroupList  hg_list_;    // List of HGs
    bool is_in_ht_;                 // Spine entry is in FT
    uint32_t hct_index_;           // Valid only if is_in_ft is FALSE
    HbmHashSpineEntry *prev_, *next_;  // Linked List

    HbmHashSpineEntry(HbmHashTableEntry *ht_entry);
    ~HbmHashSpineEntry() {}
public:
    static HbmHashSpineEntry *factory(HbmHashTableEntry *ht_entry);
    static void destroy(HbmHashSpineEntry *fse);

    void form_action_data(void *action_data);
    sdk_ret_t program_table();
    sdk_ret_t deprogram_table();
    uint32_t get_num_hgs();
    void add_hg(HbmHashHintGroup *hg);
    void del_hg(HbmHashHintGroup *hg);
    HbmHashHintGroup *get_last_hg();
    HbmHashEntry *get_last_hbm_hash_entry();
    HbmHashEntry *get_prev_hbm_hash_entry(HbmHashEntry *fe);
    HbmHashEntry *get_next_hbm_hash_entry(HbmHashEntry *fe);

	void replace_hg(HbmHashHintGroup *hg, HbmHashHintGroup *new_hg);
    sdk_ret_t entry_trace(uint32_t table_id, uint32_t index,
                          void *key, void *data);
    void print_hse();
    void entry_to_str(char *buff, uint32_t buff_size);

    // Getters & Setters
    HbmHashEntry *get_anchor_entry() { return anchor_entry_; }
    HbmHashSpineEntry *get_prev() { return prev_; }
    HbmHashSpineEntry *get_next() { return next_; }
    bool get_is_in_ft() { return is_in_ht_; }
    uint32_t get_fhct_index() { return hct_index_; }
    HbmHashTableEntry *get_ht_entry() { return ht_entry_; }

    void set_anchor_entry(HbmHashEntry *fl_entry);
    void set_prev(HbmHashSpineEntry *prev);
    void set_next(HbmHashSpineEntry *next);
    void set_is_in_ft(bool is_in_ft);
    void set_fhct_index(uint32_t idx);
    void set_ht_entry(HbmHashTableEntry *ht_entry);

};

}   // namespace table
}   // namespace sdk
#endif // __HBM_HASH_HINT_GROUP_HPP__
