#ifndef __FLOW_SPINE_ENTRY_HPP__
#define __FLOW_SPINE_ENTRY_HPP__

#include <list>
#include <base.h>
#include <p4pd_api.hpp>
#include <p4pd.h>

namespace hal {
namespace pd {
namespace utils {

class Flow;
class FlowEntry;
class FlowHintGroup;
class FlowTableEntry;

typedef std::list<FlowHintGroup*> FlowHintGroupList;

typedef struct hg_root_ {
    uint16_t hash;
    uint16_t hint;
} hg_root_t;



/** ---------------------------------------------------------------------------
  *
  * class FlowSpineEntry
  *
  * - Identifies a spine entry.
  *
  * ---------------------------------------------------------------------------
*/
class FlowSpineEntry {

private:
    FlowTableEntry *ft_entry_;      // Back Pointer to FT Entry        
    FlowEntry *anchor_entry_; 		// Anchor entry if is_in_ft_:TRUE
    FlowHintGroupList  hg_list_;    // List of FHGs
    bool is_in_ft_;                 // Spine entry is in FT
    uint32_t fhct_index_;           // Valid only if is_in_ft is FALSE
    FlowSpineEntry *prev_, *next_;  // Linked List

public:
    FlowSpineEntry(FlowTableEntry *ft_entry);
    ~FlowSpineEntry() {}

    void form_action_data(flow_hash_actiondata *action_data);
    hal_ret_t program_table();
    hal_ret_t deprogram_table();
    uint32_t get_num_hgs();
    void add_fhg(FlowHintGroup *fhg);
    void del_fhg(FlowHintGroup *fhg);
    FlowHintGroup *get_last_fhg();
    FlowEntry *get_last_flow_entry();
    FlowEntry *get_prev_flow_entry(FlowEntry *fe);
    FlowEntry *get_next_flow_entry(FlowEntry *fe);
   
	void replace_fhg(FlowHintGroup *fhg, FlowHintGroup *new_fhg);

    // Getters & Setters
    FlowEntry *get_anchor_entry() { return anchor_entry_; }
    FlowSpineEntry *get_prev() { return prev_; }
    FlowSpineEntry *get_next() { return next_; }
    bool get_is_in_ft() { return is_in_ft_; }
    uint32_t get_fhct_index() { return fhct_index_; }
    FlowTableEntry *get_ft_entry() { return ft_entry_; }

    void set_anchor_entry(FlowEntry *fl_entry);
    void set_prev(FlowSpineEntry *prev);
    void set_next(FlowSpineEntry *next);
    void set_is_in_ft(bool is_in_ft);
    void set_fhct_index(uint32_t idx);
    void set_ft_entry(FlowTableEntry *ft_entry);
    
};

}   // namespace utils
}   // namespace pd
}   // namespace hal
#endif // __FLOW_HINT_GROUP_HPP__
