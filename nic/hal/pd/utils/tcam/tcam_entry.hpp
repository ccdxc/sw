/* ============================================================================
 |
 | TcamEntry
 | 
 |   - Represents an entry in Tcam Table.
 |
 * ==========================================================================*/

#ifndef __TCAM_ENTRY_HPP__
#define __TCAM_ENTRY_HPP__

#include <base.h>
#include <acl_tcam.hpp>

using hal::pd::utils::priority_t;

namespace hal {
namespace pd {
namespace utils {

/** ---------------------------------------------------------------------------
  * 
  * class TcamEntry
  *
  *     - Represents an entry in Tcam Table.
  *
  * ---------------------------------------------------------------------------
*/
class TcamEntry {

#define TCAM_ENTRY_INVALID_INDEX 0xffffffff

private:
    void        *key_;          // sw key
    void        *key_mask_;     // sw key mask
    uint32_t    key_len_;       // sw key len
    void        *data_;         // sw/hw data 
    uint32_t    data_len_;      // sw/hw data len
    uint32_t    index_;         // tcam index
    priority_t  priority_;      // tcam entry priority

public:
    TcamEntry (void *key, void *key_mask, uint32_t key_len, 
               void *data, uint32_t data_len, uint32_t index,
               priority_t priority = 0);
    ~TcamEntry();

    void update_data(void *data);

    // Getters & Setters
    void *get_key() { return key_; };
    void *get_key_mask() { return key_mask_; }
    uint32_t get_key_len() { return key_len_; }
    void *get_data() { return data_; }
    uint32_t get_data_len() { return data_len_; }
    uint32_t get_index() { return index_; }
    void set_index(uint32_t index) { index_ = index; }
    priority_t get_priority() { return priority_; }


};

}    // namespace utils
}    // namespace pd
}    // namespace hal

#endif // __TCAM_ENTRY_HPP__
