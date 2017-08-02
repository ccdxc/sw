#include <assert.h>
#include <string.h>
#include <ctrie.hpp>

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
ctrie::ctrie(uint16_t max_keylen, uint16_t slab_block_size, bool thread_safe)
{
    assert(!(max_keylen & 0x7));
    assert(thread_safe == false);
    thread_safe_ = thread_safe;
    max_keylen_ = max_keylen;
    node_slab_ = slab::factory("ctrie", sizeof(ctnode_t) + max_keylen_,
                               slab_block_size, false);
    assert(node_slab_ != NULL);
    root_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
ctrie::~ctrie()
{
    delete this->node_slab_;
}

//------------------------------------------------------------------------------
// create a new node and initialize it
//------------------------------------------------------------------------------
ctrie::ctnode_t *
ctrie::create_ctnode_(uint8_t *key, uint16_t keylen, void *ctxt)
{
    ctnode_t    *ctnode;

    assert (keylen <= this->max_keylen_);

    ctnode = static_cast<ctnode_t *>(this->node_slab_->alloc());
    assert(ctnode != NULL);
    ctnode->left = ctnode->right = ctnode->parent = NULL;
    ctnode->ctxt = ctxt;
    ctnode->keylen = keylen;
    memcpy(ctnode->key, key, keylen);

    return ctnode;
}

//------------------------------------------------------------------------------
// given a byte stream, check if a bit at specified position is set or not
// NOTE: bit numbering is starting with 0 at the MSB (i.e., left to right)
//------------------------------------------------------------------------------
bool
ctrie::is_bit_set_(uint8_t *bytes, uint16_t posn)
{
    uint16_t byte = posn >> 3;

    return (bytes[byte] & (1 << (8 - ((posn & 0x7) + 1))));
}

//------------------------------------------------------------------------------
// return true if keys are fully equal or else return false
//------------------------------------------------------------------------------
bool
ctrie::equal_keys_(uint8_t *key1, uint16_t klen1,
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
// return true if keys are equal until given length or else false
//------------------------------------------------------------------------------
bool
ctrie::equal_keys_(uint8_t *key1, uint8_t *key2, uint16_t klen)
{
    uint16_t        byte_num;
    uint8_t         last_byte_mask;

    byte_num = klen >> 3;
    if (!memcmp(key1, key2, byte_num)) {
        last_byte_mask = 0xFF << (8 - (klen & 0x7));
        if ((key1[byte_num] & last_byte_mask) ==
                (key2[byte_num] & last_byte_mask)) {
            // found a match for the key in hand
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
// split the tree at the given node and bit position, this function creates a
// pair of new nodes and makes one of them same as the node passed in (i.e., a
// clone) and the other one contains new node to be inserted, these two are made
// children of intermediate node (which is the node passed in)
//------------------------------------------------------------------------------
void
ctrie::split_tree_ (ctnode_t *node, uint16_t bposn,
                    uint8_t *key, uint16_t keylen, void *ctxt)
{
    ctnode_t    *cloned_ctnode, *new_ctnode;

    // allocate the new node that we need to insert
    new_ctnode = create_ctnode_(key, keylen, ctxt);

    // clone the current node (this becomes a sibling to the new node)
    cloned_ctnode = static_cast<ctnode_t *>(this->node_slab_->alloc());
    cloned_ctnode->left = node->left;
    cloned_ctnode->right = node->right;
    if (cloned_ctnode->left) {
        cloned_ctnode->left->parent = cloned_ctnode;
    }
    if (cloned_ctnode->right) {
        cloned_ctnode->right->parent = cloned_ctnode;
    }

    // elevate the current node one level up as we are creating one more
    // hierarchy in the trie and make it an intermediate node
    node->keylen = bposn - 1;
    node->ctxt = NULL;
    // TODO: zero-out bits after node->keylen in node->key now that the keylen
    // has changed

    // make the cloned node and new node children of the current node
    if (is_bit_set_(new_ctnode->key, bposn)) {
        node->right = new_ctnode;
        node->left = cloned_ctnode;
    } else {
        node->left = new_ctnode;
        node->right = cloned_ctnode;
    }
    cloned_ctnode->parent = new_ctnode->parent = node;
}

//------------------------------------------------------------------------------
// add a new node with given (key, keylen) to the trie. while trying to insert
// a new node, we might have to split/expand the trie at certain position in the
// tree. ctnode->ctxt is non-NULL for all user inserted (root or leaf or
// intermediate nodes) and all other nodes with ctnode->ctxt != NULL are
// intermediate nodes created while inserting other nodes into the tree
// NOTE: as ctxt is used to differentiate between user inserted nodes and
// internally-created ones, we fail if ctxt is NULL (in theory, we can maintain
// a bit inside the ctnode_t itself)
//------------------------------------------------------------------------------
ctrie::ctrie_ret_t
ctrie::ctrie_add(uint8_t *key, uint16_t keylen, void *ctxt)
{
    uint16_t               klen = keylen, nkeylen, bposn = 0;
    ctnode_t               *ctnode, *new_ctnode, **parent, *parent_node;
    bool                   diff;

    if (!key || (keylen > max_keylen_) || (ctxt == NULL)) {
        return CTRIE_RET_EINVAL;
    }

    // TODO: make sure that bits after keylen in the last byte are 0-ed

    parent = &this->root_;
    parent_node = NULL;
    ctnode = this->root_;
    while (ctnode) {
        nkeylen = ctnode->keylen - bposn;
        for (diff = false; klen && nkeylen; klen--, nkeylen--, bposn++) {
            // check if the bit at current position matches or not
            diff = is_bit_set_(key, bposn) ^ is_bit_set_(ctnode->key, bposn);
            if (diff) {
                break;
            }
        }

        if (diff) {
            // there is a mismatch in key bits of current node under examination
            // and key bits we are adding new node with, we have to split
            // current node and branch out to current children and new node at
            // bit position mismatch is seen
            split_tree_(ctnode, bposn, key, keylen, ctxt);
            break;
        }

        // if all bits of the current node and key are matched and both
        // the key and node's key have no more bits to match, current
        // node is the node where we want to insert, just populate the
        // context there
        if ((klen == 0) && (nkeylen == 0)) {
            // ctnode better be an intermediate node !!
            assert(ctnode->ctxt == NULL);
            ctnode->ctxt = ctxt;
            break;
        } else if (!klen && nkeylen) {
            // if new key length is smaller than trie depth and all key bits
            // matched, we need to create new node at the current position
            new_ctnode = create_ctnode_(key, keylen, ctxt);
            if (is_bit_set_(ctnode->key, bposn)) {
                new_ctnode->right = ctnode;
                new_ctnode->left = NULL;
            } else {
                new_ctnode->left = ctnode;
                new_ctnode->right = NULL;
            }
            new_ctnode->parent = ctnode->parent;
            ctnode->parent = new_ctnode;
            *parent = new_ctnode;
            break;
        }

        // so far all the bits in the key matched to that of the node we picked
        // to compare, pick the next node to branch to for comparison and repeat
        if (is_bit_set_(key, bposn)) {
            parent = &ctnode->right;
        } else {
            parent = &ctnode->left;
        }
        parent_node = ctnode;
        ctnode = *parent;
    }

    if (!ctnode) {
        // we reached the end of the tree, time to insert our new node
        ctnode = create_ctnode_(key, keylen, ctxt);
        ctnode->parent = parent_node;
        *parent = ctnode;
    }

    return CTRIE_RET_OK;
}

//------------------------------------------------------------------------------
// given a key, keylen delete the matching node, if any, and its
// parent nodes that don't have any other children and no ctxt (i.e.,
// non-intermediate nodes), return true everytime a node is deleted so
// parent's left or right can be NULL-ified
//------------------------------------------------------------------------------
bool
ctrie::ctrie_del_(ctnode_t *ctnode, uint8_t *key, uint16_t keylen, void **ctxt)
{
    // compare the node's key with the key in hand
    if (equal_keys_(ctnode->key, ctnode->keylen, key, keylen)) {
        // found a match for the key in hand
        *ctxt = ctnode->ctxt;
        ctnode->ctxt = NULL;
        if ((ctnode->left == NULL) && (ctnode->right == NULL)) {
            this->node_slab_->free(ctnode);
            return true;
        }
    }

    // check if we need to take right or left branch
    if (is_bit_set_(key, ctnode->keylen + 1)) {
        // take right branch
        if (ctnode->right) {
            if (ctrie_del_(ctnode->right, key, keylen, ctxt)) {
                ctnode->right = NULL;
                if ((ctnode->ctxt == NULL) && (ctnode->left == NULL)) {
                    // this node can be deleted
                    this->node_slab_->free(ctnode);
                    return true;
                }
            }
        }
        // key has more bits, but trie isn't that deep - no match found
        return false;
    } else {
        // take left branch
        if (ctnode->left) {
            if (ctrie_del_(ctnode->left, key, keylen, ctxt)) {
                ctnode->left = NULL;
                if ((ctnode->right == NULL) && (ctnode->ctxt == NULL)) {
                    // this node can be deleted
                    this->node_slab_->free(ctnode);
                    return true;
                }
            }
        }
        // key has more bits, but trie isn't that deep - no match found
        return false;
    }
}

//------------------------------------------------------------------------------
// given a (key, keylen) delete the corresponding node, if found, and compact
// the tree as much as possible
//------------------------------------------------------------------------------
void *
ctrie::ctrie_del(uint8_t *key, uint16_t keylen)
{
    void        *ctxt = NULL;

    if (this->root_ == NULL) {
        return NULL;
    }
    assert(key != NULL);
    assert(keylen <= this->max_keylen_);

    // recursively traverse the trie until a match is found & delete node(s)
    if (ctrie_del_(this->root_, key, keylen, &ctxt)) {
        // root node itself is deleted
        this->root_ = NULL;
    }

    return ctxt;
}

//------------------------------------------------------------------------------
// given a (key, keylen), find an user inserted node and return the ctxt from
// it. if an exact match is requested and match is found at non-user inserted
// node, NULL ctxt is returned (to indicate that match failed for user-inserted
// node). if exact match is not requested, an user inserted node that matched
// the longest number of key bits is returned (useful for lpm lookups)
//------------------------------------------------------------------------------
void *
ctrie::ctrie_lookup(uint8_t *key, uint16_t keylen, bool exact_match)
{
    ctnode_t     *lpm_match = NULL, *ctnode;

    if ((this->root_ == NULL) || (key == NULL) ||
        (keylen > max_keylen_)) {
        return NULL;
    }

    ctnode = this->root_;
    while (ctnode) {
        if (exact_match == true) {
            if (equal_keys_(ctnode->key, ctnode->keylen, key, keylen)) {
                return ctnode->ctxt;
            }
        } else {
            // check for partial match on a user-inserted node
            if ((keylen >= ctnode->keylen) && ctnode->ctxt) {
                if (equal_keys_(ctnode->key, key, ctnode->keylen)) {
                    // remember this lpm match
                    lpm_match = ctnode;
                }
            }
        }

        // branch and continue looking for next longest match
        if ((ctnode->keylen >= keylen) ||
            (ctnode->keylen == this->max_keylen_)) {
            // no more matches possible, if lpm match happened return that
            break;
        }

        if (is_bit_set_(key, ctnode->keylen + 1)) {
            ctnode = ctnode->right;
        } else {
            ctnode = ctnode->left;
        }
    }

    return (exact_match ? NULL : (lpm_match ? lpm_match->ctxt : NULL));
}

//------------------------------------------------------------------------------
// internal function to recursively walk the ctrie and invoke a callback for
// every node, until the callback indicates to stop the walk
//------------------------------------------------------------------------------
bool
ctrie::ctrie_walk_(ctnode_t *ctnode, boost::function<bool (uint8_t *key, uint16_t keylen, void *ctxt)> cb)
{
    bool    stop_walk;

    stop_walk = cb(ctnode->key, ctnode->keylen, ctnode->ctxt);
    if (stop_walk) {
        return stop_walk;
    }

    if (ctnode->left) {
        stop_walk = ctrie_walk_(ctnode->left, cb);
        if (stop_walk) {
            return true;
        }
    }
    
    if (ctnode->right) {
        stop_walk = ctrie_walk_(ctnode->right, cb);
    }

    return stop_walk;
}

//------------------------------------------------------------------------------
// walk the trie and invoke the provided callback and stop the walk if the
// callback returns true (indicating walk must be stopped)
//------------------------------------------------------------------------------
void
ctrie::ctrie_walk(boost::function<bool (uint8_t *key, uint16_t keylen, void *ctxt)> cb)
{
    if (this->root_ == NULL) {
        // empty tree
        return;
    }

    ctrie_walk_(this->root_, cb);
}
