// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __NAT_ADDR_DB_HPP__
#define __NAT_ADDR_DB_HPP__

#include "nic/include/base.h"
#include "nic/include/ip.h"
#include "sdk/ht.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace utils {
namespace nat {

// key for address map entry
typedef struct addr_entry_key_s {
    vrf_id_t     vrf_id;               // VRF where the IP belongs
    ip_addr_t    ip_addr;              // original IP address
} __PACK__ addr_entry_key_t;

// address mapping entry
typedef struct addr_entry_s {
    addr_entry_key_t    key;           // Entry key
    vrf_id_t            tgt_vrf_id;    // target VRF
    ip_addr_t           tgt_ip_addr;   // mapped IP address
    nat_pool_id_t       nat_pool;      // nat pool this mapping belongs to
    uint32_t            origin:1;      // 0 ==> fte path, 1 ==> cfg path
    uint32_t            ref_cnt:31;    // ref count for this mapping
    hal_handle_t        hal_handle;    // HAL allocated unique handle
    ht_ctxt_t           db_node;       // hash table context
} __PACK__ addr_entry_t;

addr_entry_t *addr_entry_alloc(void);
hal_ret_t addr_db_init(uint32_t db_size);
hal_ret_t addr_entry_add(addr_entry_key_t *key, ip_addr_t tgt_ip_addr);
void addr_entry_del(addr_entry_key_t *key);
addr_entry_t *addr_entry_get(addr_entry_key_t *key);

} // namespace nat
} // namespace plugins
} // namespace hal

#endif    // __NAT_ADDR_DB_HPP__

