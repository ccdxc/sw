/* ============================================================================
 |
 | TcamEntry
 | 
 |   - Represents an entry in Tcam Table.
 |
 * ==========================================================================*/

#ifndef __TCAM_ENTRY_HPP__
#define __TCAM_ENTRY_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"

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
    int         refcnt_;        // tcam entry reference count

    TcamEntry (void *key, void *key_mask, uint32_t key_len, 
               void *data, uint32_t data_len, uint32_t index,
               priority_t priority = 0);
    ~TcamEntry();
public:
    static TcamEntry *factory(void *key, void *key_mask, uint32_t key_len,
                              void *data, uint32_t data_len, uint32_t index,
                              priority_t priority = 0, 
                              uint32_t mtrack_id = HAL_MEM_ALLOC_TCAM_ENTRY);
    static void destroy(TcamEntry *te, 
                        uint32_t mtrack_id = HAL_MEM_ALLOC_TCAM_ENTRY); 

    void update_key_data(void *key, void *key_mask, void *data);

    // Getters & Setters
    void *get_key() { return key_; };
    void *get_key_mask() { return key_mask_; }
    uint32_t get_key_len() { return key_len_; }
    void *get_data() { return data_; }
    uint32_t get_data_len() { return data_len_; }
    uint32_t get_index() { return index_; }
    void set_index(uint32_t index) { index_ = index; }
    priority_t get_priority() { return priority_; }
    int get_refcnt() { return refcnt_; }
    void set_refcnt(int refcnt) { refcnt_ = refcnt; }
    void incr_refcnt() { refcnt_++; }
    void decr_refcnt() { refcnt_--; }
};

}    // namespace utils
}    // namespace pd
}    // namespace hal

#endif // __TCAM_ENTRY_HPP__
