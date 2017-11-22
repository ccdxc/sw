#ifndef __FLOW_HINT_GROUP_HPP__
#define __FLOW_HINT_GROUP_HPP__

#include <list>
#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"
using namespace std;

namespace hal {
namespace pd {
namespace utils {

class FlowEntry;
class Flow;
class FlowSpineEntry;

class FlowHintGroup {

private:
    uint32_t hint_bits_;                    // hint bits
    FlowSpineEntry *fs_entry_;              // rooted on this spine entry

    FlowSpineEntry *delayed_del_fs_entry_;  // Placeholder for delayed del

    std::list<FlowEntry*> flow_entry_list_; // non-anchor flow entries.             
    std::list<FlowEntry*> anchor_list_;     // anchors go only here

    FlowHintGroup(uint32_t hint_bits, FlowSpineEntry *fs_entry);
    ~FlowHintGroup();
public:
    static FlowHintGroup *factory(uint32_t hint_bits, 
                                  FlowSpineEntry *fs_entry,
                                  uint32_t mtrack_id = HAL_MEM_ALLOC_FLOW_HINT_GROUP);
    static void destroy(FlowHintGroup *fhg, 
                        uint32_t mtrack_id = HAL_MEM_ALLOC_FLOW_HINT_GROUP);

    void add_flow_entry(FlowEntry *f_entry);
    void del_flow_entry(FlowEntry *f_entry);
    void add_anchor_flow_entry(FlowEntry *f_entry);
    void del_anchor_flow_entry(FlowEntry *f_entry);
    FlowEntry *get_first_flow_entry();
    FlowEntry *get_last_flow_entry();
    FlowEntry *get_prev_flow_entry(FlowEntry *fe);
    FlowEntry *get_next_flow_entry(FlowEntry *fe);

    uint32_t get_num_flow_entries();
    uint32_t get_num_anchor_flow_entries();
    bool check_flow_entry_exists(FlowEntry *fe);
    void print_fhg();

    // Getters & Setters
    uint32_t get_hint_bits();
    FlowSpineEntry *get_fs_entry();
    void set_fs_entry(FlowSpineEntry *fs_entry);

    FlowSpineEntry *get_delayed_del_fs_entry();
    void set_delayed_del_fs_entry(FlowSpineEntry *fs_entry);
    
};

}   // namespace utils
}   // namespace pd
}   // namespace hal
#endif // __FLOW_HINT_GROUP_HPP__
