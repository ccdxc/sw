//------------------------------------------------------------------------------
// compressed trie implementation
//------------------------------------------------------------------------------
#ifndef __PT_HPP__
#define __PT_HPP__

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/utils/slab/slab.hpp"
#include "nic/include/hal_lock.hpp"

namespace hal {
namespace utils {

#define PT_NAME_MAX_LEN        16

typedef struct ptnode_s ptnode_t;
typedef struct ptnode_s {
    ptnode_t     *left;         // left child
    ptnode_t     *right;        // right child
    ptnode_t     *parent;       // parent node
    void         *entry;        // user entry (TBD: needed ?)
    uint8_t      internal:1;    // TRUE if node is created internally
    uint16_t     key_len;       // key length in bits
    uint8_t      key[0];        // pointer to the key bits
} __PACK__ ptnode_t;

typedef void (*pt_iterate_cb_t)(uint8_t *key, uint16_t key_len,
                                void *entry, void *arg);

// NOTE: key is always in network byte order for this library
class pt {
public:
    // max_key_len is the maximum possible key length of this tree
    static pt *factory(const char *name, uint16_t max_key_len, bool thread_safe);
    ~pt();
    // key length should be in number of bits (not bytes)
    hal_ret_t insert(uint8_t *key, uint16_t key_len, void *entry);
    void *remove(uint8_t *key, uint16_t key_len);
    void *lookup(uint8_t *key, uint16_t key_len, bool exact_match=false);
    void iterate(pt_iterate_cb_t iterate_cb, void *arg);

    uint32_t num_entries(void) const { return num_entries_; }
    uint32_t num_internal_entries(void) const { return num_internal_entries_; }
    uint32_t num_inserts(void) const { return num_inserts_; }
    uint32_t num_insert_err(void) const { return num_insert_err_; }
    uint32_t num_deletes(void) const { return num_deletes_; }
    uint32_t num_delete_err(void) const { return num_delete_err_; }

private:
    char              name_[PT_NAME_MAX_LEN];    // name of the tree
    ptnode_t          *root_;                    // root of the patricia tree
    uint16_t          max_key_len_;              // in bits (multiple of 8)
    bool              thread_safe_;              // TRUE for thread safety
    hal_spinlock_t    slock_;                    // lock for thread safety
    slab              *ptnode_slab_;             // slab for internal ptnodes

    uint32_t          num_entries_;              // total no. of entries
    uint32_t          num_internal_entries_;     // no. of internal entries
    uint32_t          num_inserts_;              // no. of insert operations
    uint32_t          num_insert_err_;           // no. of insert errors
    uint32_t          num_deletes_;              // no. of delete operations
    uint32_t          num_delete_err_;           // no. of delete errors

private:
    pt() {}
    bool init(const char *name, uint16_t max_key_len, bool thread_safe);
    ptnode_t *alloc_ptnode_(uint8_t *key, uint16_t key_len,
                             void *entry, bool internal);
    ptnode_t *clone_ptnode_(ptnode_t *ptnode);
    bool remove_(ptnode_t *ptnode, uint8_t *key,
                    uint16_t key_len, void **entry);
    bool is_bit_set_(uint8_t *bytes, uint16_t posn);
    hal_ret_t split_trie_(ptnode_t *ptnode, uint16_t bposn,
                          uint8_t *key, uint16_t key_len, void *entry);
    bool equal_keys_(uint8_t *key1, uint16_t klen1,
                     uint8_t *key2, uint16_t klen2);
    void fixup_key_(uint8_t *key, uint16_t key_len);
};

}    // namespace utils
}    // namespace hal

#endif    // __PT_HPP__
