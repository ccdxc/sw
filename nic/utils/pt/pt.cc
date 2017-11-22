#include "nic/utils/pt/pt.hpp"

namespace hal {
namespace utils {

//------------------------------------------------------------------------------
// init routine for the tree
//------------------------------------------------------------------------------
bool
pt::init(const char *name, uint16_t max_key_len, bool thread_safe)
{
    if (!name) {
        return NULL;
    }

    // max. key len must be a multiple of 8
    if (max_key_len & 0x7) {
        return NULL;
    }
    root_ = NULL;
    max_key_len_ = max_key_len;
    thread_safe_ = thread_safe;
    if (thread_safe) {
        HAL_ASSERT_RETURN(!HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE),
                          false);
    }

    // delay delete is not needed for the slab here because it is entirely used
    // for nodes internally created by this library only
    ptnode_slab_ = slab::factory("pt", HAL_SLAB_RSVD,
                                 sizeof(ptnode_t) + max_key_len, 16,
                                 thread_safe, true, false, true);
    if (ptnode_slab_ == NULL) {
        return false;
    }
    strncpy(name_, name, PT_NAME_MAX_LEN);

    num_entries_ = 0;
    num_internal_entries_ = 0;
    num_inserts_ = 0;
    num_insert_err_ = 0;
    num_deletes_ = 0;
    num_delete_err_ = 0;

    return true;
}

//------------------------------------------------------------------------------
// factory routine for the tree
//------------------------------------------------------------------------------
pt *
pt::factory(const char *name, uint16_t max_key_len, bool thread_safe)
{
    void    *mem;
    pt      *new_pt;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_LIB_SLAB, sizeof(pt));
    if (!mem) {
        return NULL;
    }
    new_pt = new (mem) pt();
    if (new_pt->init(name, max_key_len, thread_safe) == false) {
        new_pt->~pt();
        HAL_FREE(HAL_MEM_ALLOC_LIB_SLAB, new_pt);
        return NULL;
    }

    return new_pt;
}

//------------------------------------------------------------------------------
// destructor for patricia tree instance
//------------------------------------------------------------------------------
pt::~pt()
{
    if (ptnode_slab_) {
        slab::destroy(ptnode_slab_);
    }

    if (thread_safe_) {
        HAL_SPINLOCK_DESTROY(&slock_);
    }
}

void
pt::destroy(pt *ptree)
{
    if (!ptree) {
        return;
    }
    ptree->~pt();
    HAL_FREE(HAL_MEM_ALLOC_LIB_SLAB, ptree);
}

//------------------------------------------------------------------------------
// create a new node and initialize it
//------------------------------------------------------------------------------
ptnode_t *
pt::alloc_ptnode_(uint8_t *key, uint16_t key_len, void *entry, bool internal)
{
    ptnode_t    *ptnode;

    if (key_len > this->max_key_len_) {
        return NULL;
    }

    ptnode = static_cast<ptnode_t *>(this->ptnode_slab_->alloc());
    if (ptnode == NULL) {
        return NULL;
    }
    ptnode->left = ptnode->right = ptnode->parent = NULL;
    ptnode->entry = entry;
    ptnode->internal = internal ? TRUE : FALSE;
    ptnode->key_len = key_len;
    memcpy(ptnode->key, key, key_len);

    return ptnode;
}

//------------------------------------------------------------------------------
// given a byte stream, check if a bit at specified position is set or not
// NOTE: bit numbering is starting with 0 at the MSB (i.e., left to right)
//------------------------------------------------------------------------------
bool
pt::is_bit_set_(uint8_t *bytes, uint16_t posn)
{
    uint16_t byte = posn >> 3;

    return (bytes[byte] & (1 << (8 - ((posn & 0x7) + 1))));
}

//------------------------------------------------------------------------------
// given key bits, make sure that bits after the key_len bits are all zeroes in
// the last byte
// NOTE: bit numbering is starting with 0 at the MSB (i.e., left to right)
//------------------------------------------------------------------------------
void
pt::fixup_key_(uint8_t *key, uint16_t key_len)
{
    uint16_t    last_byte;

    //printf("\nkey_len %u, key[0] %u key[1] %u key[2] %u "
           //"key[3] %u key[4] %u key[5] %u\n",
           //key_len, key[0], key[1], key[2], key[3], key[4], key[5]);
    //last_byte = (key_len >> 3) + ((key_len & 0x7) ? 1 : 0);
    //last_byte = ((key_len + 7) & ~0x03) >> 3;

    last_byte = key_len >> 3;
    key[last_byte] &= ~(0xFF >> (key_len & 0x7));

    //printf("~(0xFF >> (key_len & 0x7)) 0x%x\n",
           //(~(0xFF >> (key_len & 0x7))) & 0xFF);

    //printf("fixed up key last_byte %u, key[0] %u key[1] %u key[2] %u "
           //"key[3] %u key[4] %u key[5] %u\n",
           //last_byte, key[0], key[1], key[2], key[3], key[4], key[5]);
}

//------------------------------------------------------------------------------
//  allocate a new node and clone the contents of given node
//------------------------------------------------------------------------------
ptnode_t *
pt::clone_ptnode_(ptnode_t *ptnode)
{
    ptnode_t    *cloned_node;

    cloned_node = static_cast<ptnode_t *>(ptnode_slab_->alloc());
    if (cloned_node == NULL) {
        return NULL;
    }
    cloned_node->left = ptnode->left;
    cloned_node->right = ptnode->right;
    cloned_node->parent = ptnode;
    cloned_node->entry = ptnode->entry;
    cloned_node->internal = ptnode->internal;
    cloned_node->key_len = ptnode->key_len;
    memcpy(cloned_node->key, ptnode->key, ptnode->key_len);
    if (cloned_node->left) {
        cloned_node->left->parent = cloned_node;
    }
    if (cloned_node->right) {
        cloned_node->right->parent = cloned_node;
    }

    return cloned_node;
}

//------------------------------------------------------------------------------
// split the tree at the given node and bit position, this function creates a
// pair of new nodes and makes one of them same as the node passed in (i.e., a
// clone) and the other one contains new node to be inserted, these two are made
// children of intermediate node (which is the node passed in)
//------------------------------------------------------------------------------
hal_ret_t
pt::split_trie_(ptnode_t *ptnode, uint16_t bposn,
                uint8_t *key, uint16_t key_len, void *entry)
{
    ptnode_t    *cloned_node, *new_node;

    //printf("splitting trie at posn %u\n", bposn);

    // allocate the new (user) node that we need to insert
    new_node = alloc_ptnode_(key, key_len, entry, false);

    // clone the current node (this becomes a sibling to the new node)
    cloned_node = clone_ptnode_(ptnode);
    if (cloned_node == NULL) {
        return HAL_RET_OOM;
    }

    // elevate the current node one level up as we are creating one more
    // hierarchy in the trie and make it an intermediate node
    // NOTE: key contents of ptnode don't need to be changed, only key len
    ptnode->key_len = bposn;
    ptnode->entry = NULL;
    ptnode->internal = TRUE;

    // zero-out bits in the last byte of ptnode->key after ptnode->key_len
    // bits now that the key_len is decremented by 1
    fixup_key_(ptnode->key, ptnode->key_len);

    // make the cloned node and new node children of the current node
    if (is_bit_set_(new_node->key, bposn)) {
        ptnode->left = cloned_node;
        ptnode->right = new_node;
    } else {
        ptnode->left = new_node;
        ptnode->right = cloned_node;
    }
    cloned_node->parent = new_node->parent = ptnode;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// add a new node with given (key, key_len) to the trie. while trying to insert
// a new node, we might have to split/expand the trie at certain position in the
// tree. ptnode_t->entry is non-NULL for all user inserted (root or leaf or
// intermediate nodes) nodes and all other nodes with ptnode_t->entry == NULL
// are intermediate nodes created while inserting other nodes into the tree
// NOTE: as ptnode_t->entry is used to differentiate between user inserted
//       nodes and internally-created ones, we fail if entry is NULL (in theory,
//       we can maintain a bit inside the ptnode_t itself)
// TODO: make it thread safe !!
//------------------------------------------------------------------------------
hal_ret_t
pt::insert(uint8_t *key, uint16_t key_len, void *entry)
{
    hal_ret_t    ret;
    uint16_t     klen = key_len, nkeylen, bposn = 0;
    ptnode_t     *ptnode, *new_ptnode, **parent, *parent_node;
    bool         diff;

    num_inserts_++;
    if (!key || (key_len > max_key_len_) || (entry == NULL)) {
        num_insert_err_++;
        return HAL_RET_INVALID_ARG;
    }

    // make sure that bits in key after key_len bits in the last byte are 0-ed
    fixup_key_(key, key_len);

    parent = &this->root_;
    parent_node = NULL;
    ptnode = this->root_;
    while (ptnode) {
        nkeylen = ptnode->key_len - bposn;
        //printf("nkeylen = %u, key_len = %u\n", nkeylen, key_len);
        for (diff = false; klen && nkeylen; klen--, nkeylen--, bposn++) {
            // check if the bit at current position matches or not
            diff = is_bit_set_(key, bposn) ^ is_bit_set_(ptnode->key, bposn);
            if (diff) {
                break;
            }
        }

        if (diff) {
            // there is a mismatch in key bits of current node under examination
            // and key bits we are adding new node with, we have to split
            // current node and branch out to current children and new node at
            // bit position mismatch is seen
            ret = split_trie_(ptnode, bposn, key, key_len, entry);
            if (ret != HAL_RET_OK) {
                // error out
                num_insert_err_++;
                return ret;
            }
            num_internal_entries_++;
            num_entries_ += 2;
            break;
        }

        // if all bits of the current node and key are matched and both
        // the key and node's key have no more bits to match, current
        // node is the node where we want to insert, just populate the
        // context there
        if ((klen == 0) && (nkeylen == 0)) {
            // ptnode better be an internal intermediate node, or else
            // we will overwrite another user inserted node
            if (ptnode->internal == FALSE) {
                num_insert_err_++;
                return HAL_RET_ENTRY_EXISTS;
            }
            num_internal_entries_--;
            ptnode->internal = FALSE;
            ptnode->entry = entry;
            break;
        } else if (!klen && nkeylen) {
            // if new key length is smaller than trie depth and all key bits
            // matched, we need to create new node at the current position
            new_ptnode = alloc_ptnode_(key, key_len, entry, FALSE);
            if (new_ptnode == NULL) {
                num_insert_err_++;
                return HAL_RET_OOM;
            }
            if (is_bit_set_(ptnode->key, bposn)) {
                new_ptnode->right = ptnode;
                new_ptnode->left = NULL;
            } else {
                new_ptnode->left = ptnode;
                new_ptnode->right = NULL;
            }
            new_ptnode->parent = ptnode->parent;
            ptnode->parent = new_ptnode;
            *parent = new_ptnode;
            num_entries_++;
            break;
        }

        // so far all the bits in the key matched to that of the node we picked
        // to compare, pick the next node to branch to for comparison and repeat
        if (is_bit_set_(key, bposn)) {
            parent = &ptnode->right;
        } else {
            parent = &ptnode->left;
        }
        parent_node = ptnode;
        ptnode = *parent;
    }

    if (!ptnode) {
        // we reached the end of the tree, time to insert our new node
        ptnode = alloc_ptnode_(key, key_len, entry, FALSE);
        ptnode->parent = parent_node;
        *parent = ptnode;
        num_entries_++;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// given a (key, key_len) delete the matching node, if any, and its
// parent nodes that don't have any other children and no 'entry' (i.e.,
// non-intermediate nodes), return true everytime a node is deleted so
// parent's left or right can be NULL-ified
//------------------------------------------------------------------------------
bool
pt::remove_(ptnode_t *ptnode, uint8_t *key, uint16_t key_len, void **entry)
{
    // compare the node's key with the key in hand
    if (equal_keys_(ptnode->key, ptnode->key_len, key, key_len)) {
        // found a match for the key in hand
        *entry = ptnode->entry;
        ptnode->entry = NULL;
        if ((ptnode->left == NULL) && (ptnode->right == NULL)) {
            if (ptnode->internal) {
                num_internal_entries_--;
            }
            num_entries_--;
            this->ptnode_slab_->free(ptnode);
            return true;
        }
        if (!ptnode->internal) {
            ptnode->internal = TRUE;
            num_internal_entries_++;
        }
        // no further search needed as the keys matched
        return false;
    }

    // check if we need to take right or left branch
    if (is_bit_set_(key, ptnode->key_len)) {
        // take right branch
        if (ptnode->right) {
            if (remove_(ptnode->right, key, key_len, entry)) {
                ptnode->right = NULL;
                if ((ptnode->entry == NULL) && (ptnode->left == NULL)) {
                    // this node can be deleted
                    if (ptnode->internal) {
                        num_internal_entries_--;
                    }
                    num_entries_--;
                    this->ptnode_slab_->free(ptnode);
                    return true;
                }
            }
        }
        // key has more bits, but trie isn't that deep - no match found
        return false;
    } else {
        // take left branch
        if (ptnode->left) {
            if (remove_(ptnode->left, key, key_len, entry)) {
                ptnode->left = NULL;
                if ((ptnode->right == NULL) && (ptnode->entry == NULL)) {
                    // this node can be deleted
                    if (ptnode->internal) {
                        num_internal_entries_--;
                    }
                    num_entries_--;
                    this->ptnode_slab_->free(ptnode);
                    return true;
                }
            }
        }
        // key has more bits, but trie isn't that deep - no match found
        return false;
    }
}

//------------------------------------------------------------------------------
// given a (key, key_len) delete the corresponding node, if found, and compact
// the tree as much as possible
//------------------------------------------------------------------------------
void *
pt::remove(uint8_t *key, uint16_t key_len)
{
    void    *entry = NULL;

    if (this->root_ == NULL || key == NULL) {
        return NULL;
    }

    if (key_len > this->max_key_len_) {
        return NULL;
    }

    // recursively traverse the trie until a match is found & delete node(s)
    if (remove_(this->root_, key, key_len, &entry)) {
        // root node itself is deleted
        this->root_ = NULL;
    }

    return entry;
}

//------------------------------------------------------------------------------
// return true if keys are fully equal or else return false
//------------------------------------------------------------------------------
bool
pt::equal_keys_(uint8_t *key1, uint16_t klen1,
                uint8_t *key2, uint16_t klen2)
{
    uint16_t        byte_num;
    uint8_t         last_byte_mask;

    if (klen1 == klen2) {
        // length matched, now compare the key contents
        byte_num = klen1 >> 3;
        if (!memcmp(key1, key2, byte_num)) {
            last_byte_mask = 0xFF << (8 - (klen1 & 0x7));
            if ((key1[byte_num] & last_byte_mask) ==
                    (key2[byte_num] & last_byte_mask)) {
                // found a match for the key in hand
                return true;
            }
        }
    }

    return false;
}

//------------------------------------------------------------------------------
// given a (key, key_len), find an user inserted node and return the entry from
// it, if an exact match is requested and match is found at non-user inserted
// node, NULL is returned (to indicate that match failed for user-inserted
// node) and if exact match is not requested, an user inserted node that matched
// the longest number of key bits is returned (useful for lpm lookups)
//------------------------------------------------------------------------------
void *
pt::lookup(uint8_t *key, uint16_t key_len, bool exact_match)
{
    ptnode_t    *lpm_match = NULL, *ptnode;

    if ((this->root_ == NULL) || (key == NULL) ||
        (key_len > max_key_len_)) {
        return NULL;
    }

    ptnode = this->root_;
    while (ptnode) {
        if (exact_match == true) {
            if (equal_keys_(ptnode->key, ptnode->key_len, key, key_len)) {
                // exact match found
                return ptnode->internal ? ptnode->entry : NULL;
            }
        } else {
            // check for partial match on a user-inserted node
            if ((key_len >= ptnode->key_len) && ptnode->entry) {
                if (equal_keys_(ptnode->key, ptnode->key_len,
                                key, ptnode->key_len)) {
                    // remember this lpm match
                    lpm_match = ptnode;
                }
            }
        }

        // branch and continue looking for next longest match
        if ((ptnode->key_len >= key_len) ||
            (ptnode->key_len == this->max_key_len_)) {
            // no more matches possible, if lpm match happened return that
            break;
        }

        if (is_bit_set_(key, ptnode->key_len + 1)) {
            ptnode = ptnode->right;
        } else {
            ptnode = ptnode->left;
        }
    }

    return (exact_match ? NULL : (lpm_match ? lpm_match->entry : NULL));
}

//------------------------------------------------------------------------------
// recursively walk the trie in DFS fashion and call the callback for each node
//------------------------------------------------------------------------------
void
iterate_(ptnode_t *ptnode, pt_iterate_cb_t cb, void *arg)
{
    cb(ptnode->key, ptnode->key_len, ptnode->entry, arg);
    if (ptnode->left) {
        iterate_(ptnode->left, cb, arg);
    }

    if (ptnode->right) {
        iterate_(ptnode->right, cb, arg);
    }
}

//------------------------------------------------------------------------------
// walk the trie and invoke the provided callback
//------------------------------------------------------------------------------
void
pt::iterate(pt_iterate_cb_t cb, void *arg)
{
    if (this->root_ == NULL) {
        return;
    }

    iterate_(this->root_, cb, arg);
}

}    // namespace utils
}    // namespace hal
