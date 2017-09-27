/* ============================================================================
 |
 | FlowTableEntry
 | --------------
 | 
 | - Represents an entry in flow hash table.
 | - Identifies an entry whose first set of hash bits are same. 
 |
 * ============================================================================
 */
#ifndef __FLOW_TABLE_ENTRY_HPP__
#define __FLOW_TABLE_ENTRY_HPP__

#include <map>
#include "nic/include/base.h"

namespace hal {
namespace pd {
namespace utils {

class Flow;
class FlowEntry;
class FlowSpineEntry;
class FlowHintGroup;

typedef std::map<uint32_t, FlowHintGroup*> FHGMap;
class FlowTableEntry {

private:
   
    uint32_t ft_bits_;
    Flow *flow_;

    FHGMap  hint_groups_map_; // Hint (11) -> Group
    FlowSpineEntry *spine_entry_;
    uint32_t num_spine_entries_;

public:
    FlowTableEntry(uint32_t ft_bits, Flow *flow);
    ~FlowTableEntry();

    hal_ret_t insert(FlowEntry *f_entry);
    hal_ret_t remove(FlowEntry *f_entry);
    
    uint32_t get_num_flow_hgs();
    FlowSpineEntry *get_spine_entry_for_new_hg(bool *is_new);
    FlowSpineEntry *get_spine_entry_for_hg(FlowHintGroup *fhg, bool *is_new);
    FlowSpineEntry *get_last_spine_entry();
    FlowEntry *get_last_flow_entry();
    void add_fhg(uint32_t hint_bits, FlowHintGroup *fhg);
    void remove_fhg(FlowHintGroup *fhg);
    void print_fte();
    // int find_pos_of_hg(FlowHintGroup *hg);
    // void form_ft_entry_from_hgs(uint32_t begin, uint32_t end, void *ft_entry);
    


    // Getters & Setters
    uint32_t get_ft_bits();
    Flow *get_flow();

    FlowSpineEntry *get_spine_entry();
    void set_spine_entry(FlowSpineEntry *sp_entry);
    void dec_num_spine_entries() { num_spine_entries_--; }
    void inc_num_spine_entries() { num_spine_entries_++; }
};

}   // namespace utils
}   // namespace pd
}   // namespace hal
#endif // __FLOW_TABLE_ENTRY_HPP__
