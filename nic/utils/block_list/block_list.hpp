// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

/*------------------------------------------------------------------------------
 * Block List Utility
 *  - Maintaines block of elements.
 *  - Reduces the overhead needed to store each element.
 *----------------------------------------------------------------------------*/

#ifndef __BLOCK_LIST_HPP__
#define __BLOCK_LIST_HPP__

#include "nic/include/base.h"
#include "sdk/list.hpp"
#include "sdk/shmmgr.hpp"
#include "nic/utils/iterator/iterator_tpl.h"

using sdk::lib::dllist_ctxt_t;

#define BLOCK_LIST_DEFAULT_ELEMS_PER_BLOCK        8

namespace hal {
namespace utils {

// Success: Returns value true
typedef bool (*block_list_cb_t)(void *elem, void *data);

typedef struct list_node_s list_node_t;
struct list_node_s {
    dllist_ctxt_t   ctxt_;
    uint8_t         num_in_use_;
    uint8_t         elems_[0];
} __PACK__;

class block_list {
public:
    static block_list *factory(uint32_t elem_size,
                               uint32_t elems_per_block = BLOCK_LIST_DEFAULT_ELEMS_PER_BLOCK,
                               shmmgr *mmgr = NULL);
    static void destroy(block_list *blist);

    /* Insert element in the list */
    hal_ret_t insert(void *elem);
    /* Remove element from the list */
    hal_ret_t remove(void *elem);
    /* Remove all elements from the list */
    hal_ret_t remove_all();
    /* Number of elements in the list */
    uint32_t num_elems();
    /* Check for the presence of element */
    bool is_present(void *elem);
    /* Iterate */
    hal_ret_t iterate(block_list_cb_t cb, void *data = NULL);

    // Operator Overloading
    block_list& operator+=(const block_list& rhs);

    class iterator_state {
        public:
        const dllist_ctxt_t *lcurr, *lnext;
        list_node_t   *node;
        int pos;
        inline void begin(const block_list* ref) { 
            lcurr = (&ref->list_head_)->next;
            lnext = lcurr ? lcurr -> next : NULL;
            if (lcurr != &ref->list_head_) {
                node = dllist_entry(lcurr, list_node_t, ctxt_);
            } else {
                node = NULL;
            }
            pos = 0;
        }
        inline void next(const block_list* ref) {
            if (pos < (node->num_in_use_ - 1)) {
                pos++;
            } else {
                lcurr = lnext;
                lnext = lcurr->next;
                if (lcurr != &ref->list_head_) {
                    node = dllist_entry(lcurr, list_node_t, ctxt_);
                } else {
                    node = NULL;
                }
                pos = 0;
            }
        }
        inline void* get(block_list *ref) { 
            return ref->element_location_(node, pos);
        }

        inline void end(const block_list* ref) {
            lcurr = (&ref->list_head_);
            lnext = lcurr ? lcurr -> next : NULL;
            pos = 0;
        }
        inline void erase(block_list* ref) {
            bool last_elem = false;
            ref->remove_elem_(node, pos, &last_elem);
            if (last_elem) {
                // Last elem: Make iterator point to end
                this->end(ref);
            }
        }

        // Return true if state != s
        inline bool cmp(const iterator_state& s) const {
            if (lcurr == s.lcurr && lnext == s.lnext && pos == s.pos) {
                return false;
            } else {
                return true;
            }
        }
    };
    SETUP_ITERATORS(block_list, void*, iterator_state);
    // SETUP_MUTABLE_ITERATOR(block_list, void*, iterator_state);

private:
    uint8_t         elems_per_block_;
    uint32_t        elem_size_;
    dllist_ctxt_t   list_head_;
    shmmgr          *mmgr_;

private:
    block_list() {};
    ~block_list();
    bool init(uint32_t elem_size, uint32_t elems_per_block, shmmgr *mmgr);
    list_node_t *get_last_node_();
    hal_ret_t find_(void *elem, list_node_t **elem_in_node, 
                    uint32_t *elem_id);
    hal_ret_t consolidate_(list_node_t *node, uint32_t elem_id,
                           list_node_t *last_node);
    void *element_location_(list_node_t *node, 
                            uint32_t elem_id);
    hal_ret_t remove_elem_(list_node_t *node, uint32_t elem_id, bool *last_elem);
};

}
}
#endif // __BLOCK_LIST_HPP__
