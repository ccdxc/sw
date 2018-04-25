//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// NAT pool address and/or port management related API interface
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/utils/nat/addr_db.hpp"

#ifndef __NAT_POOL_HPP__
#define __NAT_POOL_HPP__

using hal::utils::bitmap;
using hal::utils::nat::addr_entry_key_t;

namespace hal {

// NAT pool key
typedef struct nat_pool_key_s {
    vrf_id_t         vrf_id;
    nat_pool_id_t    pool_id;
} __PACK__ nat_pool_key_t;
#define HAL_MAX_NAT_POOLS        1024

// NAT pool
typedef struct nat_pool_s {
    hal_spinlock_t    slock;                // lock to protect this structure
    nat_pool_key_t    key;                  // key for the nat pool
    dllist_ctxt_t     addr_ranges;          // NAT address ranges

    // operational state of nat pool
    hal_handle_t      hal_handle;          // HAL allocated handle
    uint32_t          num_addrs;           // TODO: may not need
    uint32_t          num_free;            // no. of NAT addresses available
    bitmap            *addr_bmap;          // bitmap to keep track of free/in-use addresses

    // stats
    uint32_t           num_in_use;
} __PACK__ nat_pool_t;

void *nat_pool_get_key_func(void *entry);
uint32_t nat_pool_compute_hash_func(void *key, uint32_t ht_size);
bool nat_pool_compare_key_func(void *key1, void *key2);

hal_ret_t nat_pool_address_alloc(addr_entry_key_t *key, nat_pool_t *pool,
                                 addr_entry_key_t *mapping);
hal_ret_t nat_pool_address_free(addr_entry_key_t *key, nat_pool_t *pool);

}    // namespace hal

#endif    // __NAT_POOL_HPP__

