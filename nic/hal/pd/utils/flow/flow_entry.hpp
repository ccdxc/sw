#ifndef __FLOW_ENTRY_HPP__
#define __FLOW_ENTRY_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace pd {
namespace utils {

class Flow;
class FlowHintGroup;
class FlowSpineEntry;

/** ---------------------------------------------------------------------------
  *
  * class FlowEntry
  *
  *     - A Flow entry.
  *
  * ---------------------------------------------------------------------------
*/
class FlowEntry {

public:
    enum Status {
        SUCCESS,
        FAILURE
    };
private:
    void                *key_;              // sw key
    uint32_t            key_len_;           // sw key len
    void                *data_;             // sw data
    uint32_t            data_len_;          // sw data len
    uint32_t            hwkey_len_;         // hw key len
    uint32_t            hash_val_;          // hash value from key
    FlowHintGroup       *fh_group_;         // flow hint group
    uint32_t            gl_index_;          // global index

    // Location Information:
    bool                is_anchor_entry_;   // is anchor
    FlowSpineEntry      *spine_entry_;      // back-ptr to spine if its anchor
    uint32_t            fhct_index_;        // non-anchor's flow coll. tbl idx

    FlowEntry (void *key, uint32_t key_len, void *data, uint32_t data_len,
               uint32_t hwkey_len, bool log);
    ~FlowEntry ();

public:
    static FlowEntry *factory(void *key, uint32_t key_len, void *data,
                              uint32_t data_len, uint32_t hwkey_len, bool log,
                              uint32_t mtrack_id = HAL_MEM_ALLOC_FLOW_ENTRY);
    static void destroy(FlowEntry *fe,
                        uint32_t mtrack_id = HAL_MEM_ALLOC_FLOW_ENTRY);

    hal_ret_t insert(FlowHintGroup *fhg, FlowSpineEntry *fse);
    hal_ret_t update(void *data);
    hal_ret_t remove();

    hal_ret_t alloc_fhct_index(FlowSpineEntry *fse, uint32_t *fhct_index);
    hal_ret_t free_fhct_index(FlowSpineEntry *fse, uint32_t fhct_index);
    FlowEntry* create_new_flow_entry(FlowEntry *fe);
    FlowTableEntry *get_flow_table_entry();
    FlowSpineEntry *get_eff_spine_entry();
    hal_ret_t program_table_non_anchor_entry(FlowEntry *next_fe);
    hal_ret_t deprogram_table_non_anchor_entry();
    hal_ret_t form_hw_key(uint32_t table_id, void *hwkey);
    hal_ret_t entry_trace(uint32_t table_id, uint32_t index,
                          void *data);


    // Getters & Setters
    void *get_key() { return key_; }
    uint32_t get_key_len() { return key_len_; }
    void *get_data() { return data_; }
    uint32_t get_data_len() { return data_len_; }
    uint32_t get_hash_val() { return hash_val_; }
    FlowHintGroup *get_fh_group() { return fh_group_; }
    uint32_t get_global_index() { return gl_index_; }
    bool get_is_anchor_entry() { return is_anchor_entry_; }
    FlowSpineEntry *get_spine_entry() { return spine_entry_; }
    uint32_t get_fhct_index() { return fhct_index_; }
    hal_ret_t form_action_data(FlowEntry *next_fe, void *swdata);
    void print_fe();
    void entry_to_str(char *buff, uint32_t buff_size);

    void set_key(void *key);
    void set_key_len(uint32_t key_len);
    void set_data(void *data);
    void set_data_len(uint32_t data_len);
    void set_hash_val(uint32_t hash_val);
    void set_fh_group(FlowHintGroup *fh_group);
    void set_global_index(uint32_t index);
    void set_is_anchor_entry(bool is_anchor);
    void set_spine_entry(FlowSpineEntry *spe);
    void set_fhct_index(uint32_t fhct_index);

};

}   // namespace utils
}   // namespace pd
}   // namespace hal
#endif // __FLOW_ENTRY_HPP__
