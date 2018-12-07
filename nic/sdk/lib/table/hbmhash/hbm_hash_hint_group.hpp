//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __HBM_HASH_HINT_GROUP_HPP__
#define __HBM_HASH_HINT_GROUP_HPP__

#include <list>
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"

#include "hbm_hash_mem_types.hpp"

using namespace std;

namespace sdk {
namespace table {

class HbmHashEntry;
class HbmHash;
class HbmHashSpineEntry;

class HbmHashHintGroup {

private:
    uint32_t hint_bits_;                    // hint bits
    HbmHashSpineEntry *fs_entry_;              // rooted on this spine entry

    HbmHashSpineEntry *delayed_del_fs_entry_;  // Placeholder for delayed del

    std::list<HbmHashEntry*> hbm_hash_entry_list_; // non-anchor hbm_hash entries.
    std::list<HbmHashEntry*> anchor_list_;     // anchors go only here

    HbmHashHintGroup(uint32_t hint_bits, HbmHashSpineEntry *fs_entry);
    ~HbmHashHintGroup();
public:
    static HbmHashHintGroup *factory(uint32_t hint_bits,
                                  HbmHashSpineEntry *fs_entry);
    static void destroy(HbmHashHintGroup *hg);

    void add_hbm_hash_entry(HbmHashEntry *h_entry);
    void del_hbm_hash_entry(HbmHashEntry *h_entry);
    void add_anchor_hbm_hash_entry(HbmHashEntry *h_entry);
    void del_anchor_hbm_hash_entry(HbmHashEntry *h_entry);
    HbmHashEntry *get_first_hbm_hash_entry();
    HbmHashEntry *get_last_hbm_hash_entry();
    HbmHashEntry *get_prev_hbm_hash_entry(HbmHashEntry *fe);
    HbmHashEntry *get_next_hbm_hash_entry(HbmHashEntry *fe);

    uint32_t get_num_hbm_hash_entries();
    uint32_t get_num_anchor_hbm_hash_entries();
    bool check_hbm_hash_entry_exists(HbmHashEntry *fe);
    void print_hg();
    void inter_hg_str(HbmHashEntry *h_entry,
                      char *inter_hg_buff, uint32_t inter_hg_size,
                      char *entry_buff, uint32_t entry_size,
                      uint32_t *num_recircs);

    // Getters & Setters
    uint32_t get_hint_bits();
    HbmHashSpineEntry *get_fs_entry();
    void set_fs_entry(HbmHashSpineEntry *fs_entry);

    HbmHashSpineEntry *get_delayed_del_fs_entry();
    void set_delayed_del_fs_entry(HbmHashSpineEntry *fs_entry);

};

}   // namespace table
}   // namespace sdk
#endif // __HBM_HASH_HINT_GROUP_HPP__
