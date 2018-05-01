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

// NAT_MAPPING_ORIGIN_XXX will tell us how this mapping got into the DB
#define NAT_MAPPING_ORIGIN_PKT        0
#define NAT_MAPPING_ORIGIN_CFG        1

// address mapping entry
typedef struct addr_entry_s addr_entry_t;
struct addr_entry_s {
    addr_entry_key_t    key;           // entry key
    vrf_id_t            tgt_vrf_id;    // target VRF
    ip_addr_t           tgt_ip_addr;   // mapped IP address
    nat_pool_id_t       nat_pool_id;   // nat pool this mapping belongs to
    addr_entry_t        *rentry;       // reverse address mapping, if any

    // operational state
    uint8_t             origin:1;      // 0 ==> fte path, 1 ==> cfg path
    uint8_t             bidir:1;       // TRUE if this is bidirectional mapping
    uint8_t             in_db:1;       // TRUE if this was inserted in the db
    uint32_t            ref_cnt;       // ref count for this mapping
    hal_handle_t        hal_handle;    // HAL allocated unique handle
    ht_ctxt_t           db_node;       // hash table context

    // stats
    uint32_t            num_tcp_sessions;
    uint32_t            num_udp_sessions;
    uint32_t            num_other_sessions;
} __PACK__;

hal_ret_t addr_db_init(uint32_t db_size);
hal_ret_t addr_entry_slab_init(void);
hal_ret_t addr_entry_add(addr_entry_key_t *key, vrf_id_t tgt_vrf_id, ip_addr_t tgt_ip_addr);
void addr_entry_del(addr_entry_key_t *key);
addr_entry_t *addr_entry_get(addr_entry_key_t *key);

addr_entry_t *addr_entry_alloc(void);
void addr_entry_free(addr_entry_t *entry);
hal_ret_t addr_entry_db_insert(addr_entry_t *entry);
addr_entry_t *addr_entry_db_remove(addr_entry_key_t *key);

} // namespace nat
} // namespace plugins
} // namespace hal

#endif    // __NAT_ADDR_DB_HPP__

