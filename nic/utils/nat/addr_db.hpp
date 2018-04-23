// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

namespace hal {
namespace utils {
namespace nat {

#ifndef __NAT_ADDR_DB_HPP__
#define __NAT_ADDR_DB_HPP__

// key for address map entry
typedef struct addr_entry_key_s {
    vrf_id_t     vrf_id;               // VRF where the IP belongs
    ip_addr_t    ip_addr;              // original IP address
} __PACK__ addr_entry_key_t;

// address mapping entry
typedef struct addr_entry_s {
    addr_entry_key_t    key;           // Entry key
    ip_addr_t           tgt_ip_addr;   // mapped IP address
    uint32_t            origin:1;      // 0 ==> fte path, 1 ==> cfg path
    uint32_t            ref_cnt:31;    // ref count for this mapping
    ht_ctxt_t           db_node;       // hash table context
} __PACK__ addr_entry_t;

} // namespace nat
} // namespace plugins
} // namespace hal

#endif    // __NAT_ADDR_DB_HPP__

