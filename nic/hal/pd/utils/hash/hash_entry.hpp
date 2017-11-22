#ifndef __HASH_ENTRY_HPP__
#define __HASH_ENTRY_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace pd {
namespace utils {


/** ---------------------------------------------------------------------------
  * 
  * class HashEntry
  *
  *     - Entry in Hash Table
  *
  * ---------------------------------------------------------------------------
*/
class HashEntry {

private:
    void        *key_;          // sw key
    uint32_t    key_len_;       // sw key len
    void        *data_;         // sw/hw data
    uint32_t    data_len_;      // sw/hw data len
    uint32_t    index_;         // hash index

    HashEntry (void *key, uint32_t key_len, void *data, uint32_t data_len,
               uint32_t index);
    ~HashEntry();
public:
    static HashEntry *factory(void *key, uint32_t key_len, void *data, 
                              uint32_t data_len, uint32_t index,
                              uint32_t mtrack_id = HAL_MEM_ALLOC_HASH_ENTRY);
    static void destroy(HashEntry *he, 
                        uint32_t mtrack_id = HAL_MEM_ALLOC_HASH_ENTRY);

    void update_data(void *data);

    // Getters & Setters
    void *get_key() { return key_; }
    uint32_t get_key_len() { return key_len_; }
    void *get_data() { return data_; }
    uint32_t get_data_len() { return data_len_; }
    uint32_t get_index() { return index_; }

};

}    // namespace utils
}    // namespace pd
}    // namespace hal

#endif // __HASH_ENTRY_HPP__
