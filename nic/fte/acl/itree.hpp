#pragma once

#include "nic/include/base.h"
#include "sdk/slab.hpp"
#include "nic/fte/acl/ref.hpp"
#include "nic/fte/acl/list.hpp"

namespace acl {

using sdk::lib::slab;

#define MAX(a, b) ((a) > (b) ? (a) : (b))

class itree_t {
public:
    // callback for matching entries during insert/delete/walk
    typedef acl::list_t::cb_t cb_t;

    static itree_t *create();
    const itree_t *clone() const {ref_inc(&this->ref_count_); return this;}
    void deref() const {ref_dec(&this->ref_count_);}
    itree_t *copy() const;

    uint32_t cost(uint32_t low, uint32_t high,
                  const ref_t *entry, const void *arg, cb_t match) const;
    uint32_t walk( uint32_t low, uint32_t high, const void *arg, cb_t cb) const;

    // itree update methods, updates the tree pointer
    static hal_ret_t insert(const itree_t **treep, uint32_t low, uint32_t high,
                            const ref_t *entry, const void *arg, cb_t match);
    static hal_ret_t remove(const itree_t **treep, uint32_t low, uint32_t high,
                            const void *arg, cb_t match);

    // debug counters
    static uint32_t num_trees_in_use() { return itree_slab_->num_in_use();}
    static uint32_t num_nodes_in_use() { return itree_node_slab_->num_in_use();}

    int check() const;
    void dump() const;
private:
    struct node_t {
        ref_t    ref_count;
        uint8_t  color    :2;
        uint8_t  has_list :1; // Flag to indicate if this node has multiple entries
                              // stored in a list
        uint32_t low, high, max;
        union {
            const ref_t  *entry;
            const list_t *list;
        };
        const node_t *left, *right;

        node_t(uint8_t c): ref_count{}, color(c), has_list(0), low(0),
                            high(0), max(0), entry(NULL), left(this), right(this){}

        const node_t *clone() const {ref_inc(&this->ref_count); return this;}
        void deref() const {ref_dec(&this->ref_count);}

        const node_t *update(uint8_t c, const itree_t::node_t* l,
                             const itree_t::node_t *r) const {
            uint32_t m = MAX(high, MAX(l->max, r->max));

            if (color == c && left == l && right == r && max == m) {
                return this;
            }

            node_t *copy = this->copy();
            copy->color = c;
            copy->left = l;
            copy->right = r;
            copy->max = m;
            
            return copy;
        }

        node_t *copy() const {
            if (!ref_is_shared(&ref_count)) {
                return  (itree_t::node_t *)this;
            }
    
            itree_t::node_t *copy = node_alloc();

            copy->color = color;
            copy->has_list = has_list;
            if (has_list) {
                copy->list = list->clone();
            } else {
                copy->entry = ref_clone(entry); 
            }
            copy->low = low;
            copy->high = high;
            copy->max = max;
    
            copy->left = left->clone();
            copy->right = right->clone();
    
            this->deref();
    
            return copy;

        }
    };

    class node_ref_t {
    public:
        node_ref_t(const node_t *node) :
            shared_(false), left_(NULL), right_(NULL), entry_(NULL), list_(NULL){
            if (ref_is_shared(&node->ref_count)) {
                shared_ = true;
                left_ = node->left->clone();
                right_ = node->right->clone();
                if (node->has_list) {
                    list_ = node->list->clone();
                } else {
                    entry_ = ref_clone(node->entry);
                }
            }
        }
        ~node_ref_t() {
            if (shared_) {
                left_->deref();
                right_->deref();
                ref_dec(entry_);
                if (list_)
                    list_->deref();
            }
        }
    private:
        bool shared_;
        const node_t *left_, *right_;
        const ref_t *entry_;
        const list_t *list_;
    };


    static const node_t empty_; // empty sentinal node
    static const node_t eempty_; // double black empty node

    static slab *itree_slab_;       // slab for itree
    static slab *itree_node_slab_;  // slab for tree nodes

    ref_t        ref_count_;        // ref count
    const node_t *root_;            // tree root;
    uint32_t     num_entries_;      // num entries

    static node_t* node_alloc();
    static node_t* node_create(uint32_t low, uint32_t high, const ref_t *entry);
    static node_t* node_copy(const node_t *node);

    static const node_t *balance(uint8_t color, const node_t* left,
                                 const node_t *node, const node_t *right);
    static const node_t *rotate(uint8_t color, const node_t* left,
                                 const node_t *node, const node_t *right);
    static const node_t *insert(const node_t *node, uint32_t low, uint32_t high,
                                const ref_t *entry, const void *arg, cb_t match);
    static const node_t *min_del(const node_t *node);
    static const node_t *del_entry(const node_t *node, const void *arg, cb_t match);
    static const node_t *del(const node_t *node, uint32_t low, uint32_t high,
                             const void *arg, cb_t match);
    static uint32_t walk(const node_t *node, uint32_t low, uint32_t high,
                         const void* arg, cb_t cb);
};
} // namespace acl
