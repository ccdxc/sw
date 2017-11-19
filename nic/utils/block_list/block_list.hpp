// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

/*------------------------------------------------------------------------------
 * Block List Utility
 *  - Maintaines block of elements.
 *  - Reduces the overhead needed to store each element.
 *----------------------------------------------------------------------------*/
#ifndef __BLOCK_LIST_HPP__
#define __BLOCK_LIST_HPP__

#include "nic/include/base.h"
#include "nic/utils/list/list.hpp"

using hal::utils::dllist_ctxt_t;

namespace hal {
namespace utils {


typedef void (*block_list_cb_t)(void *elem);

typedef struct list_node_s list_node_t;
struct list_node_s {
    dllist_ctxt_t   ctxt_;
    uint8_t         num_in_use_;
    uint8_t         elems_[0];
} __PACK__;

class block_list {
public:
    static block_list *factory(uint32_t elem_size, 
                               uint32_t elems_per_block = 8);
    ~block_list();

    /* Insert element in the list */
    hal_ret_t insert(void *elem);
    /* Remove element from the list */
    hal_ret_t remove(void *elem);
    /* Iterate */
    hal_ret_t iterate(block_list_cb_t cb);

private:
    uint8_t         elems_per_block_;
    uint32_t        elem_size_;
    dllist_ctxt_t   list_head_;

private:
    block_list() {};
    int init(uint32_t elem_size, uint32_t elems_per_block); 
    list_node_t *get_last_node_();
    hal_ret_t find_(void *elem, list_node_t **elem_in_node, 
                    uint32_t *elem_id);
    hal_ret_t consolidate_(list_node_t *node, uint32_t elem_id,
                           list_node_t *last_node);
    uint8_t *element_location_(list_node_t *node, 
                               uint32_t elem_id);
};

}
}
#endif // __BLOCK_LIST_HPP__
